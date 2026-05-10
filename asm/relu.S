    .syntax unified
    .arch armv7-a
    .fpu neon-vfpv4
    .section .note.GNU-stack, "", %progbits   @ non-executable stack
    .text
    .align 2

    .global relu
    .type relu, %function

@ ═══════════════════════════════════════════════════════════════════════════
@ void relu(float *data, int len)
@
@ Arguments (AAPCS):
@   r0: float *data    r1: int len
@ ═══════════════════════════════════════════════════════════════════════════

relu:
    @ ── Early exit validation ─────────────────────────────────────────────────────
    cmp     r1, #0               @ check len > 0
    ble     .L_done              @ exit if len <= 0

    @ ── Setup: load zero constant into NEON register for vectorized comparison ───────
    mov     r12, #0              @ r12 = 0 (integer zero)
    vdup.32 q1, r12              @ q1 = {0.0f, 0.0f, 0.0f, 0.0f} (vector of zeros)

    @ ── Phase 1: Vectorized processing (4 elements per iteration) ───────────────────
.L_vec_loop:
    cmp     r1, #4               @ check if at least 4 elements remain
    blt     .L_tail_setup        @ skip to scalar tail if < 4 elements

    @ Load, process, and store 4 elements in parallel
    vld1.32 {q0}, [r0]           @ q0 = {data[i], data[i+1], data[i+2], data[i+3]}
    vmax.f32 q0, q0, q1          @ q0 = max(q0, 0.0f) element-wise
    vst1.32 {q0}, [r0]!          @ store 4 processed elements and advance pointer
    
    subs    r1, r1, #4           @ decrement remaining count by 4
    bgt     .L_vec_loop          @ continue if more elements remain
    b       .L_done              @ exit if complete

    @ ── Phase 2: Scalar tail processing (0-3 remaining elements) ───────────────────
.L_tail_setup:
    cmp     r1, #0               @ check if any elements remain
    ble     .L_done              @ exit if no remaining elements
    
    @ Setup scalar zero constant
    mov     r12, #0              @ r12 = 0 (integer zero)
    vmov    s2, r12              @ s2 = 0.0f (scalar zero)

.L_tail_loop:
    vldr    s0, [r0]             @ s0 = data[i] (load current element)
    
    @ Compare with zero and clamp if negative
    vcmp.f32 s0, s2              @ compare data[i] with 0.0f
    vmrs    APSR_nzcv, fpscr     @ transfer VFP flags to ARM flags
    blt     .L_clamp_zero        @ branch if data[i] < 0.0f
    b       .L_store             @ otherwise store as-is

.L_clamp_zero:
    vmov.f32 s0, s2              @ s0 = 0.0f (clamp negative values to zero)

.L_store:
    vstr    s0, [r0]             @ store processed element
    add     r0, r0, #4           @ advance to next element (4 bytes)
    subs    r1, r1, #1           @ decrement remaining count
    bne     .L_tail_loop         @ continue if more elements remain

.L_done:
    bx      lr                   @ return
