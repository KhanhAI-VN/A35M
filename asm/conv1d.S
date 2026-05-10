.syntax unified
    .arch armv7-a
    .arch_extension idiv                      @ Bật Hardware Integer Division cho Cortex-A7
    .fpu neon-vfpv4
    .section .note.GNU-stack, "", %progbits   @ non-executable stack
    .text
    .align 2

    .global conv1d
    .type conv1d, %function

@ ═══════════════════════════════════════════════════════════════════════════
@ void conv1d(const float *input, int input_len,
@             const float *weight, const float *bias,
@             int in_channels, int out_channels,
@             int kernel_size, int stride, float *output)
@
@ Arguments (AAPCS) after Zero-Local-Stack optimization:
@   r0: const float *input     r1: int input_len
@   r2: const float *weight    r3: const float *bias
@   Stack: [sp+36]: in_channels   [sp+40]: out_channels
@          [sp+44]: kernel_size   [sp+48]: stride
@          [sp+52]: float *output
@ ═══════════════════════════════════════════════════════════════════════════

@ ── K30_MACRO: Highly Optimized 30-element kernel convolution ────────────────
@ Sử dụng Dual Accumulators (q2 và q8) để triệt tiêu hoàn toàn 4-cycle latency
@ của lệnh VMLA trên Cortex-A7 Pipeline.
.macro K30_MACRO
    @ Pre-load 2 block đầu tiên (8 phần tử)
    vld1.32 {q1}, [r12]!       @ w0-w3
    vld1.32 {q3}, [r10]!       @ i0-i3
    vld1.32 {q9}, [r12]!       @ w4-w7
    vld1.32 {q10}, [r10]!      @ i4-i7
    
    @ Iteration 1 & 2 (Interleaved Dual Accumulation)
    vmla.f32 q2, q3, q1        @ MAC 0-3 vào q2
    vld1.32 {q1}, [r12]!       @ w8-w11 (Load sớm che latency)
    vld1.32 {q3}, [r10]!       @ i8-i11
    
    vmla.f32 q8, q10, q9       @ MAC 4-7 vào q8 (Không phụ thuộc q2 -> No stall!)
    vld1.32 {q9}, [r12]!       @ w12-w15
    vld1.32 {q10}, [r10]!      @ i12-i15
    
    @ Iteration 3 & 4
    vmla.f32 q2, q3, q1        @ MAC 8-11 vào q2
    vld1.32 {q1}, [r12]!       @ w16-w19
    vld1.32 {q3}, [r10]!       @ i16-i19
    
    vmla.f32 q8, q10, q9       @ MAC 12-15 vào q8
    vld1.32 {q9}, [r12]!       @ w20-w23
    vld1.32 {q10}, [r10]!      @ i20-i23
    
    @ Iteration 5 & 6
    vmla.f32 q2, q3, q1        @ MAC 16-19 vào q2
    vld1.32 {q1}, [r12]!       @ w24-w27
    vld1.32 {q3}, [r10]!       @ i24-i27
    
    vmla.f32 q8, q10, q9       @ MAC 20-23 vào q8
    
    @ Iteration 7
    vmla.f32 q2, q3, q1        @ MAC 24-27 vào q2

    @ ── Scalar tail: remaining 2 elements (positions 28, 29) ─────────────────
    vld1.32 {d2}, [r12]!       @ Load w28, w29 vào d2 (s4, s5)
    vld1.32 {d6}, [r10]!       @ Load i28, i29 vào d6 (s12, s13)
    vmla.f32 s8, s12, s4       @ Tích luỹ w28 × i28 thẳng vào lane 0 của q2
    vmla.f32 s9, s13, s5       @ Tích luỹ w29 × i29 thẳng vào lane 1 của q2
.endm

