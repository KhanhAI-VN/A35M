.syntax unified
    .arch armv7-a
    .fpu neon
    .section .note.GNU-stack, "", %progbits
    .text
    .align 4

    .global flatten_patches
    .type flatten_patches, %function

@ ═══════════════════════════════════════════════════════════════════════════
@ void flatten_patches(const float *patches, int num_patches, int channels, float *flat)
@
@ Arguments (AAPCS Standard - 0 Stack Footprint):
@   r0: const float *patches    r1: int num_patches (Repurposed as stride)
@   r2: int channels (fixed 16) r3: float *flat (Moving pointer)
@ ═══════════════════════════════════════════════════════════════════════════

flatten_patches:
    @ ── Compute strides & setup counters (0 Stack constraint) ──────────────────────
    lsl     r2, r1, #2              @ r2 = output stride (num_patches * 4 bytes)
    lsrs    r12, r1, #2             @ r12 = loop counter (num_patches / 4)
    beq     .Lcheck_remainder       @ Skip to remainder if < 4 patches

    @ Cortex-A7 Optimization: Repurpose r1 as input patch stride (16 floats = 64 bytes)
    @ This allows zero-cost post-index addressing [r0], r1
    mov     r1, #64

.Lloop_4patches:
    @ ════ PHASE 1: Process Channels 0..7 (C0-C7) ═══════════════════════════════════
    @ Load C0..C7. Auto-increment r0 by 64 bytes (r1) to jump directly to next patch
    vld1.32 {d0-d3}, [r0], r1       @ P0_C0..C7 -> q0, q1. r0 -> P1_C0
    vld1.32 {d4-d7}, [r0], r1       @ P1_C0..C7 -> q2, q3. r0 -> P2_C0
    vld1.32 {d16-d19}, [r0], r1     @ P2_C0..C7 -> q8, q9. r0 -> P3_C0
    vld1.32 {d20-d23}, [r0], r1     @ P3_C0..C7 -> q10, q11. r0 -> P4_C0

    @ Latency Hiding: Execute pointer math while NEON waits for vld1 to complete
    @ Rewind r0 from P4_C0 back to P0_C8: (4 * 64) - 32 = 224 bytes backward
    sub     r0, r0, #224

    @ ── Interleaved 4x4 matrix transpose (1 cycle issue, 2 cycle latency) ──────────
    vtrn.32 q0, q2                  
    vtrn.32 q1, q3                  
    vtrn.32 q8, q10
    vtrn.32 q9, q11

    vswp    d1, d16                 
    vswp    d3, d18                 
    vswp    d5, d20                 
    vswp    d7, d22                 

    @ ── Store C0-C7. r3 auto-increments by r2 (output stride) perfectly ────────────
    vst1.32 {d0-d1}, [r3], r2       @ Store C0, r3 += channel_stride
    vst1.32 {d4-d5}, [r3], r2       @ Store C1
    vst1.32 {d16-d17}, [r3], r2     @ Store C2
    vst1.32 {d20-d21}, [r3], r2     @ Store C3
    
    vst1.32 {d2-d3}, [r3], r2       @ Store C4
    vst1.32 {d6-d7}, [r3], r2       @ Store C5
    vst1.32 {d18-d19}, [r3], r2     @ Store C6
    vst1.32 {d22-d23}, [r3], r2     @ Store C7

    @ ════ PHASE 2: Process Channels 8..15 (C8-C15) ═════════════════════════════════
    @ r0 is perfectly positioned at P0_C8. Load C8..C15 for all 4 patches.
    vld1.32 {d0-d3}, [r0], r1       @ P0_C8..C15. r0 -> P1_C8
    vld1.32 {d4-d7}, [r0], r1       @ P1_C8..C15. r0 -> P2_C8
    vld1.32 {d16-d19}, [r0], r1     @ P2_C8..C15. r0 -> P3_C8
    vld1.32 {d20-d23}, [r0], r1     @ P3_C8..C15. r0 -> P4_C8

    @ Latency Hiding: Prepare r0 for the next block iteration
    @ Rewind r0 from P4_C8 back to P4_C0 (32 bytes backward)
    sub     r0, r0, #32

    @ ── Interleaved 4x4 matrix transpose ───────────────────────────────────────────
    vtrn.32 q0, q2
    vtrn.32 q1, q3
    vtrn.32 q8, q10
    vtrn.32 q9, q11

    vswp    d1, d16
    vswp    d3, d18
    vswp    d5, d20
    vswp    d7, d22

    @ ── Store C8-C15 ───────────────────────────────────────────────────────────────
    vst1.32 {d0-d1}, [r3], r2       @ Store C8
    vst1.32 {d4-d5}, [r3], r2       @ Store C9
    vst1.32 {d16-d17}, [r3], r2     @ Store C10
    vst1.32 {d20-d21}, [r3], r2     @ Store C11
    
    vst1.32 {d2-d3}, [r3], r2       @ Store C12
    vst1.32 {d6-d7}, [r3], r2       @ Store C13
    vst1.32 {d18-d19}, [r3], r2     @ Store C14
    vst1.32 {d22-d23}, [r3], r2     @ Store C15

    @ ── 0-Stack Pointer Math: Reset Output Pointer ─────────────────────────────────
    @ r3 is currently at flat[16][p]. We need to reset it to flat[0][p+4].
    @ flat[0][p+4] is exactly (r3 - 16*stride) + 16 bytes.
    sub     r3, r3, r2, lsl #4      @ r3 = r3 - (16 * r2)
    add     r3, r3, #16             @ r3 = r3 + 16 bytes (advance to next 4-patch base)

    subs    r12, r12, #1            @ Decrement loop counter
    bne     .Lloop_4patches

.Lcheck_remainder:
    @ ── Handle remaining patches (0-3) with scalar fallback ────────────────────────
    lsr     r1, r2, #2              @ Recover original num_patches: r1 = r2 / 4
    ands    r1, r1, #3              @ r1 = remaining patches (num_patches % 4)
    beq     .Lend                   @ Exit if no remainder

.Lrem_loop:
    mov     r12, r3                 @ r12 = output pointer for this patch

    @ C0..C3 (Scatter individual floats to correct channels)
    vld1.32 {d0-d1}, [r0]!          @ r0 automatically advances by 16 bytes
    vst1.32 {d0[0]}, [r12], r2
    vst1.32 {d0[1]}, [r12], r2
    vst1.32 {d1[0]}, [r12], r2
    vst1.32 {d1[1]}, [r12], r2

    @ C4..C7
    vld1.32 {d0-d1}, [r0]!
    vst1.32 {d0[0]}, [r12], r2
    vst1.32 {d0[1]}, [r12], r2
    vst1.32 {d1[0]}, [r12], r2
    vst1.32 {d1[1]}, [r12], r2

    @ C8..C11
    vld1.32 {d0-d1}, [r0]!
    vst1.32 {d0[0]}, [r12], r2
    vst1.32 {d0[1]}, [r12], r2
    vst1.32 {d1[0]}, [r12], r2
    vst1.32 {d1[1]}, [r12], r2

    @ C12..C15
    vld1.32 {d0-d1}, [r0]!
    vst1.32 {d0[0]}, [r12], r2
    vst1.32 {d0[1]}, [r12], r2
    vst1.32 {d1[0]}, [r12], r2
    vst1.32 {d1[1]}, [r12], r2

    add     r3, r3, #4              @ Advance output base by 1 float for next remainder
    subs    r1, r1, #1
    bne     .Lrem_loop

.Lend:
    bx      lr                      @ Return (No stack to clean up)

    .size flatten_patches, . - flatten_patches