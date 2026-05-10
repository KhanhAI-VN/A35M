.syntax unified
    .arch armv7-a
    .fpu vfpv4
    .section .note.GNU-stack, "", %progbits   @ non-executable stack
    .text
    .align 2

    .global decompose
    .type decompose, %function

@ ═══════════════════════════════════════════════════════════════════════════
@ void decompose(const float *input, float *trend, float *residual)
@
@ Arguments (AAPCS):
@   r0: const float *input    r1: float *trend    r2: float *residual
@ ═══════════════════════════════════════════════════════════════════════════

decompose:
    @ ── Load constants & Initialize (No stack usage) ──────────────────────────
    adr     r3, .L_CONSTS
    vldr    s8, [r3]                @ s8 = 25.0 (temp use for initial sum)
    vldr    s1, [r3, #4]            @ s1 = 0.04 (inverse window size)

    vldr    s2, [r0]                @ s2 = input[0] (padding value)
    vmul.f32 s0, s2, s8             @ s0 = sum = 25.0 × input[0]
    vmov.f32 s3, s2                 @ s3 = in0 = input[0]
    add     r12, r0, #4             @ r12 = old_ptr = &input[1]

@ ── Phase 1: Initial window filling (i = 0..24) ──────────────────────────────
@ Unrolled 5 times. 25 elements total.
@ Pipeline highly interleaved: Loads for iteration N+1 happen during 
@ the VFP 4-cycle stall gaps of iteration N.
    mov     r3, #5                  @ Loop counter for Phase 1 (5 iters)

.L_phase1_unroll5:
    @ Iteration 0
    vmul.f32 s4, s0, s1             @ [1] trend0 = sum * 0.04
    vldr     s14, [r0, #4]          @ [2] in1 = input[i+1]
    vsub.f32 s15, s14, s2           @ [3] delta0 = in1 - input[0] (Moved up to hide vmul latency)
    vsub.f32 s9, s3, s4             @ [4] res0 = in0 - trend0 (s4 ready)
    vldr     s3, [r0, #8]           @ [5] PRE-LOAD in2 -> s3 (overwrites in0, safe)
    vadd.f32 s0, s0, s15            @ [6] sum1 = sum0 + delta0

    @ Iteration 1
    vmul.f32 s5, s0, s1             @ [7] trend1
    vsub.f32 s15, s3, s2            @ [8] delta1 = in2 - input[0]
    vsub.f32 s10, s14, s5           @ [9] res1 = in1 - trend1
    vldr     s14, [r0, #12]         @ [10] PRE-LOAD in3 -> s14 (overwrites in1, safe)
    vadd.f32 s0, s0, s15            @ [11] sum2 = sum1 + delta1

    @ Iteration 2
    vmul.f32 s6, s0, s1             @ [12] trend2
    vsub.f32 s15, s14, s2           @ [13] delta2 = in3 - input[0]
    vsub.f32 s11, s3, s6            @ [14] res2 = in2 - trend2
    vldr     s3, [r0, #16]          @ [15] PRE-LOAD in4 -> s3
    vadd.f32 s0, s0, s15            @ [16] sum3 = sum2 + delta2

    @ Iteration 3
    vmul.f32 s7, s0, s1             @ [17] trend3
    vsub.f32 s15, s3, s2            @ [18] delta3 = in4 - input[0]
    vsub.f32 s12, s14, s7           @ [19] res3 = in3 - trend3
    vldr     s14, [r0, #20]         @ [20] PRE-LOAD in5 -> s14 (For next iter AND next block)
    vadd.f32 s0, s0, s15            @ [21] sum4 = sum3 + delta3

    @ Iteration 4
    vmul.f32 s8, s0, s1             @ [22] trend4
    vsub.f32 s15, s14, s2           @ [23] delta4 = in5 - input[0]
    vsub.f32 s13, s3, s8            @ [24] res4 = in4 - trend4
    vmov     s3, s14                @ [25] Shift pre-loaded in5 to in0 (s3) for next loop wrap
    vadd.f32 s0, s0, s15            @ [26] sum5 = sum4 + delta4

    @ Batch Stores and Pointer Updates
    vstmia   r1!, {s4-s8}           @ Store 5 trends
    vstmia   r2!, {s9-s13}          @ Store 5 residuals
    add      r0, r0, #20            @ Advance input pointer (5 floats)
    subs     r3, r3, #1             @ Decrement phase 1 counter
    bne      .L_phase1_unroll5

@ ── Phase 2: Full sliding window (i = 25..363) ───────────────────────────────
@ Unrolled 3 times. 339 elements total.
@ Perfect Dual-Issue weaving: integer pointers advance in parallel with VFP.
@ Load-ahead technique effectively makes memory access 0-cycle on critical path.
    mov     r3, #113                @ Loop counter for Phase 2 (113 iters)

.L_phase2_unroll3:
    @ Iteration 0
    vmul.f32 s4, s0, s1             @ trend0 = sum0 * 0.04
    vldr     s10, [r0, #4]          @ load in1
    vldr     s11, [r12]             @ load old0 = input[i-24]
    vsub.f32 s12, s10, s11          @ delta0 = in1 - old0
    vsub.f32 s7, s3, s4             @ res0 = in0 - trend0 (waits for s4)
    vldr     s3, [r0, #8]           @ PRE-LOAD in2 -> s3
    vldr     s14, [r12, #4]         @ PRE-LOAD old1 -> s14
    vadd.f32 s0, s0, s12            @ sum1 = sum0 + delta0

    @ Iteration 1
    vmul.f32 s5, s0, s1             @ trend1 = sum1 * 0.04
    vsub.f32 s12, s3, s14           @ delta1 = in2 - old1 (uses pre-loaded values)
    vsub.f32 s8, s10, s5            @ res1 = in1 - trend1
    vldr     s10, [r0, #12]         @ PRE-LOAD in3 -> s10
    vldr     s11, [r12, #8]         @ PRE-LOAD old2 -> s11
    vadd.f32 s0, s0, s12            @ sum2 = sum1 + delta1

    @ Iteration 2
    vmul.f32 s6, s0, s1             @ trend2 = sum2 * 0.04
    vsub.f32 s12, s10, s11          @ delta2 = in3 - old2
    vsub.f32 s9, s3, s6             @ res2 = in2 - trend2
    add      r0, r0, #12            @ Dual-issue: Advance input ptr early
    add      r12, r12, #12          @ Dual-issue: Advance old_ptr early
    vadd.f32 s0, s0, s12            @ sum3 = sum2 + delta2 (hides behind vstmia)
    vmov     s3, s10                @ Shift pre-loaded in3 to in0 (s3) for next loop wrap
    
    @ Batch Stores (Hides vadd.f32 latency perfectly)
    vstmia   r1!, {s4-s6}           @ Store 3 trends
    vstmia   r2!, {s7-s9}           @ Store 3 residuals
    subs     r3, r3, #1             @ Decrement phase 2 counter
    bne      .L_phase2_unroll3

@ ── Phase 3: Final element (i = 364) ─────────────────────────────────────────
@ s3 inherently holds input[364] shifted from Phase 2's final loop wrap.
    vmul.f32 s4, s0, s1             @ trend[364] = sum * 0.04
    vsub.f32 s7, s3, s4             @ res[364] = input[364] - trend[364]
    vstr     s4, [r1]               @ store trend[364]
    vstr     s7, [r2]               @ store residual[364]

    bx       lr                     @ Return (0 stack overhead)

    @ ── Read-only constants ──────────────────────────────────────────────────
    .align 2                        @ 4-byte aligned for vldr
.L_CONSTS:
    .float  25.0                    @ initial window size K
    .float  0.04                    @ 1/K (inverse window size)