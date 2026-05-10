    .syntax unified
    .arch armv7-a
    .fpu neon-vfpv4
    .section .note.GNU-stack, "", %progbits   @ non-executable stack
    .text
    .align 2

    .global sigmoid
    .type sigmoid, %function

@ ═══════════════════════════════════════════════════════════════════════════
@ void sigmoid(float *data, int len)
@
@ Arguments (AAPCS):
@   r0: float *data    r1: int len
@ ═══════════════════════════════════════════════════════════════════════════ 

sigmoid:
    @ ── Early exit validation ─────────────────────────────────────────────────────
    cmp     r1, #0                  @ check len > 0
    ble     .L_done                 @ exit if len <= 0

    @ ── Load sigmoid constants and duplicate to NEON vectors ───────────────────────
    @ Constants for Schraudolph fast exp and clamping
    adr     r12, .L_SIGMOID_CONSTS  @ r12 = address of constants table
    ldr     r2, [r12]               @ r2 = 12102203.0f (exp multiplier)
    vdup.32 q1, r2                  @ q1 = {12102203.0f, 12102203.0f, 12102203.0f, 12102203.0f}
    ldr     r2, [r12, #4]           @ r2 = 1065353216.0f (exp offset)
    vdup.32 q2, r2                  @ q2 = {1065353216.0f, 1065353216.0f, 1065353216.0f, 1065353216.0f}
    ldr     r2, [r12, #8]           @ r2 = 1.0f (sigmoid numerator)
    vdup.32 q3, r2                  @ q3 = {1.0f, 1.0f, 1.0f, 1.0f}
    ldr     r2, [r12, #12]          @ r2 = -10.0f (lower clamp bound)
    vdup.32 q4, r2                  @ q4 = {-10.0f, -10.0f, -10.0f, -10.0f}
    ldr     r2, [r12, #16]          @ r2 = 10.0f (upper clamp bound)
    vdup.32 q5, r2                  @ q5 = {10.0f, 10.0f, 10.0f, 10.0f}

    @ ── Phase 1: Vectorized processing (4 elements per iteration) ───────────────────
.L_vec_loop:
    cmp     r1, #4                  @ check if at least 4 elements remain
    blt     .L_tail_setup           @ skip to scalar tail if < 4 elements

    vld1.32 {q0}, [r0]              @ q0 = {data[i], data[i+1], data[i+2], data[i+3]}
    
    @ Compute -x and clamp to [-10.0, 10.0] to prevent overflow/underflow
    vneg.f32 q0, q0                 @ q0 = {-data[i], -data[i+1], -data[i+2], -data[i+3]}
    vmax.f32 q0, q0, q4             @ clamp lower bound: max(-x, -10.0f)
    vmin.f32 q0, q0, q5             @ clamp upper bound: min(result, 10.0f)

    @ Schraudolph fast exponential approximation
    vmul.f32 q0, q0, q1             @ q0 = x * 12102203.0f (scale for exp)
    vadd.f32 q0, q0, q2             @ q0 = result + 1065353216.0f (offset for exp)
    vcvt.s32.f32 q0, q0             @ cast to int - bits now represent float(exp(-x))

    @ Compute sigmoid denominator: 1.0f + exp(-x)
    vadd.f32 q0, q0, q3             @ q0 = exp(-x) + 1.0f (implicit cast back to float)

    @ Newton-Raphson approximation for reciprocal 1.0 / (1.0f + exp(-x))
    vrecpe.f32 q6, q0               @ q6 = initial reciprocal estimate
    vrecps.f32 q7, q6, q0           @ q7 = correction factor step 1
    vmul.f32   q6, q6, q7           @ q6 = improved estimate step 1
    vrecps.f32 q7, q6, q0           @ q7 = correction factor step 2
    vmul.f32   q0, q6, q7           @ q0 = final reciprocal = 1.0 / (1.0f + exp(-x))

    vst1.32 {q0}, [r0]!             @ store 4 sigmoid results and advance pointer
    subs    r1, r1, #4              @ decrement remaining count by 4
    bgt     .L_vec_loop             @ continue if more elements remain
    b       .L_done                 @ exit if complete

    @ ── Phase 2: Scalar tail processing (0-3 remaining elements) ───────────────────
.L_tail_setup:
    cmp     r1, #0                  @ check if any elements remain
    ble     .L_done                 @ exit if no remaining elements

.L_tail_loop:
    vldr    s0, [r0]                @ s0 = data[i] (load current element)
    vneg.f32 s0, s0                 @ s0 = -data[i]
    
    @ Manual scalar clamping using VFP comparisons (no NEON available for scalar)
    ldr     r3, =0xC1200000         @ r3 = -10.0f (IEEE-754 hex representation)
    vmov    s1, r3                  @ s1 = -10.0f (clamp lower bound)
    vcmp.f32 s0, s1                 @ compare -data[i] with -10.0f
    vmrs    APSR_nzcv, fpscr        @ transfer VFP flags to ARM flags
    blt     .L_clamp_min            @ branch if -data[i] < -10.0f (too small)
    
    ldr     r3, =0x41200000         @ r3 = 10.0f (IEEE-754 hex representation)
    vmov    s1, r3                  @ s1 = 10.0f (clamp upper bound)
    vcmp.f32 s0, s1                 @ compare -data[i] with 10.0f
    vmrs    APSR_nzcv, fpscr        @ transfer VFP flags to ARM flags
    bgt     .L_clamp_max            @ branch if -data[i] > 10.0f (too large)
    b       .L_tail_calc            @ proceed with calculation if in range
    
.L_clamp_min:
    vmov    s0, s1                  @ s0 = -10.0f (clamp to minimum)
    b       .L_tail_calc
.L_clamp_max:
    vmov    s0, s1                  @ s0 = 10.0f (clamp to maximum)

.L_tail_calc:
    @ Schraudolph fast exponential for scalar
    vldr    s2, [r12]               @ s2 = 12102203.0f (exp multiplier)
    vmul.f32 s0, s0, s2             @ s0 = x * 12102203.0f
    vldr    s3, [r12, #4]           @ s3 = 1065353216.0f (exp offset)
    vadd.f32 s0, s0, s3             @ s0 = result + 1065353216.0f
    vcvt.s32.f32 s0, s0             @ cast to int - bits represent float(exp(-x))
    
    @ Compute sigmoid: 1.0f / (1.0f + exp(-x))
    vldr    s4, [r12, #8]           @ s4 = 1.0f (sigmoid numerator)
    vadd.f32 s0, s0, s4             @ s0 = exp(-x) + 1.0f
    vdiv.f32 s0, s4, s0             @ s0 = 1.0f / (exp(-x) + 1.0f) (final sigmoid)
    
    vstr    s0, [r0]                @ store sigmoid result
    add     r0, r0, #4              @ advance to next element (4 bytes)
    subs    r1, r1, #1              @ decrement remaining count
    bne     .L_tail_loop            @ continue if more elements remain

.L_done:
    bx      lr                      @ return

    @ ── Read-only constants ───────────────────────────────────────────────────────
    .align 2                        @ 4-byte aligned for vldr
.L_SIGMOID_CONSTS:
    .float 12102203.0               @ Schraudolph exp multiplier (≈ln(2) * 2^23)
    .float 1065353216.0             @ Schraudolph exp offset (2^23, IEEE-754 bias)
    .float 1.0                      @ Sigmoid numerator constant
    .float -10.0                    @ Lower input clamp bound
    .float 10.0                     @ Upper input clamp bound
