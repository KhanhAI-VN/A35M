.syntax unified
    .arch armv7-a
    .fpu neon-vfpv4              @ Nâng cấp lên NEON-VFPv4 để hỗ trợ SIMD Vectorization
    .section .note.GNU-stack, "", %progbits   @ non-executable stack
    .text
    .align 2

    .global moving_average
    .type moving_average, %function

@ ═══════════════════════════════════════════════════════════════════════════
@ void moving_average(const float *input, float *output, int len, int kernel_size)
@
@ Arguments (AAPCS):
@   r0: const float *input    r1: float *output
@   r2: int len              r3: int kernel_size
@ ═══════════════════════════════════════════════════════════════════════════

moving_average:
    @ ── Early exit validation ─────────────────────────────────────────────────────
    cmp     r2, #0               @ check len > 0
    ble     .L_done
    cmp     r3, #0               @ check kernel_size > 0
    ble     .L_done

    @ ── Convert kernel_size to float and compute inverse ─────────────────────────
    vmov    s3, r3               @ s3 = kernel_size (int → float bits)
    vcvt.f32.s32 s3, s3          @ s3 = kf = kernel_size as float
    adr     r12, .L_ONE
    vldr    s0, [r12]            @ s0 = 1.0 (constant)
    vdiv.f32 s1, s0, s3          @ s1 = inv_k = 1.0 / kf (inverse kernel size)
                                 @ Note: s1 mapped to NEON scalar d0[1]

    @ ── Initialize sliding window sum with input[0] padding ───────────────────────
    vldr    s2, [r0]             @ s2 = x0 = input[0] (padding value)
    vmul.f32 s0, s3, s2          @ s0 = sum = kf × x0 (initial padded window)
                                 @ Note: s2 mapped to NEON scalar d1[0]

    @ ── Calculate phase1_count = min(len, kernel_size-1) ───────────────────────────
    sub     r12, r3, #1          @ r12 = kernel_size - 1 (max phase1 iterations)
    cmp     r2, r12              @ compare len with kernel_size-1
    movlt   r12, r2              @ phase1_count = min(len, kernel_size-1)
    sub     r2, r2, r12          @ r2 = remaining elements for phase2

    @ ── Register optimization: repurpose r3 as next_ptr after conversion ───────────
    add     r3, r0, #4           @ r3 = next_ptr = &input[1] (no longer kernel_size)

    @ ── Phase 1: Causal padding (window grows from size 1 to kernel_size) ───────────
    cmp     r12, #0              @ skip if phase1_count = 0
    ble     .L_phase2_prep

    @ Unroll Phase 1 by 4 to maximize throughput
    subs    r12, r12, #4
    blt     .L_phase1_tail_prep

    @ Broadcast x0 (s2) to all lanes of q2 for vectorized subtraction
    vdup.32 q2, d1[0]            @ q2 (s8-s11) = {x0, x0, x0, x0}

.L_phase1_loop4:
    vld1.32 {q1}, [r3]!          @ load 4 next elements into q1 (s4-s7)
    vsub.f32 q1, q1, q2          @ q1 = {d0, d1, d2, d3} (deltas against x0)

    @ Parallel tree addition for deltas (hides 4-cycle FP latency)
    vadd.f32 s5, s4, s5          @ d01 = d0 + d1
    vadd.f32 s7, s6, s7          @ d23 = d2 + d3

    vmov.f32 s12, s0             @ q3[0] = sum0
    vadd.f32 s13, s0, s4         @ q3[1] = sum1 = sum0 + d0
    
    vadd.f32 s6, s5, s6          @ d012 = d01 + d2
    vadd.f32 s7, s5, s7          @ d0123 = d01 + d23

    vadd.f32 s14, s0, s5         @ q3[2] = sum2 = sum0 + d01
    vadd.f32 s15, s0, s6         @ q3[3] = sum3 = sum0 + d012
    vadd.f32 s0,  s0, s7         @ update s0 for next loop: sum4 = sum0 + d0123

    @ Vector multiply by scalar: multiply all 4 sums by inv_k (d0[1])
    vmul.f32 q3, q3, d0[1]       @ q3 = outputs {out0, out1, out2, out3}
    vst1.32 {q3}, [r1]!          @ store 4 outputs

    subs    r12, r12, #4
    bge     .L_phase1_loop4

