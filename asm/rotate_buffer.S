    .syntax unified
    .arch armv7-a
    .fpu vfpv4
    .section .note.GNU-stack, "", %progbits   @ non-executable stack
    .text
    .align 2

    .global rotate_buffer
    .type rotate_buffer, %function

@ ═══════════════════════════════════════════════════════════════════════════
@ void rotate_buffer(float *arr, int total_parsed, int days)
@
@ Arguments (AAPCS):
@   r0: float *arr    r1: int total_parsed    r2: int days
@ ═══════════════════════════════════════════════════════════════════════════

rotate_buffer:
    @ ── Early exit validation ─────────────────────────────────────────────────────
    cmp     r2, #0               @ check days > 0
    ble     .L_done              @ exit if days <= 0 (invalid)
    cmp     r1, r2               @ check total_parsed >= days
    ble     .L_done              @ exit if total_parsed < days (nothing to rotate)

    @ ── Compute split = total_parsed % days using manual modulo ─────────────────────
    @ ARM doesn't have modulo instruction, so we use repeated subtraction
    mov     r3, r1               @ r3 = total_parsed (working copy)

.L_mod_loop:
    cmp     r3, r2               @ compare working copy with days
    blt     .L_mod_done          @ exit loop when working copy < days
    sub     r3, r3, r2           @ subtract days from working copy
    b       .L_mod_loop          @ repeat subtraction

.L_mod_done:
    cmp     r3, #0               @ check if split = 0
    beq     .L_done              @ exit if no rotation needed (split = 0)

    @ ── Register allocation for 3-reversal algorithm ─────────────────────────────────
    @ r0 = arr (base pointer, constant)
    @ r2 = days (array size, constant)
    @ r3 = split (modulo result, constant)
    @ r1 = left_ptr (working pointer)
    @ r12 = right_ptr (working pointer)
    @ s0, s1 = NEON scratch registers for element swapping

    @ ── Phase 1: reverse(arr[0..split-1]) - reverse first segment ─────────────────────
    mov     r1, r0               @ left_ptr = arr (start of array)
    add     r12, r0, r3, lsl #2  @ right_ptr = arr + split * 4 (end of first segment)
    sub     r12, r12, #4         @ right_ptr-- (point to last element of first segment)

.L_rev1_loop:
    cmp     r1, r12              @ check if left_ptr >= right_ptr
    bhs     .L_rev2_setup        @ exit reversal when pointers meet/cross
    
    @ Swap elements using NEON scratch registers (no temporary memory needed)
    vldr    s0, [r1]             @ s0 = *left_ptr (load left element)
    vldr    s1, [r12]            @ s1 = *right_ptr (load right element)
    vstr    s1, [r1]             @ *left_ptr = s1 (store right element)
    vstr    s0, [r12]            @ *right_ptr = s0 (store left element)
    
    add     r1, r1, #4           @ left_ptr++ (advance to next element)
    sub     r12, r12, #4         @ right_ptr-- (move to previous element)
    b       .L_rev1_loop         @ continue swapping

.L_rev2_setup:
    @ ── Phase 2: reverse(arr[split..days-1]) - reverse second segment ───────────────
    add     r1, r0, r3, lsl #2   @ left_ptr = arr + split * 4 (start of second segment)
    add     r12, r0, r2, lsl #2  @ right_ptr = arr + days * 4 (end of array)
    sub     r12, r12, #4         @ right_ptr-- (point to last element of array)

.L_rev2_loop:
    cmp     r1, r12              @ check if left_ptr >= right_ptr
    bhs     .L_rev3_setup        @ exit reversal when pointers meet/cross
    
    @ Swap elements using NEON scratch registers
    vldr    s0, [r1]             @ s0 = *left_ptr
    vldr    s1, [r12]            @ s1 = *right_ptr
    vstr    s1, [r1]             @ *left_ptr = s1
    vstr    s0, [r12]            @ *right_ptr = s0
    
    add     r1, r1, #4           @ left_ptr++
    sub     r12, r12, #4         @ right_ptr--
    b       .L_rev2_loop         @ continue swapping

.L_rev3_setup:
    @ ── Phase 3: reverse(arr[0..days-1]) - reverse entire array ─────────────────────
    @ This completes the 3-reversal rotation algorithm
    mov     r1, r0               @ left_ptr = arr (start of array)
    add     r12, r0, r2, lsl #2  @ right_ptr = arr + days * 4 (end of array)
    sub     r12, r12, #4         @ right_ptr-- (point to last element of array)

.L_rev3_loop:
    cmp     r1, r12              @ check if left_ptr >= right_ptr
    bhs     .L_done              @ exit reversal when pointers meet/cross
    
    @ Final swap to complete rotation
    vldr    s0, [r1]             @ s0 = *left_ptr
    vldr    s1, [r12]            @ s1 = *right_ptr
    vstr    s1, [r1]             @ *left_ptr = s1
    vstr    s0, [r12]            @ *right_ptr = s0
    
    add     r1, r1, #4           @ left_ptr++
    sub     r12, r12, #4         @ right_ptr--
    b       .L_rev3_loop         @ continue swapping

.L_done:
    bx      lr                   @ return