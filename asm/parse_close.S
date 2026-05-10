.syntax unified
    .arch armv7-a
    .fpu vfpv4
    .section .note.GNU-stack, "", %progbits   @ non-executable stack

    .section .rodata
    .align 3                       @ 8-byte aligned for double precision loads
fp_constants:
    .double 0.0                    @ +0  offset: zero constant
    .double 1.0                    @ +8  offset: positive sign
    .double -1.0                   @ +16 offset: negative sign  
    .double 10.0                   @ +24 offset: decimal base multiplier
    .double 0.1                    @ +32 offset: decimal fraction multiplier

    .text
    .global parse_close_chunk
    .type parse_close_chunk, %function

@ ═══════════════════════════════════════════════════════════════════════════
@ int parse_close_chunk(const char *json_chunk, int days, float *out_data, ParseState *state)
@
@ Arguments (AAPCS):
@   r0: const char *json_chunk    r1: int days
@   r2: float *out_data           r3: ParseState *state
@ ═══════════════════════════════════════════════════════════════════════════

parse_close_chunk:
    @ ── Load parsing state into NEON scratch registers (no stack usage) ─────────────
    vmov    s4, r1              @ s4 = days (buffer size)
    vmov    s7, r3              @ s7 = ParseState pointer (for saving state)
    
    ldr     r1, [r3, #0]        @ r1 = state->total_parsed
    vmov    s5, r1              @ s5 = total_parsed (running count)
    ldr     r1, [r3, #4]        @ r1 = state->write_offset
    vmov    s6, r1              @ s6 = write_offset (circular buffer position)

@ ── Main parsing loop: find OHLCV arrays in JSON chunk ───────────────────────────
find_next_item:
    ldrb    r12, [r0], #1       @ r12 = current JSON character
    cmp     r12, #0x5B          @ check for '[' (OHLCV array start)
    beq     parse_value_start
    cmp     r12, #0x5D          @ check for ']' (array end)
    beq     asm_exit
    cmp     r12, #0             @ check for null terminator
    bne     find_next_item      @ continue searching
    b       asm_exit

@ ── Found OHLCV array: fast comma-skip to reach close price (index 4) ─────────────
parse_value_start:
    mov     r3, #4              @ r3 = target comma count (need 4 commas)

skip_commas_loop:
    ldrb    r12, [r0], #1       @ r12 = current character
    cmp     r12, #0x2C          @ check for ',' (field separator)
    subeq   r3, r3, #1          @ conditional execution: if comma, decrement target
    cmpeq   r3, #0              @ if comma WAS found, check if we reached index 4
    beq     find_val_start      @ reached 4 commas, start parsing close price
    cmp     r12, #0x5D          @ check for ']' (malformed/early end)
    beq     find_next_item      @ skip to next array
    cmp     r12, #0             @ check for EOF
    bne     skip_commas_loop    @ ultra-tight loop, no nested branches
    b       asm_exit

@ ── Locate start of close price number (skip whitespace) ───────────────────────
find_val_start:
    ldrb    r12, [r0], #1       @ r12 = current character
    cmp     r12, #0
    beq     asm_exit
    cmp     r12, #0x5D          @ check for ']' (incomplete number)
    beq     find_next_item
    cmp     r12, #0x2D          @ check for '-' (negative sign)
    beq     setup_minus
    cmp     r12, #'0'           @ check for digit start
    blt     find_val_start      @ skip non-digit characters
    cmp     r12, #'9'
    bgt     find_val_start      @ skip non-digit characters
    @ Fall through to setup_plus

@ ── Setup positive number parsing ───────────────────────────────────────────────
setup_plus:
    ldr     r1, =fp_constants
    vldr.f64 d5, [r1]           @ d5 = 0.0 (accumulator)
    vldr.f64 d6, [r1, #8]       @ d6 = 1.0 (positive sign)
    vldr.f64 d7, [r1, #24]      @ d7 = 10.0 (decimal base)
    b       process_first_digit

@ ── Setup negative number parsing ───────────────────────────────────────────────
setup_minus:
    ldr     r1, =fp_constants
    vldr.f64 d5, [r1]           @ d5 = 0.0 (accumulator)
    vldr.f64 d6, [r1, #16]      @ d6 = -1.0 (negative sign)
    vldr.f64 d7, [r1, #24]      @ d7 = 10.0 (decimal base)
    b       get_next_digit

@ ── Parse integer part: fast VMLA accumulation ────────────────────────────────────
get_next_digit:
    ldrb    r12, [r0], #1       @ r12 = next character
    cmp     r12, #'0'           @ check for digit
    blt     check_dot           @ not digit, check for decimal point
    cmp     r12, #'9'
    bgt     check_dot           @ not digit, check for decimal point

process_first_digit:
    sub     r1, r12, #'0'       @ r1 = digit value (0-9)
    vmov    s0, r1              @ s0 = digit as float
    vcvt.f64.u32 d4, s0         @ d4 = digit as double
    vmla.f64 d4, d5, d7         @ d4 = digit + acc * 10 (Fast Fused Mac)
    vmov.f64 d5, d4             @ update accumulator
    b       get_next_digit      @ process next digit

@ ── Check for decimal point (fractional part) ───────────────────────────────────
check_dot:
    cmp     r12, #0x2E          @ check for '.' (decimal point)
    beq     setup_fraction      @ start parsing fraction
    b       finish_number       @ no decimal point, finish number

@ ── Setup fractional part parsing (0-stack, using caller-saved d16) ───────────────
setup_fraction:
    ldr     r1, =fp_constants
    vldr.f64 d1, [r1, #32]      @ d1 = 0.1 (weight for fraction)
    vldr.f64 d16, [r1, #32]     @ d16 = 0.1 (base multiplier to avoid division)

@ ── Parse fractional part: fast multiply-accumulate without slow vdiv.f64 ────────
fraction_loop:
    ldrb    r12, [r0], #1       @ r12 = next character
    cmp     r12, #'0'           @ check for digit
    blt     finish_number       @ not digit, finish parsing
    cmp     r12, #'9'
    bgt     finish_number       @ not digit, finish parsing

    sub     r1, r12, #'0'       @ r1 = digit value (0-9)
    vmov    s0, r1              @ s0 = digit as float
    vcvt.f64.u32 d4, s0         @ d4 = digit as double
    vmla.f64 d5, d4, d1         @ accumulator += digit * weight
    vmul.f64 d1, d1, d16        @ weight *= 0.1
    b       fraction_loop       @ process next fraction digit

@ ── Apply sign and store parsed number ───────────────────────────────────────────
finish_number:
    vmul.f64 d5, d5, d6         @ d5 = accumulator * sign (apply +/-)

@ ── Store parsed close price in circular buffer ───────────────────────────────────
store_struct:
    vcvt.f32.f64 s0, d5         @ s0 = parsed value as float
    vmov    r1, s6              @ r1 = write_offset
    add     r1, r2, r1, lsl #2  @ r1 = out_data + write_offset*4 (circular address)
    vstr.32 s0, [r1]            @ store close price

@ ── Consume trailing characters directly looking for array closure ']' ────────────
consume_trailing:
    cmp     r12, #0x5D          @ check if terminating char was already ']'
    beq     end_of_valid_item
consume_trailing_loop:
    ldrb    r12, [r0], #1       @ fast consume without tracking commas
    cmp     r12, #0x5D          @ check for ']' (array end)
    beq     end_of_valid_item
    cmp     r12, #0
    bne     consume_trailing_loop
    b       asm_exit

@ ── Handle valid OHLCV item completion ───────────────────────────────────────────
end_of_valid_item:
    @ Update parsing counters
    vmov    r1, s5              @ r1 = total_parsed
    add     r1, r1, #1          @ total_parsed++
    vmov    s5, r1              @ save updated total_parsed
    
    vmov    r1, s6              @ r1 = write_offset
    add     r1, r1, #1          @ write_offset++
    
    @ Circular buffer wrap-around check (branchless logic via conditional execution)
    vmov    r3, s4              @ r3 = days (overwrites column_index)
    cmp     r1, r3              @ compare write_offset with days
    moveq   r1, #0              @ wrap to 0 if reached end
    vmov    s6, r1              @ save updated write_offset
    b       find_next_item      @ loop back to next array

@ ── Exit: dual-word store parsing state and return total_parsed ──────────────────
asm_exit:
    vmov    r1, s7              @ r1 = ParseState pointer
    vmov    r2, s5              @ r2 = total_parsed 
    vmov    r3, s6              @ r3 = write_offset
    strd    r2, r3, [r1]        @ fast store both 32-bit variables in 1 cycle
    mov     r0, r2              @ r0 = total_parsed (return value)
    bx      lr                  @ return