.L_phase1_tail_prep:
    adds    r12, r12, #4         @ restore phase1 remaining count
    beq     .L_phase1_end_check

.L_phase1_tail:
    vmul.f32 s12, s0, s1         @ output = sum × inv_k
    vstmia  r1!, {s12}           @ store output[i]; advance output pointer

    subs    r12, r12, #1         @ decrement phase1_count
    beq     .L_phase1_end_check  @ exit tail when done

    vldmia  r3!, {s4}            @ load next_val
    vsub.f32 s4, s4, s2          @ delta = next_val - x0
    vadd.f32 s0, s0, s4          @ sum += delta
    b       .L_phase1_tail

.L_phase1_end_check:
    cmp     r2, #0               @ check if any elements remain for phase2
    ble     .L_done

    @ ── Complete window: add final element to reach full kernel_size ───────────────
    vldmia  r3!, {s4}            @ load input[kernel_size-1]
    vsub.f32 s4, s4, s2          @ delta = input[k-1] - x0
    vadd.f32 s0, s0, s4          @ sum += delta (now full window)

@ ── Phase 2: Full sliding window (constant kernel_size) ───────────────────────────
@ r0 = old_ptr (points to element to remove), r3 = next_ptr (points to element to add)
.L_phase2_prep:
    cmp     r2, #0
    ble     .L_done

    @ Unroll Phase 2 by 4 to maximize throughput
    subs    r2, r2, #4
    blt     .L_phase2_tail_prep

.L_phase2_loop4:
    vld1.32 {q1}, [r3]!          @ load 4 next elements into q1 (s4-s7)
    vld1.32 {q2}, [r0]!          @ load 4 old elements into q2 (s8-s11)
    vsub.f32 q1, q1, q2          @ q1 = {d0, d1, d2, d3} (deltas)

    @ Parallel tree addition for deltas (hides 4-cycle FP latency)
    vadd.f32 s5, s4, s5          @ d01 = d0 + d1
    vadd.f32 s7, s6, s7          @ d23 = d2 + d3

    vmov.f32 s12, s0             @ q3[0] = sum0
    vadd.f32 s13, s0, s4         @ q3[1] = sum1 = sum0 + d0

    vadd.f32 s6, s5, s6          @ d012 = d01 + d2
    vadd.f32 s7, s5, s7          @ d0123 = d01 + d23

    vadd.f32 s14, s0, s5         @ q3[2] = sum2 = sum0 + d01
    vadd.f32 s15, s0, s6         @ q3[3] = sum3 = sum0 + d012
    vadd.f32 s0,  s0, s7         @ update s0 for next loop: sum4 = sum0 + d0123

    @ Vector multiply by scalar: multiply all 4 sums by inv_k (d0[1])
    vmul.f32 q3, q3, d0[1]       @ q3 = outputs {out0, out1, out2, out3}
    vst1.32 {q3}, [r1]!          @ store 4 outputs

    subs    r2, r2, #4
    bge     .L_phase2_loop4

.L_phase2_tail_prep:
    adds    r2, r2, #4           @ restore phase2 remaining count
    beq     .L_done

.L_phase2_tail:
    vmul.f32 s12, s0, s1         @ output = sum / kernel_size
    vstmia  r1!, {s12}           @ store output[i]; advance output pointer

    subs    r2, r2, #1           @ decrement remaining elements
    beq     .L_done              @ exit if no more elements

    vldmia  r3!, {s4}            @ load next_val
    vldmia  r0!, {s5}            @ load old_val
    vsub.f32 s4, s4, s5          @ delta = next_val - old_val
    vadd.f32 s0, s0, s4          @ sum += delta
    b       .L_phase2_tail

.L_done:
    bx      lr                   @ return (0-stack preserved)

    @ ── Read-only constants ─────────────────────────────────────────────────────
    .align 2                     @ 4-byte aligned for vldr
.L_ONE:
    .float  1.0                  @ constant 1.0 for division