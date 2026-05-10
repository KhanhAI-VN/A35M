.syntax unified
    .arch armv7-a
    .fpu vfpv4
    .section .note.GNU-stack, "", %progbits
    .text
    .align 2

    .global gelu
    .type gelu, %function

@ ═══════════════════════════════════════════════════════════════════════════
@ float gelu(float x)
@
@ Arguments (AAPCS):
@   s0: float x (input)
@   s0: float gelu(x) (output)
@ ═══════════════════════════════════════════════════════════════════════════
gelu:
    @ ── Load constants and initialize (Interleaved for dual-issue) ───────────────
    vmov.f32 s4, s0             @ s4 = original x
    vmul.f32 s1, s0, s0         @ s1 = x^2 (Starts VFP multi-cycle operation)

    adr      ip, .Lgelu_consts  @ [INT] Addr of core constants
    vmov.f32 s10, #0.5          @ [VFP] Load 0.5 via immediate
    vldmia   ip, {s5-s8}        @ [VFP/LSU] s5=0.044715, s6=0.79788456, s7=64.0, s8=8.0
    vmov.f32 s9, #1.0           @ [VFP] Load 1.0 via immediate
    ldr      ip, .Llut_ptr      @ [INT/LSU] Early load of LUT address

    @ ── Math Phase 1: Compute p and shorten critical path ────────────────────────
    vmul.f32 s4, s4, s10        @ s4 = 0.5 * x (Precompute early to avoid end-stall)

    @ Compute p = 0.79788456 * x * (1 + 0.044715 * x^2)
    vmul.f32 s2, s0, s6         @ s2 = x * 0.79788456
    vfma.f32 s9, s1, s5         @ s9 = 1.0 + (x^2 * 0.044715) [FMA for precision]
    vmul.f32 s0, s2, s9         @ s0 = p

    @ ── Math Phase 2: Absolute, Clamp and VMRS Latency Hiding ────────────────────
    vabs.f32 s1, s0             @ s1 = |p|
    vcmpe.f32 s1, s8            @ Compare |p| with 8.0

    @ ** ADVANCED CORTEX-A7 LATENCY HIDING **
    @ Executing pure Integer instructions while VFP comparison/flags resolve
    mov      r3, #0x80000000    @ [INT] Sign bit mask (bit 31)
    vmov     r1, s0             @ [VFP->INT] Move p to core to extract sign
    vmrs     apsr_nzcv, fpscr   @ [SYS] Transfer flags to core (Multi-cycle stall)

    vmovgt.f32 s1, s8           @ [VFP] Clamp |p| to 8.0
    and      r1, r1, r3         @ [INT] r1 = sign(p). Independent integer path

    @ ── Index calculation ────────────────────────────────────────────────────────
    vmul.f32 s1, s1, s7         @ s1 = |p| * 64.0

    @ Convert and extract fraction
    vcvt.u32.f32 s2, s1         @ s2 = (u32)index
    vcvt.f32.u32 s3, s2         @ s3 = (f32)floor(index)
    vmov     r0, s2             @ r0 = index (transfer to core register)
    vsub.f32 s1, s1, s3         @ s1 = frac = (|p| * 64.0) - floor

    @ ── LUT Access and Interpolation ─────────────────────────────────────────────
    @ ip already loaded early (no load-use stall)
    add      ip, ip, r0, lsl #2 @ ip = &LUT[index]
    vldmia   ip, {s2, s3}       @ s2 = v0, s3 = v1

    @ Interpolate: v0 + frac * (v1 - v0)
    vsub.f32 s3, s3, s2         @ s3 = v1 - v0
    vfma.f32 s2, s1, s3         @ s2 = |tanh(p)| [FMA instead of VMLA]

    @ ── Math Phase 3: Zero-Branch Sign Application ───────────────────────────────
    @ ** BITWISE FLOAT OPTIMIZATION (Eliminates 2nd VMRS and Branches completely) **
    vmov     r2, s2             @ [VFP->INT] r2 = |tanh(p)|
    orr      r2, r2, r1         @ [INT] Apply original sign bit of p via bitwise OR
    vmov     s2, r2             @ [INT->VFP] s2 = tanh(p)

    @ ── Final Math: (0.5 * x) * (1 + tanh(p)) ────────────────────────────────────
    vmov.f32 s11, #1.0          @ Load immediate 1.0 (vmov does not stall)
    vadd.f32 s2, s2, s11        @ s2 = 1 + tanh(p)
    vmul.f32 s0, s4, s2         @ s0 = (0.5 * x) * (1 + tanh(p))

    bx       lr                 @ Return (0 STACK USED)

    .align 2
