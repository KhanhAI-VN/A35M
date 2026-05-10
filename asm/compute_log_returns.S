    .syntax unified
    .arch armv7-a
    .fpu neon-vfpv4
    .section .note.GNU-stack, "", %progbits   @ non-executable stack
    .text
    .align 2

    .global compute_log_returns
    .type compute_log_returns, %function

@ ═══════════════════════════════════════════════════════════════════════════
@ void compute_log_returns(const float *data, int n, float *out)
@
@ Arguments (AAPCS):
@   r0: const float *data    r1: int n    r2: float *out
@ ═══════════════════════════════════════════════════════════════════════════

compute_log_returns:
    @ ── Early exit (need at least 2 elements for 1 log return) ───────────────
    cmp     r1, #2
    bxlt    lr                     @ n < 2: insufficient data

    @ ── Setup: n-1 iterations, load NEON constants ────────────────────────────
    sub     r1, r1, #1             @ number of log returns = n - 1

    @ Load vector constants safely and efficiently using VDUP
    adr     r12, .L_VEC_CONSTS
    ldr     r3, [r12], #4
    vdup.32 q8, r3                 @ q8 = 0x807FFFFF (mantissa mask)
    ldr     r3, [r12], #4
    vdup.32 q9, r3                 @ q9 = 1/3 (Taylor coefficient)
    ldr     r3, [r12], #4
    vdup.32 q10, r3                @ q10 = 1/5 (Taylor coefficient)
    ldr     r3, [r12]
    vdup.32 q11, r3                @ q11 = ln(2)

    vmov.f32 q12, #1.0             @ q12 = 1.0f (0x3F800000)
    vmov.i32 q13, #127           @ q13 = 127 (exponent bias)

    @ ── Process 4-element blocks with NEON SIMD ────────────────────────────────
    lsrs    r3, r1, #2             @ r3 = number of 4-element blocks
    beq     .L_check_remainder     @ Skip if < 4 elements

.L_loop_neon:
    @ Data loading (read consecutive buffer to avoid heavy unaligned penalty)
    vld1.32 {d0-d1}, [r0]          @ q0 = X [x0, x1, x2, x3]
    add     r12, r0, #4
    vld1.32 {d2-d3}, [r12]         @ q1 = Y [x1, x2, x3, x4]
    add     r0, r0, #16            @ Advance pointer early to hide latency

    @ Compute ratios R = Y/X using Newton-Raphson
    vrecpe.f32 q2, q0
    vrecps.f32 q3, q2, q0
    vmul.f32   q2, q2, q3
    vrecps.f32 q3, q2, q0
    vmul.f32   q2, q2, q3          @ q2 = refined 1/X
    vmul.f32   q1, q1, q2          @ q1 = R = Y * (1/X)

    @ Dual-issue: Extract mantissa (m) and exponent (k) interleaved
    vshl.u32   q2, q1, #1          @ [Int] Clear sign bit
    vand       q1, q1, q8          @ [Int] Clear exponent bits (get raw m)

    vshr.u32   q2, q2, #24         @ [Int] Isolate exponent (0-255)
    vorr       q1, q1, q12         @ [Int] Attach exponent 127 -> normalized m [1.0, 2.0)

    vsub.i32   q2, q2, q13         @ [Int] k = exp - 127
    vsub.f32   q3, q1, q12         @ [FP]  q3 = m - 1.0

    vcvt.f32.s32 q2, q2            @ [FP]  Convert k to float
    vadd.f32   q14, q1, q12        @ [FP]  q14 = m + 1.0

    @ Compute s = (m-1)/(m+1)
    vrecpe.f32 q15, q14
    vrecps.f32 q1, q15, q14
    vmul.f32   q15, q15, q1
    vrecps.f32 q1, q15, q14
    vmul.f32   q15, q15, q1        @ q15 = 1/(m+1)
    vmul.f32   q1, q3, q15         @ q1 = s

    @ Enhanced Taylor series using VMLA (s + s³/3 + s⁵/5)
    vmul.f32   q3, q1, q1          @ q3 = s²
    vmul.f32   q14, q3, q1         @ q14 = s³
    vmul.f32   q15, q14, q3        @ q15 = s⁵

    vmla.f32   q1, q14, q9         @ q1 = s + (s³/3)
    vmla.f32   q1, q15, q10        @ q1 = (s + s³/3) + (s⁵/5)

    @ Combine: log(R) = k * ln(2) + 2 * series
    vadd.f32   q1, q1, q1          @ q1 = 2 * log(mantissa)
    vmla.f32   q1, q2, q11         @ q1 = 2*log(m) + k*ln(2)

    @ Store result and loop
    vst1.32    {d2-d3}, [r2]!      @ Write 4 floats to RAM
    subs       r3, r3, #1
    bne        .L_loop_neon

.L_check_remainder:
    @ ── Handle remaining elements (0-3) with scalar fallback ──────────────────
    ands       r1, r1, #3          @ r1 = remaining elements
    beq        .L_end

    @ Load constants for scalar
    adr        r12, .L_LOG_CONSTS
    vldr       s4, [r12]           @ s4 = ln(2)
    vldr       s5, [r12, #4]       @ s5 = 1/3
    vldr       s6, [r12, #8]       @ s6 = 1/5
    vmov.f32   s7, #1.0            @ s7 = 1.0f

.L_loop_scalar:
    vldr.f32 s0, [r0]
    vldr.f32 s1, [r0, #4]
    add      r0, r0, #4

    vdiv.f32 s0, s1, s0            @ R = Y/X (VFP vdiv is extremely fast for scalar)

    @ Bitwise magic for ARM Cortex-A
    vmov     r3, s0
    lsr      r12, r3, #23          @ Shift to get exponent
    and      r12, r12, #0xFF
    sub      r12, r12, #127      @ k (int)
    vmov     s11, r12
    vcvt.f32.s32 s11, s11          @ k (float)

    bfc      r3, #23, #9           @ BFC (Bit Field Clear): Strip 9 bits sign and exponent. Ultra fast!
    vmov     r12, s7               @ r12 = 0x3F800000 (from s7)
    orr      r3, r3, r12           @ Attach exponent 127
    vmov     s0, r3                @ s0 = m

    vsub.f32 s1, s0, s7            @ m - 1.0
    vadd.f32 s2, s0, s7            @ m + 1.0
    vdiv.f32 s0, s1, s2            @ s = (m-1)/(m+1)

    vmul.f32 s1, s0, s0            @ s²
    vmul.f32 s2, s1, s0            @ s³
    vmul.f32 s3, s2, s1            @ s⁵

    vmla.f32 s0, s2, s5            @ s + s³/3
    vmla.f32 s0, s3, s6            @ s + s³/3 + s⁵/5

    vadd.f32 s0, s0, s0            @ 2 * series
    vmla.f32 s0, s11, s4           @ log = 2*series + k*ln(2)

    vstr.f32 s0, [r2]
    add      r2, r2, #4

    subs     r1, r1, #1
    bne      .L_loop_scalar

.L_end:
    bx       lr                    @ Return directly (no PUSH/POP)

    @ ── Constant Pools ───────────────────────────────────────────────────────
    .align 4
.L_VEC_CONSTS:
    .word   0x807FFFFF              @ Mantissa mask
    .float  0.33333334              @ 1/3 (Taylor coeff)
    .float  0.20000000              @ 1/5 (Taylor coeff)
    .float  0.69314718              @ ln(2)
.L_LOG_CONSTS:
    .float  0.69314718              @ ln(2)
    .float  0.33333334              @ 1/3
    .float  0.20000000              @ 1/5