conv1d:
    @ ── Function prologue: save callee-saved registers ───────────────────────
    push    {r4-r11, lr}       @ SP decreases by 36 bytes (No local stack needed!)

    @ ── Validation: ensure kernel_size=30 (hard-coded optimization) ──────────
    ldr     r4, [sp, #44]      @ r4 = kernel_size (Arg 7)
    cmp     r4, #30
    bne     .Ldone             @ early exit if kernel_size != 30

    @ ── Load parameters and establish constant base pointers ─────────────────
    mov     r4, r0             @ r4 = input_base (constant, never advances)
    ldr     r5, [sp, #40]      @ r5 = out_channels (loop counter, Arg 6)
    ldr     r12, [sp, #48]     @ r12 = stride (Arg 8)
    ldr     r9, [sp, #52]      @ r9 = output_channel_ptr (Arg 9)

    @ ── Compute output_len = floor((input_len - 30) / stride) + 1 ────────────
    @ TỐI ƯU HÓA: Sử dụng Hardware Integer Division thay vì Float Division
    sub     r1, r1, #30        @ r1 = input_len - kernel_size
    sdiv    r8, r1, r12        @ Integer Div: r8 = (input_len - 30) / stride
    add     r8, r8, #1         @ r8 = output_len (constant for all channels)

    @ ── Precompute stride steps (specialized for in_channels=1) ──────────────
    lsl     r6, r12, #2        @ r6 = input_step = stride * 4 (slide window)
    lsl     r7, r5, #2         @ r7 = out_step = out_channels * 4 (HWC layout)

    mov     r1, r2             @ r1 = weight_base
    @ Note: r3 continues to hold bias_ptr, no need to save/load from stack!

@ ── Outer loop: iterate over output channels ─────────────────────────────────
.Loc_loop:                     @ For each output channel oc = 0..out_channels-1
    cmp     r5, #0
    ble     .Ldone             @ exit when all channels processed

    @ Reset per-channel pointers
    mov     r2, r4             @ r2 = input_window (reset to input_base each channel)
    mov     r11, r8            @ r11 = patch counter (reset to output_len)

    @ ── Load bias for current output channel (or zero if no bias) ────────────
    cmp     r3, #0
    beq     .Lno_bias          @ branch if bias array is NULL
    vldr    s1, [r3]           @ s1 = bias[oc]
    add     r3, r3, #4         @ advance bias_ptr to next channel
    b       .Lbias_done
.Lno_bias:
    mov     r0, #0             
    vmov    s1, r0             @ s1 = 0.0f (no bias)
.Lbias_done:

    @ Get output pointer for this channel's first patch
    mov     lr, r9             @ lr = output_channel_ptr

@ ── Middle loop: iterate over time steps/patches ─────────────────────────────
.Lout_loop:                    @ For each patch t = 0..output_len-1
    @ Initialize accumulators
    vmov.i32 q2, #0            @ Primary accumulator
    vmov.i32 q8, #0            @ Secondary accumulator (hides pipeling latency)
    vmov.f32 s0, s1            @ s0 = bias (starting value)

    @ Setup pointers for kernel convolution
    mov     r12, r1            @ r12 = weight_ptr (current channel's weights)
    mov     r10, r2            @ r10 = input_ptr (current window start)
    
    K30_MACRO                  @ Perform massively parallel 30-element conv

    @ ── Horizontal sum: reduce NEON accumulators ─────────────────────────────
    vadd.f32 q2, q2, q8        @ Merge secondary accumulator q8 vào q2
    
    @ q2 is composed of d4 and d5. q2 = {s8, s9, s10, s11}
    vpadd.f32 d4, d4, d5       @ d4 = {s8+s9, s10+s11}
    vpadd.f32 d4, d4, d4       @ d4 = {(s8+s9)+(s10+s11), ...}
    vadd.f32 s0, s0, s8        @ s0 = bias + total_convolution_sum

    @ Store result and advance to next patch
    vstr    s0, [lr]           @ store output[oc][t]
    add     lr, lr, r7         @ lr += out_step (next patch, same channel)
    add     r2, r2, r6         @ r2 += input_step (slide window by stride)

    subs    r11, r11, #1       @ decrement patch counter
    bne     .Lout_loop         @ continue if more patches

    @ ── Advance to next output channel ───────────────────────────────────────
    add     r9, r9, #4         @ output_channel_ptr += 4
    add     r1, r1, #120       @ weight_base += 1*30*4 = 120 bytes

    subs    r5, r5, #1         @ decrement output channel counter
    b       .Loc_loop          @ process next output channel

.Ldone:
    @ ── Function epilogue: restore registers ─────────────────────────────────
    pop     {r4-r11, pc}       @ restore registers and return