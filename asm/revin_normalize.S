    .syntax unified
    .arch armv7-a
    .fpu neon-vfpv4
    .section .note.GNU-stack, "", %progbits   @ non-executable stack
    .text
    .align 2

    .global revin_normalize
    .type revin_normalize, %function

    .extern revin_layer_affine_weight
    .extern revin_layer_affine_bias

    .equ SEQ_LEN,       365
    .equ NUM_FEATURES,   1
    .equ ROW_STRIDE,     4

@ ═══════════════════════════════════════════════════════════════════════════
@ void revin_normalize(float *data, float *mean, float *stdev)
@
@ Arguments (AAPCS):
@   r0: float *data    r1: float *mean    r2: float *stdev
@ ═══════════════════════════════════════════════════════════════════════════

revin_normalize:
    @ ── Initialize NEON scratch registers for persistent storage ───────────────────────
    vmov    s10, r0              @ s10 = data_base (persistent across all features)
    mov     r3, #0
    vmov    s11, r3              @ s11 = feature_idx = 0 (loop counter)

    @ ── Load constants for statistical calculations ───────────────────────────────
    ldr     r12, =SEQ_LEN        @ r12 = sequence length (365)
    vmov    s14, r12
    vcvt.f64.s32 d7, s14         @ d7 = SEQ_LEN as double precision

    adr     r12, .LC_EPS         @ r12 = address of epsilon constant
    vldr    d6, [r12]            @ d6 = eps = 1.0e-5 (for numerical stability)

    @ ── Outer loop: iterate over features ───────────────────────────────────────────
.L_feature_loop:
    vmov    r3, s11              @ r3 = current feature_idx
    ldr     r12, =NUM_FEATURES   @ r12 = total number of features
    cmp     r3, r12
    bge     .L_done              @ exit when all features processed

    @ ── Pass 1: Compute mean across sequence dimension ─────────────────────────────
    vmov    r0, s10              @ r0 = data_base (reset pointer for this feature)
    add     r0, r0, r3, lsl #2   @ r0 = &data[0][f] (offset by feature index)
    
    mov     r12, #0
    vmov    d0, r12, r12         @ d0 = sum = 0.0 (double precision accumulator)
    ldr     r3, =SEQ_LEN         @ r3 = loop counter for sequence length

.L_mean_loop:
    vldr    s0, [r0]             @ s0 = data[i][f] (load current element)
    vcvt.f64.f32 d1, s0          @ d1 = data[i][f] as double precision
    vadd.f64 d0, d0, d1          @ sum += data[i][f]
    add     r0, r0, #ROW_STRIDE  @ advance to next sequence element (i+1)
    subs    r3, r3, #1           @ decrement sequence counter
    bne     .L_mean_loop         @ continue if more elements

    vdiv.f64 d0, d0, d7          @ mean = sum / SEQ_LEN
    vcvt.f32.f64 s0, d0          @ s0 = mean as single precision
    vmov    r3, s11              @ r3 = feature_idx (restore from NEON)
    add     r12, r1, r3, lsl #2  @ r12 = &mean[f] (output address)
    vstr    s0, [r12]            @ store computed mean

    @ ── Pass 2: Compute standard deviation with epsilon stabilization ─────────────────
    vldr    s1, [r12]            @ s1 = mean[f] (r12 still points to &mean[f])
    vcvt.f64.f32 d2, s1          @ d2 = mean[f] as double precision

    vmov    r0, s10              @ r0 = data_base (reset pointer for this feature)
    add     r0, r0, r3, lsl #2   @ r0 = &data[0][f] (same starting position)

    mov     r12, #0
    vmov    d3, r12, r12         @ d3 = sum_sq = 0.0 (variance accumulator)
    ldr     r3, =SEQ_LEN         @ r3 = loop counter for sequence length

.L_stdev_loop:
    vldr    s0, [r0]             @ s0 = data[i][f] (load current element)
    vcvt.f64.f32 d1, s0          @ d1 = data[i][f] as double precision
    vsub.f64 d4, d1, d2          @ d4 = diff = data[i][f] - mean[f]
    vmul.f64 d4, d4, d4          @ d4 = diff² (squared difference)
    vadd.f64 d3, d3, d4          @ sum_sq += diff²
    add     r0, r0, #ROW_STRIDE  @ advance to next sequence element (i+1)
    subs    r3, r3, #1           @ decrement sequence counter
    bne     .L_stdev_loop        @ continue if more elements

    vdiv.f64 d3, d3, d7          @ variance = sum_sq / SEQ_LEN
    vadd.f64 d3, d3, d6          @ variance += eps (numerical stability)
    vsqrt.f64 d3, d3             @ stdev = sqrt(variance + eps)
    vcvt.f32.f64 s2, d3          @ s2 = stdev[f] as single precision
    
    vmov    r3, s11              @ r3 = feature_idx (restore from NEON)
    add     r12, r2, r3, lsl #2  @ r12 = &stdev[f] (output address)
    vstr    s2, [r12]            @ store computed standard deviation

    @ ── Pass 3: Normalize data and apply affine transformation ───────────────────────
    @ s2 already contains stdev[f] from previous pass
    add     r12, r1, r3, lsl #2  @ r12 = &mean[f] (reload mean address)
    vldr    s1, [r12]            @ s1 = mean[f] (load mean for normalization)

    @ Load affine transformation parameters
    ldr     r12, =revin_layer_affine_weight
    ldr     r12, [r12]               @ dereference: r12 = *revin_layer_affine_weight
    add     r12, r12, r3, lsl #2 @ r12 = &weight[f]
    vldr    s3, [r12]            @ s3 = weight[f]

    ldr     r12, =revin_layer_affine_bias
    ldr     r12, [r12]               @ dereference: r12 = *revin_layer_affine_bias
    add     r12, r12, r3, lsl #2 @ r12 = &bias[f]
    vldr    s4, [r12]            @ s4 = bias[f]

    @ Setup for normalization loop
    vmov    r0, s10              @ r0 = data_base (reset pointer for this feature)
    add     r0, r0, r3, lsl #2   @ r0 = &data[0][f] (same starting position)
    ldr     r3, =SEQ_LEN         @ r3 = loop counter for sequence length

.L_norm_loop:
    vldr    s0, [r0]             @ s0 = data[i][f] (load current element)
    vsub.f32 s0, s0, s1          @ s0 = data[i][f] - mean[f] (center)
    vdiv.f32 s0, s0, s2          @ s0 = (data[i][f] - mean[f]) / stdev[f] (scale)
    vmul.f32 s0, s0, s3          @ s0 = normalized * weight[f] (affine scale)
    vadd.f32 s0, s0, s4          @ s0 = normalized * weight[f] + bias[f] (affine shift)
    vstr    s0, [r0]             @ store normalized data[i][f] (in-place)
    add     r0, r0, #ROW_STRIDE  @ advance to next sequence element (i+1)
    subs    r3, r3, #1           @ decrement sequence counter
    bne     .L_norm_loop         @ continue if more elements

    @ ── Increment feature index and continue to next feature ───────────────────────
    vmov    r3, s11              @ r3 = current feature_idx
    add     r3, r3, #1           @ feature_idx++
    vmov    s11, r3              @ save updated feature_idx to NEON
    b       .L_feature_loop      @ process next feature

.L_done:
    bx      lr                   @ return

    @ ── Read-only constants ───────────────────────────────────────────────────────
    .align 3                     @ 8-byte aligned for double precision loads
.LC_EPS:
    .double 1.0e-5               @ epsilon constant for numerical stability