.Lgelu_consts:
    .float 0.044715
    .float 0.79788456
    .float 64.0
    .float 8.0

.Llut_ptr:
    .word tanh_lut_data

    .size gelu, .-gelu

    @ ── LUT data (514 entries for tanh approximation) ─────────────────────────────
    .data
    .align 2
    .global tanh_lut_data
tanh_lut_data:
    .word 0x00000000, 0x3c7ffaab, 0x3cffeaad, 0x3d3fdc08
    .word 0x3d7faacd, 0x3d9facdf, 0x3dbf7081, 0x3ddf1c6c
    .word 0x3dfeacc9, 0x3e0f0ee8, 0x3e1eb5e4, 0x3e2e4984
    .word 0x3e3dc7fd, 0x3e4d2f8e, 0x3e5c7e82, 0x3e6bb32e
    .word 0x3e7acbf5, 0x3e84e3a2, 0x3e8c51cd, 0x3e93afbf
    .word 0x3e9afcc5, 0x3ea23833, 0x3ea96163, 0x3eb077b9
    .word 0x3eb77a9f, 0x3ebe6989, 0x3ec543f1, 0x3ecc095a
    .word 0x3ed2b94f, 0x3ed95365, 0x3edfd735, 0x3ee64465
    .word 0x3eec9a9f, 0x3ef2d997, 0x3ef90109, 0x3eff10b8
    .word 0x3f028438, 0x3f057401, 0x3f0857a4, 0x3f0b2f12
    .word 0x3f0dfa3f, 0x3f10b923, 0x3f136bb8, 0x3f1611fc
    .word 0x3f18abf0, 0x3f1b3999, 0x3f1dbafc, 0x3f203025
    .word 0x3f22991f, 0x3f24f5f9, 0x3f2746c5, 0x3f298b95
    .word 0x3f2bc480, 0x3f2df19c, 0x3f301305, 0x3f3228d4
    .word 0x3f343328, 0x3f36321f, 0x3f3825d9, 0x3f3a0e77
    .word 0x3f3bec1c, 0x3f3dbeed, 0x3f3f870d, 0x3f4144a3
    .word 0x3f42f7d6, 0x3f44a0cc, 0x3f463fae, 0x3f47d4a5
    .word 0x3f495fda, 0x3f4ae176, 0x3f4c59a3, 0x3f4dc88c
    .word 0x3f4f2e5b, 0x3f508b3a, 0x3f51df55, 0x3f532ad5
    .word 0x3f546de6, 0x3f55a8b2, 0x3f56db63, 0x3f580624
    .word 0x3f59291e, 0x3f5a447c, 0x3f5b5866, 0x3f5c6506
    .word 0x3f5d6a85, 0x3f5e690b, 0x3f5f60c0, 0x3f6051ca
    .word 0x3f613c53, 0x3f62207e, 0x3f62fe74, 0x3f63d659
    .word 0x3f64a852, 0x3f657483, 0x3f663b10, 0x3f66fc1d
    .word 0x3f67b7cc, 0x3f686e3f, 0x3f691f97, 0x3f69cbf5
    .word 0x3f6a737a, 0x3f6b1645, 0x3f6bb475, 0x3f6c4e29
    .word 0x3f6ce37d, 0x3f6d7491, 0x3f6e017f, 0x3f6e8a64
    .word 0x3f6f0f5b, 0x3f6f907f, 0x3f700dea, 0x3f7087b7
    .word 0x3f70fdfd, 0x3f7170d5, 0x3f71e058, 0x3f724c9b
    .word 0x3f72b5b8, 0x3f731bc2, 0x3f737ed1, 0x3f73def9
    .word 0x3f743c4f, 0x3f7496e8, 0x3f74eed6, 0x3f75442d
    .word 0x3f7596ff, 0x3f75e760, 0x3f763560, 0x3f768111
    .word 0x3f76ca83, 0x3f7711c7, 0x3f7756ee, 0x3f779a06
    .word 0x3f77db1e, 0x3f781a46, 0x3f78578b, 0x3f7892fd
    .word 0x3f78cca7, 0x3f790498, 0x3f793adc, 0x3f796f7f
    .word 0x3f79a28f, 0x3f79d416, 0x3f7a0421, 0x3f7a32b9
    .word 0x3f7a5feb, 0x3f7a8bc1, 0x3f7ab645, 0x3f7adf80
    .word 0x3f7b077e, 0x3f7b2e46, 0x3f7b53e2, 0x3f7b785c
    .word 0x3f7b9bbb, 0x3f7bbe09, 0x3f7bdf4c, 0x3f7bff8e
    .word 0x3f7c1ed6, 0x3f7c3d2b, 0x3f7c5a95, 0x3f7c771a
    .word 0x3f7c92c1, 0x3f7cad92, 0x3f7cc792, 0x3f7ce0c8
    .word 0x3f7cf939, 0x3f7d10ed, 0x3f7d27e8, 0x3f7d3e2f
    .word 0x3f7d53ca, 0x3f7d68bb, 0x3f7d7d0a, 0x3f7d90ba
    .word 0x3f7da3d0, 0x3f7db652, 0x3f7dc843, 0x3f7dd9a8
    .word 0x3f7dea85, 0x3f7dfade, 0x3f7e0ab8, 0x3f7e1a15
    .word 0x3f7e28fb, 0x3f7e376c, 0x3f7e456c, 0x3f7e52fe
    .word 0x3f7e6026, 0x3f7e6ce8, 0x3f7e7945, 0x3f7e8542
    .word 0x3f7e90e0, 0x3f7e9c24, 0x3f7ea710, 0x3f7eb1a6
    .word 0x3f7ebbe9, 0x3f7ec5db, 0x3f7ecf80, 0x3f7ed8d9
    .word 0x3f7ee1e8, 0x3f7eeab1, 0x3f7ef334, 0x3f7efb75
    .word 0x3f7f0376, 0x3f7f0b37, 0x3f7f12bb, 0x3f7f1a05
    .word 0x3f7f2115, 0x3f7f27ee, 0x3f7f2e91, 0x3f7f3500
    .word 0x3f7f3b3d, 0x3f7f4148, 0x3f7f4724, 0x3f7f4cd2
    .word 0x3f7f5254, 0x3f7f57aa, 0x3f7f5cd6, 0x3f7f61da
    .word 0x3f7f66b6, 0x3f7f6b6c, 0x3f7f6ffd, 0x3f7f746a
    .word 0x3f7f78b4, 0x3f7f7cdd, 0x3f7f80e5, 0x3f7f84cd
    .word 0x3f7f8896, 0x3f7f8c42, 0x3f7f8fd1, 0x3f7f9344
    .word 0x3f7f969b, 0x3f7f99d9, 0x3f7f9cfd, 0x3f7fa008
    .word 0x3f7fa2fb, 0x3f7fa5d8, 0x3f7fa89d, 0x3f7fab4d
    .word 0x3f7fade8, 0x3f7fb06e, 0x3f7fb2e0, 0x3f7fb53f
    .word 0x3f7fb78c, 0x3f7fb9c6, 0x3f7fbbef, 0x3f7fbe07
    .word 0x3f7fc00e, 0x3f7fc206, 0x3f7fc3ee, 0x3f7fc5c7
    .word 0x3f7fc791, 0x3f7fc94d, 0x3f7fcafc, 0x3f7fcc9d
    .word 0x3f7fce32, 0x3f7fcfba, 0x3f7fd136, 0x3f7fd2a6
    .word 0x3f7fd40c, 0x3f7fd566, 0x3f7fd6b5, 0x3f7fd7fa
    .word 0x3f7fd935, 0x3f7fda67, 0x3f7fdb8f, 0x3f7fdcae
    .word 0x3f7fddc4, 0x3f7fded1, 0x3f7fdfd7, 0x3f7fe0d4
    .word 0x3f7fe1c9, 0x3f7fe2b7, 0x3f7fe39e, 0x3f7fe47d
    .word 0x3f7fe556, 0x3f7fe628, 0x3f7fe6f4, 0x3f7fe7b9
    .word 0x3f7fe878, 0x3f7fe931, 0x3f7fe9e5, 0x3f7fea93
    .word 0x3f7feb3c, 0x3f7febdf, 0x3f7fec7e, 0x3f7fed17
    .word 0x3f7fedac, 0x3f7fee3d, 0x3f7feec9, 0x3f7fef50
    .word 0x3f7fefd4, 0x3f7ff053, 0x3f7ff0ce, 0x3f7ff146
    .word 0x3f7ff1ba, 0x3f7ff22a, 0x3f7ff297, 0x3f7ff301
    .word 0x3f7ff367, 0x3f7ff3cb, 0x3f7ff42b, 0x3f7ff488
    .word 0x3f7ff4e2, 0x3f7ff53a, 0x3f7ff58f, 0x3f7ff5e1
    .word 0x3f7ff631, 0x3f7ff67e, 0x3f7ff6c9, 0x3f7ff711
    .word 0x3f7ff758, 0x3f7ff79c, 0x3f7ff7de, 0x3f7ff81e
    .word 0x3f7ff85c, 0x3f7ff898, 0x3f7ff8d3, 0x3f7ff90b
    .word 0x3f7ff942, 0x3f7ff977, 0x3f7ff9aa, 0x3f7ff9dc
    .word 0x3f7ffa0d, 0x3f7ffa3c, 0x3f7ffa69, 0x3f7ffa95
    .word 0x3f7ffac0, 0x3f7ffae9, 0x3f7ffb11, 0x3f7ffb38
    .word 0x3f7ffb5e, 0x3f7ffb82, 0x3f7ffba6, 0x3f7ffbc8
    .word 0x3f7ffbe9, 0x3f7ffc09, 0x3f7ffc28, 0x3f7ffc47
    .word 0x3f7ffc64, 0x3f7ffc80, 0x3f7ffc9c, 0x3f7ffcb7
    .word 0x3f7ffcd1, 0x3f7ffcea, 0x3f7ffd02, 0x3f7ffd1a
    .word 0x3f7ffd30, 0x3f7ffd47, 0x3f7ffd5c, 0x3f7ffd71
    .word 0x3f7ffd85, 0x3f7ffd99, 0x3f7ffdab, 0x3f7ffdbe
    .word 0x3f7ffdd0, 0x3f7ffde1, 0x3f7ffdf2, 0x3f7ffe02
    .word 0x3f7ffe11, 0x3f7ffe21, 0x3f7ffe2f, 0x3f7ffe3e
    .word 0x3f7ffe4c, 0x3f7ffe59, 0x3f7ffe66, 0x3f7ffe73
    .word 0x3f7ffe7f, 0x3f7ffe8b, 0x3f7ffe96, 0x3f7ffea1
    .word 0x3f7ffeac, 0x3f7ffeb7, 0x3f7ffec1, 0x3f7ffecb
    .word 0x3f7ffed4, 0x3f7ffedd, 0x3f7ffee6, 0x3f7ffeef
    .word 0x3f7ffef7, 0x3f7ffeff, 0x3f7fff07, 0x3f7fff0f
    .word 0x3f7fff16, 0x3f7fff1e, 0x3f7fff25, 0x3f7fff2b
    .word 0x3f7fff32, 0x3f7fff38, 0x3f7fff3e, 0x3f7fff44
    .word 0x3f7fff4a, 0x3f7fff50, 0x3f7fff55, 0x3f7fff5a
    .word 0x3f7fff5f, 0x3f7fff64, 0x3f7fff69, 0x3f7fff6e
    .word 0x3f7fff72, 0x3f7fff77, 0x3f7fff7b, 0x3f7fff7f
    .word 0x3f7fff83, 0x3f7fff87, 0x3f7fff8b, 0x3f7fff8e
    .word 0x3f7fff92, 0x3f7fff95, 0x3f7fff98, 0x3f7fff9c
    .word 0x3f7fff9f, 0x3f7fffa2, 0x3f7fffa5, 0x3f7fffa7
    .word 0x3f7fffaa, 0x3f7fffad, 0x3f7fffaf, 0x3f7fffb2
    .word 0x3f7fffb4, 0x3f7fffb6, 0x3f7fffb9, 0x3f7fffbb
    .word 0x3f7fffbd, 0x3f7fffbf, 0x3f7fffc1, 0x3f7fffc3
    .word 0x3f7fffc5, 0x3f7fffc7, 0x3f7fffc9, 0x3f7fffca
    .word 0x3f7fffcc, 0x3f7fffcd, 0x3f7fffcf, 0x3f7fffd1
    .word 0x3f7fffd2, 0x3f7fffd3, 0x3f7fffd5, 0x3f7fffd6
    .word 0x3f7fffd7, 0x3f7fffd9, 0x3f7fffda, 0x3f7fffdb
    .word 0x3f7fffdc, 0x3f7fffdd, 0x3f7fffde, 0x3f7fffdf
    .word 0x3f7fffe0, 0x3f7fffe1, 0x3f7fffe2, 0x3f7fffe3
    .word 0x3f7fffe4, 0x3f7fffe5, 0x3f7fffe6, 0x3f7fffe7
    .word 0x3f7fffe7, 0x3f7fffe8, 0x3f7fffe9, 0x3f7fffea
    .word 0x3f7fffea, 0x3f7fffeb, 0x3f7fffec, 0x3f7fffec
    .word 0x3f7fffed, 0x3f7fffed, 0x3f7fffee, 0x3f7fffef
    .word 0x3f7fffef, 0x3f7ffff0, 0x3f7ffff0, 0x3f7ffff1
    .word 0x3f7ffff1, 0x3f7ffff2, 0x3f7ffff2, 0x3f7ffff2
    .word 0x3f7ffff3, 0x3f7ffff3, 0x3f7ffff4, 0x3f7ffff4
    .word 0x3f7ffff4, 0x3f7ffff5, 0x3f7ffff5, 0x3f7ffff5
    .word 0x3f7ffff6, 0x3f7ffff6, 0x3f7ffff6, 0x3f7ffff7
    .word 0x3f7ffff7, 0x3f7ffff7, 0x3f7ffff7, 0x3f7ffff8
    .word 0x3f7ffff8, 0x3f7ffff8, 0x3f7ffff8, 0x3f7ffff9
    .word 0x3f7ffff9, 0x3f7ffff9, 0x3f7ffff9, 0x3f7ffffa
    .word 0x3f7ffffa, 0x3f7ffffa, 0x3f7ffffa, 0x3f7ffffa
    .word 0x3f7ffffb, 0x3f7ffffb, 0x3f7ffffb, 0x3f7ffffb
    .word 0x3f7ffffb, 0x3f7ffffb, 0x3f7ffffb, 0x3f7ffffc
    .word 0x3f7ffffc, 0x3f7ffffc, 0x3f7ffffc, 0x3f7ffffc
    .word 0x3f7ffffc, 0x3f800000
    .section .note.GNU-stack, "", %progbits