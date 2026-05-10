.syntax unified
    .arch armv7-a
    .fpu neon-vfpv4
    .section .note.GNU-stack, "", %progbits   @ non-executable stack
    .text
    .align 2

    .global load_model_bin
    .type load_model_bin, %function

@ ═══════════════════════════════════════════════════════════════════════════
@ int load_model_bin(const uint8_t *model_data, int data_size)
@
@ Arguments (AAPCS):
@   r0: const uint8_t *model_data  r1: int data_size
@ ═══════════════════════════════════════════════════════════════════════════

load_model_bin:
    @ ── Function prologue: save callee-saved registers ───────────────────────────
    push    {r4-r12, lr}        @ SP decreases by 36 bytes (8-byte aligned)

    @ ── Basic validation: ensure minimum header size ───────────────────────────
    cmp     r0, #12             @ data_size must be >= 12 bytes (header)
    blt     .L_fail

    @ ── Initialize pointers and counters ─────────────────────────────────────────
    ldr     r1, =g_shared_buf   @ r1 = model_data_ptr
    add     r2, r1, r0          @ r2 = model_data_end
    mov     r3, r1              @ r3 = output_ptr (shared buffer start)

    @ ── Validate magic and version ─────────────────────────────────────────────
    ldr     r4, [r1]            @ r4 = magic from header
    ldr     r5, =0x4D353341     @ r5 = "A35M" magic constant
    cmp     r4, r5
    bne     .L_fail
    ldr     r4, [r1, #4]        @ r4 = version from header
    cmp     r4, #1              @ version must be 1
    bne     .L_fail

    @ ── Extract tensor count and setup main loop ────────────────────────────────
    ldr     r4, [r1, #8]        @ r4 = num_tensors
    add     r1, r1, #12         @ r1 = cur_ptr (skip header)
    mov     r5, #0              @ r5 = loaded_count

@ ── Main tensor processing loop ───────────────────────────────────────────────
.L_tensor_loop:
    cmp     r4, #0              @ check if num_tensors == 0
    beq     .L_done             @ exit when all tensors processed

    @ ── Bounds check: ensure we can read name_len field ─────────────────────────
    add     r12, r1, #2         @ r12 = cur_ptr + sizeof(uint16)
    cmp     r12, r2             @ check against model_data_end
    bhi     .L_fail             @ fail if buffer overflow

    @ ── Parse tensor name (optimized unaligned 16-bit read) ─────────────────────
    ldrh    r6, [r1]            @ r6 = name_len (uint16)
    add     r1, r1, #2          @ cur_ptr += sizeof(uint16)

    @ ── Bounds check: ensure we can read name string ───────────────────────────
    add     r12, r1, r6         @ r12 = cur_ptr + name_len
    cmp     r12, r2             @ check against model_data_end
    bhi     .L_fail             @ fail if buffer overflow

    mov     r7, r1              @ r7 = name_ptr (save for comparison)
    add     r1, r1, r6          @ cur_ptr += name_len

    @ ── Parse dimensions (skip for now) ─────────────────────────────────────────
    add     r12, r1, #1         @ check ndims field exists
    cmp     r12, r2
    bhi     .L_fail
    ldrb    r11, [r1], #1       @ r11 = ndims (uint8)
    
    add     r1, r1, r11, lsl #2 @ cur_ptr += ndims * sizeof(uint32)

    @ ── Parse data type (must be float32) ───────────────────────────────────────
    add     r12, r1, #1         @ check dtype field exists
    cmp     r12, r2
    bhi     .L_fail
    ldrb    r11, [r1], #1       @ r11 = dtype (uint8)
    cmp     r11, #1              @ must be 1 (float32)
    bne     .L_fail

    @ ── Parse data count (optimized unaligned 32-bit read) ─────────────────────
    add     r12, r1, #4         @ check data_count field exists
    cmp     r12, r2
    bhi     .L_fail

    ldr     r8, [r1]            @ r8 = data_count (uint32)
    add     r1, r1, #4          @ cur_ptr += sizeof(uint32)

    @ ── Bounds check: ensure we can read tensor data ───────────────────────────
    lsl     r12, r8, #2         @ r12 = data_bytes = data_count * 4
    add     r11, r1, r12        @ r11 = cur_ptr + data_bytes
    cmp     r11, r2             @ check against model_data_end
    bhi     .L_fail             @ fail if buffer overflow

    @ ── Search for matching tensor slot ───────────────────────────────────────────
    ldr     r9, =g_slots        @ r9 = slots_array_ptr
    ldr     r10, =NUM_SLOTS     @ r10 = &NUM_SLOTS
    ldr     r10, [r10]          @ r10 = num_slots (loop counter)

.L_slot_loop:
    cmp     r10, #0             @ check if slots remaining
    beq     .L_slot_skip        @ skip to next tensor if no slots

    @ ── Perform string comparison: tensor_name vs slot_name ───────────────────────
    ldr     r11, [r9, #0]       @ r11 = slot_name_ptr
    mov     r12, #0             @ r12 = string_index = 0

.L_strcmp_loop:
    cmp     r12, r6             @ compare with name_len
    beq     .L_strcmp_check_null @ check for null terminator
    
    ldrb    r0, [r7, r12]       @ r0 = tensor_name[string_index]
    ldrb    lr, [r11, r12]      @ lr = slot_name[string_index]
    cmp     r0, lr              @ compare characters
    bne     .L_slot_next        @ mismatch, try next slot
    add     r12, r12, #1        @ increment string_index
    b       .L_strcmp_loop      @ continue comparison

.L_strcmp_check_null:
    ldrb    lr, [r11, r12]      @ check null terminator in slot_name
    cmp     lr, #0              @ must be null for exact match
    bne     .L_slot_next        @ not null, try next slot

    @ ── Validate tensor size matches slot capacity ───────────────────────────────
    ldrh    lr, [r9, #4]        @ lr = slot_max_elements
    cmp     r8, lr              @ compare data_count vs slot_capacity
    bne     .L_fail             @ size mismatch, fail

    @ ── NEON-optimized in-place memory copy (32-byte blocks) ─────────────────────
    lsl     r0, r8, #2          @ r0 = data_bytes = data_count * 4
    mov     r12, r1             @ r12 = src_ptr (tensor_data)
    mov     lr, r3              @ lr = dst_ptr (shared_buffer)

.L_neon_copy_32:
    cmp     r0, #32             @ check if >= 32 bytes remaining
    blt     .L_neon_copy_16     @ use smaller block if less
    vld1.8  {d0-d3}, [r12]!    @ load 32 bytes (4 registers) with post-increment
    vst1.8  {d0-d3}, [lr]!     @ store 32 bytes with post-increment
    sub     r0, r0, #32         @ decrement remaining bytes
    b       .L_neon_copy_32     @ continue 32-byte blocks

.L_neon_copy_16:
    cmp     r0, #16             @ check if >= 16 bytes remaining
    blt     .L_neon_copy_rem    @ use byte copy if less
    vld1.8  {d0-d1}, [r12]!    @ load 16 bytes (2 registers) with post-increment
    vst1.8  {d0-d1}, [lr]!     @ store 16 bytes with post-increment
    sub     r0, r0, #16         @ decrement remaining bytes

.L_neon_copy_rem:
    cmp     r0, #0              @ check if any bytes remaining
    beq     .L_neon_copy_done   @ finished if none
.L_copy_byte:                   @ fallback: byte-by-byte copy
    ldrb    r11, [r12], #1      @ load one byte from source
    strb    r11, [lr], #1       @ store one byte to destination
    subs    r0, r0, #1          @ decrement and set flags
    bgt     .L_copy_byte        @ continue if more bytes
.L_neon_copy_done:

    @ ── Update slot pointer and advance output buffer ───────────────────────────
    ldr     r12, [r9, #8]       @ r12 = &slot->tensor_ptr
    str     r3, [r12]           @ store tensor_ptr = current output location

    lsl     r12, r8, #2         @ r12 = data_bytes = data_count * 4
    add     r3, r3, r12         @ advance output_ptr by data_bytes
    add     r5, r5, #1          @ increment loaded_count
    
    b       .L_slot_skip        @ skip to next tensor (slot found and filled)

.L_slot_next:
    add     r9, r9, #12         @ advance to next slot (12 bytes per slot)
    sub     r10, r10, #1        @ decrement remaining_slots
    b       .L_slot_loop        @ continue slot search

.L_slot_skip:
    lsl     r12, r8, #2         @ r12 = data_bytes = data_count * 4
    add     r1, r1, r12         @ advance cur_ptr past tensor data
    sub     r4, r4, #1          @ decrement remaining_tensors
    b       .L_tensor_loop      @ process next tensor

.L_done:
    @ ── Final validation: ensure all expected tensors were loaded ─────────────────
    ldr     r1, =NUM_SLOTS      @ r1 = &NUM_SLOTS
    ldr     r1, [r1]            @ r1 = expected_slots_count
    cmp     r5, r1              @ compare loaded_count vs expected_count
    moveq   r0, #0              @ success: return 0 if all loaded
    movne   r0, #-1             @ failure: return -1 if missing tensors
    pop     {r4-r12, pc}        @ restore registers and return

.L_fail:
    @ ── Error handling: return failure code ──────────────────────────────────────
    mov     r0, #-1             @ return -1 on any validation failure
    pop     {r4-r12, pc}        @ restore registers and return

    .section .note.GNU-stack,"",%progbits   @ non-executable stack for security