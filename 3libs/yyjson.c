#include "yyjson.h"

#include <math.h>

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-label"
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wunused-variable"
#elif YYJSON_IS_REAL_GCC && yyjson_gcc_available(4, 2, 0)
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-label"
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wunused-variable"
#elif defined(_MSC_VER)
#pragma warning(disable : 4100)
#pragma warning(disable : 4101)
#pragma warning(disable : 4102)
#pragma warning(disable : 4127)
#pragma warning(disable : 4702)
#pragma warning(disable : 4706)
#endif

uint32_t yyjson_version(void) { return YYJSON_VERSION_HEX; }

#if YYJSON_MSC_VER >= 1400
#include <intrin.h>
#if defined(_M_AMD64) || defined(_M_ARM64)
#define MSC_HAS_BIT_SCAN_64 1
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)
#else
#define MSC_HAS_BIT_SCAN_64 0
#endif
#if defined(_M_AMD64) || defined(_M_ARM64) || defined(_M_IX86) || \
    defined(_M_ARM)
#define MSC_HAS_BIT_SCAN 1
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)
#else
#define MSC_HAS_BIT_SCAN 0
#endif
#if defined(_M_AMD64)
#define MSC_HAS_UMUL128 1
#pragma intrinsic(_umul128)
#else
#define MSC_HAS_UMUL128 0
#endif
#else
#define MSC_HAS_BIT_SCAN_64 0
#define MSC_HAS_BIT_SCAN 0
#define MSC_HAS_UMUL128 0
#endif

#if yyjson_has_builtin(__builtin_clzll) || yyjson_gcc_available(3, 4, 0)
#define GCC_HAS_CLZLL 1
#else
#define GCC_HAS_CLZLL 0
#endif

#if yyjson_has_builtin(__builtin_ctzll) || yyjson_gcc_available(3, 4, 0)
#define GCC_HAS_CTZLL 1
#else
#define GCC_HAS_CTZLL 0
#endif

#if defined(__SIZEOF_INT128__) && (__SIZEOF_INT128__ == 16) && \
    (defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER))
#define YYJSON_HAS_INT128 1
#else
#define YYJSON_HAS_INT128 0
#endif

#if defined(__STDC_IEC_559__) || defined(__STDC_IEC_60559_BFP__)
#define YYJSON_HAS_IEEE_754 1
#elif FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_DIG == 6 &&                \
    FLT_MIN_EXP == -125 && FLT_MAX_EXP == 128 && FLT_MIN_10_EXP == -37 &&    \
    FLT_MAX_10_EXP == 38 && DBL_MANT_DIG == 53 && DBL_DIG == 15 &&           \
    DBL_MIN_EXP == -1021 && DBL_MAX_EXP == 1024 && DBL_MIN_10_EXP == -307 && \
    DBL_MAX_10_EXP == 308
#define YYJSON_HAS_IEEE_754 1
#else
#define YYJSON_HAS_IEEE_754 0
#undef YYJSON_DISABLE_FAST_FP_CONV
#define YYJSON_DISABLE_FAST_FP_CONV 1
#endif

#if !defined(FLT_EVAL_METHOD) && defined(__FLT_EVAL_METHOD__)
#define FLT_EVAL_METHOD __FLT_EVAL_METHOD__
#endif

#if defined(FLT_EVAL_METHOD) && FLT_EVAL_METHOD != 0 && FLT_EVAL_METHOD != 1
#define YYJSON_DOUBLE_MATH_CORRECT 0
#elif defined(i386) || defined(__i386) || defined(__i386__) || \
    defined(_X86_) || defined(__X86__) || defined(_M_IX86) ||  \
    defined(__I86__) || defined(__IA32__) || defined(__THW_INTEL)
#if (defined(_MSC_VER) && defined(_M_IX86_FP) && _M_IX86_FP == 2) || \
    (defined(__SSE2_MATH__) && __SSE2_MATH__)
#define YYJSON_DOUBLE_MATH_CORRECT 1
#else
#define YYJSON_DOUBLE_MATH_CORRECT 0
#endif
#elif defined(__mc68000__) || defined(__pnacl__) || defined(__native_client__)
#define YYJSON_DOUBLE_MATH_CORRECT 0
#else
#define YYJSON_DOUBLE_MATH_CORRECT 1
#endif

#define YYJSON_BIG_ENDIAN 4321
#define YYJSON_LITTLE_ENDIAN 1234

#if yyjson_has_include(<sys / types.h>)
#include <sys/types.h>
#endif
#if yyjson_has_include(<endian.h>)
#include <endian.h>
#elif yyjson_has_include(<sys / endian.h>)
#include <sys/endian.h>
#elif yyjson_has_include(<machine / endian.h>)
#include <machine/endian.h>
#endif

#if defined(BYTE_ORDER) && BYTE_ORDER
#if defined(BIG_ENDIAN) && (BYTE_ORDER == BIG_ENDIAN)
#define YYJSON_ENDIAN YYJSON_BIG_ENDIAN
#elif defined(LITTLE_ENDIAN) && (BYTE_ORDER == LITTLE_ENDIAN)
#define YYJSON_ENDIAN YYJSON_LITTLE_ENDIAN
#endif
#elif defined(__BYTE_ORDER) && __BYTE_ORDER
#if defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN)
#define YYJSON_ENDIAN YYJSON_BIG_ENDIAN
#elif defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN)
#define YYJSON_ENDIAN YYJSON_LITTLE_ENDIAN
#endif
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__
#if defined(__ORDER_BIG_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define YYJSON_ENDIAN YYJSON_BIG_ENDIAN
#elif defined(__ORDER_LITTLE_ENDIAN__) && \
    (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define YYJSON_ENDIAN YYJSON_LITTLE_ENDIAN
#endif
#elif (defined(__LITTLE_ENDIAN__) && __LITTLE_ENDIAN__ == 1) ||            \
    defined(__i386) || defined(__i386__) || defined(_X86_) ||              \
    defined(__X86__) || defined(_M_IX86) || defined(__THW_INTEL__) ||      \
    defined(__x86_64) || defined(__x86_64__) || defined(__amd64) ||        \
    defined(__amd64__) || defined(_M_AMD64) || defined(_M_X64) ||          \
    defined(_M_ARM) || defined(_M_ARM64) || defined(__ARMEL__) ||          \
    defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(_MIPSEL) ||  \
    defined(__MIPSEL) || defined(__MIPSEL__) || defined(__EMSCRIPTEN__) || \
    defined(__wasm__) || defined(__loongarch__)
#define YYJSON_ENDIAN YYJSON_LITTLE_ENDIAN
#elif (defined(__BIG_ENDIAN__) && __BIG_ENDIAN__ == 1) ||                   \
    defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) || \
    defined(_MIPSEB) || defined(__MIPSEB) || defined(__MIPSEB__) ||         \
    defined(__or1k__) || defined(__OR1K__)
#define YYJSON_ENDIAN YYJSON_BIG_ENDIAN
#else
#define YYJSON_ENDIAN 0
#endif

#ifndef YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
#if defined(__ia64) || defined(_IA64) || defined(__IA64__) || \
    defined(__ia64__) || defined(_M_IA64) || defined(__itanium__)
#define YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS 1
#elif (defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) && \
    (defined(__GNUC__) || defined(__clang__)) &&                          \
    (!defined(__ARM_FEATURE_UNALIGNED) || !__ARM_FEATURE_UNALIGNED)
#define YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS 1
#elif defined(__sparc) || defined(__sparc__)
#define YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS 1
#elif defined(__mips) || defined(__mips__) || defined(__MIPS__)
#define YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS 1
#elif defined(__m68k__) || defined(M68000)
#define YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS 1
#else
#define YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS 0
#endif
#endif

#define YYJSON_READER_ESTIMATED_PRETTY_RATIO 16
#define YYJSON_READER_ESTIMATED_MINIFY_RATIO 6
#define YYJSON_WRITER_ESTIMATED_PRETTY_RATIO 32
#define YYJSON_WRITER_ESTIMATED_MINIFY_RATIO 18

#define YYJSON_MUT_DOC_STR_POOL_INIT_SIZE 0x100
#define YYJSON_MUT_DOC_STR_POOL_MAX_SIZE 0x10000000
#define YYJSON_MUT_DOC_VAL_POOL_INIT_SIZE (0x10 * sizeof(yyjson_mut_val))
#define YYJSON_MUT_DOC_VAL_POOL_MAX_SIZE (0x1000000 * sizeof(yyjson_mut_val))

#define YYJSON_ALC_DYN_MIN_SIZE 0x1000

#ifndef YYJSON_DISABLE_READER
#define YYJSON_DISABLE_READER 0
#endif
#ifndef YYJSON_DISABLE_WRITER
#define YYJSON_DISABLE_WRITER 0
#endif
#ifndef YYJSON_DISABLE_INCR_READER
#define YYJSON_DISABLE_INCR_READER 0
#endif
#ifndef YYJSON_DISABLE_UTILS
#define YYJSON_DISABLE_UTILS 0
#endif
#ifndef YYJSON_DISABLE_FAST_FP_CONV
#define YYJSON_DISABLE_FAST_FP_CONV 0
#endif
#ifndef YYJSON_DISABLE_NON_STANDARD
#define YYJSON_DISABLE_NON_STANDARD 0
#endif
#ifndef YYJSON_DISABLE_UTF8_VALIDATION
#define YYJSON_DISABLE_UTF8_VALIDATION 0
#endif
#ifndef YYJSON_READER_DEPTH_LIMIT
#define YYJSON_READER_DEPTH_LIMIT 0
#endif

#define repeat2(x) \
  { x x }
#define repeat4(x) \
  { x x x x }
#define repeat8(x) \
  { x x x x x x x x }
#define repeat16(x) \
  { x x x x x x x x x x x x x x x x }

#define repeat2_incr(x) \
  { x(0) x(1) }
#define repeat4_incr(x) \
  { x(0) x(1) x(2) x(3) }
#define repeat8_incr(x) \
  { x(0) x(1) x(2) x(3) x(4) x(5) x(6) x(7) }
#define repeat16_incr(x)                                                      \
  {                                                                           \
    x(0) x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11) x(12) x(13) \
        x(14) x(15)                                                           \
  }
#define repeat_in_1_18(x)                                                      \
  {                                                                            \
    x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11) x(12) x(13) x(14) \
        x(15) x(16) x(17) x(18)                                                \
  }

#undef likely
#define likely(x) yyjson_likely(x)
#undef unlikely
#define unlikely(x) yyjson_unlikely(x)

#undef static_inline
#define static_inline static yyjson_inline
#undef static_noinline
#define static_noinline static yyjson_noinline

#undef yyjson_min
#define yyjson_min(x, y) ((x) < (y) ? (x) : (y))
#undef yyjson_max
#define yyjson_max(x, y) ((x) > (y) ? (x) : (y))

#undef U64
#define U64(hi, lo) ((((u64)hi##UL) << 32U) + lo##UL)
#undef U32
#define U32(hi) ((u32)(hi##UL))

#define constcast yyjson_constcast

#if YYJSON_IS_REAL_GCC
#define gcc_load_barrier(val) __asm__ volatile("" ::"m"(val))
#define gcc_store_barrier(val) __asm__ volatile("" : "=m"(val))
#define gcc_full_barrier(val) __asm__ volatile("" : "=m"(val) : "m"(val))
#else
#define gcc_load_barrier(val)
#define gcc_store_barrier(val)
#define gcc_full_barrier(val)
#endif

#define MSG_FOPEN "failed to open file"
#define MSG_FREAD "failed to read file"
#define MSG_FWRITE "failed to write file"
#define MSG_FCLOSE "failed to close file"
#define MSG_MALLOC "failed to allocate memory"
#define MSG_CHAR_T "invalid literal, expected 'true'"
#define MSG_CHAR_F "invalid literal, expected 'false'"
#define MSG_CHAR_N "invalid literal, expected 'null'"
#define MSG_CHAR "unexpected character, expected a JSON value"
#define MSG_ARR_END "unexpected character, expected ',' or ']'"
#define MSG_OBJ_KEY "unexpected character, expected a string key"
#define MSG_OBJ_SEP "unexpected character, expected ':' after key"
#define MSG_OBJ_END "unexpected character, expected ',' or '}'"
#define MSG_GARBAGE "unexpected content after document"
#define MSG_NOT_END "unexpected end of data"
#define MSG_COMMENT "unclosed multiline comment"
#define MSG_COMMA "trailing comma is not allowed"
#define MSG_NAN_INF "nan or inf number is not allowed"
#define MSG_ERR_TYPE "invalid JSON value type"
#define MSG_ERR_BOM "UTF-8 byte order mark (BOM) is not supported"
#define MSG_ERR_UTF8 "invalid utf-8 encoding in string"
#define MSG_ERR_UTF16 "UTF-16 encoding is not supported"
#define MSG_ERR_UTF32 "UTF-32 encoding is not supported"
#define MSG_DEPTH "depth limit exceeded"

#undef U64_MAX
#define U64_MAX U64(0xFFFFFFFF, 0xFFFFFFFF)
#undef I64_MAX
#define I64_MAX U64(0x7FFFFFFF, 0xFFFFFFFF)
#undef USIZE_MAX
#define USIZE_MAX ((usize)(~(usize)0))

#undef U32_SAFE_DIG
#define U32_SAFE_DIG 9
#undef U64_SAFE_DIG
#define U64_SAFE_DIG 19
#undef USIZE_SAFE_DIG
#define USIZE_SAFE_DIG (sizeof(usize) == 8 ? U64_SAFE_DIG : U32_SAFE_DIG)

#define F64_BITS_INF U64(0x7FF00000, 0x00000000)

#if defined(__hppa__) || (defined(__mips__) && !defined(__mips_nan2008))
#define F64_BITS_NAN U64(0x7FF7FFFF, 0xFFFFFFFF)
#else
#define F64_BITS_NAN U64(0x7FF80000, 0x00000000)
#endif

#define F64_MAX_DEC_DIG 768

#define F64_MAX_DEC_EXP 308

#define F64_MIN_DEC_EXP (-324)

#define F64_MAX_BIN_EXP 1024

#define F64_MIN_BIN_EXP (-1021)

#define F32_BITS 32
#define F64_BITS 64

#define F32_EXP_BITS 8
#define F64_EXP_BITS 11

#define F32_SIG_BITS 23
#define F64_SIG_BITS 52

#define F32_SIG_FULL_BITS 24
#define F64_SIG_FULL_BITS 53

#define F32_SIG_MASK U32(0x007FFFFF)
#define F64_SIG_MASK U64(0x000FFFFF, 0xFFFFFFFF)

#define F32_EXP_MASK U32(0x7F800000)
#define F64_EXP_MASK U64(0x7FF00000, 0x00000000)

#define F32_EXP_BIAS 127
#define F64_EXP_BIAS 1023

#define F32_DEC_DIG 9
#define F64_DEC_DIG 17

#define FP_BUF_LEN 40

#define INCR_NUM_MAX_LEN 1024

typedef float f32;
typedef double f64;
typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef size_t usize;

#if YYJSON_HAS_INT128
__extension__ typedef __int128 i128;
__extension__ typedef unsigned __int128 u128;
#endif

typedef struct v16 {
  char c[2];
} v16;
typedef struct v32 {
  char c[4];
} v32;
typedef struct v64 {
  char c[8];
} v64;

typedef union v16_uni {
  v16 v;
  u16 u;
} v16_uni;
typedef union v32_uni {
  v32 v;
  u32 u;
} v32_uni;
typedef union v64_uni {
  v64 v;
  u64 u;
} v64_uni;

#define byte_move_idx(x) ((char *)dst)[x] = ((const char *)src)[x];
#define byte_move_src(x) ((char *)tmp)[x] = ((const char *)src)[x];
#define byte_move_dst(x) ((char *)dst)[x] = ((const char *)tmp)[x];

static_inline void byte_copy_2(void *dst, const void *src) {
#if !YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
  memcpy(dst, src, 2);
#else
  repeat2_incr(byte_move_idx)
#endif
}

static_inline void byte_copy_4(void *dst, const void *src) {
#if !YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
  memcpy(dst, src, 4);
#else
  repeat4_incr(byte_move_idx)
#endif
}

static_inline void byte_copy_8(void *dst, const void *src) {
#if !YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
  memcpy(dst, src, 8);
#else
  repeat8_incr(byte_move_idx)
#endif
}

static_inline void byte_copy_16(void *dst, const void *src) {
#if !YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
  memcpy(dst, src, 16);
#else
  repeat16_incr(byte_move_idx)
#endif
}

static_inline void byte_move_2(void *dst, const void *src) {
#if !YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
  u16 tmp;
  memcpy(&tmp, src, 2);
  memcpy(dst, &tmp, 2);
#else
  char tmp[2];
  repeat2_incr(byte_move_src) repeat2_incr(byte_move_dst)
#endif
}

static_inline void byte_move_4(void *dst, const void *src) {
#if !YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
  u32 tmp;
  memcpy(&tmp, src, 4);
  memcpy(dst, &tmp, 4);
#else
  char tmp[4];
  repeat4_incr(byte_move_src) repeat4_incr(byte_move_dst)
#endif
}

static_inline void byte_move_8(void *dst, const void *src) {
#if !YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
  u64 tmp;
  memcpy(&tmp, src, 8);
  memcpy(dst, &tmp, 8);
#else
  char tmp[8];
  repeat8_incr(byte_move_src) repeat8_incr(byte_move_dst)
#endif
}

static_inline void byte_move_16(void *dst, const void *src) {
#if !YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
  char *pdst = (char *)dst;
  const char *psrc = (const char *)src;
  u64 tmp1, tmp2;
  memcpy(&tmp1, psrc, 8);
  memcpy(&tmp2, psrc + 8, 8);
  memcpy(pdst, &tmp1, 8);
  memcpy(pdst + 8, &tmp2, 8);
#else
  char tmp[16];
  repeat16_incr(byte_move_src) repeat16_incr(byte_move_dst)
#endif
}

static_inline void byte_move_forward(void *dst, void *src, usize n) {
  char *d = (char *)dst, *s = (char *)src;
  n += (n % 2);
  if (n == 16) {
    byte_move_16(d, s);
    return;
  }
  if (n >= 8) {
    byte_move_8(d, s);
    n -= 8;
    d += 8;
    s += 8;
  }
  if (n >= 4) {
    byte_move_4(d, s);
    n -= 4;
    d += 4;
    s += 4;
  }
  if (n >= 2) {
    byte_move_2(d, s);
  }
}

static_inline bool byte_match_2(void *buf, const char *pat) {
#if !YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
  v16_uni u1, u2;
  memcpy(&u1, buf, 2);
  memcpy(&u2, pat, 2);
  return u1.u == u2.u;
#else
  return ((char *)buf)[0] == ((const char *)pat)[0] &&
         ((char *)buf)[1] == ((const char *)pat)[1];
#endif
}

static_inline bool byte_match_4(void *buf, const char *pat) {
#if !YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
  v32_uni u1, u2;
  memcpy(&u1, buf, 4);
  memcpy(&u2, pat, 4);
  return u1.u == u2.u;
#else
  return ((char *)buf)[0] == ((const char *)pat)[0] &&
         ((char *)buf)[1] == ((const char *)pat)[1] &&
         ((char *)buf)[2] == ((const char *)pat)[2] &&
         ((char *)buf)[3] == ((const char *)pat)[3];
#endif
}

static_inline u16 byte_load_2(const void *src) {
  v16_uni uni;
#if !YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
  memcpy(&uni, src, 2);
#else
  uni.v.c[0] = ((const char *)src)[0];
  uni.v.c[1] = ((const char *)src)[1];
#endif
  return uni.u;
}

static_inline u32 byte_load_3(const void *src) {
  v32_uni uni;
#if !YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
  memcpy(&uni, src, 2);
  uni.v.c[2] = ((const char *)src)[2];
  uni.v.c[3] = 0;
#else
  uni.v.c[0] = ((const char *)src)[0];
  uni.v.c[1] = ((const char *)src)[1];
  uni.v.c[2] = ((const char *)src)[2];
  uni.v.c[3] = 0;
#endif
  return uni.u;
}

static_inline u32 byte_load_4(const void *src) {
  v32_uni uni;
#if !YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
  memcpy(&uni, src, 4);
#else
  uni.v.c[0] = ((const char *)src)[0];
  uni.v.c[1] = ((const char *)src)[1];
  uni.v.c[2] = ((const char *)src)[2];
  uni.v.c[3] = ((const char *)src)[3];
#endif
  return uni.u;
}

#define CHAR_TYPE_ASCII (1 << 0)
#define CHAR_TYPE_ASCII_SQ (1 << 1)
#define CHAR_TYPE_SPACE (1 << 2)
#define CHAR_TYPE_SPACE_EXT (1 << 3)
#define CHAR_TYPE_NUM (1 << 4)
#define CHAR_TYPE_COMMENT (1 << 5)

#define CHAR_TYPE_EOL (1 << 0)
#define CHAR_TYPE_EOL_EXT (1 << 1)
#define CHAR_TYPE_ID_START (1 << 2)
#define CHAR_TYPE_ID_NEXT (1 << 3)
#define CHAR_TYPE_ID_ASCII (1 << 4)

#define CHAR_TYPE_SIGN (1 << 0)
#define CHAR_TYPE_DIGIT (1 << 1)
#define CHAR_TYPE_NONZERO (1 << 2)
#define CHAR_TYPE_EXP (1 << 3)
#define CHAR_TYPE_DOT (1 << 4)

static const u8 char_table1[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x08,
    0x08, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x03, 0x02, 0x03,
    0x03, 0x03, 0x03, 0x01, 0x03, 0x03, 0x03, 0x13, 0x03, 0x13, 0x13, 0x23,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};

static const u8 char_table2[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C,
    0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C,
    0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x00, 0x0C, 0x00, 0x00, 0x1C,
    0x00, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C,
    0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C,
    0x1C, 0x1C, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0E, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C};

static const u8 char_table3[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x10, 0x00,
    0x02, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};

static_inline bool char_is_space(u8 c) {
  return !!(char_table1[c] & CHAR_TYPE_SPACE);
}

static_inline bool char_is_space_ext(u8 c) {
  return !!(char_table1[c] & CHAR_TYPE_SPACE_EXT);
}

static_inline bool char_is_num(u8 c) {
  return !!(char_table1[c] & CHAR_TYPE_NUM);
}

static_inline bool char_is_ascii_skip(u8 c) {
  return !!(char_table1[c] & CHAR_TYPE_ASCII);
}

static_inline bool char_is_ascii_skip_sq(u8 c) {
  return !!(char_table1[c] & CHAR_TYPE_ASCII_SQ);
}

static_inline bool char_is_trivia(u8 c) {
  return !!(char_table1[c] & (CHAR_TYPE_SPACE_EXT | CHAR_TYPE_COMMENT));
}

static_inline bool char_is_eol(u8 c) {
  return !!(char_table2[c] & CHAR_TYPE_EOL);
}

static_inline bool char_is_eol_ext(u8 c) {
  return !!(char_table2[c] & CHAR_TYPE_EOL_EXT);
}

static_inline bool char_is_id_start(u8 c) {
  return !!(char_table2[c] & CHAR_TYPE_ID_START);
}

static_inline bool char_is_id_next(u8 c) {
  return !!(char_table2[c] & CHAR_TYPE_ID_NEXT);
}

static_inline bool char_is_id_ascii(u8 c) {
  return !!(char_table2[c] & CHAR_TYPE_ID_ASCII);
}

static_inline bool char_is_sign(u8 d) {
  return !!(char_table3[d] & CHAR_TYPE_SIGN);
}

static_inline bool char_is_nonzero(u8 d) {
  return !!(char_table3[d] & CHAR_TYPE_NONZERO);
}

static_inline bool char_is_digit(u8 d) {
  return !!(char_table3[d] & CHAR_TYPE_DIGIT);
}

static_inline bool char_is_exp(u8 d) {
  return !!(char_table3[d] & CHAR_TYPE_EXP);
}

static_inline bool char_is_fp(u8 d) {
  return !!(char_table3[d] & (CHAR_TYPE_DOT | CHAR_TYPE_EXP));
}

static_inline bool char_is_digit_or_fp(u8 d) {
  return !!(char_table3[d] & (CHAR_TYPE_DIGIT | CHAR_TYPE_DOT | CHAR_TYPE_EXP));
}

static_inline bool char_is_ctn(u8 c) { return (c & 0xDF) == 0x5B; }

static_inline u8 char_to_lower(u8 c) { return c | 0x20; }

static_inline bool is_utf8_bom(const u8 *cur) {
  return byte_load_3(cur) == byte_load_3("\xEF\xBB\xBF");
}

static_inline bool is_utf16_bom(const u8 *cur) {
  return byte_load_2(cur) == byte_load_2("\xFE\xFF") ||
         byte_load_2(cur) == byte_load_2("\xFF\xFE");
}

static_inline bool is_utf32_bom(const u8 *cur) {
  return byte_load_4(cur) == byte_load_4("\x00\x00\xFE\xFF") ||
         byte_load_4(cur) == byte_load_4("\xFF\xFE\x00\x00");
}

static_inline usize ext_eol_len(const u8 *cur) {
  if (cur[0] < 0x80) return 1;
  if (cur[1] == 0x80 && (cur[2] == 0xA8 || cur[2] == 0xA9)) return 3;
  return 0;
}

static_inline usize ext_space_len(const u8 *cur) {
  if (cur[0] < 0x80) {
    return 1;
  } else if (byte_load_2(cur) == byte_load_2("\xC2\xA0")) {
    return 2;
  } else if (byte_load_2(cur) == byte_load_2("\xE2\x80")) {
    if (cur[2] >= 0x80 && cur[2] <= 0x8A) return 3;
    if (cur[2] == 0xA8 || cur[2] == 0xA9 || cur[2] == 0xAF) return 3;
  } else {
    u32 uni = byte_load_3(cur);
    if (uni == byte_load_3("\xE1\x9A\x80") ||
        uni == byte_load_3("\xE2\x81\x9F") ||
        uni == byte_load_3("\xE3\x80\x80") ||
        uni == byte_load_3("\xEF\xBB\xBF"))
      return 3;
  }
  return 0;
}

static const u8 hex_conv_table[256] = {
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xF0, 0xF0, 0xF0, 0xF0};

static_inline bool hex_load_4(const u8 *src, u16 *dst) {
  u16 c0 = hex_conv_table[src[0]];
  u16 c1 = hex_conv_table[src[1]];
  u16 c2 = hex_conv_table[src[2]];
  u16 c3 = hex_conv_table[src[3]];
  u16 t0 = (u16)((c0 << 8) | c2);
  u16 t1 = (u16)((c1 << 8) | c3);
  *dst = (u16)((t0 << 4) | t1);
  return ((t0 | t1) & (u16)0xF0F0) == 0;
}

static_inline bool hex_load_2(const u8 *src, u8 *dst) {
  u8 c0 = hex_conv_table[src[0]];
  u8 c1 = hex_conv_table[src[1]];
  *dst = (u8)((c0 << 4) | c1);
  return ((c0 | c1) & 0xF0) == 0;
}

static_inline bool char_is_hex(u8 c) { return hex_conv_table[c] != 0xF0; }

#if YYJSON_ENDIAN == YYJSON_LITTLE_ENDIAN
#define utf8_seq_def(name, a, b, c, d) \
  static const u32 utf8_seq_##name = 0x##d##c##b##a##UL;
#define utf8_seq(name) utf8_seq_##name
#elif YYJSON_ENDIAN == YYJSON_BIG_ENDIAN
#define utf8_seq_def(name, a, b, c, d) \
  static const u32 utf8_seq_##name = 0x##a##b##c##d##UL;
#define utf8_seq(name) utf8_seq_##name
#else
#define utf8_seq_def(name, a, b, c, d) \
  static const v32_uni utf8_uni_##name = {{0x##a, 0x##b, 0x##c, 0x##d}};
#define utf8_seq(name) utf8_uni_##name.u
#endif

utf8_seq_def(b1_mask, 80, 00, 00, 00) utf8_seq_def(b1_patt, 00, 00, 00, 00)
#define is_utf8_seq1(uni) (((uni & utf8_seq(b1_mask)) == utf8_seq(b1_patt)))

    utf8_seq_def(b2_mask, E0, C0, 00, 00) utf8_seq_def(b2_patt, C0, 80, 00, 00)
        utf8_seq_def(b2_requ, 1E, 00, 00, 00)
#define is_utf8_seq2(uni)                              \
  (((uni & utf8_seq(b2_mask)) == utf8_seq(b2_patt)) && \
   ((uni & utf8_seq(b2_requ))))

            utf8_seq_def(b3_mask, F0, C0, C0, 00)
                utf8_seq_def(b3_patt, E0, 80, 80, 00) utf8_seq_def(b3_requ, 0F,
                                                                   20, 00, 00)
                    utf8_seq_def(b3_erro, 0D, 20, 00, 00)
#define is_utf8_seq3(uni)                              \
  (((uni & utf8_seq(b3_mask)) == utf8_seq(b3_patt)) && \
   ((tmp = (uni & utf8_seq(b3_requ)))) && ((tmp != utf8_seq(b3_erro))))

                        utf8_seq_def(b4_mask, F8, C0, C0, C0)
                            utf8_seq_def(b4_patt, F0, 80, 80, 80)
                                utf8_seq_def(b4_requ, 07, 30, 00, 00)
                                    utf8_seq_def(b4_req1, 04, 00, 00, 00)
                                        utf8_seq_def(b4_req2, 03, 30, 00, 00)
#define is_utf8_seq4(uni)                              \
  (((uni & utf8_seq(b4_mask)) == utf8_seq(b4_patt)) && \
   ((tmp = (uni & utf8_seq(b4_requ)))) &&              \
   ((tmp & utf8_seq(b4_req1)) == 0 || (tmp & utf8_seq(b4_req2)) == 0))

#if !YYJSON_DISABLE_FAST_FP_CONV

#define F64_POW10_MAX_EXACT_EXP 22

#if YYJSON_DOUBLE_MATH_CORRECT

                                            static const f64
    f64_pow10_table[F64_POW10_MAX_EXACT_EXP + 1] = {
        1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10, 1e11,
        1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};
#endif

#define U64_POW10_MAX_EXACT_EXP 19

static const u64 u64_pow10_table[U64_POW10_MAX_EXACT_EXP + 1] = {
    U64(0x00000000, 0x00000001), U64(0x00000000, 0x0000000A),
    U64(0x00000000, 0x00000064), U64(0x00000000, 0x000003E8),
    U64(0x00000000, 0x00002710), U64(0x00000000, 0x000186A0),
    U64(0x00000000, 0x000F4240), U64(0x00000000, 0x00989680),
    U64(0x00000000, 0x05F5E100), U64(0x00000000, 0x3B9ACA00),
    U64(0x00000002, 0x540BE400), U64(0x00000017, 0x4876E800),
    U64(0x000000E8, 0xD4A51000), U64(0x00000918, 0x4E72A000),
    U64(0x00005AF3, 0x107A4000), U64(0x00038D7E, 0xA4C68000),
    U64(0x002386F2, 0x6FC10000), U64(0x01634578, 0x5D8A0000),
    U64(0x0DE0B6B3, 0xA7640000), U64(0x8AC72304, 0x89E80000)};

#define POW10_SIG_TABLE_MIN_EXP -343

#define POW10_SIG_TABLE_MAX_EXP 324

#define POW10_SIG_TABLE_MIN_EXACT_EXP 0

#define POW10_SIG_TABLE_MAX_EXACT_EXP 55

static const u64 pow10_sig_table[] = {
    U64(0xBF29DCAB, 0xA82FDEAE), U64(0x7432EE87, 0x3880FC33),
    U64(0xEEF453D6, 0x923BD65A), U64(0x113FAA29, 0x06A13B3F),
    U64(0x9558B466, 0x1B6565F8), U64(0x4AC7CA59, 0xA424C507),
    U64(0xBAAEE17F, 0xA23EBF76), U64(0x5D79BCF0, 0x0D2DF649),
    U64(0xE95A99DF, 0x8ACE6F53), U64(0xF4D82C2C, 0x107973DC),
    U64(0x91D8A02B, 0xB6C10594), U64(0x79071B9B, 0x8A4BE869),
    U64(0xB64EC836, 0xA47146F9), U64(0x9748E282, 0x6CDEE284),
    U64(0xE3E27A44, 0x4D8D98B7), U64(0xFD1B1B23, 0x08169B25),
    U64(0x8E6D8C6A, 0xB0787F72), U64(0xFE30F0F5, 0xE50E20F7),
    U64(0xB208EF85, 0x5C969F4F), U64(0xBDBD2D33, 0x5E51A935),
    U64(0xDE8B2B66, 0xB3BC4723), U64(0xAD2C7880, 0x35E61382),
    U64(0x8B16FB20, 0x3055AC76), U64(0x4C3BCB50, 0x21AFCC31),
    U64(0xADDCB9E8, 0x3C6B1793), U64(0xDF4ABE24, 0x2A1BBF3D),
    U64(0xD953E862, 0x4B85DD78), U64(0xD71D6DAD, 0x34A2AF0D),
    U64(0x87D4713D, 0x6F33AA6B), U64(0x8672648C, 0x40E5AD68),
    U64(0xA9C98D8C, 0xCB009506), U64(0x680EFDAF, 0x511F18C2),
    U64(0xD43BF0EF, 0xFDC0BA48), U64(0x0212BD1B, 0x2566DEF2),
    U64(0x84A57695, 0xFE98746D), U64(0x014BB630, 0xF7604B57),
    U64(0xA5CED43B, 0x7E3E9188), U64(0x419EA3BD, 0x35385E2D),
    U64(0xCF42894A, 0x5DCE35EA), U64(0x52064CAC, 0x828675B9),
    U64(0x818995CE, 0x7AA0E1B2), U64(0x7343EFEB, 0xD1940993),
    U64(0xA1EBFB42, 0x19491A1F), U64(0x1014EBE6, 0xC5F90BF8),
    U64(0xCA66FA12, 0x9F9B60A6), U64(0xD41A26E0, 0x77774EF6),
    U64(0xFD00B897, 0x478238D0), U64(0x8920B098, 0x955522B4),
    U64(0x9E20735E, 0x8CB16382), U64(0x55B46E5F, 0x5D5535B0),
    U64(0xC5A89036, 0x2FDDBC62), U64(0xEB2189F7, 0x34AA831D),
    U64(0xF712B443, 0xBBD52B7B), U64(0xA5E9EC75, 0x01D523E4),
    U64(0x9A6BB0AA, 0x55653B2D), U64(0x47B233C9, 0x2125366E),
    U64(0xC1069CD4, 0xEABE89F8), U64(0x999EC0BB, 0x696E840A),
    U64(0xF148440A, 0x256E2C76), U64(0xC00670EA, 0x43CA250D),
    U64(0x96CD2A86, 0x5764DBCA), U64(0x38040692, 0x6A5E5728),
    U64(0xBC807527, 0xED3E12BC), U64(0xC6050837, 0x04F5ECF2),
    U64(0xEBA09271, 0xE88D976B), U64(0xF7864A44, 0xC633682E),
    U64(0x93445B87, 0x31587EA3), U64(0x7AB3EE6A, 0xFBE0211D),
    U64(0xB8157268, 0xFDAE9E4C), U64(0x5960EA05, 0xBAD82964),
    U64(0xE61ACF03, 0x3D1A45DF), U64(0x6FB92487, 0x298E33BD),
    U64(0x8FD0C162, 0x06306BAB), U64(0xA5D3B6D4, 0x79F8E056),
    U64(0xB3C4F1BA, 0x87BC8696), U64(0x8F48A489, 0x9877186C),
    U64(0xE0B62E29, 0x29ABA83C), U64(0x331ACDAB, 0xFE94DE87),
    U64(0x8C71DCD9, 0xBA0B4925), U64(0x9FF0C08B, 0x7F1D0B14),
    U64(0xAF8E5410, 0x288E1B6F), U64(0x07ECF0AE, 0x5EE44DD9),
    U64(0xDB71E914, 0x32B1A24A), U64(0xC9E82CD9, 0xF69D6150),
    U64(0x892731AC, 0x9FAF056E), U64(0xBE311C08, 0x3A225CD2),
    U64(0xAB70FE17, 0xC79AC6CA), U64(0x6DBD630A, 0x48AAF406),
    U64(0xD64D3D9D, 0xB981787D), U64(0x092CBBCC, 0xDAD5B108),
    U64(0x85F04682, 0x93F0EB4E), U64(0x25BBF560, 0x08C58EA5),
    U64(0xA76C5823, 0x38ED2621), U64(0xAF2AF2B8, 0x0AF6F24E),
    U64(0xD1476E2C, 0x07286FAA), U64(0x1AF5AF66, 0x0DB4AEE1),
    U64(0x82CCA4DB, 0x847945CA), U64(0x50D98D9F, 0xC890ED4D),
    U64(0xA37FCE12, 0x6597973C), U64(0xE50FF107, 0xBAB528A0),
    U64(0xCC5FC196, 0xFEFD7D0C), U64(0x1E53ED49, 0xA96272C8),
    U64(0xFF77B1FC, 0xBEBCDC4F), U64(0x25E8E89C, 0x13BB0F7A),
    U64(0x9FAACF3D, 0xF73609B1), U64(0x77B19161, 0x8C54E9AC),
    U64(0xC795830D, 0x75038C1D), U64(0xD59DF5B9, 0xEF6A2417),
    U64(0xF97AE3D0, 0xD2446F25), U64(0x4B057328, 0x6B44AD1D),
    U64(0x9BECCE62, 0x836AC577), U64(0x4EE367F9, 0x430AEC32),
    U64(0xC2E801FB, 0x244576D5), U64(0x229C41F7, 0x93CDA73F),
    U64(0xF3A20279, 0xED56D48A), U64(0x6B435275, 0x78C1110F),
    U64(0x9845418C, 0x345644D6), U64(0x830A1389, 0x6B78AAA9),
    U64(0xBE5691EF, 0x416BD60C), U64(0x23CC986B, 0xC656D553),
    U64(0xEDEC366B, 0x11C6CB8F), U64(0x2CBFBE86, 0xB7EC8AA8),
    U64(0x94B3A202, 0xEB1C3F39), U64(0x7BF7D714, 0x32F3D6A9),
    U64(0xB9E08A83, 0xA5E34F07), U64(0xDAF5CCD9, 0x3FB0CC53),
    U64(0xE858AD24, 0x8F5C22C9), U64(0xD1B3400F, 0x8F9CFF68),
    U64(0x91376C36, 0xD99995BE), U64(0x23100809, 0xB9C21FA1),
    U64(0xB5854744, 0x8FFFFB2D), U64(0xABD40A0C, 0x2832A78A),
    U64(0xE2E69915, 0xB3FFF9F9), U64(0x16C90C8F, 0x323F516C),
    U64(0x8DD01FAD, 0x907FFC3B), U64(0xAE3DA7D9, 0x7F6792E3),
    U64(0xB1442798, 0xF49FFB4A), U64(0x99CD11CF, 0xDF41779C),
    U64(0xDD95317F, 0x31C7FA1D), U64(0x40405643, 0xD711D583),
    U64(0x8A7D3EEF, 0x7F1CFC52), U64(0x482835EA, 0x666B2572),
    U64(0xAD1C8EAB, 0x5EE43B66), U64(0xDA324365, 0x0005EECF),
    U64(0xD863B256, 0x369D4A40), U64(0x90BED43E, 0x40076A82),
    U64(0x873E4F75, 0xE2224E68), U64(0x5A7744A6, 0xE804A291),
    U64(0xA90DE353, 0x5AAAE202), U64(0x711515D0, 0xA205CB36),
    U64(0xD3515C28, 0x31559A83), U64(0x0D5A5B44, 0xCA873E03),
    U64(0x8412D999, 0x1ED58091), U64(0xE858790A, 0xFE9486C2),
    U64(0xA5178FFF, 0x668AE0B6), U64(0x626E974D, 0xBE39A872),
    U64(0xCE5D73FF, 0x402D98E3), U64(0xFB0A3D21, 0x2DC8128F),
    U64(0x80FA687F, 0x881C7F8E), U64(0x7CE66634, 0xBC9D0B99),
    U64(0xA139029F, 0x6A239F72), U64(0x1C1FFFC1, 0xEBC44E80),
    U64(0xC9874347, 0x44AC874E), U64(0xA327FFB2, 0x66B56220),
    U64(0xFBE91419, 0x15D7A922), U64(0x4BF1FF9F, 0x0062BAA8),
    U64(0x9D71AC8F, 0xADA6C9B5), U64(0x6F773FC3, 0x603DB4A9),
    U64(0xC4CE17B3, 0x99107C22), U64(0xCB550FB4, 0x384D21D3),
    U64(0xF6019DA0, 0x7F549B2B), U64(0x7E2A53A1, 0x46606A48),
    U64(0x99C10284, 0x4F94E0FB), U64(0x2EDA7444, 0xCBFC426D),
    U64(0xC0314325, 0x637A1939), U64(0xFA911155, 0xFEFB5308),
    U64(0xF03D93EE, 0xBC589F88), U64(0x793555AB, 0x7EBA27CA),
    U64(0x96267C75, 0x35B763B5), U64(0x4BC1558B, 0x2F3458DE),
    U64(0xBBB01B92, 0x83253CA2), U64(0x9EB1AAED, 0xFB016F16),
    U64(0xEA9C2277, 0x23EE8BCB), U64(0x465E15A9, 0x79C1CADC),
    U64(0x92A1958A, 0x7675175F), U64(0x0BFACD89, 0xEC191EC9),
    U64(0xB749FAED, 0x14125D36), U64(0xCEF980EC, 0x671F667B),
    U64(0xE51C79A8, 0x5916F484), U64(0x82B7E127, 0x80E7401A),
    U64(0x8F31CC09, 0x37AE58D2), U64(0xD1B2ECB8, 0xB0908810),
    U64(0xB2FE3F0B, 0x8599EF07), U64(0x861FA7E6, 0xDCB4AA15),
    U64(0xDFBDCECE, 0x67006AC9), U64(0x67A791E0, 0x93E1D49A),
    U64(0x8BD6A141, 0x006042BD), U64(0xE0C8BB2C, 0x5C6D24E0),
    U64(0xAECC4991, 0x4078536D), U64(0x58FAE9F7, 0x73886E18),
    U64(0xDA7F5BF5, 0x90966848), U64(0xAF39A475, 0x506A899E),
    U64(0x888F9979, 0x7A5E012D), U64(0x6D8406C9, 0x52429603),
    U64(0xAAB37FD7, 0xD8F58178), U64(0xC8E5087B, 0xA6D33B83),
    U64(0xD5605FCD, 0xCF32E1D6), U64(0xFB1E4A9A, 0x90880A64),
    U64(0x855C3BE0, 0xA17FCD26), U64(0x5CF2EEA0, 0x9A55067F),
    U64(0xA6B34AD8, 0xC9DFC06F), U64(0xF42FAA48, 0xC0EA481E),
    U64(0xD0601D8E, 0xFC57B08B), U64(0xF13B94DA, 0xF124DA26),
    U64(0x823C1279, 0x5DB6CE57), U64(0x76C53D08, 0xD6B70858),
    U64(0xA2CB1717, 0xB52481ED), U64(0x54768C4B, 0x0C64CA6E),
    U64(0xCB7DDCDD, 0xA26DA268), U64(0xA9942F5D, 0xCF7DFD09),
    U64(0xFE5D5415, 0x0B090B02), U64(0xD3F93B35, 0x435D7C4C),
    U64(0x9EFA548D, 0x26E5A6E1), U64(0xC47BC501, 0x4A1A6DAF),
    U64(0xC6B8E9B0, 0x709F109A), U64(0x359AB641, 0x9CA1091B),
    U64(0xF867241C, 0x8CC6D4C0), U64(0xC30163D2, 0x03C94B62),
    U64(0x9B407691, 0xD7FC44F8), U64(0x79E0DE63, 0x425DCF1D),
    U64(0xC2109436, 0x4DFB5636), U64(0x985915FC, 0x12F542E4),
    U64(0xF294B943, 0xE17A2BC4), U64(0x3E6F5B7B, 0x17B2939D),
    U64(0x979CF3CA, 0x6CEC5B5A), U64(0xA705992C, 0xEECF9C42),
    U64(0xBD8430BD, 0x08277231), U64(0x50C6FF78, 0x2A838353),
    U64(0xECE53CEC, 0x4A314EBD), U64(0xA4F8BF56, 0x35246428),
    U64(0x940F4613, 0xAE5ED136), U64(0x871B7795, 0xE136BE99),
    U64(0xB9131798, 0x99F68584), U64(0x28E2557B, 0x59846E3F),
    U64(0xE757DD7E, 0xC07426E5), U64(0x331AEADA, 0x2FE589CF),
    U64(0x9096EA6F, 0x3848984F), U64(0x3FF0D2C8, 0x5DEF7621),
    U64(0xB4BCA50B, 0x065ABE63), U64(0x0FED077A, 0x756B53A9),
    U64(0xE1EBCE4D, 0xC7F16DFB), U64(0xD3E84959, 0x12C62894),
    U64(0x8D3360F0, 0x9CF6E4BD), U64(0x64712DD7, 0xABBBD95C),
    U64(0xB080392C, 0xC4349DEC), U64(0xBD8D794D, 0x96AACFB3),
    U64(0xDCA04777, 0xF541C567), U64(0xECF0D7A0, 0xFC5583A0),
    U64(0x89E42CAA, 0xF9491B60), U64(0xF41686C4, 0x9DB57244),
    U64(0xAC5D37D5, 0xB79B6239), U64(0x311C2875, 0xC522CED5),
    U64(0xD77485CB, 0x25823AC7), U64(0x7D633293, 0x366B828B),
    U64(0x86A8D39E, 0xF77164BC), U64(0xAE5DFF9C, 0x02033197),
    U64(0xA8530886, 0xB54DBDEB), U64(0xD9F57F83, 0x0283FDFC),
    U64(0xD267CAA8, 0x62A12D66), U64(0xD072DF63, 0xC324FD7B),
    U64(0x8380DEA9, 0x3DA4BC60), U64(0x4247CB9E, 0x59F71E6D),
    U64(0xA4611653, 0x8D0DEB78), U64(0x52D9BE85, 0xF074E608),
    U64(0xCD795BE8, 0x70516656), U64(0x67902E27, 0x6C921F8B),
    U64(0x806BD971, 0x4632DFF6), U64(0x00BA1CD8, 0xA3DB53B6),
    U64(0xA086CFCD, 0x97BF97F3), U64(0x80E8A40E, 0xCCD228A4),
    U64(0xC8A883C0, 0xFDAF7DF0), U64(0x6122CD12, 0x8006B2CD),
    U64(0xFAD2A4B1, 0x3D1B5D6C), U64(0x796B8057, 0x20085F81),
    U64(0x9CC3A6EE, 0xC6311A63), U64(0xCBE33036, 0x74053BB0),
    U64(0xC3F490AA, 0x77BD60FC), U64(0xBEDBFC44, 0x11068A9C),
    U64(0xF4F1B4D5, 0x15ACB93B), U64(0xEE92FB55, 0x15482D44),
    U64(0x99171105, 0x2D8BF3C5), U64(0x751BDD15, 0x2D4D1C4A),
    U64(0xBF5CD546, 0x78EEF0B6), U64(0xD262D45A, 0x78A0635D),
    U64(0xEF340A98, 0x172AACE4), U64(0x86FB8971, 0x16C87C34),
    U64(0x9580869F, 0x0E7AAC0E), U64(0xD45D35E6, 0xAE3D4DA0),
    U64(0xBAE0A846, 0xD2195712), U64(0x89748360, 0x59CCA109),
    U64(0xE998D258, 0x869FACD7), U64(0x2BD1A438, 0x703FC94B),
    U64(0x91FF8377, 0x5423CC06), U64(0x7B6306A3, 0x4627DDCF),
    U64(0xB67F6455, 0x292CBF08), U64(0x1A3BC84C, 0x17B1D542),
    U64(0xE41F3D6A, 0x7377EECA), U64(0x20CABA5F, 0x1D9E4A93),
    U64(0x8E938662, 0x882AF53E), U64(0x547EB47B, 0x7282EE9C),
    U64(0xB23867FB, 0x2A35B28D), U64(0xE99E619A, 0x4F23AA43),
    U64(0xDEC681F9, 0xF4C31F31), U64(0x6405FA00, 0xE2EC94D4),
    U64(0x8B3C113C, 0x38F9F37E), U64(0xDE83BC40, 0x8DD3DD04),
    U64(0xAE0B158B, 0x4738705E), U64(0x9624AB50, 0xB148D445),
    U64(0xD98DDAEE, 0x19068C76), U64(0x3BADD624, 0xDD9B0957),
    U64(0x87F8A8D4, 0xCFA417C9), U64(0xE54CA5D7, 0x0A80E5D6),
    U64(0xA9F6D30A, 0x038D1DBC), U64(0x5E9FCF4C, 0xCD211F4C),
    U64(0xD47487CC, 0x8470652B), U64(0x7647C320, 0x0069671F),
    U64(0x84C8D4DF, 0xD2C63F3B), U64(0x29ECD9F4, 0x0041E073),
    U64(0xA5FB0A17, 0xC777CF09), U64(0xF4681071, 0x00525890),
    U64(0xCF79CC9D, 0xB955C2CC), U64(0x7182148D, 0x4066EEB4),
    U64(0x81AC1FE2, 0x93D599BF), U64(0xC6F14CD8, 0x48405530),
    U64(0xA21727DB, 0x38CB002F), U64(0xB8ADA00E, 0x5A506A7C),
    U64(0xCA9CF1D2, 0x06FDC03B), U64(0xA6D90811, 0xF0E4851C),
    U64(0xFD442E46, 0x88BD304A), U64(0x908F4A16, 0x6D1DA663),
    U64(0x9E4A9CEC, 0x15763E2E), U64(0x9A598E4E, 0x043287FE),
    U64(0xC5DD4427, 0x1AD3CDBA), U64(0x40EFF1E1, 0x853F29FD),
    U64(0xF7549530, 0xE188C128), U64(0xD12BEE59, 0xE68EF47C),
    U64(0x9A94DD3E, 0x8CF578B9), U64(0x82BB74F8, 0x301958CE),
    U64(0xC13A148E, 0x3032D6E7), U64(0xE36A5236, 0x3C1FAF01),
    U64(0xF18899B1, 0xBC3F8CA1), U64(0xDC44E6C3, 0xCB279AC1),
    U64(0x96F5600F, 0x15A7B7E5), U64(0x29AB103A, 0x5EF8C0B9),
    U64(0xBCB2B812, 0xDB11A5DE), U64(0x7415D448, 0xF6B6F0E7),
    U64(0xEBDF6617, 0x91D60F56), U64(0x111B495B, 0x3464AD21),
    U64(0x936B9FCE, 0xBB25C995), U64(0xCAB10DD9, 0x00BEEC34),
    U64(0xB84687C2, 0x69EF3BFB), U64(0x3D5D514F, 0x40EEA742),
    U64(0xE65829B3, 0x046B0AFA), U64(0x0CB4A5A3, 0x112A5112),
    U64(0x8FF71A0F, 0xE2C2E6DC), U64(0x47F0E785, 0xEABA72AB),
    U64(0xB3F4E093, 0xDB73A093), U64(0x59ED2167, 0x65690F56),
    U64(0xE0F218B8, 0xD25088B8), U64(0x306869C1, 0x3EC3532C),
    U64(0x8C974F73, 0x83725573), U64(0x1E414218, 0xC73A13FB),
    U64(0xAFBD2350, 0x644EEACF), U64(0xE5D1929E, 0xF90898FA),
    U64(0xDBAC6C24, 0x7D62A583), U64(0xDF45F746, 0xB74ABF39),
    U64(0x894BC396, 0xCE5DA772), U64(0x6B8BBA8C, 0x328EB783),
    U64(0xAB9EB47C, 0x81F5114F), U64(0x066EA92F, 0x3F326564),
    U64(0xD686619B, 0xA27255A2), U64(0xC80A537B, 0x0EFEFEBD),
    U64(0x8613FD01, 0x45877585), U64(0xBD06742C, 0xE95F5F36),
    U64(0xA798FC41, 0x96E952E7), U64(0x2C481138, 0x23B73704),
    U64(0xD17F3B51, 0xFCA3A7A0), U64(0xF75A1586, 0x2CA504C5),
    U64(0x82EF8513, 0x3DE648C4), U64(0x9A984D73, 0xDBE722FB),
    U64(0xA3AB6658, 0x0D5FDAF5), U64(0xC13E60D0, 0xD2E0EBBA),
    U64(0xCC963FEE, 0x10B7D1B3), U64(0x318DF905, 0x079926A8),
    U64(0xFFBBCFE9, 0x94E5C61F), U64(0xFDF17746, 0x497F7052),
    U64(0x9FD561F1, 0xFD0F9BD3), U64(0xFEB6EA8B, 0xEDEFA633),
    U64(0xC7CABA6E, 0x7C5382C8), U64(0xFE64A52E, 0xE96B8FC0),
    U64(0xF9BD690A, 0x1B68637B), U64(0x3DFDCE7A, 0xA3C673B0),
    U64(0x9C1661A6, 0x51213E2D), U64(0x06BEA10C, 0xA65C084E),
    U64(0xC31BFA0F, 0xE5698DB8), U64(0x486E494F, 0xCFF30A62),
    U64(0xF3E2F893, 0xDEC3F126), U64(0x5A89DBA3, 0xC3EFCCFA),
    U64(0x986DDB5C, 0x6B3A76B7), U64(0xF8962946, 0x5A75E01C),
    U64(0xBE895233, 0x86091465), U64(0xF6BBB397, 0xF1135823),
    U64(0xEE2BA6C0, 0x678B597F), U64(0x746AA07D, 0xED582E2C),
    U64(0x94DB4838, 0x40B717EF), U64(0xA8C2A44E, 0xB4571CDC),
    U64(0xBA121A46, 0x50E4DDEB), U64(0x92F34D62, 0x616CE413),
    U64(0xE896A0D7, 0xE51E1566), U64(0x77B020BA, 0xF9C81D17),
    U64(0x915E2486, 0xEF32CD60), U64(0x0ACE1474, 0xDC1D122E),
    U64(0xB5B5ADA8, 0xAAFF80B8), U64(0x0D819992, 0x132456BA),
    U64(0xE3231912, 0xD5BF60E6), U64(0x10E1FFF6, 0x97ED6C69),
    U64(0x8DF5EFAB, 0xC5979C8F), U64(0xCA8D3FFA, 0x1EF463C1),
    U64(0xB1736B96, 0xB6FD83B3), U64(0xBD308FF8, 0xA6B17CB2),
    U64(0xDDD0467C, 0x64BCE4A0), U64(0xAC7CB3F6, 0xD05DDBDE),
    U64(0x8AA22C0D, 0xBEF60EE4), U64(0x6BCDF07A, 0x423AA96B),
    U64(0xAD4AB711, 0x2EB3929D), U64(0x86C16C98, 0xD2C953C6),
    U64(0xD89D64D5, 0x7A607744), U64(0xE871C7BF, 0x077BA8B7),
    U64(0x87625F05, 0x6C7C4A8B), U64(0x11471CD7, 0x64AD4972),
    U64(0xA93AF6C6, 0xC79B5D2D), U64(0xD598E40D, 0x3DD89BCF),
    U64(0xD389B478, 0x79823479), U64(0x4AFF1D10, 0x8D4EC2C3),
    U64(0x843610CB, 0x4BF160CB), U64(0xCEDF722A, 0x585139BA),
    U64(0xA54394FE, 0x1EEDB8FE), U64(0xC2974EB4, 0xEE658828),
    U64(0xCE947A3D, 0xA6A9273E), U64(0x733D2262, 0x29FEEA32),
    U64(0x811CCC66, 0x8829B887), U64(0x0806357D, 0x5A3F525F),
    U64(0xA163FF80, 0x2A3426A8), U64(0xCA07C2DC, 0xB0CF26F7),
    U64(0xC9BCFF60, 0x34C13052), U64(0xFC89B393, 0xDD02F0B5),
    U64(0xFC2C3F38, 0x41F17C67), U64(0xBBAC2078, 0xD443ACE2),
    U64(0x9D9BA783, 0x2936EDC0), U64(0xD54B944B, 0x84AA4C0D),
    U64(0xC5029163, 0xF384A931), U64(0x0A9E795E, 0x65D4DF11),
    U64(0xF64335BC, 0xF065D37D), U64(0x4D4617B5, 0xFF4A16D5),
    U64(0x99EA0196, 0x163FA42E), U64(0x504BCED1, 0xBF8E4E45),
    U64(0xC06481FB, 0x9BCF8D39), U64(0xE45EC286, 0x2F71E1D6),
    U64(0xF07DA27A, 0x82C37088), U64(0x5D767327, 0xBB4E5A4C),
    U64(0x964E858C, 0x91BA2655), U64(0x3A6A07F8, 0xD510F86F),
    U64(0xBBE226EF, 0xB628AFEA), U64(0x890489F7, 0x0A55368B),
    U64(0xEADAB0AB, 0xA3B2DBE5), U64(0x2B45AC74, 0xCCEA842E),
    U64(0x92C8AE6B, 0x464FC96F), U64(0x3B0B8BC9, 0x0012929D),
    U64(0xB77ADA06, 0x17E3BBCB), U64(0x09CE6EBB, 0x40173744),
    U64(0xE5599087, 0x9DDCAABD), U64(0xCC420A6A, 0x101D0515),
    U64(0x8F57FA54, 0xC2A9EAB6), U64(0x9FA94682, 0x4A12232D),
    U64(0xB32DF8E9, 0xF3546564), U64(0x47939822, 0xDC96ABF9),
    U64(0xDFF97724, 0x70297EBD), U64(0x59787E2B, 0x93BC56F7),
    U64(0x8BFBEA76, 0xC619EF36), U64(0x57EB4EDB, 0x3C55B65A),
    U64(0xAEFAE514, 0x77A06B03), U64(0xEDE62292, 0x0B6B23F1),
    U64(0xDAB99E59, 0x958885C4), U64(0xE95FAB36, 0x8E45ECED),
    U64(0x88B402F7, 0xFD75539B), U64(0x11DBCB02, 0x18EBB414),
    U64(0xAAE103B5, 0xFCD2A881), U64(0xD652BDC2, 0x9F26A119),
    U64(0xD59944A3, 0x7C0752A2), U64(0x4BE76D33, 0x46F0495F),
    U64(0x857FCAE6, 0x2D8493A5), U64(0x6F70A440, 0x0C562DDB),
    U64(0xA6DFBD9F, 0xB8E5B88E), U64(0xCB4CCD50, 0x0F6BB952),
    U64(0xD097AD07, 0xA71F26B2), U64(0x7E2000A4, 0x1346A7A7),
    U64(0x825ECC24, 0xC873782F), U64(0x8ED40066, 0x8C0C28C8),
    U64(0xA2F67F2D, 0xFA90563B), U64(0x72890080, 0x2F0F32FA),
    U64(0xCBB41EF9, 0x79346BCA), U64(0x4F2B40A0, 0x3AD2FFB9),
    U64(0xFEA126B7, 0xD78186BC), U64(0xE2F610C8, 0x4987BFA8),
    U64(0x9F24B832, 0xE6B0F436), U64(0x0DD9CA7D, 0x2DF4D7C9),
    U64(0xC6EDE63F, 0xA05D3143), U64(0x91503D1C, 0x79720DBB),
    U64(0xF8A95FCF, 0x88747D94), U64(0x75A44C63, 0x97CE912A),
    U64(0x9B69DBE1, 0xB548CE7C), U64(0xC986AFBE, 0x3EE11ABA),
    U64(0xC24452DA, 0x229B021B), U64(0xFBE85BAD, 0xCE996168),
    U64(0xF2D56790, 0xAB41C2A2), U64(0xFAE27299, 0x423FB9C3),
    U64(0x97C560BA, 0x6B0919A5), U64(0xDCCD879F, 0xC967D41A),
    U64(0xBDB6B8E9, 0x05CB600F), U64(0x5400E987, 0xBBC1C920),
    U64(0xED246723, 0x473E3813), U64(0x290123E9, 0xAAB23B68),
    U64(0x9436C076, 0x0C86E30B), U64(0xF9A0B672, 0x0AAF6521),
    U64(0xB9447093, 0x8FA89BCE), U64(0xF808E40E, 0x8D5B3E69),
    U64(0xE7958CB8, 0x7392C2C2), U64(0xB60B1D12, 0x30B20E04),
    U64(0x90BD77F3, 0x483BB9B9), U64(0xB1C6F22B, 0x5E6F48C2),
    U64(0xB4ECD5F0, 0x1A4AA828), U64(0x1E38AEB6, 0x360B1AF3),
    U64(0xE2280B6C, 0x20DD5232), U64(0x25C6DA63, 0xC38DE1B0),
    U64(0x8D590723, 0x948A535F), U64(0x579C487E, 0x5A38AD0E),
    U64(0xB0AF48EC, 0x79ACE837), U64(0x2D835A9D, 0xF0C6D851),
    U64(0xDCDB1B27, 0x98182244), U64(0xF8E43145, 0x6CF88E65),
    U64(0x8A08F0F8, 0xBF0F156B), U64(0x1B8E9ECB, 0x641B58FF),
    U64(0xAC8B2D36, 0xEED2DAC5), U64(0xE272467E, 0x3D222F3F),
    U64(0xD7ADF884, 0xAA879177), U64(0x5B0ED81D, 0xCC6ABB0F),
    U64(0x86CCBB52, 0xEA94BAEA), U64(0x98E94712, 0x9FC2B4E9),
    U64(0xA87FEA27, 0xA539E9A5), U64(0x3F2398D7, 0x47B36224),
    U64(0xD29FE4B1, 0x8E88640E), U64(0x8EEC7F0D, 0x19A03AAD),
    U64(0x83A3EEEE, 0xF9153E89), U64(0x1953CF68, 0x300424AC),
    U64(0xA48CEAAA, 0xB75A8E2B), U64(0x5FA8C342, 0x3C052DD7),
    U64(0xCDB02555, 0x653131B6), U64(0x3792F412, 0xCB06794D),
    U64(0x808E1755, 0x5F3EBF11), U64(0xE2BBD88B, 0xBEE40BD0),
    U64(0xA0B19D2A, 0xB70E6ED6), U64(0x5B6ACEAE, 0xAE9D0EC4),
    U64(0xC8DE0475, 0x64D20A8B), U64(0xF245825A, 0x5A445275),
    U64(0xFB158592, 0xBE068D2E), U64(0xEED6E2F0, 0xF0D56712),
    U64(0x9CED737B, 0xB6C4183D), U64(0x55464DD6, 0x9685606B),
    U64(0xC428D05A, 0xA4751E4C), U64(0xAA97E14C, 0x3C26B886),
    U64(0xF5330471, 0x4D9265DF), U64(0xD53DD99F, 0x4B3066A8),
    U64(0x993FE2C6, 0xD07B7FAB), U64(0xE546A803, 0x8EFE4029),
    U64(0xBF8FDB78, 0x849A5F96), U64(0xDE985204, 0x72BDD033),
    U64(0xEF73D256, 0xA5C0F77C), U64(0x963E6685, 0x8F6D4440),
    U64(0x95A86376, 0x27989AAD), U64(0xDDE70013, 0x79A44AA8),
    U64(0xBB127C53, 0xB17EC159), U64(0x5560C018, 0x580D5D52),
    U64(0xE9D71B68, 0x9DDE71AF), U64(0xAAB8F01E, 0x6E10B4A6),
    U64(0x92267121, 0x62AB070D), U64(0xCAB39613, 0x04CA70E8),
    U64(0xB6B00D69, 0xBB55C8D1), U64(0x3D607B97, 0xC5FD0D22),
    U64(0xE45C10C4, 0x2A2B3B05), U64(0x8CB89A7D, 0xB77C506A),
    U64(0x8EB98A7A, 0x9A5B04E3), U64(0x77F3608E, 0x92ADB242),
    U64(0xB267ED19, 0x40F1C61C), U64(0x55F038B2, 0x37591ED3),
    U64(0xDF01E85F, 0x912E37A3), U64(0x6B6C46DE, 0xC52F6688),
    U64(0x8B61313B, 0xBABCE2C6), U64(0x2323AC4B, 0x3B3DA015),
    U64(0xAE397D8A, 0xA96C1B77), U64(0xABEC975E, 0x0A0D081A),
    U64(0xD9C7DCED, 0x53C72255), U64(0x96E7BD35, 0x8C904A21),
    U64(0x881CEA14, 0x545C7575), U64(0x7E50D641, 0x77DA2E54),
    U64(0xAA242499, 0x697392D2), U64(0xDDE50BD1, 0xD5D0B9E9),
    U64(0xD4AD2DBF, 0xC3D07787), U64(0x955E4EC6, 0x4B44E864),
    U64(0x84EC3C97, 0xDA624AB4), U64(0xBD5AF13B, 0xEF0B113E),
    U64(0xA6274BBD, 0xD0FADD61), U64(0xECB1AD8A, 0xEACDD58E),
    U64(0xCFB11EAD, 0x453994BA), U64(0x67DE18ED, 0xA5814AF2),
    U64(0x81CEB32C, 0x4B43FCF4), U64(0x80EACF94, 0x8770CED7),
    U64(0xA2425FF7, 0x5E14FC31), U64(0xA1258379, 0xA94D028D),
    U64(0xCAD2F7F5, 0x359A3B3E), U64(0x096EE458, 0x13A04330),
    U64(0xFD87B5F2, 0x8300CA0D), U64(0x8BCA9D6E, 0x188853FC),
    U64(0x9E74D1B7, 0x91E07E48), U64(0x775EA264, 0xCF55347D),
    U64(0xC6120625, 0x76589DDA), U64(0x95364AFE, 0x032A819D),
    U64(0xF79687AE, 0xD3EEC551), U64(0x3A83DDBD, 0x83F52204),
    U64(0x9ABE14CD, 0x44753B52), U64(0xC4926A96, 0x72793542),
    U64(0xC16D9A00, 0x95928A27), U64(0x75B7053C, 0x0F178293),
    U64(0xF1C90080, 0xBAF72CB1), U64(0x5324C68B, 0x12DD6338),
    U64(0x971DA050, 0x74DA7BEE), U64(0xD3F6FC16, 0xEBCA5E03),
    U64(0xBCE50864, 0x92111AEA), U64(0x88F4BB1C, 0xA6BCF584),
    U64(0xEC1E4A7D, 0xB69561A5), U64(0x2B31E9E3, 0xD06C32E5),
    U64(0x9392EE8E, 0x921D5D07), U64(0x3AFF322E, 0x62439FCF),
    U64(0xB877AA32, 0x36A4B449), U64(0x09BEFEB9, 0xFAD487C2),
    U64(0xE69594BE, 0xC44DE15B), U64(0x4C2EBE68, 0x7989A9B3),
    U64(0x901D7CF7, 0x3AB0ACD9), U64(0x0F9D3701, 0x4BF60A10),
    U64(0xB424DC35, 0x095CD80F), U64(0x538484C1, 0x9EF38C94),
    U64(0xE12E1342, 0x4BB40E13), U64(0x2865A5F2, 0x06B06FB9),
    U64(0x8CBCCC09, 0x6F5088CB), U64(0xF93F87B7, 0x442E45D3),
    U64(0xAFEBFF0B, 0xCB24AAFE), U64(0xF78F69A5, 0x1539D748),
    U64(0xDBE6FECE, 0xBDEDD5BE), U64(0xB573440E, 0x5A884D1B),
    U64(0x89705F41, 0x36B4A597), U64(0x31680A88, 0xF8953030),
    U64(0xABCC7711, 0x8461CEFC), U64(0xFDC20D2B, 0x36BA7C3D),
    U64(0xD6BF94D5, 0xE57A42BC), U64(0x3D329076, 0x04691B4C),
    U64(0x8637BD05, 0xAF6C69B5), U64(0xA63F9A49, 0xC2C1B10F),
    U64(0xA7C5AC47, 0x1B478423), U64(0x0FCF80DC, 0x33721D53),
    U64(0xD1B71758, 0xE219652B), U64(0xD3C36113, 0x404EA4A8),
    U64(0x83126E97, 0x8D4FDF3B), U64(0x645A1CAC, 0x083126E9),
    U64(0xA3D70A3D, 0x70A3D70A), U64(0x3D70A3D7, 0x0A3D70A3),
    U64(0xCCCCCCCC, 0xCCCCCCCC), U64(0xCCCCCCCC, 0xCCCCCCCC),
    U64(0x80000000, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0xA0000000, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0xC8000000, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0xFA000000, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0x9C400000, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0xC3500000, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0xF4240000, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0x98968000, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0xBEBC2000, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0xEE6B2800, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0x9502F900, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0xBA43B740, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0xE8D4A510, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0x9184E72A, 0x00000000), U64(0x00000000, 0x00000000),
    U64(0xB5E620F4, 0x80000000), U64(0x00000000, 0x00000000),
    U64(0xE35FA931, 0xA0000000), U64(0x00000000, 0x00000000),
    U64(0x8E1BC9BF, 0x04000000), U64(0x00000000, 0x00000000),
    U64(0xB1A2BC2E, 0xC5000000), U64(0x00000000, 0x00000000),
    U64(0xDE0B6B3A, 0x76400000), U64(0x00000000, 0x00000000),
    U64(0x8AC72304, 0x89E80000), U64(0x00000000, 0x00000000),
    U64(0xAD78EBC5, 0xAC620000), U64(0x00000000, 0x00000000),
    U64(0xD8D726B7, 0x177A8000), U64(0x00000000, 0x00000000),
    U64(0x87867832, 0x6EAC9000), U64(0x00000000, 0x00000000),
    U64(0xA968163F, 0x0A57B400), U64(0x00000000, 0x00000000),
    U64(0xD3C21BCE, 0xCCEDA100), U64(0x00000000, 0x00000000),
    U64(0x84595161, 0x401484A0), U64(0x00000000, 0x00000000),
    U64(0xA56FA5B9, 0x9019A5C8), U64(0x00000000, 0x00000000),
    U64(0xCECB8F27, 0xF4200F3A), U64(0x00000000, 0x00000000),
    U64(0x813F3978, 0xF8940984), U64(0x40000000, 0x00000000),
    U64(0xA18F07D7, 0x36B90BE5), U64(0x50000000, 0x00000000),
    U64(0xC9F2C9CD, 0x04674EDE), U64(0xA4000000, 0x00000000),
    U64(0xFC6F7C40, 0x45812296), U64(0x4D000000, 0x00000000),
    U64(0x9DC5ADA8, 0x2B70B59D), U64(0xF0200000, 0x00000000),
    U64(0xC5371912, 0x364CE305), U64(0x6C280000, 0x00000000),
    U64(0xF684DF56, 0xC3E01BC6), U64(0xC7320000, 0x00000000),
    U64(0x9A130B96, 0x3A6C115C), U64(0x3C7F4000, 0x00000000),
    U64(0xC097CE7B, 0xC90715B3), U64(0x4B9F1000, 0x00000000),
    U64(0xF0BDC21A, 0xBB48DB20), U64(0x1E86D400, 0x00000000),
    U64(0x96769950, 0xB50D88F4), U64(0x13144480, 0x00000000),
    U64(0xBC143FA4, 0xE250EB31), U64(0x17D955A0, 0x00000000),
    U64(0xEB194F8E, 0x1AE525FD), U64(0x5DCFAB08, 0x00000000),
    U64(0x92EFD1B8, 0xD0CF37BE), U64(0x5AA1CAE5, 0x00000000),
    U64(0xB7ABC627, 0x050305AD), U64(0xF14A3D9E, 0x40000000),
    U64(0xE596B7B0, 0xC643C719), U64(0x6D9CCD05, 0xD0000000),
    U64(0x8F7E32CE, 0x7BEA5C6F), U64(0xE4820023, 0xA2000000),
    U64(0xB35DBF82, 0x1AE4F38B), U64(0xDDA2802C, 0x8A800000),
    U64(0xE0352F62, 0xA19E306E), U64(0xD50B2037, 0xAD200000),
    U64(0x8C213D9D, 0xA502DE45), U64(0x4526F422, 0xCC340000),
    U64(0xAF298D05, 0x0E4395D6), U64(0x9670B12B, 0x7F410000),
    U64(0xDAF3F046, 0x51D47B4C), U64(0x3C0CDD76, 0x5F114000),
    U64(0x88D8762B, 0xF324CD0F), U64(0xA5880A69, 0xFB6AC800),
    U64(0xAB0E93B6, 0xEFEE0053), U64(0x8EEA0D04, 0x7A457A00),
    U64(0xD5D238A4, 0xABE98068), U64(0x72A49045, 0x98D6D880),
    U64(0x85A36366, 0xEB71F041), U64(0x47A6DA2B, 0x7F864750),
    U64(0xA70C3C40, 0xA64E6C51), U64(0x999090B6, 0x5F67D924),
    U64(0xD0CF4B50, 0xCFE20765), U64(0xFFF4B4E3, 0xF741CF6D),
    U64(0x82818F12, 0x81ED449F), U64(0xBFF8F10E, 0x7A8921A4),
    U64(0xA321F2D7, 0x226895C7), U64(0xAFF72D52, 0x192B6A0D),
    U64(0xCBEA6F8C, 0xEB02BB39), U64(0x9BF4F8A6, 0x9F764490),
    U64(0xFEE50B70, 0x25C36A08), U64(0x02F236D0, 0x4753D5B4),
    U64(0x9F4F2726, 0x179A2245), U64(0x01D76242, 0x2C946590),
    U64(0xC722F0EF, 0x9D80AAD6), U64(0x424D3AD2, 0xB7B97EF5),
    U64(0xF8EBAD2B, 0x84E0D58B), U64(0xD2E08987, 0x65A7DEB2),
    U64(0x9B934C3B, 0x330C8577), U64(0x63CC55F4, 0x9F88EB2F),
    U64(0xC2781F49, 0xFFCFA6D5), U64(0x3CBF6B71, 0xC76B25FB),
    U64(0xF316271C, 0x7FC3908A), U64(0x8BEF464E, 0x3945EF7A),
    U64(0x97EDD871, 0xCFDA3A56), U64(0x97758BF0, 0xE3CBB5AC),
    U64(0xBDE94E8E, 0x43D0C8EC), U64(0x3D52EEED, 0x1CBEA317),
    U64(0xED63A231, 0xD4C4FB27), U64(0x4CA7AAA8, 0x63EE4BDD),
    U64(0x945E455F, 0x24FB1CF8), U64(0x8FE8CAA9, 0x3E74EF6A),
    U64(0xB975D6B6, 0xEE39E436), U64(0xB3E2FD53, 0x8E122B44),
    U64(0xE7D34C64, 0xA9C85D44), U64(0x60DBBCA8, 0x7196B616),
    U64(0x90E40FBE, 0xEA1D3A4A), U64(0xBC8955E9, 0x46FE31CD),
    U64(0xB51D13AE, 0xA4A488DD), U64(0x6BABAB63, 0x98BDBE41),
    U64(0xE264589A, 0x4DCDAB14), U64(0xC696963C, 0x7EED2DD1),
    U64(0x8D7EB760, 0x70A08AEC), U64(0xFC1E1DE5, 0xCF543CA2),
    U64(0xB0DE6538, 0x8CC8ADA8), U64(0x3B25A55F, 0x43294BCB),
    U64(0xDD15FE86, 0xAFFAD912), U64(0x49EF0EB7, 0x13F39EBE),
    U64(0x8A2DBF14, 0x2DFCC7AB), U64(0x6E356932, 0x6C784337),
    U64(0xACB92ED9, 0x397BF996), U64(0x49C2C37F, 0x07965404),
    U64(0xD7E77A8F, 0x87DAF7FB), U64(0xDC33745E, 0xC97BE906),
    U64(0x86F0AC99, 0xB4E8DAFD), U64(0x69A028BB, 0x3DED71A3),
    U64(0xA8ACD7C0, 0x222311BC), U64(0xC40832EA, 0x0D68CE0C),
    U64(0xD2D80DB0, 0x2AABD62B), U64(0xF50A3FA4, 0x90C30190),
    U64(0x83C7088E, 0x1AAB65DB), U64(0x792667C6, 0xDA79E0FA),
    U64(0xA4B8CAB1, 0xA1563F52), U64(0x577001B8, 0x91185938),
    U64(0xCDE6FD5E, 0x09ABCF26), U64(0xED4C0226, 0xB55E6F86),
    U64(0x80B05E5A, 0xC60B6178), U64(0x544F8158, 0x315B05B4),
    U64(0xA0DC75F1, 0x778E39D6), U64(0x696361AE, 0x3DB1C721),
    U64(0xC913936D, 0xD571C84C), U64(0x03BC3A19, 0xCD1E38E9),
    U64(0xFB587849, 0x4ACE3A5F), U64(0x04AB48A0, 0x4065C723),
    U64(0x9D174B2D, 0xCEC0E47B), U64(0x62EB0D64, 0x283F9C76),
    U64(0xC45D1DF9, 0x42711D9A), U64(0x3BA5D0BD, 0x324F8394),
    U64(0xF5746577, 0x930D6500), U64(0xCA8F44EC, 0x7EE36479),
    U64(0x9968BF6A, 0xBBE85F20), U64(0x7E998B13, 0xCF4E1ECB),
    U64(0xBFC2EF45, 0x6AE276E8), U64(0x9E3FEDD8, 0xC321A67E),
    U64(0xEFB3AB16, 0xC59B14A2), U64(0xC5CFE94E, 0xF3EA101E),
    U64(0x95D04AEE, 0x3B80ECE5), U64(0xBBA1F1D1, 0x58724A12),
    U64(0xBB445DA9, 0xCA61281F), U64(0x2A8A6E45, 0xAE8EDC97),
    U64(0xEA157514, 0x3CF97226), U64(0xF52D09D7, 0x1A3293BD),
    U64(0x924D692C, 0xA61BE758), U64(0x593C2626, 0x705F9C56),
    U64(0xB6E0C377, 0xCFA2E12E), U64(0x6F8B2FB0, 0x0C77836C),
    U64(0xE498F455, 0xC38B997A), U64(0x0B6DFB9C, 0x0F956447),
    U64(0x8EDF98B5, 0x9A373FEC), U64(0x4724BD41, 0x89BD5EAC),
    U64(0xB2977EE3, 0x00C50FE7), U64(0x58EDEC91, 0xEC2CB657),
    U64(0xDF3D5E9B, 0xC0F653E1), U64(0x2F2967B6, 0x6737E3ED),
    U64(0x8B865B21, 0x5899F46C), U64(0xBD79E0D2, 0x0082EE74),
    U64(0xAE67F1E9, 0xAEC07187), U64(0xECD85906, 0x80A3AA11),
    U64(0xDA01EE64, 0x1A708DE9), U64(0xE80E6F48, 0x20CC9495),
    U64(0x884134FE, 0x908658B2), U64(0x3109058D, 0x147FDCDD),
    U64(0xAA51823E, 0x34A7EEDE), U64(0xBD4B46F0, 0x599FD415),
    U64(0xD4E5E2CD, 0xC1D1EA96), U64(0x6C9E18AC, 0x7007C91A),
    U64(0x850FADC0, 0x9923329E), U64(0x03E2CF6B, 0xC604DDB0),
    U64(0xA6539930, 0xBF6BFF45), U64(0x84DB8346, 0xB786151C),
    U64(0xCFE87F7C, 0xEF46FF16), U64(0xE6126418, 0x65679A63),
    U64(0x81F14FAE, 0x158C5F6E), U64(0x4FCB7E8F, 0x3F60C07E),
    U64(0xA26DA399, 0x9AEF7749), U64(0xE3BE5E33, 0x0F38F09D),
    U64(0xCB090C80, 0x01AB551C), U64(0x5CADF5BF, 0xD3072CC5),
    U64(0xFDCB4FA0, 0x02162A63), U64(0x73D9732F, 0xC7C8F7F6),
    U64(0x9E9F11C4, 0x014DDA7E), U64(0x2867E7FD, 0xDCDD9AFA),
    U64(0xC646D635, 0x01A1511D), U64(0xB281E1FD, 0x541501B8),
    U64(0xF7D88BC2, 0x4209A565), U64(0x1F225A7C, 0xA91A4226),
    U64(0x9AE75759, 0x6946075F), U64(0x3375788D, 0xE9B06958),
    U64(0xC1A12D2F, 0xC3978937), U64(0x0052D6B1, 0x641C83AE),
    U64(0xF209787B, 0xB47D6B84), U64(0xC0678C5D, 0xBD23A49A),
    U64(0x9745EB4D, 0x50CE6332), U64(0xF840B7BA, 0x963646E0),
    U64(0xBD176620, 0xA501FBFF), U64(0xB650E5A9, 0x3BC3D898),
    U64(0xEC5D3FA8, 0xCE427AFF), U64(0xA3E51F13, 0x8AB4CEBE),
    U64(0x93BA47C9, 0x80E98CDF), U64(0xC66F336C, 0x36B10137),
    U64(0xB8A8D9BB, 0xE123F017), U64(0xB80B0047, 0x445D4184),
    U64(0xE6D3102A, 0xD96CEC1D), U64(0xA60DC059, 0x157491E5),
    U64(0x9043EA1A, 0xC7E41392), U64(0x87C89837, 0xAD68DB2F),
    U64(0xB454E4A1, 0x79DD1877), U64(0x29BABE45, 0x98C311FB),
    U64(0xE16A1DC9, 0xD8545E94), U64(0xF4296DD6, 0xFEF3D67A),
    U64(0x8CE2529E, 0x2734BB1D), U64(0x1899E4A6, 0x5F58660C),
    U64(0xB01AE745, 0xB101E9E4), U64(0x5EC05DCF, 0xF72E7F8F),
    U64(0xDC21A117, 0x1D42645D), U64(0x76707543, 0xF4FA1F73),
    U64(0x899504AE, 0x72497EBA), U64(0x6A06494A, 0x791C53A8),
    U64(0xABFA45DA, 0x0EDBDE69), U64(0x0487DB9D, 0x17636892),
    U64(0xD6F8D750, 0x9292D603), U64(0x45A9D284, 0x5D3C42B6),
    U64(0x865B8692, 0x5B9BC5C2), U64(0x0B8A2392, 0xBA45A9B2),
    U64(0xA7F26836, 0xF282B732), U64(0x8E6CAC77, 0x68D7141E),
    U64(0xD1EF0244, 0xAF2364FF), U64(0x3207D795, 0x430CD926),
    U64(0x8335616A, 0xED761F1F), U64(0x7F44E6BD, 0x49E807B8),
    U64(0xA402B9C5, 0xA8D3A6E7), U64(0x5F16206C, 0x9C6209A6),
    U64(0xCD036837, 0x130890A1), U64(0x36DBA887, 0xC37A8C0F),
    U64(0x80222122, 0x6BE55A64), U64(0xC2494954, 0xDA2C9789),
    U64(0xA02AA96B, 0x06DEB0FD), U64(0xF2DB9BAA, 0x10B7BD6C),
    U64(0xC83553C5, 0xC8965D3D), U64(0x6F928294, 0x94E5ACC7),
    U64(0xFA42A8B7, 0x3ABBF48C), U64(0xCB772339, 0xBA1F17F9),
    U64(0x9C69A972, 0x84B578D7), U64(0xFF2A7604, 0x14536EFB),
    U64(0xC38413CF, 0x25E2D70D), U64(0xFEF51385, 0x19684ABA),
    U64(0xF46518C2, 0xEF5B8CD1), U64(0x7EB25866, 0x5FC25D69),
    U64(0x98BF2F79, 0xD5993802), U64(0xEF2F773F, 0xFBD97A61),
    U64(0xBEEEFB58, 0x4AFF8603), U64(0xAAFB550F, 0xFACFD8FA),
    U64(0xEEAABA2E, 0x5DBF6784), U64(0x95BA2A53, 0xF983CF38),
    U64(0x952AB45C, 0xFA97A0B2), U64(0xDD945A74, 0x7BF26183),
    U64(0xBA756174, 0x393D88DF), U64(0x94F97111, 0x9AEEF9E4),
    U64(0xE912B9D1, 0x478CEB17), U64(0x7A37CD56, 0x01AAB85D),
    U64(0x91ABB422, 0xCCB812EE), U64(0xAC62E055, 0xC10AB33A),
    U64(0xB616A12B, 0x7FE617AA), U64(0x577B986B, 0x314D6009),
    U64(0xE39C4976, 0x5FDF9D94), U64(0xED5A7E85, 0xFDA0B80B),
    U64(0x8E41ADE9, 0xFBEBC27D), U64(0x14588F13, 0xBE847307),
    U64(0xB1D21964, 0x7AE6B31C), U64(0x596EB2D8, 0xAE258FC8),
    U64(0xDE469FBD, 0x99A05FE3), U64(0x6FCA5F8E, 0xD9AEF3BB),
    U64(0x8AEC23D6, 0x80043BEE), U64(0x25DE7BB9, 0x480D5854),
    U64(0xADA72CCC, 0x20054AE9), U64(0xAF561AA7, 0x9A10AE6A),
    U64(0xD910F7FF, 0x28069DA4), U64(0x1B2BA151, 0x8094DA04),
    U64(0x87AA9AFF, 0x79042286), U64(0x90FB44D2, 0xF05D0842),
    U64(0xA99541BF, 0x57452B28), U64(0x353A1607, 0xAC744A53),
    U64(0xD3FA922F, 0x2D1675F2), U64(0x42889B89, 0x97915CE8),
    U64(0x847C9B5D, 0x7C2E09B7), U64(0x69956135, 0xFEBADA11),
    U64(0xA59BC234, 0xDB398C25), U64(0x43FAB983, 0x7E699095),
    U64(0xCF02B2C2, 0x1207EF2E), U64(0x94F967E4, 0x5E03F4BB),
    U64(0x8161AFB9, 0x4B44F57D), U64(0x1D1BE0EE, 0xBAC278F5),
    U64(0xA1BA1BA7, 0x9E1632DC), U64(0x6462D92A, 0x69731732),
    U64(0xCA28A291, 0x859BBF93), U64(0x7D7B8F75, 0x03CFDCFE),
    U64(0xFCB2CB35, 0xE702AF78), U64(0x5CDA7352, 0x44C3D43E),
    U64(0x9DEFBF01, 0xB061ADAB), U64(0x3A088813, 0x6AFA64A7),
    U64(0xC56BAEC2, 0x1C7A1916), U64(0x088AAA18, 0x45B8FDD0),
    U64(0xF6C69A72, 0xA3989F5B), U64(0x8AAD549E, 0x57273D45),
    U64(0x9A3C2087, 0xA63F6399), U64(0x36AC54E2, 0xF678864B),
    U64(0xC0CB28A9, 0x8FCF3C7F), U64(0x84576A1B, 0xB416A7DD),
    U64(0xF0FDF2D3, 0xF3C30B9F), U64(0x656D44A2, 0xA11C51D5),
    U64(0x969EB7C4, 0x7859E743), U64(0x9F644AE5, 0xA4B1B325),
    U64(0xBC4665B5, 0x96706114), U64(0x873D5D9F, 0x0DDE1FEE),
    U64(0xEB57FF22, 0xFC0C7959), U64(0xA90CB506, 0xD155A7EA),
    U64(0x9316FF75, 0xDD87CBD8), U64(0x09A7F124, 0x42D588F2),
    U64(0xB7DCBF53, 0x54E9BECE), U64(0x0C11ED6D, 0x538AEB2F),
    U64(0xE5D3EF28, 0x2A242E81), U64(0x8F1668C8, 0xA86DA5FA),
    U64(0x8FA47579, 0x1A569D10), U64(0xF96E017D, 0x694487BC),
    U64(0xB38D92D7, 0x60EC4455), U64(0x37C981DC, 0xC395A9AC),
    U64(0xE070F78D, 0x3927556A), U64(0x85BBE253, 0xF47B1417),
    U64(0x8C469AB8, 0x43B89562), U64(0x93956D74, 0x78CCEC8E),
    U64(0xAF584166, 0x54A6BABB), U64(0x387AC8D1, 0x970027B2),
    U64(0xDB2E51BF, 0xE9D0696A), U64(0x06997B05, 0xFCC0319E),
    U64(0x88FCF317, 0xF22241E2), U64(0x441FECE3, 0xBDF81F03),
    U64(0xAB3C2FDD, 0xEEAAD25A), U64(0xD527E81C, 0xAD7626C3),
    U64(0xD60B3BD5, 0x6A5586F1), U64(0x8A71E223, 0xD8D3B074),
    U64(0x85C70565, 0x62757456), U64(0xF6872D56, 0x67844E49),
    U64(0xA738C6BE, 0xBB12D16C), U64(0xB428F8AC, 0x016561DB),
    U64(0xD106F86E, 0x69D785C7), U64(0xE13336D7, 0x01BEBA52),
    U64(0x82A45B45, 0x0226B39C), U64(0xECC00246, 0x61173473),
    U64(0xA34D7216, 0x42B06084), U64(0x27F002D7, 0xF95D0190),
    U64(0xCC20CE9B, 0xD35C78A5), U64(0x31EC038D, 0xF7B441F4),
    U64(0xFF290242, 0xC83396CE), U64(0x7E670471, 0x75A15271),
    U64(0x9F79A169, 0xBD203E41), U64(0x0F0062C6, 0xE984D386),
    U64(0xC75809C4, 0x2C684DD1), U64(0x52C07B78, 0xA3E60868),
    U64(0xF92E0C35, 0x37826145), U64(0xA7709A56, 0xCCDF8A82),
    U64(0x9BBCC7A1, 0x42B17CCB), U64(0x88A66076, 0x400BB691),
    U64(0xC2ABF989, 0x935DDBFE), U64(0x6ACFF893, 0xD00EA435),
    U64(0xF356F7EB, 0xF83552FE), U64(0x0583F6B8, 0xC4124D43),
    U64(0x98165AF3, 0x7B2153DE), U64(0xC3727A33, 0x7A8B704A),
    U64(0xBE1BF1B0, 0x59E9A8D6), U64(0x744F18C0, 0x592E4C5C),
    U64(0xEDA2EE1C, 0x7064130C), U64(0x1162DEF0, 0x6F79DF73),
    U64(0x9485D4D1, 0xC63E8BE7), U64(0x8ADDCB56, 0x45AC2BA8),
    U64(0xB9A74A06, 0x37CE2EE1), U64(0x6D953E2B, 0xD7173692),
    U64(0xE8111C87, 0xC5C1BA99), U64(0xC8FA8DB6, 0xCCDD0437),
    U64(0x910AB1D4, 0xDB9914A0), U64(0x1D9C9892, 0x400A22A2),
    U64(0xB54D5E4A, 0x127F59C8), U64(0x2503BEB6, 0xD00CAB4B),
    U64(0xE2A0B5DC, 0x971F303A), U64(0x2E44AE64, 0x840FD61D),
    U64(0x8DA471A9, 0xDE737E24), U64(0x5CEAECFE, 0xD289E5D2),
    U64(0xB10D8E14, 0x56105DAD), U64(0x7425A83E, 0x872C5F47),
    U64(0xDD50F199, 0x6B947518), U64(0xD12F124E, 0x28F77719),
    U64(0x8A5296FF, 0xE33CC92F), U64(0x82BD6B70, 0xD99AAA6F),
    U64(0xACE73CBF, 0xDC0BFB7B), U64(0x636CC64D, 0x1001550B),
    U64(0xD8210BEF, 0xD30EFA5A), U64(0x3C47F7E0, 0x5401AA4E),
    U64(0x8714A775, 0xE3E95C78), U64(0x65ACFAEC, 0x34810A71),
    U64(0xA8D9D153, 0x5CE3B396), U64(0x7F1839A7, 0x41A14D0D),
    U64(0xD31045A8, 0x341CA07C), U64(0x1EDE4811, 0x1209A050),
    U64(0x83EA2B89, 0x2091E44D), U64(0x934AED0A, 0xAB460432),
    U64(0xA4E4B66B, 0x68B65D60), U64(0xF81DA84D, 0x5617853F),
    U64(0xCE1DE406, 0x42E3F4B9), U64(0x36251260, 0xAB9D668E),
    U64(0x80D2AE83, 0xE9CE78F3), U64(0xC1D72B7C, 0x6B426019),
    U64(0xA1075A24, 0xE4421730), U64(0xB24CF65B, 0x8612F81F),
    U64(0xC94930AE, 0x1D529CFC), U64(0xDEE033F2, 0x6797B627),
    U64(0xFB9B7CD9, 0xA4A7443C), U64(0x169840EF, 0x017DA3B1),
    U64(0x9D412E08, 0x06E88AA5), U64(0x8E1F2895, 0x60EE864E),
    U64(0xC491798A, 0x08A2AD4E), U64(0xF1A6F2BA, 0xB92A27E2),
    U64(0xF5B5D7EC, 0x8ACB58A2), U64(0xAE10AF69, 0x6774B1DB),
    U64(0x9991A6F3, 0xD6BF1765), U64(0xACCA6DA1, 0xE0A8EF29),
    U64(0xBFF610B0, 0xCC6EDD3F), U64(0x17FD090A, 0x58D32AF3),
    U64(0xEFF394DC, 0xFF8A948E), U64(0xDDFC4B4C, 0xEF07F5B0),
    U64(0x95F83D0A, 0x1FB69CD9), U64(0x4ABDAF10, 0x1564F98E),
    U64(0xBB764C4C, 0xA7A4440F), U64(0x9D6D1AD4, 0x1ABE37F1),
    U64(0xEA53DF5F, 0xD18D5513), U64(0x84C86189, 0x216DC5ED),
    U64(0x92746B9B, 0xE2F8552C), U64(0x32FD3CF5, 0xB4E49BB4),
    U64(0xB7118682, 0xDBB66A77), U64(0x3FBC8C33, 0x221DC2A1),
    U64(0xE4D5E823, 0x92A40515), U64(0x0FABAF3F, 0xEAA5334A),
    U64(0x8F05B116, 0x3BA6832D), U64(0x29CB4D87, 0xF2A7400E),
    U64(0xB2C71D5B, 0xCA9023F8), U64(0x743E20E9, 0xEF511012),
    U64(0xDF78E4B2, 0xBD342CF6), U64(0x914DA924, 0x6B255416),
    U64(0x8BAB8EEF, 0xB6409C1A), U64(0x1AD089B6, 0xC2F7548E),
    U64(0xAE9672AB, 0xA3D0C320), U64(0xA184AC24, 0x73B529B1),
    U64(0xDA3C0F56, 0x8CC4F3E8), U64(0xC9E5D72D, 0x90A2741E),
    U64(0x88658996, 0x17FB1871), U64(0x7E2FA67C, 0x7A658892),
    U64(0xAA7EEBFB, 0x9DF9DE8D), U64(0xDDBB901B, 0x98FEEAB7),
    U64(0xD51EA6FA, 0x85785631), U64(0x552A7422, 0x7F3EA565),
    U64(0x8533285C, 0x936B35DE), U64(0xD53A8895, 0x8F87275F),
    U64(0xA67FF273, 0xB8460356), U64(0x8A892ABA, 0xF368F137),
    U64(0xD01FEF10, 0xA657842C), U64(0x2D2B7569, 0xB0432D85),
    U64(0x8213F56A, 0x67F6B29B), U64(0x9C3B2962, 0x0E29FC73),
    U64(0xA298F2C5, 0x01F45F42), U64(0x8349F3BA, 0x91B47B8F),
    U64(0xCB3F2F76, 0x42717713), U64(0x241C70A9, 0x36219A73),
    U64(0xFE0EFB53, 0xD30DD4D7), U64(0xED238CD3, 0x83AA0110),
    U64(0x9EC95D14, 0x63E8A506), U64(0xF4363804, 0x324A40AA),
    U64(0xC67BB459, 0x7CE2CE48), U64(0xB143C605, 0x3EDCD0D5),
    U64(0xF81AA16F, 0xDC1B81DA), U64(0xDD94B786, 0x8E94050A),
    U64(0x9B10A4E5, 0xE9913128), U64(0xCA7CF2B4, 0x191C8326),
    U64(0xC1D4CE1F, 0x63F57D72), U64(0xFD1C2F61, 0x1F63A3F0),
    U64(0xF24A01A7, 0x3CF2DCCF), U64(0xBC633B39, 0x673C8CEC),
    U64(0x976E4108, 0x8617CA01), U64(0xD5BE0503, 0xE085D813),
    U64(0xBD49D14A, 0xA79DBC82), U64(0x4B2D8644, 0xD8A74E18),
    U64(0xEC9C459D, 0x51852BA2), U64(0xDDF8E7D6, 0x0ED1219E),
    U64(0x93E1AB82, 0x52F33B45), U64(0xCABB90E5, 0xC942B503),
    U64(0xB8DA1662, 0xE7B00A17), U64(0x3D6A751F, 0x3B936243),
    U64(0xE7109BFB, 0xA19C0C9D), U64(0x0CC51267, 0x0A783AD4),
    U64(0x906A617D, 0x450187E2), U64(0x27FB2B80, 0x668B24C5),
    U64(0xB484F9DC, 0x9641E9DA), U64(0xB1F9F660, 0x802DEDF6),
    U64(0xE1A63853, 0xBBD26451), U64(0x5E7873F8, 0xA0396973),
    U64(0x8D07E334, 0x55637EB2), U64(0xDB0B487B, 0x6423E1E8),
    U64(0xB049DC01, 0x6ABC5E5F), U64(0x91CE1A9A, 0x3D2CDA62),
    U64(0xDC5C5301, 0xC56B75F7), U64(0x7641A140, 0xCC7810FB),
    U64(0x89B9B3E1, 0x1B6329BA), U64(0xA9E904C8, 0x7FCB0A9D),
    U64(0xAC2820D9, 0x623BF429), U64(0x546345FA, 0x9FBDCD44),
    U64(0xD732290F, 0xBACAF133), U64(0xA97C1779, 0x47AD4095),
    U64(0x867F59A9, 0xD4BED6C0), U64(0x49ED8EAB, 0xCCCC485D),
    U64(0xA81F3014, 0x49EE8C70), U64(0x5C68F256, 0xBFFF5A74),
    U64(0xD226FC19, 0x5C6A2F8C), U64(0x73832EEC, 0x6FFF3111),
    U64(0x83585D8F, 0xD9C25DB7), U64(0xC831FD53, 0xC5FF7EAB),
    U64(0xA42E74F3, 0xD032F525), U64(0xBA3E7CA8, 0xB77F5E55),
    U64(0xCD3A1230, 0xC43FB26F), U64(0x28CE1BD2, 0xE55F35EB),
    U64(0x80444B5E, 0x7AA7CF85), U64(0x7980D163, 0xCF5B81B3),
    U64(0xA0555E36, 0x1951C366), U64(0xD7E105BC, 0xC332621F),
    U64(0xC86AB5C3, 0x9FA63440), U64(0x8DD9472B, 0xF3FEFAA7),
    U64(0xFA856334, 0x878FC150), U64(0xB14F98F6, 0xF0FEB951),
    U64(0x9C935E00, 0xD4B9D8D2), U64(0x6ED1BF9A, 0x569F33D3),
    U64(0xC3B83581, 0x09E84F07), U64(0x0A862F80, 0xEC4700C8),
    U64(0xF4A642E1, 0x4C6262C8), U64(0xCD27BB61, 0x2758C0FA),
    U64(0x98E7E9CC, 0xCFBD7DBD), U64(0x8038D51C, 0xB897789C),
    U64(0xBF21E440, 0x03ACDD2C), U64(0xE0470A63, 0xE6BD56C3),
    U64(0xEEEA5D50, 0x04981478), U64(0x1858CCFC, 0xE06CAC74),
    U64(0x95527A52, 0x02DF0CCB), U64(0x0F37801E, 0x0C43EBC8),
    U64(0xBAA718E6, 0x8396CFFD), U64(0xD3056025, 0x8F54E6BA),
    U64(0xE950DF20, 0x247C83FD), U64(0x47C6B82E, 0xF32A2069),
    U64(0x91D28B74, 0x16CDD27E), U64(0x4CDC331D, 0x57FA5441),
    U64(0xB6472E51, 0x1C81471D), U64(0xE0133FE4, 0xADF8E952),
    U64(0xE3D8F9E5, 0x63A198E5), U64(0x58180FDD, 0xD97723A6),
    U64(0x8E679C2F, 0x5E44FF8F), U64(0x570F09EA, 0xA7EA7648),
    U64(0xB201833B, 0x35D63F73), U64(0x2CD2CC65, 0x51E513DA),
    U64(0xDE81E40A, 0x034BCF4F), U64(0xF8077F7E, 0xA65E58D1),
    U64(0x8B112E86, 0x420F6191), U64(0xFB04AFAF, 0x27FAF782),
    U64(0xADD57A27, 0xD29339F6), U64(0x79C5DB9A, 0xF1F9B563),
    U64(0xD94AD8B1, 0xC7380874), U64(0x18375281, 0xAE7822BC),
    U64(0x87CEC76F, 0x1C830548), U64(0x8F229391, 0x0D0B15B5),
    U64(0xA9C2794A, 0xE3A3C69A), U64(0xB2EB3875, 0x504DDB22),
    U64(0xD433179D, 0x9C8CB841), U64(0x5FA60692, 0xA46151EB),
    U64(0x849FEEC2, 0x81D7F328), U64(0xDBC7C41B, 0xA6BCD333),
    U64(0xA5C7EA73, 0x224DEFF3), U64(0x12B9B522, 0x906C0800),
    U64(0xCF39E50F, 0xEAE16BEF), U64(0xD768226B, 0x34870A00),
    U64(0x81842F29, 0xF2CCE375), U64(0xE6A11583, 0x00D46640),
    U64(0xA1E53AF4, 0x6F801C53), U64(0x60495AE3, 0xC1097FD0),
    U64(0xCA5E89B1, 0x8B602368), U64(0x385BB19C, 0xB14BDFC4),
    U64(0xFCF62C1D, 0xEE382C42), U64(0x46729E03, 0xDD9ED7B5),
    U64(0x9E19DB92, 0xB4E31BA9), U64(0x6C07A2C2, 0x6A8346D1)};

static_inline void pow10_table_get_sig(i32 exp10, u64 *hi, u64 *lo) {
  i32 idx = exp10 - (POW10_SIG_TABLE_MIN_EXP);
  *hi = pow10_sig_table[idx * 2];
  *lo = pow10_sig_table[idx * 2 + 1];
}

static_inline void pow10_table_get_exp(i32 exp10, i32 *exp2) {
  *exp2 = (exp10 * 217706 - 4128768) >> 16;
}

#endif

static_inline f64 f64_from_bits(u64 u) {
  f64 f;
  memcpy(&f, &u, sizeof(u));
  return f;
}

static_inline u64 f64_to_bits(f64 f) {
  u64 u;
  memcpy(&u, &f, sizeof(u));
  return u;
}

static_inline u32 f32_to_bits(f32 f) {
  u32 u;
  memcpy(&u, &f, sizeof(u));
  return u;
}

static_inline u64 f64_bits_inf(bool sign) {
#if YYJSON_HAS_IEEE_754
  return F64_BITS_INF | ((u64)sign << 63);
#elif defined(INFINITY)
  return f64_to_bits(sign ? -INFINITY : INFINITY);
#else
  return f64_to_bits(sign ? -HUGE_VAL : HUGE_VAL);
#endif
}

static_inline u64 f64_bits_nan(bool sign) {
#if YYJSON_HAS_IEEE_754
  return F64_BITS_NAN | ((u64)sign << 63);
#elif defined(NAN)
  return f64_to_bits(sign ? (f64)-NAN : (f64)NAN);
#else
  return f64_to_bits((sign ? -0.0 : 0.0) / 0.0);
#endif
}

#if yyjson_has_attribute(no_sanitize)
__attribute__((no_sanitize("undefined")))
#elif yyjson_gcc_available(4, 9, 0)
__attribute__((__no_sanitize_undefined__))
#endif
static_inline f32
f64_to_f32(f64 val) {
  return (f32)val;
}

static_inline u32 u64_lz_bits(u64 v) {
#if GCC_HAS_CLZLL
  return (u32)__builtin_clzll(v);
#elif MSC_HAS_BIT_SCAN_64
  unsigned long r;
  _BitScanReverse64(&r, v);
  return (u32)63 - (u32)r;
#elif MSC_HAS_BIT_SCAN
  unsigned long hi, lo;
  bool hi_set = _BitScanReverse(&hi, (u32)(v >> 32)) != 0;
  _BitScanReverse(&lo, (u32)v);
  hi |= 32;
  return (u32)63 - (u32)(hi_set ? hi : lo);
#else

  const u8 table[64] = {63, 16, 62, 7,  15, 36, 61, 3,  6,  14, 22, 26, 35,
                        47, 60, 2,  9,  5,  28, 11, 13, 21, 42, 19, 25, 31,
                        34, 40, 46, 52, 59, 1,  17, 8,  37, 4,  23, 27, 48,
                        10, 29, 12, 43, 20, 32, 41, 53, 18, 38, 24, 49, 30,
                        44, 33, 54, 39, 50, 45, 55, 51, 56, 57, 58, 0};
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v |= v >> 32;
  return table[(v * U64(0x03F79D71, 0xB4CB0A89)) >> 58];
#endif
}

static_inline u32 u64_tz_bits(u64 v) {
#if GCC_HAS_CTZLL
  return (u32)__builtin_ctzll(v);
#elif MSC_HAS_BIT_SCAN_64
  unsigned long r;
  _BitScanForward64(&r, v);
  return (u32)r;
#elif MSC_HAS_BIT_SCAN
  unsigned long lo, hi;
  bool lo_set = _BitScanForward(&lo, (u32)(v)) != 0;
  _BitScanForward(&hi, (u32)(v >> 32));
  hi += 32;
  return lo_set ? lo : hi;
#else

  const u8 table[64] = {0,  1,  2,  53, 3,  7,  54, 27, 4,  38, 41, 8,  34,
                        55, 48, 28, 62, 5,  39, 46, 44, 42, 22, 9,  24, 35,
                        59, 56, 49, 18, 29, 11, 63, 52, 6,  26, 37, 40, 33,
                        47, 61, 45, 43, 21, 23, 58, 17, 10, 51, 25, 36, 32,
                        60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12};
  return table[((v & (~v + 1)) * U64(0x022FDD63, 0xCC95386D)) >> 58];
#endif
}

static_inline void u128_mul(u64 a, u64 b, u64 *hi, u64 *lo) {
#if YYJSON_HAS_INT128
  u128 m = (u128)a * b;
  *hi = (u64)(m >> 64);
  *lo = (u64)(m);
#elif MSC_HAS_UMUL128
  *lo = _umul128(a, b, hi);
#else
  u32 a0 = (u32)(a), a1 = (u32)(a >> 32);
  u32 b0 = (u32)(b), b1 = (u32)(b >> 32);
  u64 p00 = (u64)a0 * b0, p01 = (u64)a0 * b1;
  u64 p10 = (u64)a1 * b0, p11 = (u64)a1 * b1;
  u64 m0 = p01 + (p00 >> 32);
  u32 m00 = (u32)(m0), m01 = (u32)(m0 >> 32);
  u64 m1 = p10 + m00;
  u32 m10 = (u32)(m1), m11 = (u32)(m1 >> 32);
  *hi = p11 + m01 + m11;
  *lo = ((u64)m10 << 32) | (u32)p00;
#endif
}

static_inline void u128_mul_add(u64 a, u64 b, u64 c, u64 *hi, u64 *lo) {
#if YYJSON_HAS_INT128
  u128 m = (u128)a * b + c;
  *hi = (u64)(m >> 64);
  *lo = (u64)(m);
#else
  u64 h, l, t;
  u128_mul(a, b, &h, &l);
  t = l + c;
  h += (u64)(((t < l) | (t < c)));
  *hi = h;
  *lo = t;
#endif
}

#define YYJSON_FOPEN_E
#if !defined(_MSC_VER) && defined(__GLIBC__) && defined(__GLIBC_PREREQ)
#if __GLIBC_PREREQ(2, 7)
#undef YYJSON_FOPEN_E
#define YYJSON_FOPEN_E "e"
#endif
#endif

static_inline FILE *fopen_safe(const char *path, const char *mode) {
#if YYJSON_MSC_VER >= 1400
  FILE *file = NULL;
  if (fopen_s(&file, path, mode) != 0) return NULL;
  return file;
#else
  return fopen(path, mode);
#endif
}

static_inline FILE *fopen_readonly(const char *path) {
  return fopen_safe(path, "rb" YYJSON_FOPEN_E);
}

static_inline FILE *fopen_writeonly(const char *path) {
  return fopen_safe(path, "wb" YYJSON_FOPEN_E);
}

static_inline usize fread_safe(void *buf, usize size, FILE *file) {
#if YYJSON_MSC_VER >= 1400
  return fread_s(buf, size, 1, size, file);
#else
  return fread(buf, 1, size, file);
#endif
}

static_inline bool size_add_is_overflow(usize size, usize add) {
  return size > (size + add);
}

static_inline bool size_is_pow2(usize size) { return (size & (size - 1)) == 0; }

static_inline usize size_align_up(usize size, usize align) {
  if (size_is_pow2(align)) {
    return (size + (align - 1)) & ~(align - 1);
  } else {
    return size + align - (size + align - 1) % align - 1;
  }
}

static_inline usize size_align_down(usize size, usize align) {
  if (size_is_pow2(align)) {
    return size & ~(align - 1);
  } else {
    return size - (size % align);
  }
}

static_inline void *mem_align_up(void *mem, usize align) {
  usize size;
  memcpy(&size, &mem, sizeof(usize));
  size = size_align_up(size, align);
  memcpy(&mem, &size, sizeof(usize));
  return mem;
}

static void *default_malloc(void *ctx, usize size) { return malloc(size); }

static void *default_realloc(void *ctx, void *ptr, usize old_size, usize size) {
  return realloc(ptr, size);
}

static void default_free(void *ctx, void *ptr) { free(ptr); }

static const yyjson_alc YYJSON_DEFAULT_ALC = {default_malloc, default_realloc,
                                              default_free, NULL};

static void *null_malloc(void *ctx, usize size) { return NULL; }

static void *null_realloc(void *ctx, void *ptr, usize old_size, usize size) {
  return NULL;
}

static void null_free(void *ctx, void *ptr) { return; }

static const yyjson_alc YYJSON_NULL_ALC = {null_malloc, null_realloc, null_free,
                                           NULL};

typedef struct pool_chunk {
  usize size;
  struct pool_chunk *next;

} pool_chunk;

typedef struct pool_ctx {
  usize size;
  pool_chunk *free_list;

} pool_ctx;

static_inline void pool_size_align(usize *size) {
  *size = size_align_up(*size, sizeof(pool_chunk)) + sizeof(pool_chunk);
}

static void *pool_malloc(void *ctx_ptr, usize size) {
  pool_ctx *ctx = (pool_ctx *)ctx_ptr;
  pool_chunk *next, *prev = NULL, *cur = ctx->free_list;

  if (unlikely(size >= ctx->size)) return NULL;
  pool_size_align(&size);

  while (cur) {
    if (cur->size < size) {
      prev = cur;
      cur = cur->next;
      continue;
    }
    if (cur->size >= size + sizeof(pool_chunk) * 2) {
      next = (pool_chunk *)(void *)((u8 *)cur + size);
      next->size = cur->size - size;
      next->next = cur->next;
      cur->size = size;
    } else {
      next = cur->next;
    }
    if (prev)
      prev->next = next;
    else
      ctx->free_list = next;
    return (void *)(cur + 1);
  }
  return NULL;
}

static void pool_free(void *ctx_ptr, void *ptr) {
  pool_ctx *ctx = (pool_ctx *)ctx_ptr;
  pool_chunk *cur = ((pool_chunk *)ptr) - 1;
  pool_chunk *prev = NULL, *next = ctx->free_list;

  while (next && next < cur) {
    prev = next;
    next = next->next;
  }
  if (prev)
    prev->next = cur;
  else
    ctx->free_list = cur;
  cur->next = next;

  if (next && ((u8 *)cur + cur->size) == (u8 *)next) {
    cur->size += next->size;
    cur->next = next->next;
  }
  if (prev && ((u8 *)prev + prev->size) == (u8 *)cur) {
    prev->size += cur->size;
    prev->next = cur->next;
  }
}

static void *pool_realloc(void *ctx_ptr, void *ptr, usize old_size,
                          usize size) {
  pool_ctx *ctx = (pool_ctx *)ctx_ptr;
  pool_chunk *cur = ((pool_chunk *)ptr) - 1, *prev, *next, *tmp;

  if (unlikely(size >= ctx->size)) return NULL;
  pool_size_align(&old_size);
  pool_size_align(&size);
  if (unlikely(old_size == size)) return ptr;

  prev = NULL;
  next = ctx->free_list;
  while (next && next < cur) {
    prev = next;
    next = next->next;
  }

  if ((u8 *)cur + cur->size == (u8 *)next && cur->size + next->size >= size) {
    usize free_size = cur->size + next->size - size;
    if (free_size > sizeof(pool_chunk) * 2) {
      tmp = (pool_chunk *)(void *)((u8 *)cur + size);
      if (prev)
        prev->next = tmp;
      else
        ctx->free_list = tmp;
      tmp->next = next->next;
      tmp->size = free_size;
      cur->size = size;
    } else {
      if (prev)
        prev->next = next->next;
      else
        ctx->free_list = next->next;
      cur->size += next->size;
    }
    return ptr;
  } else {
    void *new_ptr = pool_malloc(ctx_ptr, size - sizeof(pool_chunk));
    if (new_ptr) {
      memcpy(new_ptr, ptr, cur->size - sizeof(pool_chunk));
      pool_free(ctx_ptr, ptr);
    }
    return new_ptr;
  }
}

bool yyjson_alc_pool_init(yyjson_alc *alc, void *buf, usize size) {
  pool_chunk *chunk;
  pool_ctx *ctx;

  if (unlikely(!alc)) return false;
  *alc = YYJSON_NULL_ALC;
  if (size < sizeof(pool_ctx) * 4) return false;
  ctx = (pool_ctx *)mem_align_up(buf, sizeof(pool_ctx));
  if (unlikely(!ctx)) return false;
  size -= (usize)((u8 *)ctx - (u8 *)buf);
  size = size_align_down(size, sizeof(pool_ctx));

  chunk = (pool_chunk *)(ctx + 1);
  chunk->size = size - sizeof(pool_ctx);
  chunk->next = NULL;
  ctx->size = size;
  ctx->free_list = chunk;

  alc->malloc = pool_malloc;
  alc->realloc = pool_realloc;
  alc->free = pool_free;
  alc->ctx = (void *)ctx;
  return true;
}

typedef struct dyn_chunk {
  usize size;
  struct dyn_chunk *next;

} dyn_chunk;

typedef struct {
  dyn_chunk free_list;
  dyn_chunk used_list;
} dyn_ctx;

static_inline bool dyn_size_align(usize *size) {
  usize alc_size = *size + sizeof(dyn_chunk);
  alc_size = size_align_up(alc_size, YYJSON_ALC_DYN_MIN_SIZE);
  if (unlikely(alc_size < *size)) return false;
  *size = alc_size;
  return true;
}

static_inline void dyn_chunk_list_remove(dyn_chunk *list, dyn_chunk *chunk) {
  dyn_chunk *prev = list, *cur;
  for (cur = prev->next; cur; cur = cur->next) {
    if (cur == chunk) {
      prev->next = cur->next;
      cur->next = NULL;
      return;
    }
    prev = cur;
  }
}

static_inline void dyn_chunk_list_add(dyn_chunk *list, dyn_chunk *chunk) {
  chunk->next = list->next;
  list->next = chunk;
}

static void *dyn_malloc(void *ctx_ptr, usize size) {
  const yyjson_alc def = YYJSON_DEFAULT_ALC;
  dyn_ctx *ctx = (dyn_ctx *)ctx_ptr;
  dyn_chunk *chunk, *prev;
  if (unlikely(!dyn_size_align(&size))) return NULL;

  if (!ctx->free_list.next) {
    chunk = (dyn_chunk *)def.malloc(def.ctx, size);
    if (unlikely(!chunk)) return NULL;
    chunk->size = size;
    chunk->next = NULL;
    dyn_chunk_list_add(&ctx->used_list, chunk);
    return (void *)(chunk + 1);
  }

  prev = &ctx->free_list;
  while (true) {
    chunk = prev->next;
    if (chunk->size >= size) {
      prev->next = chunk->next;
      dyn_chunk_list_add(&ctx->used_list, chunk);
      return (void *)(chunk + 1);
    }
    if (!chunk->next) {
      chunk = (dyn_chunk *)def.realloc(def.ctx, chunk, chunk->size, size);
      if (unlikely(!chunk)) return NULL;
      prev->next = NULL;
      chunk->size = size;
      dyn_chunk_list_add(&ctx->used_list, chunk);
      return (void *)(chunk + 1);
    }
    prev = chunk;
  }
}

static void *dyn_realloc(void *ctx_ptr, void *ptr, usize old_size, usize size) {
  const yyjson_alc def = YYJSON_DEFAULT_ALC;
  dyn_ctx *ctx = (dyn_ctx *)ctx_ptr;
  dyn_chunk *new_chunk, *chunk = (dyn_chunk *)ptr - 1;
  if (unlikely(!dyn_size_align(&size))) return NULL;
  if (chunk->size >= size) return ptr;

  dyn_chunk_list_remove(&ctx->used_list, chunk);
  new_chunk = (dyn_chunk *)def.realloc(def.ctx, chunk, chunk->size, size);
  if (likely(new_chunk)) {
    new_chunk->size = size;
    chunk = new_chunk;
  }
  dyn_chunk_list_add(&ctx->used_list, chunk);
  return new_chunk ? (void *)(new_chunk + 1) : NULL;
}

static void dyn_free(void *ctx_ptr, void *ptr) {
  dyn_ctx *ctx = (dyn_ctx *)ctx_ptr;
  dyn_chunk *chunk = (dyn_chunk *)ptr - 1, *prev;

  dyn_chunk_list_remove(&ctx->used_list, chunk);
  for (prev = &ctx->free_list; prev; prev = prev->next) {
    if (!prev->next || prev->next->size >= chunk->size) {
      chunk->next = prev->next;
      prev->next = chunk;
      break;
    }
  }
}

yyjson_alc *yyjson_alc_dyn_new(void) {
  const yyjson_alc def = YYJSON_DEFAULT_ALC;
  usize hdr_len = sizeof(yyjson_alc) + sizeof(dyn_ctx);
  yyjson_alc *alc;
  dyn_ctx *ctx;
  alc = (yyjson_alc *)def.malloc(def.ctx, hdr_len);
  if (unlikely(!alc)) return NULL;
  ctx = (dyn_ctx *)(void *)(alc + 1);
  alc->malloc = dyn_malloc;
  alc->realloc = dyn_realloc;
  alc->free = dyn_free;
  alc->ctx = alc + 1;
  memset(ctx, 0, sizeof(*ctx));
  return alc;
}

void yyjson_alc_dyn_free(yyjson_alc *alc) {
  const yyjson_alc def = YYJSON_DEFAULT_ALC;
  dyn_ctx *ctx;
  dyn_chunk *chunk, *next;
  if (unlikely(!alc)) return;
  ctx = (dyn_ctx *)(void *)(alc + 1);
  for (chunk = ctx->free_list.next; chunk; chunk = next) {
    next = chunk->next;
    def.free(def.ctx, chunk);
  }
  for (chunk = ctx->used_list.next; chunk; chunk = next) {
    next = chunk->next;
    def.free(def.ctx, chunk);
  }
  def.free(def.ctx, alc);
}

static_inline void unsafe_yyjson_str_pool_release(yyjson_str_pool *pool,
                                                  yyjson_alc *alc) {
  yyjson_str_chunk *chunk = pool->chunks, *next;
  while (chunk) {
    next = chunk->next;
    alc->free(alc->ctx, chunk);
    chunk = next;
  }
}

static_inline void unsafe_yyjson_val_pool_release(yyjson_val_pool *pool,
                                                  yyjson_alc *alc) {
  yyjson_val_chunk *chunk = pool->chunks, *next;
  while (chunk) {
    next = chunk->next;
    alc->free(alc->ctx, chunk);
    chunk = next;
  }
}

bool unsafe_yyjson_str_pool_grow(yyjson_str_pool *pool, const yyjson_alc *alc,
                                 usize len) {
  yyjson_str_chunk *chunk;
  usize size, max_len;

  max_len = USIZE_MAX - sizeof(yyjson_str_chunk);
  if (unlikely(len > max_len)) return false;
  size = len + sizeof(yyjson_str_chunk);
  size = yyjson_max(pool->chunk_size, size);
  chunk = (yyjson_str_chunk *)alc->malloc(alc->ctx, size);
  if (unlikely(!chunk)) return false;

  chunk->next = pool->chunks;
  chunk->chunk_size = size;
  pool->chunks = chunk;
  pool->cur = (char *)chunk + sizeof(yyjson_str_chunk);
  pool->end = (char *)chunk + size;

  size = yyjson_min(pool->chunk_size * 2, pool->chunk_size_max);
  if (size < pool->chunk_size) size = pool->chunk_size_max;
  pool->chunk_size = size;
  return true;
}

bool unsafe_yyjson_val_pool_grow(yyjson_val_pool *pool, const yyjson_alc *alc,
                                 usize count) {
  yyjson_val_chunk *chunk;
  usize size, max_count;

  max_count = USIZE_MAX / sizeof(yyjson_mut_val) - 1;
  if (unlikely(count > max_count)) return false;
  size = (count + 1) * sizeof(yyjson_mut_val);
  size = yyjson_max(pool->chunk_size, size);
  chunk = (yyjson_val_chunk *)alc->malloc(alc->ctx, size);
  if (unlikely(!chunk)) return false;

  chunk->next = pool->chunks;
  chunk->chunk_size = size;
  pool->chunks = chunk;
  pool->cur = (yyjson_mut_val *)(void *)((u8 *)chunk) + 1;
  pool->end = (yyjson_mut_val *)(void *)((u8 *)chunk + size);

  size = yyjson_min(pool->chunk_size * 2, pool->chunk_size_max);
  if (size < pool->chunk_size) size = pool->chunk_size_max;
  pool->chunk_size = size;
  return true;
}

bool yyjson_mut_doc_set_str_pool_size(yyjson_mut_doc *doc, size_t len) {
  usize max_size = USIZE_MAX - sizeof(yyjson_str_chunk);
  if (!doc || !len || len > max_size) return false;
  doc->str_pool.chunk_size = len + sizeof(yyjson_str_chunk);
  return true;
}

bool yyjson_mut_doc_set_val_pool_size(yyjson_mut_doc *doc, size_t count) {
  usize max_count = USIZE_MAX / sizeof(yyjson_mut_val) - 1;
  if (!doc || !count || count > max_count) return false;
  doc->val_pool.chunk_size = (count + 1) * sizeof(yyjson_mut_val);
  return true;
}

void yyjson_mut_doc_free(yyjson_mut_doc *doc) {
  if (doc) {
    yyjson_alc alc = doc->alc;
    memset(&doc->alc, 0, sizeof(alc));
    unsafe_yyjson_str_pool_release(&doc->str_pool, &alc);
    unsafe_yyjson_val_pool_release(&doc->val_pool, &alc);
    alc.free(alc.ctx, doc);
  }
}

yyjson_mut_doc *yyjson_mut_doc_new(const yyjson_alc *alc) {
  yyjson_mut_doc *doc;
  if (!alc) alc = &YYJSON_DEFAULT_ALC;
  doc = (yyjson_mut_doc *)alc->malloc(alc->ctx, sizeof(yyjson_mut_doc));
  if (!doc) return NULL;
  memset(doc, 0, sizeof(yyjson_mut_doc));

  doc->alc = *alc;
  doc->str_pool.chunk_size = YYJSON_MUT_DOC_STR_POOL_INIT_SIZE;
  doc->str_pool.chunk_size_max = YYJSON_MUT_DOC_STR_POOL_MAX_SIZE;
  doc->val_pool.chunk_size = YYJSON_MUT_DOC_VAL_POOL_INIT_SIZE;
  doc->val_pool.chunk_size_max = YYJSON_MUT_DOC_VAL_POOL_MAX_SIZE;
  return doc;
}

yyjson_mut_doc *yyjson_doc_mut_copy(const yyjson_doc *doc,
                                    const yyjson_alc *alc) {
  yyjson_mut_doc *m_doc;
  yyjson_mut_val *m_val;

  if (!doc || !doc->root) return NULL;
  m_doc = yyjson_mut_doc_new(alc);
  if (!m_doc) return NULL;
  m_val = yyjson_val_mut_copy(m_doc, doc->root);
  if (!m_val) {
    yyjson_mut_doc_free(m_doc);
    return NULL;
  }
  yyjson_mut_doc_set_root(m_doc, m_val);
  return m_doc;
}

yyjson_mut_doc *yyjson_mut_doc_mut_copy(const yyjson_mut_doc *doc,
                                        const yyjson_alc *alc) {
  yyjson_mut_doc *m_doc;
  yyjson_mut_val *m_val;

  if (!doc) return NULL;
  if (!doc->root) return yyjson_mut_doc_new(alc);

  m_doc = yyjson_mut_doc_new(alc);
  if (!m_doc) return NULL;
  m_val = yyjson_mut_val_mut_copy(m_doc, doc->root);
  if (!m_val) {
    yyjson_mut_doc_free(m_doc);
    return NULL;
  }
  yyjson_mut_doc_set_root(m_doc, m_val);
  return m_doc;
}

yyjson_mut_val *yyjson_val_mut_copy(yyjson_mut_doc *m_doc,
                                    const yyjson_val *i_vals) {
  usize i_vals_len;
  yyjson_mut_val *m_vals, *m_val;
  yyjson_val *i_val, *i_end;

  if (!m_doc || !i_vals) return NULL;
  i_end = unsafe_yyjson_get_next(i_vals);
  i_vals_len = (usize)(unsafe_yyjson_get_next(i_vals) - i_vals);
  m_vals = unsafe_yyjson_mut_val(m_doc, i_vals_len);
  if (!m_vals) return NULL;
  i_val = constcast(yyjson_val *) i_vals;
  m_val = m_vals;

  for (; i_val < i_end; i_val++, m_val++) {
    yyjson_type type = unsafe_yyjson_get_type(i_val);
    m_val->tag = i_val->tag;
    m_val->uni.u64 = i_val->uni.u64;
    if (type == YYJSON_TYPE_STR || type == YYJSON_TYPE_RAW) {
      const char *str = i_val->uni.str;
      usize str_len = unsafe_yyjson_get_len(i_val);
      m_val->uni.str = unsafe_yyjson_mut_strncpy(m_doc, str, str_len);
      if (!m_val->uni.str) return NULL;
    } else if (type == YYJSON_TYPE_ARR) {
      usize len = unsafe_yyjson_get_len(i_val);
      if (len > 0) {
        yyjson_val *ii_val = i_val + 1, *ii_next;
        yyjson_mut_val *mm_val = m_val + 1, *mm_ctn = m_val, *mm_next;
        while (len-- > 1) {
          ii_next = unsafe_yyjson_get_next(ii_val);
          mm_next = mm_val + (ii_next - ii_val);
          mm_val->next = mm_next;
          ii_val = ii_next;
          mm_val = mm_next;
        }
        mm_val->next = mm_ctn + 1;
        mm_ctn->uni.ptr = mm_val;
      }
    } else if (type == YYJSON_TYPE_OBJ) {
      usize len = unsafe_yyjson_get_len(i_val);
      if (len > 0) {
        yyjson_val *ii_key = i_val + 1, *ii_nextkey;
        yyjson_mut_val *mm_key = m_val + 1, *mm_ctn = m_val;
        yyjson_mut_val *mm_nextkey;
        while (len-- > 1) {
          ii_nextkey = unsafe_yyjson_get_next(ii_key + 1);
          mm_nextkey = mm_key + (ii_nextkey - ii_key);
          mm_key->next = mm_key + 1;
          mm_key->next->next = mm_nextkey;
          ii_key = ii_nextkey;
          mm_key = mm_nextkey;
        }
        mm_key->next = mm_key + 1;
        mm_key->next->next = mm_ctn + 1;
        mm_ctn->uni.ptr = mm_key;
      }
    }
  }
  return m_vals;
}

static yyjson_mut_val *unsafe_yyjson_mut_val_mut_copy(
    yyjson_mut_doc *m_doc, const yyjson_mut_val *m_vals) {
  yyjson_mut_val *m_val = unsafe_yyjson_mut_val(m_doc, 1);
  if (unlikely(!m_val)) return NULL;
  m_val->tag = m_vals->tag;

  switch (unsafe_yyjson_get_type(m_vals)) {
    case YYJSON_TYPE_OBJ:
    case YYJSON_TYPE_ARR:
      if (unsafe_yyjson_get_len(m_vals) > 0) {
        yyjson_mut_val *last = (yyjson_mut_val *)m_vals->uni.ptr;
        yyjson_mut_val *next = last->next, *prev;
        prev = unsafe_yyjson_mut_val_mut_copy(m_doc, last);
        if (!prev) return NULL;
        m_val->uni.ptr = (void *)prev;
        while (next != last) {
          prev->next = unsafe_yyjson_mut_val_mut_copy(m_doc, next);
          if (!prev->next) return NULL;
          prev = prev->next;
          next = next->next;
        }
        prev->next = (yyjson_mut_val *)m_val->uni.ptr;
      }
      break;
    case YYJSON_TYPE_RAW:
    case YYJSON_TYPE_STR: {
      const char *str = m_vals->uni.str;
      usize str_len = unsafe_yyjson_get_len(m_vals);
      m_val->uni.str = unsafe_yyjson_mut_strncpy(m_doc, str, str_len);
      if (!m_val->uni.str) return NULL;
      break;
    }
    default:
      m_val->uni = m_vals->uni;
      break;
  }
  return m_val;
}

yyjson_mut_val *yyjson_mut_val_mut_copy(yyjson_mut_doc *doc,
                                        const yyjson_mut_val *val) {
  if (doc && val) return unsafe_yyjson_mut_val_mut_copy(doc, val);
  return NULL;
}

static void yyjson_mut_stat(const yyjson_mut_val *val, usize *val_sum,
                            usize *str_sum) {
  yyjson_type type = unsafe_yyjson_get_type(val);
  *val_sum += 1;
  if (type == YYJSON_TYPE_ARR || type == YYJSON_TYPE_OBJ) {
    yyjson_mut_val *child = (yyjson_mut_val *)val->uni.ptr;
    usize len = unsafe_yyjson_get_len(val), i;
    len <<= (u8)(type == YYJSON_TYPE_OBJ);
    *val_sum += len;
    for (i = 0; i < len; i++) {
      yyjson_type stype = unsafe_yyjson_get_type(child);
      if (stype == YYJSON_TYPE_STR || stype == YYJSON_TYPE_RAW) {
        *str_sum += unsafe_yyjson_get_len(child) + 1;
      } else if (stype == YYJSON_TYPE_ARR || stype == YYJSON_TYPE_OBJ) {
        yyjson_mut_stat(child, val_sum, str_sum);
        *val_sum -= 1;
      }
      child = child->next;
    }
  } else if (type == YYJSON_TYPE_STR || type == YYJSON_TYPE_RAW) {
    *str_sum += unsafe_yyjson_get_len(val) + 1;
  }
}

static usize yyjson_imut_copy(yyjson_val **val_ptr, char **buf_ptr,
                              const yyjson_mut_val *mval) {
  yyjson_val *val = *val_ptr;
  yyjson_type type = unsafe_yyjson_get_type(mval);
  if (type == YYJSON_TYPE_ARR || type == YYJSON_TYPE_OBJ) {
    yyjson_mut_val *child = (yyjson_mut_val *)mval->uni.ptr;
    usize len = unsafe_yyjson_get_len(mval), i;
    usize val_sum = 1;
    if (type == YYJSON_TYPE_OBJ) {
      if (len) child = child->next->next;
      len <<= 1;
    } else {
      if (len) child = child->next;
    }
    *val_ptr = val + 1;
    for (i = 0; i < len; i++) {
      val_sum += yyjson_imut_copy(val_ptr, buf_ptr, child);
      child = child->next;
    }
    val->tag = mval->tag;
    val->uni.ofs = val_sum * sizeof(yyjson_val);
    return val_sum;
  } else if (type == YYJSON_TYPE_STR || type == YYJSON_TYPE_RAW) {
    char *buf = *buf_ptr;
    usize len = unsafe_yyjson_get_len(mval);
    memcpy((void *)buf, (const void *)mval->uni.str, len);
    buf[len] = '\0';
    val->tag = mval->tag;
    val->uni.str = buf;
    *val_ptr = val + 1;
    *buf_ptr = buf + len + 1;
    return 1;
  } else {
    val->tag = mval->tag;
    val->uni = mval->uni;
    *val_ptr = val + 1;
    return 1;
  }
}

yyjson_doc *yyjson_mut_doc_imut_copy(const yyjson_mut_doc *mdoc,
                                     const yyjson_alc *alc) {
  if (!mdoc) return NULL;
  return yyjson_mut_val_imut_copy(mdoc->root, alc);
}

yyjson_doc *yyjson_mut_val_imut_copy(const yyjson_mut_val *mval,
                                     const yyjson_alc *alc) {
  usize val_num = 0, str_sum = 0, hdr_size, buf_size;
  yyjson_doc *doc = NULL;
  yyjson_val *val_hdr = NULL;

  char *str_hdr = (char *)(void *)&str_sum;
  if (!mval) return NULL;
  if (!alc) alc = &YYJSON_DEFAULT_ALC;

  yyjson_mut_stat(mval, &val_num, &str_sum);

  hdr_size = size_align_up(sizeof(yyjson_doc), sizeof(yyjson_val));
  buf_size = hdr_size + val_num * sizeof(yyjson_val);
  doc = (yyjson_doc *)alc->malloc(alc->ctx, buf_size);
  if (!doc) return NULL;
  memset(doc, 0, sizeof(yyjson_doc));
  val_hdr = (yyjson_val *)(void *)((char *)(void *)doc + hdr_size);
  doc->root = val_hdr;
  doc->alc = *alc;

  if (str_sum > 0) {
    str_hdr = (char *)alc->malloc(alc->ctx, str_sum);
    doc->str_pool = str_hdr;
    if (!str_hdr) {
      alc->free(alc->ctx, (void *)doc);
      return NULL;
    }
  }

  doc->val_read = yyjson_imut_copy(&val_hdr, &str_hdr, mval);
  doc->dat_read = str_sum + 1;
  return doc;
}

static_inline bool unsafe_yyjson_num_equals(const void *lhs, const void *rhs) {
  const yyjson_val_uni *luni = &((const yyjson_val *)lhs)->uni;
  const yyjson_val_uni *runi = &((const yyjson_val *)rhs)->uni;
  yyjson_subtype lt = unsafe_yyjson_get_subtype(lhs);
  yyjson_subtype rt = unsafe_yyjson_get_subtype(rhs);
  if (lt == rt) return luni->u64 == runi->u64;
  if (lt == YYJSON_SUBTYPE_SINT && rt == YYJSON_SUBTYPE_UINT) {
    return luni->i64 >= 0 && luni->u64 == runi->u64;
  }
  if (lt == YYJSON_SUBTYPE_UINT && rt == YYJSON_SUBTYPE_SINT) {
    return runi->i64 >= 0 && luni->u64 == runi->u64;
  }
  return false;
}

static_inline bool unsafe_yyjson_str_equals(const void *lhs, const void *rhs) {
  usize len = unsafe_yyjson_get_len(lhs);
  if (len != unsafe_yyjson_get_len(rhs)) return false;
  return !memcmp(unsafe_yyjson_get_str(lhs), unsafe_yyjson_get_str(rhs), len);
}

bool unsafe_yyjson_equals(const yyjson_val *lhs, const yyjson_val *rhs) {
  yyjson_type type = unsafe_yyjson_get_type(lhs);
  if (type != unsafe_yyjson_get_type(rhs)) return false;

  switch (type) {
    case YYJSON_TYPE_OBJ: {
      usize len = unsafe_yyjson_get_len(lhs);
      if (len != unsafe_yyjson_get_len(rhs)) return false;
      if (len > 0) {
        yyjson_obj_iter iter;
        yyjson_obj_iter_init(rhs, &iter);
        lhs = unsafe_yyjson_get_first(lhs);
        while (len-- > 0) {
          rhs = yyjson_obj_iter_getn(&iter, lhs->uni.str,
                                     unsafe_yyjson_get_len(lhs));
          if (!rhs) return false;
          if (!unsafe_yyjson_equals(lhs + 1, rhs)) return false;
          lhs = unsafe_yyjson_get_next(lhs + 1);
        }
      }

      return true;
    }

    case YYJSON_TYPE_ARR: {
      usize len = unsafe_yyjson_get_len(lhs);
      if (len != unsafe_yyjson_get_len(rhs)) return false;
      if (len > 0) {
        lhs = unsafe_yyjson_get_first(lhs);
        rhs = unsafe_yyjson_get_first(rhs);
        while (len-- > 0) {
          if (!unsafe_yyjson_equals(lhs, rhs)) return false;
          lhs = unsafe_yyjson_get_next(lhs);
          rhs = unsafe_yyjson_get_next(rhs);
        }
      }
      return true;
    }

    case YYJSON_TYPE_NUM:
      return unsafe_yyjson_num_equals(lhs, rhs);

    case YYJSON_TYPE_RAW:
    case YYJSON_TYPE_STR:
      return unsafe_yyjson_str_equals(lhs, rhs);

    case YYJSON_TYPE_NULL:
    case YYJSON_TYPE_BOOL:
      return lhs->tag == rhs->tag;

    default:
      return false;
  }
}

bool unsafe_yyjson_mut_equals(const yyjson_mut_val *lhs,
                              const yyjson_mut_val *rhs) {
  yyjson_type type = unsafe_yyjson_get_type(lhs);
  if (type != unsafe_yyjson_get_type(rhs)) return false;

  switch (type) {
    case YYJSON_TYPE_OBJ: {
      usize len = unsafe_yyjson_get_len(lhs);
      if (len != unsafe_yyjson_get_len(rhs)) return false;
      if (len > 0) {
        yyjson_mut_obj_iter iter;
        yyjson_mut_obj_iter_init(constcast(yyjson_mut_val *) rhs, &iter);
        lhs = (yyjson_mut_val *)lhs->uni.ptr;
        while (len-- > 0) {
          rhs = yyjson_mut_obj_iter_getn(&iter, lhs->uni.str,
                                         unsafe_yyjson_get_len(lhs));
          if (!rhs) return false;
          if (!unsafe_yyjson_mut_equals(lhs->next, rhs)) return false;
          lhs = lhs->next->next;
        }
      }

      return true;
    }

    case YYJSON_TYPE_ARR: {
      usize len = unsafe_yyjson_get_len(lhs);
      if (len != unsafe_yyjson_get_len(rhs)) return false;
      if (len > 0) {
        lhs = (yyjson_mut_val *)lhs->uni.ptr;
        rhs = (yyjson_mut_val *)rhs->uni.ptr;
        while (len-- > 0) {
          if (!unsafe_yyjson_mut_equals(lhs, rhs)) return false;
          lhs = lhs->next;
          rhs = rhs->next;
        }
      }
      return true;
    }

    case YYJSON_TYPE_NUM:
      return unsafe_yyjson_num_equals(lhs, rhs);

    case YYJSON_TYPE_RAW:
    case YYJSON_TYPE_STR:
      return unsafe_yyjson_str_equals(lhs, rhs);

    case YYJSON_TYPE_NULL:
    case YYJSON_TYPE_BOOL:
      return lhs->tag == rhs->tag;

    default:
      return false;
  }
}

bool yyjson_locate_pos(const char *str, size_t len, size_t pos, size_t *line,
                       size_t *col, size_t *chr) {
  usize line_sum = 0, line_pos = 0, chr_sum = 0;
  const u8 *cur = (const u8 *)str;
  const u8 *end = cur + pos;

  if (!str || pos > len) {
    if (line) *line = 0;
    if (col) *col = 0;
    if (chr) *chr = 0;
    return false;
  }

  if (pos >= 3 && is_utf8_bom(cur)) cur += 3;
  while (cur < end) {
    u8 c = *cur;
    chr_sum += 1;
    if (likely(c < 0x80)) {
      if (c == '\n') {
        line_sum += 1;
        line_pos = chr_sum;
      }
      cur += 1;
    } else if (c < 0xC0)
      cur += 1;
    else if (c < 0xE0)
      cur += 2;
    else if (c < 0xF0)
      cur += 3;
    else if (c < 0xF8)
      cur += 4;
    else
      cur += 1;
  }
  if (line) *line = line_sum + 1;
  if (col) *col = chr_sum - line_pos + 1;
  if (chr) *chr = chr_sum;
  return true;
}

#if !YYJSON_DISABLE_READER

#define has_flg(_flg) unlikely(has_rflag(flg, YYJSON_READ_##_flg, 0))
#define has_allow(_flg) unlikely(has_rflag(flg, YYJSON_READ_ALLOW_##_flg, 1))
#define YYJSON_READ_ALLOW_TRIVIA \
  (YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_EXT_WHITESPACE)
static_inline bool has_rflag(yyjson_read_flag flg, yyjson_read_flag chk,
                             bool non_standard) {
#if YYJSON_DISABLE_NON_STANDARD
  if (non_standard) return false;
#endif
  return (flg & chk) != 0;
}

static_inline bool read_true(u8 **ptr, yyjson_val *val) {
  u8 *cur = *ptr;
  if (likely(byte_match_4(cur, "true"))) {
    val->tag = YYJSON_TYPE_BOOL | YYJSON_SUBTYPE_TRUE;
    *ptr = cur + 4;
    return true;
  }
  return false;
}

static_inline bool read_false(u8 **ptr, yyjson_val *val) {
  u8 *cur = *ptr;
  if (likely(byte_match_4(cur + 1, "alse"))) {
    val->tag = YYJSON_TYPE_BOOL | YYJSON_SUBTYPE_FALSE;
    *ptr = cur + 5;
    return true;
  }
  return false;
}

static_inline bool read_null(u8 **ptr, yyjson_val *val) {
  u8 *cur = *ptr;
  if (likely(byte_match_4(cur, "null"))) {
    val->tag = YYJSON_TYPE_NULL;
    *ptr = cur + 4;
    return true;
  }
  return false;
}

static_inline bool read_inf(u8 **ptr, u8 **pre, yyjson_read_flag flg,
                            yyjson_val *val) {
  u8 *hdr = *ptr;
  u8 *cur = *ptr;
  u8 **end = ptr;
  bool sign = (*cur == '-');
  if (*cur == '+' && !has_allow(EXT_NUMBER)) return false;
  cur += char_is_sign(*cur);
  if (char_to_lower(cur[0]) == 'i' && char_to_lower(cur[1]) == 'n' &&
      char_to_lower(cur[2]) == 'f') {
    if (char_to_lower(cur[3]) == 'i') {
      if (char_to_lower(cur[4]) == 'n' && char_to_lower(cur[5]) == 'i' &&
          char_to_lower(cur[6]) == 't' && char_to_lower(cur[7]) == 'y') {
        cur += 8;
      } else {
        return false;
      }
    } else {
      cur += 3;
    }
    *end = cur;
    if (has_flg(NUMBER_AS_RAW)) {
      **pre = '\0';
      *pre = cur;
      val->tag = ((u64)(cur - hdr) << YYJSON_TAG_BIT) | YYJSON_TYPE_RAW;
      val->uni.str = (const char *)hdr;
    } else {
      val->tag = YYJSON_TYPE_NUM | YYJSON_SUBTYPE_REAL;
      val->uni.u64 = f64_bits_inf(sign);
    }
    return true;
  }
  return false;
}

static_inline bool read_nan(u8 **ptr, u8 **pre, yyjson_read_flag flg,
                            yyjson_val *val) {
  u8 *hdr = *ptr;
  u8 *cur = *ptr;
  u8 **end = ptr;
  bool sign = (*cur == '-');
  if (*cur == '+' && !has_allow(EXT_NUMBER)) return false;
  cur += char_is_sign(*cur);
  if (char_to_lower(cur[0]) == 'n' && char_to_lower(cur[1]) == 'a' &&
      char_to_lower(cur[2]) == 'n') {
    cur += 3;
    *end = cur;
    if (has_flg(NUMBER_AS_RAW)) {
      **pre = '\0';
      *pre = cur;
      val->tag = ((u64)(cur - hdr) << YYJSON_TAG_BIT) | YYJSON_TYPE_RAW;
      val->uni.str = (const char *)hdr;
    } else {
      val->tag = YYJSON_TYPE_NUM | YYJSON_SUBTYPE_REAL;
      val->uni.u64 = f64_bits_nan(sign);
    }
    return true;
  }
  return false;
}

static_inline bool read_inf_or_nan(u8 **ptr, u8 **pre, yyjson_read_flag flg,
                                   yyjson_val *val) {
  if (read_inf(ptr, pre, flg, val)) return true;
  if (read_nan(ptr, pre, flg, val)) return true;
  return false;
}

static_noinline bool read_num_raw(u8 **ptr, u8 **pre, yyjson_read_flag flg,
                                  yyjson_val *val, const char **msg) {
#define return_err(_pos, _msg) \
  do {                         \
    *msg = _msg;               \
    *end = _pos;               \
    return false;              \
  } while (false)

#define return_raw()                                                   \
  do {                                                                 \
    val->tag = ((u64)(cur - hdr) << YYJSON_TAG_BIT) | YYJSON_TYPE_RAW; \
    val->uni.str = (const char *)hdr;                                  \
    **pre = '\0';                                                      \
    *pre = cur;                                                        \
    *end = cur;                                                        \
    return true;                                                       \
  } while (false)

  u8 *hdr = *ptr;
  u8 *cur = *ptr;
  u8 **end = ptr;

  cur += (*cur == '-');

  while (unlikely(!char_is_digit(*cur))) {
    if (has_allow(EXT_NUMBER)) {
      if (*cur == '+' && cur == hdr) {
        cur++;
        continue;
      }
      if (*cur == '.' && char_is_digit(cur[1])) {
        goto read_double;
      }
    }
    if (has_allow(INF_AND_NAN)) {
      if (read_inf_or_nan(ptr, pre, flg, val)) return true;
    }
    return_err(cur, "no digit after sign");
  }

  if (*cur == '0') {
    cur++;
    if (unlikely(char_is_digit(*cur))) {
      return_err(cur - 1, "number with leading zero is not allowed");
    }
    if (!char_is_fp(*cur)) {
      if (has_allow(EXT_NUMBER) && char_to_lower(*cur) == 'x') {
        if (!char_is_hex(*++cur)) return_err(cur, "invalid hex number");
        while (char_is_hex(*cur)) cur++;
      }
      return_raw();
    }
  } else {
    while (char_is_digit(*cur)) cur++;
    if (!char_is_fp(*cur)) return_raw();
  }

read_double:

  if (*cur == '.') {
    cur++;
    if (!char_is_digit(*cur)) {
      if (has_allow(EXT_NUMBER)) {
        if (!char_is_exp(*cur)) return_raw();
      } else {
        return_err(cur, "no digit after decimal point");
      }
    }
    while (char_is_digit(*cur)) cur++;
  }

  if (char_is_exp(*cur)) {
    cur += 1 + char_is_sign(cur[1]);
    if (!char_is_digit(*cur++)) {
      return_err(cur, "no digit after exponent sign");
    }
    while (char_is_digit(*cur)) cur++;
  }

  return_raw();

#undef return_err
#undef return_raw
}

static_noinline bool read_num_hex(u8 **ptr, u8 **pre, yyjson_read_flag flg,
                                  yyjson_val *val, const char **msg) {
  u8 *hdr = *ptr;
  u8 *cur = *ptr;
  u8 **end = ptr;
  u64 sig = 0, i = 0;
  bool sign;

  sign = (*cur == '-');
  cur += (*cur == '-' || *cur == '+') + 2;

  for (; i < 16; i++) {
    u8 c = hex_conv_table[cur[i]];
    if (c == 0xF0) break;
    sig <<= 4;
    sig |= c;
  }

  if (unlikely(i == 0)) {
    *msg = "invalid hex number";
    return false;
  }

  if (unlikely(i == 16)) {
    if (char_is_hex(cur[16]) || (sign && sig > ((u64)1 << 63))) {
      if (!has_flg(BIGNUM_AS_RAW)) {
        *msg = "hex number overflow";
        return false;
      }
      cur += 16;
      while (char_is_hex(*cur)) cur++;
      **pre = '\0';
      val->tag = ((u64)(cur - hdr) << YYJSON_TAG_BIT) | YYJSON_TYPE_RAW;
      val->uni.str = (const char *)hdr;
      *pre = cur;
      *end = cur;
      return true;
    }
  }

  val->tag = YYJSON_TYPE_NUM | (u64)((u8)sign << 3);
  val->uni.u64 = (u64)(sign ? (u64)(~(sig) + 1) : (u64)(sig));
  *end = cur + i;
  return true;
}

static_noinline bool skip_trivia(u8 **ptr, u8 *eof, yyjson_read_flag flg) {
  u8 *hdr = *ptr, *cur = *ptr;
  usize len;

  while (cur < eof) {
    u8 *loop_begin = cur;

    while (char_is_space(*cur)) cur++;

    if (has_allow(EXT_WHITESPACE)) {
      while (char_is_space_ext(*cur)) {
        cur += (len = ext_space_len(cur));
        if (!len) break;
      }
    }

    if (has_allow(COMMENTS) && cur[0] == '/') {
      if (cur[1] == '/') {
        cur += 2;
        if (has_allow(EXT_WHITESPACE)) {
          while (cur < eof) {
            if (char_is_eol_ext(*cur)) {
              cur += (len = ext_eol_len(cur));
              if (len) break;
            }
            cur++;
          }
        } else {
          while (cur < eof && !char_is_eol(*cur)) cur++;
        }
      } else if (cur[1] == '*') {
        cur += 2;
        while (!byte_match_2(cur, "*/") && cur < eof) cur++;
        if (cur == eof) {
          *ptr = eof;
          return false;
        }
        cur += 2;
      }
    }
    if (cur == loop_begin) break;
  }
  *ptr = cur;
  return cur > hdr;
}

static bool is_truncated_utf8(u8 *cur, u8 *eof) {
  u8 c0, c1, c2;
  usize len = (usize)(eof - cur);
  if (cur >= eof || len >= 4) return false;
  c0 = cur[0];
  c1 = cur[1];
  c2 = cur[2];

  if (c0 < 0x80) return false;
  if (len == 1) {
    if ((c0 & 0xE0) == 0xC0 && (c0 & 0x1E) != 0x00) return true;

    if ((c0 & 0xF0) == 0xE0) return true;

    if ((c0 & 0xF8) == 0xF0 && (c0 & 0x07) <= 0x04) return true;
  } else if (len == 2) {
    if ((c0 & 0xF0) == 0xE0 && (c1 & 0xC0) == 0x80) {
      u8 t = (u8)(((c0 & 0x0F) << 1) | ((c1 & 0x20) >> 5));
      return 0x01 <= t && t != 0x1B;
    }

    if ((c0 & 0xF8) == 0xF0 && (c1 & 0xC0) == 0x80) {
      u8 t = (u8)(((c0 & 0x07) << 2) | ((c1 & 0x30) >> 4));
      return 0x01 <= t && t <= 0x10;
    }
  } else if (len == 3) {
    if ((c0 & 0xF8) == 0xF0 && (c1 & 0xC0) == 0x80 && (c2 & 0xC0) == 0x80) {
      u8 t = (u8)(((c0 & 0x07) << 2) | ((c1 & 0x30) >> 4));
      return 0x01 <= t && t <= 0x10;
    }
  }
  return false;
}

static bool is_truncated_str(u8 *cur, u8 *eof, const char *str,
                             bool case_sensitive) {
  usize len = strlen(str);
  if (cur + len <= eof || eof <= cur) return false;
  if (case_sensitive) {
    return memcmp(cur, str, (usize)(eof - cur)) == 0;
  }
  for (; cur < eof; cur++, str++) {
    if (char_to_lower(*cur) != *(const u8 *)str) return false;
  }
  return true;
}

static_noinline bool is_truncated_end(u8 *hdr, u8 *cur, u8 *eof,
                                      yyjson_read_code code,
                                      yyjson_read_flag flg) {
  if (cur >= eof) return true;
  if (code == YYJSON_READ_ERROR_LITERAL) {
    if (is_truncated_str(cur, eof, "true", true) ||
        is_truncated_str(cur, eof, "false", true) ||
        is_truncated_str(cur, eof, "null", true)) {
      return true;
    }
  }
  if (code == YYJSON_READ_ERROR_UNEXPECTED_CHARACTER ||
      code == YYJSON_READ_ERROR_INVALID_NUMBER ||
      code == YYJSON_READ_ERROR_LITERAL) {
    if (has_allow(INF_AND_NAN)) {
      if (*cur == '-') cur++;
      if (is_truncated_str(cur, eof, "infinity", false) ||
          is_truncated_str(cur, eof, "nan", false)) {
        return true;
      }
    }
  }
  if (code == YYJSON_READ_ERROR_UNEXPECTED_CONTENT) {
    if (has_allow(INF_AND_NAN)) {
      if (hdr + 3 <= cur && is_truncated_str(cur - 3, eof, "infinity", false)) {
        return true;
      }
    }
  }
  if (code == YYJSON_READ_ERROR_INVALID_STRING) {
    usize len = (usize)(eof - cur);

    if (*cur == '\\') {
      if (len == 1) return true;
      if (len <= 5) {
        if (*++cur != 'u') return false;
        for (++cur; cur < eof; cur++) {
          if (!char_is_hex(*cur)) return false;
        }
        return true;
      } else if (len <= 11) {
        u16 hi;
        if (*++cur != 'u') return false;
        if (!hex_load_4(++cur, &hi)) return false;
        if ((hi & 0xF800) != 0xD800) return false;
        cur += 4;
        if (cur >= eof) return true;

        if (*cur != '\\') return false;
        if (++cur >= eof) return true;
        if (*cur != 'u') return false;
        if (++cur >= eof) return true;
        if (*cur != 'd' && *cur != 'D') return false;
        if (++cur >= eof) return true;
        if ((*cur < 'c' || *cur > 'f') && (*cur < 'C' || *cur > 'F'))
          return false;
        if (++cur >= eof) return true;
        if (!char_is_hex(*cur)) return false;
        return true;
      }
      return false;
    }

    if (is_truncated_utf8(cur, eof)) {
      return true;
    }
  }
  if (has_allow(COMMENTS)) {
    if (code == YYJSON_READ_ERROR_INVALID_COMMENT) {
      return true;
    }
    if (code == YYJSON_READ_ERROR_UNEXPECTED_CHARACTER && *cur == '/' &&
        cur + 1 == eof) {
      return true;
    }
  }
  if (code == YYJSON_READ_ERROR_UNEXPECTED_CHARACTER && has_allow(BOM)) {
    usize len = (usize)(eof - cur);
    if (cur == hdr && len < 3 && !memcmp(hdr, "\xEF\xBB\xBF", len)) {
      return true;
    }
  }
  return false;
}

#if !YYJSON_DISABLE_FAST_FP_CONV

typedef struct bigint {
  u32 used;
  u64 bits[64];
} bigint;

static_inline void bigint_add_u64(bigint *big, u64 val) {
  u32 idx, max;
  u64 num = big->bits[0];
  u64 add = num + val;
  big->bits[0] = add;
  if (likely((add >= num) || (add >= val))) return;
  for ((void)(idx = 1), max = big->used; idx < max; idx++) {
    if (likely(big->bits[idx] != U64_MAX)) {
      big->bits[idx] += 1;
      return;
    }
    big->bits[idx] = 0;
  }
  big->bits[big->used++] = 1;
}

static_inline void bigint_mul_u64(bigint *big, u64 val) {
  u32 idx = 0, max = big->used;
  u64 hi, lo, carry = 0;
  for (; idx < max; idx++) {
    if (big->bits[idx]) break;
  }
  for (; idx < max; idx++) {
    u128_mul_add(big->bits[idx], val, carry, &hi, &lo);
    big->bits[idx] = lo;
    carry = hi;
  }
  if (carry) big->bits[big->used++] = carry;
}

static_inline void bigint_mul_pow2(bigint *big, u32 exp) {
  u32 shft = exp % 64;
  u32 move = exp / 64;
  u32 idx = big->used;
  if (unlikely(shft == 0)) {
    for (; idx > 0; idx--) {
      big->bits[idx + move - 1] = big->bits[idx - 1];
    }
    big->used += move;
    while (move) big->bits[--move] = 0;
  } else {
    big->bits[idx] = 0;
    for (; idx > 0; idx--) {
      u64 num = big->bits[idx] << shft;
      num |= big->bits[idx - 1] >> (64 - shft);
      big->bits[idx + move] = num;
    }
    big->bits[move] = big->bits[0] << shft;
    big->used += move + (big->bits[big->used + move] > 0);
    while (move) big->bits[--move] = 0;
  }
}

static_inline void bigint_mul_pow10(bigint *big, i32 exp) {
  for (; exp >= U64_POW10_MAX_EXACT_EXP; exp -= U64_POW10_MAX_EXACT_EXP) {
    bigint_mul_u64(big, u64_pow10_table[U64_POW10_MAX_EXACT_EXP]);
  }
  if (exp) {
    bigint_mul_u64(big, u64_pow10_table[exp]);
  }
}

static_inline i32 bigint_cmp(bigint *a, bigint *b) {
  u32 idx = a->used;
  if (a->used < b->used) return -1;
  if (a->used > b->used) return +1;
  while (idx-- > 0) {
    u64 av = a->bits[idx];
    u64 bv = b->bits[idx];
    if (av < bv) return -1;
    if (av > bv) return +1;
  }
  return 0;
}

static_inline void bigint_set_u64(bigint *big, u64 val) {
  big->used = 1;
  big->bits[0] = val;
}

static_noinline void bigint_set_buf(bigint *big, u64 sig, i32 *exp, u8 *sig_cut,
                                    u8 *sig_end, u8 *dot_pos) {
  if (unlikely(!sig_cut)) {
    bigint_set_u64(big, sig);
    return;

  } else {
    u8 *hdr = sig_cut;
    u8 *cur = hdr;
    u32 len = 0;
    u64 val = 0;
    bool dig_big_cut = false;
    bool has_dot = (hdr < dot_pos) & (dot_pos < sig_end);
    u32 dig_len_total = U64_SAFE_DIG + (u32)(sig_end - hdr) - has_dot;

    sig -= (*sig_cut >= '5');
    if (dig_len_total > F64_MAX_DEC_DIG) {
      dig_big_cut = true;
      sig_end -= dig_len_total - (F64_MAX_DEC_DIG + 1);
      sig_end -= (dot_pos + 1 == sig_end);
      dig_len_total = (F64_MAX_DEC_DIG + 1);
    }
    *exp -= (i32)dig_len_total - U64_SAFE_DIG;

    big->used = 1;
    big->bits[0] = sig;
    while (cur < sig_end) {
      if (likely(cur != dot_pos)) {
        val = val * 10 + (u8)(*cur++ - '0');
        len++;
        if (unlikely(cur == sig_end && dig_big_cut)) {
          val = val - (val % 10) + 1;
        }
        if (len == U64_SAFE_DIG || cur == sig_end) {
          bigint_mul_pow10(big, (i32)len);
          bigint_add_u64(big, val);
          val = 0;
          len = 0;
        }
      } else {
        cur++;
      }
    }
  }
}

typedef struct diy_fp {
  u64 sig;
  i32 exp;
  i32 pad;
} diy_fp;

static_inline diy_fp diy_fp_get_cached_pow10(i32 exp10) {
  diy_fp fp;
  u64 sig_ext;
  pow10_table_get_sig(exp10, &fp.sig, &sig_ext);
  pow10_table_get_exp(exp10, &fp.exp);
  fp.sig += (sig_ext >> 63);
  return fp;
}

static_inline diy_fp diy_fp_mul(diy_fp fp, diy_fp fp2) {
  u64 hi, lo;
  u128_mul(fp.sig, fp2.sig, &hi, &lo);
  fp.sig = hi + (lo >> 63);
  fp.exp += fp2.exp + 64;
  return fp;
}

static_inline u64 diy_fp_to_ieee_raw(diy_fp fp) {
  u64 sig = fp.sig;
  i32 exp = fp.exp;
  u32 lz_bits;
  if (unlikely(fp.sig == 0)) return 0;

  lz_bits = u64_lz_bits(sig);
  sig <<= lz_bits;
  sig >>= F64_BITS - F64_SIG_FULL_BITS;
  exp -= (i32)lz_bits;
  exp += F64_BITS - F64_SIG_FULL_BITS;
  exp += F64_SIG_BITS;

  if (unlikely(exp >= F64_MAX_BIN_EXP)) {
    return F64_BITS_INF;
  } else if (likely(exp >= F64_MIN_BIN_EXP - 1)) {
    exp += F64_EXP_BIAS;
    return ((u64)exp << F64_SIG_BITS) | (sig & F64_SIG_MASK);
  } else if (likely(exp >= F64_MIN_BIN_EXP - F64_SIG_FULL_BITS)) {
    return sig >> (F64_MIN_BIN_EXP - exp - 1);
  } else {
    return 0;
  }
}

static_inline bool read_num(u8 **ptr, u8 **pre, yyjson_read_flag flg,
                            yyjson_val *val, const char **msg) {
#define return_err(_pos, _msg) \
  do {                         \
    *msg = _msg;               \
    *end = _pos;               \
    return false;              \
  } while (false)

#define return_0()                                    \
  do {                                                \
    val->tag = YYJSON_TYPE_NUM | (u8)((u8)sign << 3); \
    val->uni.u64 = 0;                                 \
    *end = cur;                                       \
    return true;                                      \
  } while (false)

#define return_i64(_v)                                         \
  do {                                                         \
    val->tag = YYJSON_TYPE_NUM | (u8)((u8)sign << 3);          \
    val->uni.u64 = (u64)(sign ? (u64)(~(_v) + 1) : (u64)(_v)); \
    *end = cur;                                                \
    return true;                                               \
  } while (false)

#define return_f64(_v)                                \
  do {                                                \
    val->tag = YYJSON_TYPE_NUM | YYJSON_SUBTYPE_REAL; \
    val->uni.f64 = sign ? -(f64)(_v) : (f64)(_v);     \
    *end = cur;                                       \
    return true;                                      \
  } while (false)

#define return_f64_bin(_v)                            \
  do {                                                \
    val->tag = YYJSON_TYPE_NUM | YYJSON_SUBTYPE_REAL; \
    val->uni.u64 = ((u64)sign << 63) | (u64)(_v);     \
    *end = cur;                                       \
    return true;                                      \
  } while (false)

#define return_inf()                                               \
  do {                                                             \
    if (has_flg(BIGNUM_AS_RAW)) return_raw();                      \
    if (has_allow(INF_AND_NAN))                                    \
      return_f64_bin(F64_BITS_INF);                                \
    else                                                           \
      return_err(hdr, "number is infinity when parsed as double"); \
  } while (false)

#define return_raw()                                                   \
  do {                                                                 \
    **pre = '\0';                                                      \
    val->tag = ((u64)(cur - hdr) << YYJSON_TAG_BIT) | YYJSON_TYPE_RAW; \
    val->uni.str = (const char *)hdr;                                  \
    *pre = cur;                                                        \
    *end = cur;                                                        \
    return true;                                                       \
  } while (false)

  u8 *sig_cut = NULL;
  u8 *sig_end = NULL;
  u8 *dot_pos = NULL;

  u64 sig = 0;
  i32 exp = 0;

  bool exp_sign;
  i64 exp_sig = 0;
  i64 exp_lit = 0;
  u64 num;
  u8 *tmp;

  u8 *hdr = *ptr;
  u8 *cur = *ptr;
  u8 **end = ptr;
  bool sign;

  if (has_flg(NUMBER_AS_RAW)) {
    return read_num_raw(ptr, pre, flg, val, msg);
  }

  sign = (*hdr == '-');
  cur += sign;

  while (unlikely(!char_is_nonzero(*cur))) {
    if (unlikely(*cur != '0')) {
      if (has_allow(EXT_NUMBER)) {
        if (*cur == '+' && cur == hdr) {
          cur++;
          continue;
        }
        if (*cur == '.' && char_is_digit(cur[1])) {
          goto leading_dot;
        }
      }
      if (has_allow(INF_AND_NAN)) {
        if (read_inf_or_nan(ptr, pre, flg, val)) return true;
      }
      return_err(cur, "no digit after sign");
    }

    if (likely(!char_is_digit_or_fp(*++cur))) {
      if (has_allow(EXT_NUMBER) && char_to_lower(*cur) == 'x') {
        return read_num_hex(ptr, pre, flg, val, msg);
      }
      return_0();
    }
    if (likely(*cur == '.')) {
    leading_dot:
      dot_pos = cur++;
      if (unlikely(!char_is_digit(*cur))) {
        if (has_allow(EXT_NUMBER)) {
          if (char_is_exp(*cur)) {
            goto digi_exp_more;
          } else {
            return_f64_bin(0);
          }
        }
        return_err(cur, "no digit after decimal point");
      }
      while (unlikely(*cur == '0')) cur++;
      if (likely(char_is_digit(*cur))) {
        sig = (u64)(*cur - '0');
        cur--;
        goto digi_frac_1;
      }
    }
    if (unlikely(char_is_digit(*cur))) {
      return_err(cur - 1, "number with leading zero is not allowed");
    }
    if (unlikely(char_is_exp(*cur))) {
      cur += (usize)1 + char_is_sign(cur[1]);
      if (unlikely(!char_is_digit(*cur))) {
        return_err(cur, "no digit after exponent sign");
      }
      while (char_is_digit(*++cur));
    }
    return_f64_bin(0);
  }

  sig = (u64)(*cur - '0');

#define expr_intg(i)                                \
  if (likely((num = (u64)(cur[i] - (u8)'0')) <= 9)) \
    sig = num + sig * 10;                           \
  else {                                            \
    goto digi_sepr_##i;                             \
  }
  repeat_in_1_18(expr_intg)
#undef expr_intg

      cur += 19;
  if (!char_is_digit_or_fp(*cur)) {
    if (sign && (sig > ((u64)1 << 63))) {
      if (has_flg(BIGNUM_AS_RAW)) return_raw();
      return_f64(unsafe_yyjson_u64_to_f64(sig));
    }
    return_i64(sig);
  }
  goto digi_intg_more;

#define expr_sepr(i)                                 \
  digi_sepr_##i : if (likely(!char_is_fp(cur[i]))) { \
    cur += i;                                        \
    return_i64(sig);                                 \
  }                                                  \
  dot_pos = cur + i;                                 \
  if (likely(cur[i] == '.')) goto digi_frac_##i;     \
  cur += i;                                          \
  sig_end = cur;                                     \
  goto digi_exp_more;
  repeat_in_1_18(expr_sepr)
#undef expr_sepr

#define expr_frac(i)                                                          \
  digi_frac_##i : if (likely((num = (u64)(cur[i + 1] - (u8)'0')) <= 9)) sig = \
                      num + sig * 10;                                         \
  else {                                                                      \
    goto digi_stop_##i;                                                       \
  }
      repeat_in_1_18(expr_frac)
#undef expr_frac

          cur += 20;
  if (!char_is_digit(*cur)) goto digi_frac_end;
  goto digi_frac_more;

#define expr_stop(i)            \
  digi_stop_##i : cur += i + 1; \
  goto digi_frac_end;
  repeat_in_1_18(expr_stop)
#undef expr_stop

      digi_intg_more : if (char_is_digit(*cur)) {
    if (!char_is_digit_or_fp(cur[1])) {
      num = (u64)(*cur - '0');
      if ((sig < (U64_MAX / 10)) ||
          (sig == (U64_MAX / 10) && num <= (U64_MAX % 10))) {
        sig = num + sig * 10;
        cur++;

        if (sign) {
          if (has_flg(BIGNUM_AS_RAW)) return_raw();
          return_f64(unsafe_yyjson_u64_to_f64(sig));
        }
        return_i64(sig);
      }
    }
  }

  if (char_is_exp(*cur)) {
    dot_pos = cur;
    goto digi_exp_more;
  }

  if (*cur == '.') {
    dot_pos = cur++;
    if (unlikely(!char_is_digit(*cur))) {
      if (has_allow(EXT_NUMBER)) {
        goto digi_frac_end;
      }
      return_err(cur, "no digit after decimal point");
    }
  }

digi_frac_more:
  sig_cut = cur;
  sig += (*cur >= '5');
  while (char_is_digit(*++cur));
  if (!dot_pos) {
    if (!char_is_fp(*cur) && has_flg(BIGNUM_AS_RAW)) {
      return_raw();
    }
    dot_pos = cur;
    if (*cur == '.') {
      if (unlikely(!char_is_digit(*++cur))) {
        if (!has_allow(EXT_NUMBER)) {
          return_err(cur, "no digit after decimal point");
        }
      }
      while (char_is_digit(*cur)) cur++;
    }
  }
  exp_sig = (i64)(dot_pos - sig_cut);
  exp_sig += (dot_pos < sig_cut);

  tmp = cur - 1;
  while ((*tmp == '0' || *tmp == '.') && tmp > hdr) tmp--;
  if (tmp < sig_cut) {
    sig_cut = NULL;
  } else {
    sig_end = cur;
  }

  if (char_is_exp(*cur)) goto digi_exp_more;
  goto digi_exp_finish;

digi_frac_end:
  if (unlikely(dot_pos + 1 == cur)) {
    if (!has_allow(EXT_NUMBER)) {
      return_err(cur, "no digit after decimal point");
    }
  }
  sig_end = cur;
  exp_sig = -(i64)((u64)(cur - dot_pos) - 1);
  if (likely(!char_is_exp(*cur))) {
    if (unlikely(exp_sig < F64_MIN_DEC_EXP - 19)) {
      return_f64_bin(0);
    }
    exp = (i32)exp_sig;
    goto digi_finish;
  } else {
    goto digi_exp_more;
  }

digi_exp_more:
  exp_sign = (*++cur == '-');
  cur += char_is_sign(*cur);
  if (unlikely(!char_is_digit(*cur))) {
    return_err(cur, "no digit after exponent sign");
  }
  while (*cur == '0') cur++;

  tmp = cur;
  while (char_is_digit(*cur)) {
    exp_lit = (i64)((u8)(*cur++ - '0') + (u64)exp_lit * 10);
  }
  if (unlikely(cur - tmp >= U64_SAFE_DIG)) {
    if (exp_sign) {
      return_f64_bin(0);
    } else {
      return_inf();
    }
  }
  exp_sig += exp_sign ? -exp_lit : exp_lit;

digi_exp_finish:
  if (unlikely(exp_sig < F64_MIN_DEC_EXP - 19)) {
    return_f64_bin(0);
  }
  if (unlikely(exp_sig > F64_MAX_DEC_EXP)) {
    return_inf();
  }
  exp = (i32)exp_sig;

digi_finish:

#if YYJSON_DOUBLE_MATH_CORRECT
  if (sig < ((u64)1 << 53) && exp >= -F64_POW10_MAX_EXACT_EXP &&
      exp <= +F64_POW10_MAX_EXACT_EXP) {
    f64 dbl = (f64)sig;
    if (exp < 0) {
      dbl /= f64_pow10_table[-exp];
    } else {
      dbl *= f64_pow10_table[+exp];
    }
    return_f64(dbl);
  }
#endif
  if (unlikely(sig == 0)) return_f64_bin(0);

  if (likely(!sig_cut && exp > -F64_MAX_DEC_EXP + 1 &&
             exp < +F64_MAX_DEC_EXP - 20)) {
    u64 raw;
    u64 sig1, sig2, sig2_ext, hi, lo, hi2, lo2, add, bits;
    i32 exp2;
    u32 lz;
    bool exact = false, carry, round_up;

    pow10_table_get_sig(exp, &sig2, &sig2_ext);
    pow10_table_get_exp(exp, &exp2);

    lz = u64_lz_bits(sig);
    sig1 = sig << lz;
    exp2 -= (i32)lz;
    u128_mul(sig1, sig2, &hi, &lo);

    bits = hi & (((u64)1 << (64 - 54 - 1)) - 1);
    if (bits - 1 < (((u64)1 << (64 - 54 - 1)) - 2)) {
      exact = true;

    } else {
      u128_mul(sig1, sig2_ext, &hi2, &lo2);

      add = lo + hi2;
      if (add + 1 > (u64)1) {
        carry = add < lo || add < hi2;
        hi += carry;
        exact = true;
      }
    }

    if (exact) {
      lz = hi < ((u64)1 << 63);
      hi <<= lz;
      exp2 -= (i32)lz;
      exp2 += 64;

      round_up = (hi & ((u64)1 << (64 - 54))) > (u64)0;
      hi += (round_up ? ((u64)1 << (64 - 54)) : (u64)0);

      if (hi < ((u64)1 << (64 - 54))) {
        hi = ((u64)1 << 63);
        exp2 += 1;
      }

      hi >>= F64_BITS - F64_SIG_FULL_BITS;
      exp2 += F64_BITS - F64_SIG_FULL_BITS + F64_SIG_BITS;
      exp2 += F64_EXP_BIAS;
      raw = ((u64)exp2 << F64_SIG_BITS) | (hi & F64_SIG_MASK);
      return_f64_bin(raw);
    }
  }

  {
    const i32 ERR_ULP_LOG = 3;
    const i32 ERR_ULP = 1 << ERR_ULP_LOG;
    const i32 ERR_CACHED_POW = ERR_ULP / 2;
    const i32 ERR_MUL_FIXED = ERR_ULP / 2;
    const i32 DIY_SIG_BITS = 64;
    const i32 EXP_BIAS = F64_EXP_BIAS + F64_SIG_BITS;
    const i32 EXP_SUBNORMAL = -EXP_BIAS + 1;

    u64 fp_err;
    u32 bits;
    i32 order_of_magnitude;
    i32 effective_significand_size;
    i32 precision_digits_count;
    u64 precision_bits;
    u64 half_way;

    u64 raw;
    diy_fp fp, fp_upper;
    bigint big_full, big_comp;
    i32 cmp;

    fp.sig = sig;
    fp.exp = 0;
    fp_err = sig_cut ? (u64)(ERR_ULP / 2) : (u64)0;

    bits = u64_lz_bits(fp.sig);
    fp.sig <<= bits;
    fp.exp -= (i32)bits;
    fp_err <<= bits;

    fp = diy_fp_mul(fp, diy_fp_get_cached_pow10(exp));
    fp_err += (u64)ERR_CACHED_POW + (fp_err != 0) + (u64)ERR_MUL_FIXED;

    bits = u64_lz_bits(fp.sig);
    fp.sig <<= bits;
    fp.exp -= (i32)bits;
    fp_err <<= bits;

    order_of_magnitude = DIY_SIG_BITS + fp.exp;
    if (likely(order_of_magnitude >= EXP_SUBNORMAL + F64_SIG_FULL_BITS)) {
      effective_significand_size = F64_SIG_FULL_BITS;
    } else if (order_of_magnitude <= EXP_SUBNORMAL) {
      effective_significand_size = 0;
    } else {
      effective_significand_size = order_of_magnitude - EXP_SUBNORMAL;
    }

    precision_digits_count = DIY_SIG_BITS - effective_significand_size;
    if (unlikely(precision_digits_count + ERR_ULP_LOG >= DIY_SIG_BITS)) {
      i32 shr = (precision_digits_count + ERR_ULP_LOG) - DIY_SIG_BITS + 1;
      fp.sig >>= shr;
      fp.exp += shr;
      fp_err = (fp_err >> shr) + 1 + (u32)ERR_ULP;
      precision_digits_count -= shr;
    }

    precision_bits = fp.sig & (((u64)1 << precision_digits_count) - 1);
    precision_bits *= (u32)ERR_ULP;
    half_way = (u64)1 << (precision_digits_count - 1);
    half_way *= (u32)ERR_ULP;

    fp.sig >>= precision_digits_count;
    fp.sig += (precision_bits >= half_way + fp_err);
    fp.exp += precision_digits_count;

    raw = diy_fp_to_ieee_raw(fp);
    if (unlikely(raw == F64_BITS_INF)) return_inf();
    if (likely(precision_bits <= half_way - fp_err ||
               precision_bits >= half_way + fp_err)) {
      return_f64_bin(raw);
    }

    if (raw & F64_EXP_MASK) {
      fp_upper.sig = (raw & F64_SIG_MASK) + ((u64)1 << F64_SIG_BITS);
      fp_upper.exp = (i32)((raw & F64_EXP_MASK) >> F64_SIG_BITS);
    } else {
      fp_upper.sig = (raw & F64_SIG_MASK);
      fp_upper.exp = 1;
    }
    fp_upper.exp -= F64_EXP_BIAS + F64_SIG_BITS;
    fp_upper.sig <<= 1;
    fp_upper.exp -= 1;
    fp_upper.sig += 1;

    bigint_set_buf(&big_full, sig, &exp, sig_cut, sig_end, dot_pos);
    bigint_set_u64(&big_comp, fp_upper.sig);
    if (exp >= 0) {
      bigint_mul_pow10(&big_full, +exp);
    } else {
      bigint_mul_pow10(&big_comp, -exp);
    }
    if (fp_upper.exp > 0) {
      bigint_mul_pow2(&big_comp, (u32) + fp_upper.exp);
    } else {
      bigint_mul_pow2(&big_full, (u32)-fp_upper.exp);
    }
    cmp = bigint_cmp(&big_full, &big_comp);
    if (likely(cmp != 0)) {
      raw += (cmp > 0);
    } else {
      raw += (raw & 1);
    }

    if (unlikely(raw == F64_BITS_INF)) return_inf();
    return_f64_bin(raw);
  }

#undef return_err
#undef return_inf
#undef return_0
#undef return_i64
#undef return_f64
#undef return_f64_bin
#undef return_raw
}

#else

static_inline bool read_num(u8 **ptr, u8 **pre, yyjson_read_flag flg,
                            yyjson_val *val, const char **msg) {
#define return_err(_pos, _msg) \
  do {                         \
    *msg = _msg;               \
    *end = _pos;               \
    return false;              \
  } while (false)

#define return_0()                                     \
  do {                                                 \
    val->tag = YYJSON_TYPE_NUM | (u64)((u8)sign << 3); \
    val->uni.u64 = 0;                                  \
    *end = cur;                                        \
    return true;                                       \
  } while (false)

#define return_i64(_v)                                         \
  do {                                                         \
    val->tag = YYJSON_TYPE_NUM | (u64)((u8)sign << 3);         \
    val->uni.u64 = (u64)(sign ? (u64)(~(_v) + 1) : (u64)(_v)); \
    *end = cur;                                                \
    return true;                                               \
  } while (false)

#define return_f64(_v)                                \
  do {                                                \
    val->tag = YYJSON_TYPE_NUM | YYJSON_SUBTYPE_REAL; \
    val->uni.f64 = sign ? -(f64)(_v) : (f64)(_v);     \
    *end = cur;                                       \
    return true;                                      \
  } while (false)

#define return_f64_bin(_v)                            \
  do {                                                \
    val->tag = YYJSON_TYPE_NUM | YYJSON_SUBTYPE_REAL; \
    val->uni.u64 = ((u64)sign << 63) | (u64)(_v);     \
    *end = cur;                                       \
    return true;                                      \
  } while (false)

#define return_inf()                                               \
  do {                                                             \
    if (has_flg(BIGNUM_AS_RAW)) return_raw();                      \
    if (has_allow(INF_AND_NAN))                                    \
      return_f64_bin(F64_BITS_INF);                                \
    else                                                           \
      return_err(hdr, "number is infinity when parsed as double"); \
  } while (false)

#define return_raw()                                                   \
  do {                                                                 \
    val->tag = ((u64)(cur - hdr) << YYJSON_TAG_BIT) | YYJSON_TYPE_RAW; \
    val->uni.str = (const char *)hdr;                                  \
    **pre = '\0';                                                      \
    *pre = cur;                                                        \
    *end = cur;                                                        \
    return true;                                                       \
  } while (false)

  u64 sig, num;
  u8 *hdr = *ptr;
  u8 *cur = *ptr;
  u8 **end = ptr;
  u8 *dot = NULL;
  u8 *f64_end = NULL;
  bool sign;

  if (has_flg(NUMBER_AS_RAW)) {
    return read_num_raw(ptr, pre, flg, val, msg);
  }

  sign = (*hdr == '-');
  cur += sign;
  sig = (u8)(*cur - '0');

  while (unlikely(!char_is_digit(*cur))) {
    if (has_allow(EXT_NUMBER)) {
      if (*cur == '+' && cur == hdr) {
        cur++;
        sig = (u8)(*cur - '0');
        continue;
      }
      if (*cur == '.' && char_is_num(cur[1])) {
        goto read_double;
      }
    }
    if (has_allow(INF_AND_NAN)) {
      if (read_inf_or_nan(ptr, pre, flg, val)) return true;
    }
    return_err(cur, "no digit after sign");
  }
  if (*cur == '0') {
    cur++;
    if (unlikely(char_is_digit(*cur))) {
      return_err(cur - 1, "number with leading zero is not allowed");
    }
    if (!char_is_fp(*cur)) {
      if (has_allow(EXT_NUMBER) && (*cur == 'x' || *cur == 'X')) {
        return read_num_hex(ptr, pre, flg, val, msg);
      }
      return_0();
    }
    goto read_double;
  }

#define expr_intg(i)                                \
  if (likely((num = (u64)(cur[i] - (u8)'0')) <= 9)) \
    sig = num + sig * 10;                           \
  else {                                            \
    cur += i;                                       \
    goto intg_end;                                  \
  }
  repeat_in_1_18(expr_intg)
#undef expr_intg

      cur += 19;
  if (char_is_digit(cur[0]) && !char_is_digit_or_fp(cur[1])) {
    num = (u8)(*cur - '0');
    if ((sig < (U64_MAX / 10)) ||
        (sig == (U64_MAX / 10) && num <= (U64_MAX % 10))) {
      sig = num + sig * 10;
      cur++;
      if (sign) {
        if (has_flg(BIGNUM_AS_RAW)) return_raw();
        return_f64(unsafe_yyjson_u64_to_f64(sig));
      }
      return_i64(sig);
    }
  }

intg_end:

  if (!char_is_digit_or_fp(*cur)) {
    if (sign && (sig > ((u64)1 << 63))) {
      if (has_flg(BIGNUM_AS_RAW)) return_raw();
      return_f64(unsafe_yyjson_u64_to_f64(sig));
    }
    return_i64(sig);
  }

read_double:

  while (char_is_digit(*cur)) cur++;
  if (!char_is_fp(*cur) && has_flg(BIGNUM_AS_RAW)) {
    return_raw();
  }
  while (*cur == '.') {
    dot = cur;
    cur++;
    if (!char_is_digit(*cur)) {
      if (has_allow(EXT_NUMBER)) {
        break;
      } else {
        return_err(cur, "no digit after decimal point");
      }
    }
    cur++;
    while (char_is_digit(*cur)) cur++;
    break;
  }
  if (char_is_exp(*cur)) {
    cur += 1 + char_is_sign(cur[1]);
    if (!char_is_digit(*cur)) {
      return_err(cur, "no digit after exponent sign");
    }
    cur++;
    while (char_is_digit(*cur)) cur++;
  }

  val->uni.f64 = strtod((const char *)hdr, (char **)&f64_end);
  if (unlikely(f64_end != cur)) {
    bool cut = (*cur == ',');
    if (cut) *cur = ' ';
    if (dot) *dot = ',';
    val->uni.f64 = strtod((const char *)hdr, (char **)&f64_end);

    if (cut) *cur = ',';
    if (dot) *dot = '.';
    if (unlikely(f64_end != cur)) {
      return_err(hdr, "strtod() failed to parse the number");
    }
  }
  if (unlikely(val->uni.f64 >= HUGE_VAL || val->uni.f64 <= -HUGE_VAL)) {
    return_inf();
  }
  val->tag = YYJSON_TYPE_NUM | YYJSON_SUBTYPE_REAL;
  *end = cur;
  return true;

#undef return_err
#undef return_0
#undef return_i64
#undef return_f64
#undef return_f64_bin
#undef return_inf
#undef return_raw
}

#endif

static_inline bool read_uni_esc(u8 **src_ptr, u8 **dst_ptr, const char **msg) {
#define return_err(_end, _msg) \
  *msg = _msg;                 \
  *src_ptr = _end;             \
  return false

  u8 *src = *src_ptr;
  u8 *dst = *dst_ptr;
  u16 hi, lo;
  u32 uni;

  src += 2;
  if (unlikely(!hex_load_4(src, &hi))) {
    return_err(src - 2, "invalid escaped sequence in string");
  }
  src += 4;
  if (likely((hi & 0xF800) != 0xD800)) {
    if (hi >= 0x800) {
      *dst++ = (u8)(0xE0 | (hi >> 12));
      *dst++ = (u8)(0x80 | ((hi >> 6) & 0x3F));
      *dst++ = (u8)(0x80 | (hi & 0x3F));
    } else if (hi >= 0x80) {
      *dst++ = (u8)(0xC0 | (hi >> 6));
      *dst++ = (u8)(0x80 | (hi & 0x3F));
    } else {
      *dst++ = (u8)hi;
    }
  } else {
    if (unlikely((hi & 0xFC00) != 0xD800)) {
      return_err(src - 6, "invalid high surrogate in string");
    }
    if (unlikely(!byte_match_2(src, "\\u"))) {
      return_err(src - 6, "no low surrogate in string");
    }
    if (unlikely(!hex_load_4(src + 2, &lo))) {
      return_err(src - 6, "invalid escape in string");
    }
    if (unlikely((lo & 0xFC00) != 0xDC00)) {
      return_err(src - 6, "invalid low surrogate in string");
    }
    uni = ((((u32)hi - 0xD800) << 10) | ((u32)lo - 0xDC00)) + 0x10000;
    *dst++ = (u8)(0xF0 | (uni >> 18));
    *dst++ = (u8)(0x80 | ((uni >> 12) & 0x3F));
    *dst++ = (u8)(0x80 | ((uni >> 6) & 0x3F));
    *dst++ = (u8)(0x80 | (uni & 0x3F));
    src += 6;
  }
  *src_ptr = src;
  *dst_ptr = dst;
  return true;
#undef return_err
}

static_inline bool read_str_opt(u8 quo, u8 **ptr, u8 *eof, yyjson_read_flag flg,
                                yyjson_val *val, const char **msg, u8 *con[2]) {
#define return_err(_end, _msg) \
  do {                         \
    *msg = _msg;               \
    *end = _end;               \
    if (con) {                 \
      con[0] = _end;           \
      con[1] = dst;            \
    }                          \
    return false;              \
  } while (false)

  u8 *hdr = *ptr + 1;
  u8 **end = ptr;
  u8 *src = hdr, *dst = NULL, *pos;
  u32 uni, tmp;

  if (con && unlikely(con[0])) {
    src = con[0];
    dst = con[1];
    if (dst) goto copy_ascii;
  }

skip_ascii:

  if (quo == '"') {
#define expr_jump(i)                        \
  if (likely(char_is_ascii_skip(src[i]))) { \
  } else                                    \
    goto skip_ascii_stop##i;

#define expr_stop(i)             \
  skip_ascii_stop##i : src += i; \
  goto skip_ascii_end;

    repeat16_incr(expr_jump) src += 16;
    goto skip_ascii;
    repeat16_incr(expr_stop)

#undef expr_jump
#undef expr_stop
  } else {
#define expr_jump(i)                           \
  if (likely(char_is_ascii_skip_sq(src[i]))) { \
  } else                                       \
    goto skip_ascii_stop_sq##i;

#define expr_stop(i)                \
  skip_ascii_stop_sq##i : src += i; \
  goto skip_ascii_end;

    repeat16_incr(expr_jump) src += 16;
    goto skip_ascii;
    repeat16_incr(expr_stop)

#undef expr_jump
#undef expr_stop
  }

skip_ascii_end:
  gcc_store_barrier(*src);
  if (likely(*src == quo)) {
    val->tag = ((u64)(src - hdr) << YYJSON_TAG_BIT) | YYJSON_TYPE_STR |
               (quo == '"' ? YYJSON_SUBTYPE_NOESC : 0);
    val->uni.str = (const char *)hdr;
    *src = '\0';
    *end = src + 1;
    if (con) con[0] = con[1] = NULL;
    return true;
  }

skip_utf8:
  if (*src & 0x80) {
    pos = src;
#if YYJSON_DISABLE_UTF8_VALIDATION
    while (true)
      repeat8({
        if (likely((*src & 0xF0) == 0xE0))
          src += 3;
        else
          break;
      }) if (*src < 0x80) goto skip_ascii;
    while (true)
      repeat8({
        if (likely((*src & 0xE0) == 0xC0))
          src += 2;
        else
          break;
      }) while (true) repeat8({
        if (likely((*src & 0xF8) == 0xF0))
          src += 4;
        else
          break;
      })
#else
    uni = byte_load_4(src);
    while (is_utf8_seq3(uni)) {
      src += 3;
      uni = byte_load_4(src);
    }
    if (is_utf8_seq1(uni)) goto skip_ascii;
    while (is_utf8_seq2(uni)) {
      src += 2;
      uni = byte_load_4(src);
    }
    while (is_utf8_seq4(uni)) {
      src += 4;
      uni = byte_load_4(src);
    }
#endif
          if (unlikely(pos == src)) {
        if (has_allow(INVALID_UNICODE))
          ++src;
        else
          return_err(src, "invalid UTF-8 encoding in string");
      }
    goto skip_ascii;
  }

  dst = src;
copy_escape:
  if (likely(*src == '\\')) {
    switch (*++src) {
      case '"':
        *dst++ = '"';
        src++;
        break;
      case '\\':
        *dst++ = '\\';
        src++;
        break;
      case '/':
        *dst++ = '/';
        src++;
        break;
      case 'b':
        *dst++ = '\b';
        src++;
        break;
      case 'f':
        *dst++ = '\f';
        src++;
        break;
      case 'n':
        *dst++ = '\n';
        src++;
        break;
      case 'r':
        *dst++ = '\r';
        src++;
        break;
      case 't':
        *dst++ = '\t';
        src++;
        break;
      case 'u':
        src--;
        if (!read_uni_esc(&src, &dst, msg)) return_err(src, *msg);
        break;
      default: {
        if (has_allow(EXT_ESCAPE)) {
          switch (*src) {
            case '\'':
              *dst++ = '\'';
              src++;
              break;
            case 'a':
              *dst++ = '\a';
              src++;
              break;
            case 'v':
              *dst++ = '\v';
              src++;
              break;
            case '?':
              *dst++ = '\?';
              src++;
              break;
            case 'e':
              *dst++ = 0x1B;
              src++;
              break;
            case '0':
              if (!char_is_digit(src[1])) {
                *dst++ = '\0';
                src++;
                break;
              }
              return_err(src - 1, "octal escape is not allowed");
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
              return_err(src - 1, "invalid number escape");
            case 'x': {
              u8 c;
              if (hex_load_2(src + 1, &c)) {
                src += 3;
                if (c <= 0x7F) {
                  *dst++ = c;
                } else {
                  *dst++ = (u8)(0xC0 | (c >> 6));
                  *dst++ = (u8)(0x80 | (c & 0x3F));
                }
                break;
              }
              return_err(src - 1, "invalid hex escape");
            }
            case '\n':
              src++;
              break;
            case '\r':
              src++;
              src += (*src == '\n');
              break;
            case 0xE2:
              if ((src[1] == 0x80 && src[2] == 0xA8) ||
                  (src[1] == 0x80 && src[2] == 0xA9)) {
                src += 3;
              }
              break;
            default:
              break;
          }
        } else if (quo == '\'' && *src == '\'') {
          *dst++ = '\'';
          src++;
          break;
        } else {
          return_err(src - 1, "invalid escaped sequence in string");
        }
      }
    }
  } else if (likely(*src == quo)) {
    val->tag = ((u64)(dst - hdr) << YYJSON_TAG_BIT) | YYJSON_TYPE_STR;
    val->uni.str = (const char *)hdr;
    *dst = '\0';
    *end = src + 1;
    if (con) con[0] = con[1] = NULL;
    return true;
  } else {
    if (!has_allow(INVALID_UNICODE)) {
      return_err(src, "unexpected control character in string");
    }
    if (src >= eof) return_err(src, "unclosed string");
    *dst++ = *src++;
  }

copy_ascii:

  if (quo == '"') {
#define expr_jump(i)                          \
  if (likely((char_is_ascii_skip(src[i])))) { \
  } else {                                    \
    gcc_store_barrier(src[i]);                \
    goto copy_ascii_stop_##i;                 \
  }
    repeat16_incr(expr_jump)
#undef expr_jump
  } else {
#define expr_jump(i)                             \
  if (likely((char_is_ascii_skip_sq(src[i])))) { \
  } else {                                       \
    gcc_store_barrier(src[i]);                   \
    goto copy_ascii_stop_##i;                    \
  }
    repeat16_incr(expr_jump)
#undef expr_jump
  }

  byte_move_16(dst, src);
  dst += 16;
  src += 16;
  goto copy_ascii;

#define expr_jump(i)                                    \
  copy_ascii_stop_##i : byte_move_forward(dst, src, i); \
  dst += i;                                             \
  src += i;                                             \
  goto copy_utf8;
  repeat16_incr(expr_jump)
#undef expr_jump

      copy_utf8 : if (*src & 0x80) {
    pos = src;
    uni = byte_load_4(src);
#if YYJSON_DISABLE_UTF8_VALIDATION
    while (true)
      repeat4({
        if ((uni & utf8_seq(b3_mask)) == utf8_seq(b3_patt)) {
          byte_copy_4(dst, &uni);
          dst += 3;
          src += 3;
          uni = byte_load_4(src);
        } else
          break;
      }) if ((uni & utf8_seq(b1_mask)) == utf8_seq(b1_patt)) goto copy_ascii;
    while (true)
      repeat4({
        if ((uni & utf8_seq(b2_mask)) == utf8_seq(b2_patt)) {
          byte_copy_2(dst, &uni);
          dst += 2;
          src += 2;
          uni = byte_load_4(src);
        } else
          break;
      }) while (true) repeat4({
        if ((uni & utf8_seq(b4_mask)) == utf8_seq(b4_patt)) {
          byte_copy_4(dst, &uni);
          dst += 4;
          src += 4;
          uni = byte_load_4(src);
        } else
          break;
      })
#else
    while (is_utf8_seq3(uni)) {
      byte_copy_4(dst, &uni);
      dst += 3;
      src += 3;
      uni = byte_load_4(src);
    }
    if (is_utf8_seq1(uni)) goto copy_ascii;
    while (is_utf8_seq2(uni)) {
      byte_copy_2(dst, &uni);
      dst += 2;
      src += 2;
      uni = byte_load_4(src);
    }
    while (is_utf8_seq4(uni)) {
      byte_copy_4(dst, &uni);
      dst += 4;
      src += 4;
      uni = byte_load_4(src);
    }
#endif
          if (unlikely(pos == src)) {
        if (!has_allow(INVALID_UNICODE)) {
          return_err(src, MSG_ERR_UTF8);
        }
        goto copy_ascii_stop_1;
      }
    goto copy_ascii;
  }
  goto copy_escape;

#undef return_err
}

static_inline bool read_str(u8 **ptr, u8 *eof, yyjson_read_flag flg,
                            yyjson_val *val, const char **msg) {
  return read_str_opt('\"', ptr, eof, flg, val, msg, NULL);
}

static_inline bool read_str_con(u8 **ptr, u8 *eof, yyjson_read_flag flg,
                                yyjson_val *val, const char **msg, u8 **con) {
  return read_str_opt('\"', ptr, eof, flg, val, msg, con);
}

static_noinline bool read_str_sq(u8 **ptr, u8 *eof, yyjson_read_flag flg,
                                 yyjson_val *val, const char **msg) {
  return read_str_opt('\'', ptr, eof, flg, val, msg, NULL);
}

static_noinline bool read_str_id(u8 **ptr, u8 *eof, yyjson_read_flag flg,
                                 u8 **pre, yyjson_val *val, const char **msg) {
#define return_err(_end, _msg) \
  do {                         \
    *msg = _msg;               \
    *end = _end;               \
    return false;              \
  } while (false)

#define return_suc(_str_end, _cur_end)                                      \
  do {                                                                      \
    val->tag =                                                              \
        ((u64)(_str_end - hdr) << YYJSON_TAG_BIT) | (u64)(YYJSON_TYPE_STR); \
    val->uni.str = (const char *)hdr;                                       \
    *pre = _str_end;                                                        \
    *end = _cur_end;                                                        \
    return true;                                                            \
  } while (false)

  u8 *hdr = *ptr;
  u8 **end = ptr;
  u8 *src = hdr, *dst = NULL;
  u32 uni, tmp;

  **pre = '\0';

skip_ascii:
#define expr_jump(i)                      \
  if (likely(char_is_id_ascii(src[i]))) { \
  } else                                  \
    goto skip_ascii_stop##i;

#define expr_stop(i)             \
  skip_ascii_stop##i : src += i; \
  goto skip_ascii_end;

  repeat16_incr(expr_jump) src += 16;
  goto skip_ascii;
  repeat16_incr(expr_stop)

#undef expr_jump
#undef expr_stop

      skip_ascii_end : gcc_store_barrier(*src);
  if (likely(!char_is_id_next(*src))) {
    return_suc(src, src);
  }

skip_utf8:
  while (*src >= 0x80) {
    if (has_allow(EXT_WHITESPACE)) {
      if (char_is_space_ext(*src) && ext_space_len(src)) {
        return_suc(src, src);
      }
    }
    uni = byte_load_4(src);
    if (is_utf8_seq2(uni)) {
      src += 2;
    } else if (is_utf8_seq3(uni)) {
      src += 3;
    } else if (is_utf8_seq4(uni)) {
      src += 4;
    } else {
#if !YYJSON_DISABLE_UTF8_VALIDATION
      if (!has_allow(INVALID_UNICODE)) return_err(src, MSG_ERR_UTF8);
#endif
      src += 1;
    }
  }
  if (char_is_id_ascii(*src)) goto skip_ascii;

  dst = src;
copy_escape:
  if (byte_match_2(src, "\\u")) {
    if (!read_uni_esc(&src, &dst, msg)) return_err(src, *msg);
  } else {
    if (!char_is_id_next(*src)) return_suc(dst, src);
    return_err(src, "unexpected character in key");
  }

copy_ascii:

#define expr_jump(i)                        \
  if (likely((char_is_id_ascii(src[i])))) { \
  } else {                                  \
    gcc_store_barrier(src[i]);              \
    goto copy_ascii_stop_##i;               \
  }
  repeat16_incr(expr_jump)
#undef expr_jump

      byte_move_16(dst, src);
  dst += 16;
  src += 16;
  goto copy_ascii;

#define expr_jump(i)                                    \
  copy_ascii_stop_##i : byte_move_forward(dst, src, i); \
  dst += i;                                             \
  src += i;                                             \
  goto copy_utf8;
  repeat16_incr(expr_jump)
#undef expr_jump

      copy_utf8 : while (*src >= 0x80) {
    if (has_allow(EXT_WHITESPACE)) {
      if (char_is_space_ext(*src) && ext_space_len(src)) {
        return_suc(dst, src);
      }
    }
    uni = byte_load_4(src);
    if (is_utf8_seq2(uni)) {
      byte_copy_2(dst, &uni);
      dst += 2;
      src += 2;
    } else if (is_utf8_seq3(uni)) {
      byte_copy_4(dst, &uni);
      dst += 3;
      src += 3;
    } else if (is_utf8_seq4(uni)) {
      byte_copy_4(dst, &uni);
      dst += 4;
      src += 4;
    } else {
#if !YYJSON_DISABLE_UTF8_VALIDATION
      if (!has_allow(INVALID_UNICODE)) return_err(src, MSG_ERR_UTF8);
#endif
      *dst = *src;
      dst += 1;
      src += 1;
    }
  }
  if (char_is_id_ascii(*src)) goto copy_ascii;
  goto copy_escape;

#undef return_err
#undef return_suc
}

static_noinline yyjson_doc *read_root_single(u8 *hdr, u8 *cur, u8 *eof,
                                             yyjson_alc alc,
                                             yyjson_read_flag flg,
                                             yyjson_read_err *err) {
#define return_err(_pos, _code, _msg)                                       \
  do {                                                                      \
    if (is_truncated_end(hdr, _pos, eof, YYJSON_READ_ERROR_##_code, flg)) { \
      err->pos = (usize)(eof - hdr);                                        \
      err->code = YYJSON_READ_ERROR_UNEXPECTED_END;                         \
      err->msg = MSG_NOT_END;                                               \
    } else {                                                                \
      err->pos = (usize)(_pos - hdr);                                       \
      err->code = YYJSON_READ_ERROR_##_code;                                \
      err->msg = _msg;                                                      \
    }                                                                       \
    if (val_hdr) alc.free(alc.ctx, val_hdr);                                \
    return NULL;                                                            \
  } while (false)

  usize hdr_len;
  usize alc_num;
  yyjson_val *val_hdr;
  yyjson_val *val;
  yyjson_doc *doc;
  const char *msg;

  u8 raw_end[1];
  u8 *raw_ptr = raw_end;
  u8 **pre = &raw_ptr;

  hdr_len = sizeof(yyjson_doc) / sizeof(yyjson_val);
  hdr_len += (sizeof(yyjson_doc) % sizeof(yyjson_val)) > 0;
  alc_num = hdr_len + 1;

  val_hdr = (yyjson_val *)alc.malloc(alc.ctx, alc_num * sizeof(yyjson_val));
  if (unlikely(!val_hdr)) goto fail_alloc;
  val = val_hdr + hdr_len;

  if (char_is_num(*cur)) {
    if (likely(read_num(&cur, pre, flg, val, &msg))) goto doc_end;
    goto fail_number;
  }
  if (*cur == '"') {
    if (likely(read_str(&cur, eof, flg, val, &msg))) goto doc_end;
    goto fail_string;
  }
  if (*cur == 't') {
    if (likely(read_true(&cur, val))) goto doc_end;
    goto fail_literal_true;
  }
  if (*cur == 'f') {
    if (likely(read_false(&cur, val))) goto doc_end;
    goto fail_literal_false;
  }
  if (*cur == 'n') {
    if (likely(read_null(&cur, val))) goto doc_end;
    if (has_allow(INF_AND_NAN)) {
      if (read_nan(&cur, pre, flg, val)) goto doc_end;
    }
    goto fail_literal_null;
  }
  if (has_allow(INF_AND_NAN)) {
    if (read_inf_or_nan(&cur, pre, flg, val)) goto doc_end;
  }
  if (has_allow(SINGLE_QUOTED_STR) && *cur == '\'') {
    if (likely(read_str_sq(&cur, eof, flg, val, &msg))) goto doc_end;
    goto fail_string;
  }
  goto fail_character;

doc_end:

  if (unlikely(cur < eof) && !has_flg(STOP_WHEN_DONE)) {
    while (char_is_space(*cur)) cur++;
    if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
      if (!skip_trivia(&cur, eof, flg) && cur == eof) {
        goto fail_comment;
      }
    }
    if (unlikely(cur < eof)) goto fail_garbage;
  }

  **pre = '\0';
  doc = (yyjson_doc *)val_hdr;
  doc->root = val_hdr + hdr_len;
  doc->alc = alc;
  doc->dat_read = (usize)(cur - hdr);
  doc->val_read = 1;
  doc->str_pool = has_flg(INSITU) ? NULL : (char *)hdr;
  return doc;

fail_string:
  return_err(cur, INVALID_STRING, msg);
fail_number:
  return_err(cur, INVALID_NUMBER, msg);
fail_alloc:
  return_err(cur, MEMORY_ALLOCATION, MSG_MALLOC);
fail_literal_true:
  return_err(cur, LITERAL, MSG_CHAR_T);
fail_literal_false:
  return_err(cur, LITERAL, MSG_CHAR_F);
fail_literal_null:
  return_err(cur, LITERAL, MSG_CHAR_N);
fail_character:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_CHAR);
fail_comment:
  return_err(cur, INVALID_COMMENT, MSG_COMMENT);
fail_garbage:
  return_err(cur, UNEXPECTED_CONTENT, MSG_GARBAGE);
fail_depth:
  return_err(cur, DEPTH, MSG_DEPTH);

#undef return_err
}

static_inline yyjson_doc *read_root_minify(u8 *hdr, u8 *cur, u8 *eof,
                                           yyjson_alc alc, yyjson_read_flag flg,
                                           yyjson_read_err *err) {
#define return_err(_pos, _code, _msg)                                       \
  do {                                                                      \
    if (is_truncated_end(hdr, _pos, eof, YYJSON_READ_ERROR_##_code, flg)) { \
      err->pos = (usize)(eof - hdr);                                        \
      err->code = YYJSON_READ_ERROR_UNEXPECTED_END;                         \
      err->msg = MSG_NOT_END;                                               \
    } else {                                                                \
      err->pos = (usize)(_pos - hdr);                                       \
      err->code = YYJSON_READ_ERROR_##_code;                                \
      err->msg = _msg;                                                      \
    }                                                                       \
    if (val_hdr) alc.free(alc.ctx, val_hdr);                                \
    return NULL;                                                            \
  } while (false)

#define val_incr()                                                       \
  do {                                                                   \
    val++;                                                               \
    if (unlikely(val >= val_end)) {                                      \
      usize alc_old = alc_len;                                           \
      usize val_ofs = (usize)(val - val_hdr);                            \
      usize ctn_ofs = (usize)(ctn - val_hdr);                            \
      alc_len += alc_len / 2;                                            \
      if ((sizeof(usize) < 8) && (alc_len >= alc_max)) goto fail_alloc;  \
      val_tmp = (yyjson_val *)alc.realloc(alc.ctx, (void *)val_hdr,      \
                                          alc_old * sizeof(yyjson_val),  \
                                          alc_len * sizeof(yyjson_val)); \
      if ((!val_tmp)) goto fail_alloc;                                   \
      val = val_tmp + val_ofs;                                           \
      ctn = val_tmp + ctn_ofs;                                           \
      val_hdr = val_tmp;                                                 \
      val_end = val_tmp + (alc_len - 2);                                 \
    }                                                                    \
  } while (false)

  usize dat_len;
  usize hdr_len;
  usize alc_len;
  usize alc_max;
  usize ctn_len;
  yyjson_val *val_hdr;
  yyjson_val *val_end;
  yyjson_val *val_tmp;
  yyjson_val *val;
  yyjson_val *ctn;
  yyjson_val *ctn_parent;
  yyjson_doc *doc;
  const char *msg;

  u8 raw_end[1];
  u8 *raw_ptr = raw_end;
  u8 **pre = &raw_ptr;

#if YYJSON_READER_DEPTH_LIMIT
  u32 container_depth = 0;
#endif

  dat_len = has_flg(STOP_WHEN_DONE) ? 256 : (usize)(eof - cur);
  hdr_len = sizeof(yyjson_doc) / sizeof(yyjson_val);
  hdr_len += (sizeof(yyjson_doc) % sizeof(yyjson_val)) > 0;
  alc_max = USIZE_MAX / sizeof(yyjson_val);
  alc_len = hdr_len + (dat_len / YYJSON_READER_ESTIMATED_MINIFY_RATIO) + 4;
  alc_len = yyjson_min(alc_len, alc_max);

  val_hdr = (yyjson_val *)alc.malloc(alc.ctx, alc_len * sizeof(yyjson_val));
  if (unlikely(!val_hdr)) goto fail_alloc;
  val_end = val_hdr + (alc_len - 2);
  val = val_hdr + hdr_len;
  ctn = val;
  ctn_len = 0;

  if (*cur++ == '{') {
    ctn->tag = YYJSON_TYPE_OBJ;
    ctn->uni.ofs = 0;
    goto obj_key_begin;
  } else {
    ctn->tag = YYJSON_TYPE_ARR;
    ctn->uni.ofs = 0;
    goto arr_val_begin;
  }

arr_begin:
#if YYJSON_READER_DEPTH_LIMIT
  container_depth++;
  if (unlikely(container_depth >= YYJSON_READER_DEPTH_LIMIT)) {
    goto fail_depth;
  }
#endif

  ctn->tag =
      (((u64)ctn_len + 1) << YYJSON_TAG_BIT) | (ctn->tag & YYJSON_TAG_MASK);

  val_incr();
  val->tag = YYJSON_TYPE_ARR;
  val->uni.ofs = (usize)((u8 *)val - (u8 *)ctn);

  ctn = val;
  ctn_len = 0;

arr_val_begin:
  if (*cur == '{') {
    cur++;
    goto obj_begin;
  }
  if (*cur == '[') {
    cur++;
    goto arr_begin;
  }
  if (char_is_num(*cur)) {
    val_incr();
    ctn_len++;
    if (likely(read_num(&cur, pre, flg, val, &msg))) goto arr_val_end;
    goto fail_number;
  }
  if (*cur == '"') {
    val_incr();
    ctn_len++;
    if (likely(read_str(&cur, eof, flg, val, &msg))) goto arr_val_end;
    goto fail_string;
  }
  if (*cur == 't') {
    val_incr();
    ctn_len++;
    if (likely(read_true(&cur, val))) goto arr_val_end;
    goto fail_literal_true;
  }
  if (*cur == 'f') {
    val_incr();
    ctn_len++;
    if (likely(read_false(&cur, val))) goto arr_val_end;
    goto fail_literal_false;
  }
  if (*cur == 'n') {
    val_incr();
    ctn_len++;
    if (likely(read_null(&cur, val))) goto arr_val_end;
    if (has_allow(INF_AND_NAN)) {
      if (read_nan(&cur, pre, flg, val)) goto arr_val_end;
    }
    goto fail_literal_null;
  }
  if (*cur == ']') {
    cur++;
    if (likely(ctn_len == 0)) goto arr_end;
    if (has_allow(TRAILING_COMMAS)) goto arr_end;
    do {
      cur--;
    } while (*cur != ',');
    goto fail_trailing_comma;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto arr_val_begin;
  }
  if (has_allow(INF_AND_NAN) && (*cur == 'i' || *cur == 'I' || *cur == 'N')) {
    val_incr();
    ctn_len++;
    if (read_inf_or_nan(&cur, pre, flg, val)) goto arr_val_end;
    goto fail_character_val;
  }
  if (has_allow(SINGLE_QUOTED_STR) && *cur == '\'') {
    val_incr();
    ctn_len++;
    if (likely(read_str_sq(&cur, eof, flg, val, &msg))) goto arr_val_end;
    goto fail_string;
  }
  if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
    if (skip_trivia(&cur, eof, flg)) goto arr_val_begin;
    if (cur == eof) goto fail_comment;
  }
  goto fail_character_val;

arr_val_end:
  if (*cur == ',') {
    cur++;
    goto arr_val_begin;
  }
  if (*cur == ']') {
    cur++;
    goto arr_end;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto arr_val_end;
  }
  if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
    if (skip_trivia(&cur, eof, flg)) goto arr_val_end;
    if (cur == eof) goto fail_comment;
  }
  goto fail_character_arr_end;

arr_end:
#if YYJSON_READER_DEPTH_LIMIT
  container_depth--;
#endif

  ctn_parent = (yyjson_val *)(void *)((u8 *)ctn - ctn->uni.ofs);

  ctn->uni.ofs = (usize)((u8 *)val - (u8 *)ctn) + sizeof(yyjson_val);
  ctn->tag = ((ctn_len) << YYJSON_TAG_BIT) | YYJSON_TYPE_ARR;
  if (unlikely(ctn == ctn_parent)) goto doc_end;

  ctn = ctn_parent;
  ctn_len = (usize)(ctn->tag >> YYJSON_TAG_BIT);
  if ((ctn->tag & YYJSON_TYPE_MASK) == YYJSON_TYPE_OBJ) {
    goto obj_val_end;
  } else {
    goto arr_val_end;
  }

obj_begin:
#if YYJSON_READER_DEPTH_LIMIT
  container_depth++;
  if (unlikely(container_depth >= YYJSON_READER_DEPTH_LIMIT)) {
    goto fail_depth;
  }
#endif

  ctn->tag =
      (((u64)ctn_len + 1) << YYJSON_TAG_BIT) | (ctn->tag & YYJSON_TAG_MASK);
  val_incr();
  val->tag = YYJSON_TYPE_OBJ;

  val->uni.ofs = (usize)((u8 *)val - (u8 *)ctn);
  ctn = val;
  ctn_len = 0;

obj_key_begin:
  if (likely(*cur == '"')) {
    val_incr();
    ctn_len++;
    if (likely(read_str(&cur, eof, flg, val, &msg))) goto obj_key_end;
    goto fail_string;
  }
  if (likely(*cur == '}')) {
    cur++;
    if (likely(ctn_len == 0)) goto obj_end;
    if (has_allow(TRAILING_COMMAS)) goto obj_end;
    do {
      cur--;
    } while (*cur != ',');
    goto fail_trailing_comma;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto obj_key_begin;
  }
  if (has_allow(SINGLE_QUOTED_STR) && *cur == '\'') {
    val_incr();
    ctn_len++;
    if (likely(read_str_sq(&cur, eof, flg, val, &msg))) goto obj_key_end;
    goto fail_string;
  }
  if (has_allow(UNQUOTED_KEY) && char_is_id_start(*cur)) {
    val_incr();
    ctn_len++;
    if (read_str_id(&cur, eof, flg, pre, val, &msg)) goto obj_key_end;
    goto fail_string;
  }
  if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
    if (skip_trivia(&cur, eof, flg)) goto obj_key_begin;
    if (cur == eof) goto fail_comment;
  }
  goto fail_character_obj_key;

obj_key_end:
  if (*cur == ':') {
    cur++;
    goto obj_val_begin;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto obj_key_end;
  }
  if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
    if (skip_trivia(&cur, eof, flg)) goto obj_key_end;
    if (cur == eof) goto fail_comment;
  }
  goto fail_character_obj_sep;

obj_val_begin:
  if (*cur == '"') {
    val++;
    ctn_len++;
    if (likely(read_str(&cur, eof, flg, val, &msg))) goto obj_val_end;
    goto fail_string;
  }
  if (char_is_num(*cur)) {
    val++;
    ctn_len++;
    if (likely(read_num(&cur, pre, flg, val, &msg))) goto obj_val_end;
    goto fail_number;
  }
  if (*cur == '{') {
    cur++;
    goto obj_begin;
  }
  if (*cur == '[') {
    cur++;
    goto arr_begin;
  }
  if (*cur == 't') {
    val++;
    ctn_len++;
    if (likely(read_true(&cur, val))) goto obj_val_end;
    goto fail_literal_true;
  }
  if (*cur == 'f') {
    val++;
    ctn_len++;
    if (likely(read_false(&cur, val))) goto obj_val_end;
    goto fail_literal_false;
  }
  if (*cur == 'n') {
    val++;
    ctn_len++;
    if (likely(read_null(&cur, val))) goto obj_val_end;
    if (has_allow(INF_AND_NAN)) {
      if (read_nan(&cur, pre, flg, val)) goto obj_val_end;
    }
    goto fail_literal_null;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto obj_val_begin;
  }
  if (has_allow(INF_AND_NAN) && (*cur == 'i' || *cur == 'I' || *cur == 'N')) {
    val++;
    ctn_len++;
    if (read_inf_or_nan(&cur, pre, flg, val)) goto obj_val_end;
    goto fail_character_val;
  }
  if (has_allow(SINGLE_QUOTED_STR) && *cur == '\'') {
    val++;
    ctn_len++;
    if (likely(read_str_sq(&cur, eof, flg, val, &msg))) goto obj_val_end;
    goto fail_string;
  }
  if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
    if (skip_trivia(&cur, eof, flg)) goto obj_val_begin;
    if (cur == eof) goto fail_comment;
  }
  goto fail_character_val;

obj_val_end:
  if (likely(*cur == ',')) {
    cur++;
    goto obj_key_begin;
  }
  if (likely(*cur == '}')) {
    cur++;
    goto obj_end;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto obj_val_end;
  }
  if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
    if (skip_trivia(&cur, eof, flg)) goto obj_val_end;
    if (cur == eof) goto fail_comment;
  }
  goto fail_character_obj_end;

obj_end:
#if YYJSON_READER_DEPTH_LIMIT
  container_depth--;
#endif

  ctn_parent = (yyjson_val *)(void *)((u8 *)ctn - ctn->uni.ofs);

  ctn->uni.ofs = (usize)((u8 *)val - (u8 *)ctn) + sizeof(yyjson_val);
  ctn->tag = (ctn_len << (YYJSON_TAG_BIT - 1)) | YYJSON_TYPE_OBJ;
  if (unlikely(ctn == ctn_parent)) goto doc_end;
  ctn = ctn_parent;
  ctn_len = (usize)(ctn->tag >> YYJSON_TAG_BIT);
  if ((ctn->tag & YYJSON_TYPE_MASK) == YYJSON_TYPE_OBJ) {
    goto obj_val_end;
  } else {
    goto arr_val_end;
  }

doc_end:

  if (unlikely(cur < eof) && !has_flg(STOP_WHEN_DONE)) {
    while (char_is_space(*cur)) cur++;
    if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
      if (!skip_trivia(&cur, eof, flg) && cur == eof) {
        goto fail_comment;
      }
    }
    if (unlikely(cur < eof)) goto fail_garbage;
  }

  **pre = '\0';
  doc = (yyjson_doc *)val_hdr;
  doc->root = val_hdr + hdr_len;
  doc->alc = alc;
  doc->dat_read = (usize)(cur - hdr);
  doc->val_read = (usize)((val - doc->root) + 1);
  doc->str_pool = has_flg(INSITU) ? NULL : (char *)hdr;
  return doc;

fail_string:
  return_err(cur, INVALID_STRING, msg);
fail_number:
  return_err(cur, INVALID_NUMBER, msg);
fail_alloc:
  return_err(cur, MEMORY_ALLOCATION, MSG_MALLOC);
fail_trailing_comma:
  return_err(cur, JSON_STRUCTURE, MSG_COMMA);
fail_literal_true:
  return_err(cur, LITERAL, MSG_CHAR_T);
fail_literal_false:
  return_err(cur, LITERAL, MSG_CHAR_F);
fail_literal_null:
  return_err(cur, LITERAL, MSG_CHAR_N);
fail_character_val:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_CHAR);
fail_character_arr_end:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_ARR_END);
fail_character_obj_key:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_OBJ_KEY);
fail_character_obj_sep:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_OBJ_SEP);
fail_character_obj_end:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_OBJ_END);
fail_comment:
  return_err(cur, INVALID_COMMENT, MSG_COMMENT);
fail_garbage:
  return_err(cur, UNEXPECTED_CONTENT, MSG_GARBAGE);
fail_depth:
  return_err(cur, DEPTH, MSG_DEPTH);

#undef val_incr
#undef return_err
}

static_inline yyjson_doc *read_root_pretty(u8 *hdr, u8 *cur, u8 *eof,
                                           yyjson_alc alc, yyjson_read_flag flg,
                                           yyjson_read_err *err) {
#define return_err(_pos, _code, _msg)                                       \
  do {                                                                      \
    if (is_truncated_end(hdr, _pos, eof, YYJSON_READ_ERROR_##_code, flg)) { \
      err->pos = (usize)(eof - hdr);                                        \
      err->code = YYJSON_READ_ERROR_UNEXPECTED_END;                         \
      err->msg = MSG_NOT_END;                                               \
    } else {                                                                \
      err->pos = (usize)(_pos - hdr);                                       \
      err->code = YYJSON_READ_ERROR_##_code;                                \
      err->msg = _msg;                                                      \
    }                                                                       \
    if (val_hdr) alc.free(alc.ctx, val_hdr);                                \
    return NULL;                                                            \
  } while (false)

#define val_incr()                                                       \
  do {                                                                   \
    val++;                                                               \
    if (unlikely(val >= val_end)) {                                      \
      usize alc_old = alc_len;                                           \
      usize val_ofs = (usize)(val - val_hdr);                            \
      usize ctn_ofs = (usize)(ctn - val_hdr);                            \
      alc_len += alc_len / 2;                                            \
      if ((sizeof(usize) < 8) && (alc_len >= alc_max)) goto fail_alloc;  \
      val_tmp = (yyjson_val *)alc.realloc(alc.ctx, (void *)val_hdr,      \
                                          alc_old * sizeof(yyjson_val),  \
                                          alc_len * sizeof(yyjson_val)); \
      if ((!val_tmp)) goto fail_alloc;                                   \
      val = val_tmp + val_ofs;                                           \
      ctn = val_tmp + ctn_ofs;                                           \
      val_hdr = val_tmp;                                                 \
      val_end = val_tmp + (alc_len - 2);                                 \
    }                                                                    \
  } while (false)

  usize dat_len;
  usize hdr_len;
  usize alc_len;
  usize alc_max;
  usize ctn_len;
  yyjson_val *val_hdr;
  yyjson_val *val_end;
  yyjson_val *val_tmp;
  yyjson_val *val;
  yyjson_val *ctn;
  yyjson_val *ctn_parent;
  yyjson_doc *doc;
  const char *msg;

  u8 raw_end[1];
  u8 *raw_ptr = raw_end;
  u8 **pre = &raw_ptr;
#if YYJSON_READER_DEPTH_LIMIT
  u32 container_depth = 0;
#endif

  dat_len = has_flg(STOP_WHEN_DONE) ? 256 : (usize)(eof - cur);
  hdr_len = sizeof(yyjson_doc) / sizeof(yyjson_val);
  hdr_len += (sizeof(yyjson_doc) % sizeof(yyjson_val)) > 0;
  alc_max = USIZE_MAX / sizeof(yyjson_val);
  alc_len = hdr_len + (dat_len / YYJSON_READER_ESTIMATED_PRETTY_RATIO) + 4;
  alc_len = yyjson_min(alc_len, alc_max);

  val_hdr = (yyjson_val *)alc.malloc(alc.ctx, alc_len * sizeof(yyjson_val));
  if (unlikely(!val_hdr)) goto fail_alloc;
  val_end = val_hdr + (alc_len - 2);
  val = val_hdr + hdr_len;
  ctn = val;
  ctn_len = 0;

  if (*cur++ == '{') {
    ctn->tag = YYJSON_TYPE_OBJ;
    ctn->uni.ofs = 0;
    if (*cur == '\n') cur++;
    goto obj_key_begin;
  } else {
    ctn->tag = YYJSON_TYPE_ARR;
    ctn->uni.ofs = 0;
    if (*cur == '\n') cur++;
    goto arr_val_begin;
  }

arr_begin:
#if YYJSON_READER_DEPTH_LIMIT
  container_depth++;
  if (unlikely(container_depth >= YYJSON_READER_DEPTH_LIMIT)) {
    goto fail_depth;
  }
#endif

  ctn->tag =
      (((u64)ctn_len + 1) << YYJSON_TAG_BIT) | (ctn->tag & YYJSON_TAG_MASK);

  val_incr();
  val->tag = YYJSON_TYPE_ARR;
  val->uni.ofs = (usize)((u8 *)val - (u8 *)ctn);

  ctn = val;
  ctn_len = 0;
  if (*cur == '\n') cur++;

arr_val_begin:
#if YYJSON_IS_REAL_GCC
  while (true)
    repeat16({
      if (byte_match_2(cur, "  "))
        cur += 2;
      else
        break;
    })
#else
  while (true)
    repeat16({
      if (likely(byte_match_2(cur, "  ")))
        cur += 2;
      else
        break;
    })
#endif

        if (*cur == '{') {
      cur++;
      goto obj_begin;
    }
  if (*cur == '[') {
    cur++;
    goto arr_begin;
  }
  if (char_is_num(*cur)) {
    val_incr();
    ctn_len++;
    if (likely(read_num(&cur, pre, flg, val, &msg))) goto arr_val_end;
    goto fail_number;
  }
  if (*cur == '"') {
    val_incr();
    ctn_len++;
    if (likely(read_str(&cur, eof, flg, val, &msg))) goto arr_val_end;
    goto fail_string;
  }
  if (*cur == 't') {
    val_incr();
    ctn_len++;
    if (likely(read_true(&cur, val))) goto arr_val_end;
    goto fail_literal_true;
  }
  if (*cur == 'f') {
    val_incr();
    ctn_len++;
    if (likely(read_false(&cur, val))) goto arr_val_end;
    goto fail_literal_false;
  }
  if (*cur == 'n') {
    val_incr();
    ctn_len++;
    if (likely(read_null(&cur, val))) goto arr_val_end;
    if (has_allow(INF_AND_NAN)) {
      if (read_nan(&cur, pre, flg, val)) goto arr_val_end;
    }
    goto fail_literal_null;
  }
  if (*cur == ']') {
    cur++;
    if (likely(ctn_len == 0)) goto arr_end;
    if (has_allow(TRAILING_COMMAS)) goto arr_end;
    do {
      cur--;
    } while (*cur != ',');
    goto fail_trailing_comma;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto arr_val_begin;
  }
  if (has_allow(INF_AND_NAN) && (*cur == 'i' || *cur == 'I' || *cur == 'N')) {
    val_incr();
    ctn_len++;
    if (read_inf_or_nan(&cur, pre, flg, val)) goto arr_val_end;
    goto fail_character_val;
  }
  if (has_allow(SINGLE_QUOTED_STR) && *cur == '\'') {
    val_incr();
    ctn_len++;
    if (likely(read_str_sq(&cur, eof, flg, val, &msg))) goto arr_val_end;
    goto fail_string;
  }
  if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
    if (skip_trivia(&cur, eof, flg)) goto arr_val_begin;
    if (cur == eof) goto fail_comment;
  }
  goto fail_character_val;

arr_val_end:
  if (byte_match_2(cur, ",\n")) {
    cur += 2;
    goto arr_val_begin;
  }
  if (*cur == ',') {
    cur++;
    goto arr_val_begin;
  }
  if (*cur == ']') {
    cur++;
    goto arr_end;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto arr_val_end;
  }
  if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
    if (skip_trivia(&cur, eof, flg)) goto arr_val_end;
    if (cur == eof) goto fail_comment;
  }
  goto fail_character_arr_end;

arr_end:
#if YYJSON_READER_DEPTH_LIMIT
  container_depth--;
#endif

  ctn_parent = (yyjson_val *)(void *)((u8 *)ctn - ctn->uni.ofs);

  ctn->uni.ofs = (usize)((u8 *)val - (u8 *)ctn) + sizeof(yyjson_val);
  ctn->tag = ((ctn_len) << YYJSON_TAG_BIT) | YYJSON_TYPE_ARR;
  if (unlikely(ctn == ctn_parent)) goto doc_end;

  ctn = ctn_parent;
  ctn_len = (usize)(ctn->tag >> YYJSON_TAG_BIT);
  if (*cur == '\n') cur++;
  if ((ctn->tag & YYJSON_TYPE_MASK) == YYJSON_TYPE_OBJ) {
    goto obj_val_end;
  } else {
    goto arr_val_end;
  }

obj_begin:
#if YYJSON_READER_DEPTH_LIMIT
  container_depth++;
  if (unlikely(container_depth >= YYJSON_READER_DEPTH_LIMIT)) {
    goto fail_depth;
  }
#endif

  ctn->tag =
      (((u64)ctn_len + 1) << YYJSON_TAG_BIT) | (ctn->tag & YYJSON_TAG_MASK);
  val_incr();
  val->tag = YYJSON_TYPE_OBJ;

  val->uni.ofs = (usize)((u8 *)val - (u8 *)ctn);
  ctn = val;
  ctn_len = 0;
  if (*cur == '\n') cur++;

obj_key_begin:
#if YYJSON_IS_REAL_GCC
  while (true)
    repeat16({
      if (byte_match_2(cur, "  "))
        cur += 2;
      else
        break;
    })
#else
  while (true)
    repeat16({
      if (likely(byte_match_2(cur, "  ")))
        cur += 2;
      else
        break;
    })
#endif
        if (likely(*cur == '"')) {
      val_incr();
      ctn_len++;
      if (likely(read_str(&cur, eof, flg, val, &msg))) goto obj_key_end;
      goto fail_string;
    }
  if (likely(*cur == '}')) {
    cur++;
    if (likely(ctn_len == 0)) goto obj_end;
    if (has_allow(TRAILING_COMMAS)) goto obj_end;
    do {
      cur--;
    } while (*cur != ',');
    goto fail_trailing_comma;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto obj_key_begin;
  }
  if (has_allow(SINGLE_QUOTED_STR) && *cur == '\'') {
    val_incr();
    ctn_len++;
    if (likely(read_str_sq(&cur, eof, flg, val, &msg))) goto obj_key_end;
    goto fail_string;
  }
  if (has_allow(UNQUOTED_KEY) && char_is_id_start(*cur)) {
    val_incr();
    ctn_len++;
    if (read_str_id(&cur, eof, flg, pre, val, &msg)) goto obj_key_end;
    goto fail_string;
  }
  if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
    if (skip_trivia(&cur, eof, flg)) goto obj_key_begin;
    if (cur == eof) goto fail_comment;
  }
  goto fail_character_obj_key;

obj_key_end:
  if (byte_match_2(cur, ": ")) {
    cur += 2;
    goto obj_val_begin;
  }
  if (*cur == ':') {
    cur++;
    goto obj_val_begin;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto obj_key_end;
  }
  if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
    if (skip_trivia(&cur, eof, flg)) goto obj_key_end;
    if (cur == eof) goto fail_comment;
  }
  goto fail_character_obj_sep;

obj_val_begin:
  if (*cur == '"') {
    val++;
    ctn_len++;
    if (likely(read_str(&cur, eof, flg, val, &msg))) goto obj_val_end;
    goto fail_string;
  }
  if (char_is_num(*cur)) {
    val++;
    ctn_len++;
    if (likely(read_num(&cur, pre, flg, val, &msg))) goto obj_val_end;
    goto fail_number;
  }
  if (*cur == '{') {
    cur++;
    goto obj_begin;
  }
  if (*cur == '[') {
    cur++;
    goto arr_begin;
  }
  if (*cur == 't') {
    val++;
    ctn_len++;
    if (likely(read_true(&cur, val))) goto obj_val_end;
    goto fail_literal_true;
  }
  if (*cur == 'f') {
    val++;
    ctn_len++;
    if (likely(read_false(&cur, val))) goto obj_val_end;
    goto fail_literal_false;
  }
  if (*cur == 'n') {
    val++;
    ctn_len++;
    if (likely(read_null(&cur, val))) goto obj_val_end;
    if (has_allow(INF_AND_NAN)) {
      if (read_nan(&cur, pre, flg, val)) goto obj_val_end;
    }
    goto fail_literal_null;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto obj_val_begin;
  }
  if (has_allow(INF_AND_NAN) && (*cur == 'i' || *cur == 'I' || *cur == 'N')) {
    val++;
    ctn_len++;
    if (read_inf_or_nan(&cur, pre, flg, val)) goto obj_val_end;
    goto fail_character_val;
  }
  if (has_allow(SINGLE_QUOTED_STR) && *cur == '\'') {
    val++;
    ctn_len++;
    if (likely(read_str_sq(&cur, eof, flg, val, &msg))) goto obj_val_end;
    goto fail_string;
  }
  if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
    if (skip_trivia(&cur, eof, flg)) goto obj_val_begin;
    if (cur == eof) goto fail_comment;
  }
  goto fail_character_val;

obj_val_end:
  if (byte_match_2(cur, ",\n")) {
    cur += 2;
    goto obj_key_begin;
  }
  if (likely(*cur == ',')) {
    cur++;
    goto obj_key_begin;
  }
  if (likely(*cur == '}')) {
    cur++;
    goto obj_end;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto obj_val_end;
  }
  if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
    if (skip_trivia(&cur, eof, flg)) goto obj_val_end;
    if (cur == eof) goto fail_comment;
  }
  goto fail_character_obj_end;

obj_end:
#if YYJSON_READER_DEPTH_LIMIT
  container_depth--;
#endif

  ctn_parent = (yyjson_val *)(void *)((u8 *)ctn - ctn->uni.ofs);

  ctn->uni.ofs = (usize)((u8 *)val - (u8 *)ctn) + sizeof(yyjson_val);
  ctn->tag = (ctn_len << (YYJSON_TAG_BIT - 1)) | YYJSON_TYPE_OBJ;
  if (unlikely(ctn == ctn_parent)) goto doc_end;
  ctn = ctn_parent;
  ctn_len = (usize)(ctn->tag >> YYJSON_TAG_BIT);
  if (*cur == '\n') cur++;
  if ((ctn->tag & YYJSON_TYPE_MASK) == YYJSON_TYPE_OBJ) {
    goto obj_val_end;
  } else {
    goto arr_val_end;
  }

doc_end:

  if (unlikely(cur < eof) && !has_flg(STOP_WHEN_DONE)) {
    while (char_is_space(*cur)) cur++;
    if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
      if (!skip_trivia(&cur, eof, flg) && cur == eof) {
        goto fail_comment;
      }
    }
    if (unlikely(cur < eof)) goto fail_garbage;
  }

  **pre = '\0';
  doc = (yyjson_doc *)val_hdr;
  doc->root = val_hdr + hdr_len;
  doc->alc = alc;
  doc->dat_read = (usize)(cur - hdr);
  doc->val_read = (usize)((val - doc->root) + 1);
  doc->str_pool = has_flg(INSITU) ? NULL : (char *)hdr;
  return doc;

fail_string:
  return_err(cur, INVALID_STRING, msg);
fail_number:
  return_err(cur, INVALID_NUMBER, msg);
fail_alloc:
  return_err(cur, MEMORY_ALLOCATION, MSG_MALLOC);
fail_trailing_comma:
  return_err(cur, JSON_STRUCTURE, MSG_COMMA);
fail_literal_true:
  return_err(cur, LITERAL, MSG_CHAR_T);
fail_literal_false:
  return_err(cur, LITERAL, MSG_CHAR_F);
fail_literal_null:
  return_err(cur, LITERAL, MSG_CHAR_N);
fail_character_val:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_CHAR);
fail_character_arr_end:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_ARR_END);
fail_character_obj_key:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_OBJ_KEY);
fail_character_obj_sep:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_OBJ_SEP);
fail_character_obj_end:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_OBJ_END);
fail_comment:
  return_err(cur, INVALID_COMMENT, MSG_COMMENT);
fail_garbage:
  return_err(cur, UNEXPECTED_CONTENT, MSG_GARBAGE);
fail_depth:
  return_err(cur, DEPTH, MSG_DEPTH);

#undef val_incr
#undef return_err
}

yyjson_doc *yyjson_read_opts(char *dat, usize len, yyjson_read_flag flg,
                             const yyjson_alc *alc_ptr, yyjson_read_err *err) {
#define return_err(_pos, _code, _msg)                            \
  do {                                                           \
    err->pos = (usize)(_pos);                                    \
    err->msg = _msg;                                             \
    err->code = YYJSON_READ_ERROR_##_code;                       \
    if (!has_flg(INSITU) && hdr) alc.free(alc.ctx, (void *)hdr); \
    return NULL;                                                 \
  } while (false)

  yyjson_read_err tmp_err;
  yyjson_alc alc = alc_ptr ? *alc_ptr : YYJSON_DEFAULT_ALC;
  yyjson_doc *doc;
  u8 *hdr = NULL, *eof, *cur;

  if (!err) err = &tmp_err;
  if (unlikely(!dat)) return_err(0, INVALID_PARAMETER, "input data is NULL");
  if (unlikely(!len)) return_err(0, INVALID_PARAMETER, "input length is 0");

  if (has_flg(INSITU)) {
    hdr = (u8 *)dat;
    eof = (u8 *)dat + len;
    cur = (u8 *)dat;
  } else {
    if (unlikely(len >= USIZE_MAX - YYJSON_PADDING_SIZE)) {
      return_err(0, MEMORY_ALLOCATION, MSG_MALLOC);
    }
    hdr = (u8 *)alc.malloc(alc.ctx, len + YYJSON_PADDING_SIZE);
    if (unlikely(!hdr)) {
      return_err(0, MEMORY_ALLOCATION, MSG_MALLOC);
    }
    eof = hdr + len;
    cur = hdr;
    memcpy(hdr, dat, len);
  }
  memset(eof, 0, YYJSON_PADDING_SIZE);

  if (has_allow(BOM)) {
    if (len >= 3 && is_utf8_bom(cur)) cur += 3;
  }

  if (unlikely(!char_is_ctn(*cur))) {
    while (char_is_space(*cur)) cur++;
    if (unlikely(!char_is_ctn(*cur))) {
      if (has_allow(TRIVIA) && char_is_trivia(*cur)) {
        if (!skip_trivia(&cur, eof, flg) && cur == eof) {
          return_err(cur - hdr, INVALID_COMMENT, MSG_COMMENT);
        }
      }
    }
    if (unlikely(cur >= eof)) {
      return_err(0, EMPTY_CONTENT, "input data is empty");
    }
  }

  if (likely(char_is_ctn(*cur))) {
    if (char_is_space(cur[1]) && char_is_space(cur[2])) {
      doc = read_root_pretty(hdr, cur, eof, alc, flg, err);
    } else {
      doc = read_root_minify(hdr, cur, eof, alc, flg, err);
    }
  } else {
    doc = read_root_single(hdr, cur, eof, alc, flg, err);
  }

  if (likely(doc)) {
    memset(err, 0, sizeof(yyjson_read_err));
  } else {
    if (err->pos == 0 && err->code != YYJSON_READ_ERROR_MEMORY_ALLOCATION) {
      if (is_utf8_bom(hdr))
        err->msg = MSG_ERR_BOM;
      else if (len >= 4 && is_utf32_bom(hdr))
        err->msg = MSG_ERR_UTF32;
      else if (len >= 2 && is_utf16_bom(hdr))
        err->msg = MSG_ERR_UTF16;
    }
    if (!has_flg(INSITU)) alc.free(alc.ctx, hdr);
  }
  return doc;

#undef return_err
}

yyjson_doc *yyjson_read_file(const char *path, yyjson_read_flag flg,
                             const yyjson_alc *alc_ptr, yyjson_read_err *err) {
#define return_err(_code, _msg)            \
  do {                                     \
    err->pos = 0;                          \
    err->msg = _msg;                       \
    err->code = YYJSON_READ_ERROR_##_code; \
    return NULL;                           \
  } while (false)

  yyjson_read_err tmp_err;
  yyjson_doc *doc;
  FILE *file;

  if (!err) err = &tmp_err;
  if (unlikely(!path)) return_err(INVALID_PARAMETER, "input path is NULL");

  file = fopen_readonly(path);
  if (unlikely(!file)) return_err(FILE_OPEN, MSG_FREAD);

  doc = yyjson_read_fp(file, flg, alc_ptr, err);
  fclose(file);
  return doc;

#undef return_err
}

yyjson_doc *yyjson_read_fp(FILE *file, yyjson_read_flag flg,
                           const yyjson_alc *alc_ptr, yyjson_read_err *err) {
#define return_err(_code, _msg)            \
  do {                                     \
    err->pos = 0;                          \
    err->msg = _msg;                       \
    err->code = YYJSON_READ_ERROR_##_code; \
    if (buf) alc.free(alc.ctx, buf);       \
    return NULL;                           \
  } while (false)

  yyjson_read_err tmp_err;
  yyjson_alc alc = alc_ptr ? *alc_ptr : YYJSON_DEFAULT_ALC;
  yyjson_doc *doc;

  long file_size = 0, file_pos;
  void *buf = NULL;
  usize buf_size = 0;

  if (!err) err = &tmp_err;
  if (unlikely(!file)) return_err(INVALID_PARAMETER, "input file is NULL");

  file_pos = ftell(file);
  if (file_pos != -1) {
    if (fseek(file, 0, SEEK_END) == 0) file_size = ftell(file);

    if (fseek(file, file_pos, SEEK_SET) != 0) file_size = 0;

    if (file_size > 0) file_size -= file_pos;
  }

  if (file_size > 0) {
    buf_size = (usize)file_size + YYJSON_PADDING_SIZE;
    buf = alc.malloc(alc.ctx, buf_size);
    if (buf == NULL) {
      return_err(MEMORY_ALLOCATION, MSG_MALLOC);
    }
    if (fread_safe(buf, (usize)file_size, file) != (usize)file_size) {
      return_err(FILE_READ, MSG_FREAD);
    }
  } else {
    usize chunk_min = (usize)64;
    usize chunk_max = (usize)512 * 1024 * 1024;
    usize chunk_now = chunk_min;
    usize read_size;
    void *tmp;

    buf_size = YYJSON_PADDING_SIZE;
    while (true) {
      if (buf_size + chunk_now < buf_size) {
        return_err(MEMORY_ALLOCATION, MSG_MALLOC);
      }
      buf_size += chunk_now;
      if (!buf) {
        buf = alc.malloc(alc.ctx, buf_size);
        if (!buf) return_err(MEMORY_ALLOCATION, MSG_MALLOC);
      } else {
        tmp = alc.realloc(alc.ctx, buf, buf_size - chunk_now, buf_size);
        if (!tmp) return_err(MEMORY_ALLOCATION, MSG_MALLOC);
        buf = tmp;
      }
      tmp = ((u8 *)buf) + buf_size - YYJSON_PADDING_SIZE - chunk_now;
      read_size = fread_safe(tmp, chunk_now, file);
      file_size += (long)read_size;
      if (read_size != chunk_now) break;

      chunk_now *= 2;
      if (chunk_now > chunk_max) chunk_now = chunk_max;
    }
  }

  memset((u8 *)buf + file_size, 0, YYJSON_PADDING_SIZE);
  flg |= YYJSON_READ_INSITU;
  doc = yyjson_read_opts((char *)buf, (usize)file_size, flg, &alc, err);
  if (doc) {
    doc->str_pool = (char *)buf;
    return doc;
  } else {
    alc.free(alc.ctx, buf);
    return NULL;
  }

#undef return_err
}

const char *yyjson_read_number(const char *dat, yyjson_val *val,
                               yyjson_read_flag flg, const yyjson_alc *alc,
                               yyjson_read_err *err) {
#define return_err(_pos, _code, _msg)                \
  do {                                               \
    err->pos = _pos > hdr ? (usize)(_pos - hdr) : 0; \
    err->msg = _msg;                                 \
    err->code = YYJSON_READ_ERROR_##_code;           \
    return NULL;                                     \
  } while (false)

  u8 *hdr = constcast(u8 *) dat, *cur = hdr;
  u8 raw_end[1];
  u8 *raw_ptr = raw_end;
  u8 **pre = &raw_ptr;
  const char *msg;
  yyjson_read_err tmp_err;

#if YYJSON_DISABLE_FAST_FP_CONV
  u8 buf[128];
  usize dat_len;
#endif

  if (!err) err = &tmp_err;
  if (unlikely(!dat)) {
    return_err(cur, INVALID_PARAMETER, "input data is NULL");
  }
  if (unlikely(!val)) {
    return_err(cur, INVALID_PARAMETER, "output value is NULL");
  }

#if YYJSON_DISABLE_FAST_FP_CONV
  if (!alc) alc = &YYJSON_DEFAULT_ALC;
  dat_len = strlen(dat);
  if (dat_len < sizeof(buf)) {
    memcpy(buf, dat, dat_len + 1);
    hdr = buf;
    cur = hdr;
  } else {
    hdr = (u8 *)alc->malloc(alc->ctx, dat_len + 1);
    cur = hdr;
    if (unlikely(!hdr)) {
      return_err(cur, MEMORY_ALLOCATION, MSG_MALLOC);
    }
    memcpy(hdr, dat, dat_len + 1);
  }
  hdr[dat_len] = 0;
#endif

#if YYJSON_DISABLE_FAST_FP_CONV
  if (!read_num(&cur, pre, flg, val, &msg)) {
    if (dat_len >= sizeof(buf)) alc->free(alc->ctx, hdr);
    return_err(cur, INVALID_NUMBER, msg);
  }
  if (dat_len >= sizeof(buf)) alc->free(alc->ctx, hdr);
  if (yyjson_is_raw(val)) val->uni.str = dat;
  return dat + (cur - hdr);
#else
  if (!read_num(&cur, pre, flg, val, &msg)) {
    return_err(cur, INVALID_NUMBER, msg);
  }
  return (const char *)cur;
#endif

#undef return_err
}

#if !YYJSON_DISABLE_INCR_READER

#define LABEL_doc_begin 0
#define LABEL_arr_val_begin 1
#define LABEL_arr_val_end 2
#define LABEL_obj_key_begin 3
#define LABEL_obj_key_end 4
#define LABEL_obj_val_begin 5
#define LABEL_obj_val_end 6
#define LABEL_doc_end 7

struct yyjson_incr_state {
  u32 label;
  yyjson_alc alc;
  yyjson_read_flag flg;
  u8 *hdr;
  u8 *cur;
  usize buf_len;
  usize hdr_len;
  usize alc_len;
  usize ctn_len;
  yyjson_val *val_hdr;
  yyjson_val *val_end;
  yyjson_val *val;
  yyjson_val *ctn;
  u8 *str_con[2];
};

yyjson_incr_state *yyjson_incr_new(char *buf, size_t buf_len,
                                   yyjson_read_flag flg,
                                   const yyjson_alc *alc_ptr) {
  yyjson_incr_state *state = NULL;
  yyjson_alc alc = alc_ptr ? *alc_ptr : YYJSON_DEFAULT_ALC;

  flg &= ~YYJSON_READ_JSON5;
  flg &= ~YYJSON_READ_ALLOW_BOM;
  flg &= ~YYJSON_READ_ALLOW_INVALID_UNICODE;

  if (unlikely(!buf)) return NULL;
  if (unlikely(buf_len >= USIZE_MAX - YYJSON_PADDING_SIZE)) return NULL;
  state = (yyjson_incr_state *)alc.malloc(alc.ctx, sizeof(*state));
  if (!state) return NULL;
  memset(state, 0, sizeof(yyjson_incr_state));
  state->alc = alc;
  state->flg = flg;
  state->buf_len = buf_len;

  if (has_flg(INSITU)) {
    state->hdr = (u8 *)buf;
  } else {
    state->hdr = (u8 *)alc.malloc(alc.ctx, buf_len + YYJSON_PADDING_SIZE);
    if (unlikely(!state->hdr)) {
      alc.free(alc.ctx, state);
      return NULL;
    }
    memcpy(state->hdr, buf, buf_len);
  }
  memset(state->hdr + buf_len, 0, YYJSON_PADDING_SIZE);
  state->cur = state->hdr;
  state->label = LABEL_doc_begin;
  return state;
}

void yyjson_incr_free(yyjson_incr_state *state) {
  if (state) {
    yyjson_alc alc = state->alc;
    memset(&state->alc, 0, sizeof(alc));
    if (state->val_hdr) {
      alc.free(alc.ctx, (void *)state->val_hdr);
    }
    if (state->hdr && !(state->flg & YYJSON_READ_INSITU)) {
      alc.free(alc.ctx, state->hdr);
    }
    alc.free(alc.ctx, state);
  }
}

yyjson_doc *yyjson_incr_read(yyjson_incr_state *state, size_t len,
                             yyjson_read_err *err) {
#define return_err_inv_param(_msg)                   \
  do {                                               \
    err->pos = 0;                                    \
    err->msg = _msg;                                 \
    err->code = YYJSON_READ_ERROR_INVALID_PARAMETER; \
    return NULL;                                     \
  } while (false)

#define return_err(_pos, _code, _msg)                                       \
  do {                                                                      \
    if (is_truncated_end(hdr, _pos, end, YYJSON_READ_ERROR_##_code, flg)) { \
      goto unexpected_end;                                                  \
    } else {                                                                \
      err->pos = (usize)(_pos - hdr);                                       \
      err->code = YYJSON_READ_ERROR_##_code;                                \
      err->msg = _msg;                                                      \
    }                                                                       \
    return NULL;                                                            \
  } while (false)

#define val_incr()                                                       \
  do {                                                                   \
    val++;                                                               \
    if (unlikely(val >= val_end)) {                                      \
      usize alc_old = alc_len;                                           \
      alc_len += alc_len / 2;                                            \
      if ((sizeof(usize) < 8) && (alc_len >= alc_max)) goto fail_alloc;  \
      val_tmp = (yyjson_val *)alc.realloc(alc.ctx, (void *)val_hdr,      \
                                          alc_old * sizeof(yyjson_val),  \
                                          alc_len * sizeof(yyjson_val)); \
      if ((!val_tmp)) goto fail_alloc;                                   \
      val = val_tmp + (usize)(val - val_hdr);                            \
      ctn = val_tmp + (usize)(ctn - val_hdr);                            \
      state->val = val_tmp + (usize)(state->val - val_hdr);              \
      state->val_hdr = val_hdr = val_tmp;                                \
      val_end = val_tmp + (alc_len - 2);                                 \
      state->val_end = val_end;                                          \
    }                                                                    \
  } while (false)

#define save_incr_state(_label)                    \
  do {                                             \
    state->label = LABEL_##_label;                 \
    state->cur = cur;                              \
    state->val = val;                              \
    state->ctn_len = ctn_len;                      \
    state->hdr_len = hdr_len;                      \
    if (unlikely(cur >= end)) goto unexpected_end; \
  } while (false)

#define check_maybe_truncated_number()                     \
  do {                                                     \
    if (unlikely(cur >= end)) {                            \
      if (unlikely(cur > state->cur + INCR_NUM_MAX_LEN)) { \
        msg = "number too long";                           \
        goto fail_number;                                  \
      }                                                    \
      goto unexpected_end;                                 \
    }                                                      \
  } while (false)

  u8 *hdr = NULL, *end = NULL, *cur = NULL;
  yyjson_read_flag flg;
  yyjson_alc alc;
  usize dat_len;
  usize hdr_len;
  usize alc_len;
  usize alc_max;
  usize ctn_len;
  yyjson_val *val_hdr;
  yyjson_val *val_end;
  yyjson_val *val_tmp;
  yyjson_val *val;
  yyjson_val *ctn;
  yyjson_val *ctn_parent;
  yyjson_doc *doc;
  const char *msg;

  yyjson_read_err tmp_err;
  u8 raw_end[1];
  u8 *raw_ptr = raw_end;
  u8 **pre = &raw_ptr;
  u8 **con = NULL;
  u8 saved_end = '\0';

#if YYJSON_READER_DEPTH_LIMIT
  u32 container_depth = 0;
#endif

  if (!err) err = &tmp_err;
  if (unlikely(!state)) {
    return_err_inv_param("input state is NULL");
  }
  if (unlikely(!len)) {
    return_err_inv_param("input length is 0");
  }
  if (unlikely(len > state->buf_len)) {
    return_err_inv_param("length is greater than total input length");
  }

  hdr = state->hdr;
  end = state->hdr + len;
  cur = state->cur;
  flg = state->flg;
  alc = state->alc;
  ctn_len = state->ctn_len;
  hdr_len = state->hdr_len;
  alc_len = state->alc_len;
  val = state->val;
  val_hdr = state->val_hdr;
  val_end = state->val_end;
  ctn = state->ctn;
  con = state->str_con;
  alc_max = USIZE_MAX / sizeof(yyjson_val);

  saved_end = *end;
  *end = '\0';

  switch (state->label) {
    case LABEL_doc_begin:
      goto doc_begin;
    case LABEL_arr_val_begin:
      goto arr_val_begin;
    case LABEL_arr_val_end:
      goto arr_val_end;
    case LABEL_obj_key_begin:
      goto obj_key_begin;
    case LABEL_obj_key_end:
      goto obj_key_end;
    case LABEL_obj_val_begin:
      goto obj_val_begin;
    case LABEL_obj_val_end:
      goto obj_val_end;
    case LABEL_doc_end:
      goto doc_end;
    default:
      return_err_inv_param("invalid incremental state");
  }

doc_begin:

  if (unlikely(!char_is_ctn(*cur))) {
    while (char_is_space(*cur)) cur++;
    if (unlikely(cur >= end)) goto unexpected_end;
  }

  if (!val_hdr) {
    hdr_len = sizeof(yyjson_doc) / sizeof(yyjson_val);
    hdr_len += (sizeof(yyjson_doc) % sizeof(yyjson_val)) > 0;
    if (likely(char_is_ctn(*cur))) {
      dat_len = has_flg(STOP_WHEN_DONE) ? 256 : state->buf_len;
      alc_len = hdr_len + (dat_len / YYJSON_READER_ESTIMATED_MINIFY_RATIO) + 4;
      alc_len = yyjson_min(alc_len, alc_max);
    } else {
      alc_len = hdr_len + 1;
    }
    val_hdr = (yyjson_val *)alc.malloc(alc.ctx, alc_len * sizeof(yyjson_val));
    if (unlikely(!val_hdr)) goto fail_alloc;
    val_end = val_hdr + (alc_len - 2);
    val = val_hdr + hdr_len;
    ctn = val;
    ctn_len = 0;
    state->val_hdr = val_hdr;
    state->val_end = val_end;
    save_incr_state(doc_begin);
  }

  if (*cur == '{') {
    cur++;
    ctn->tag = YYJSON_TYPE_OBJ;
    ctn->uni.ofs = 0;
    goto obj_key_begin;
  }
  if (*cur == '[') {
    cur++;
    ctn->tag = YYJSON_TYPE_ARR;
    ctn->uni.ofs = 0;
    goto arr_val_begin;
  }
  if (char_is_num(*cur)) {
    if (likely(read_num(&cur, pre, flg, val, &msg))) goto doc_end;
    goto fail_number;
  }
  if (*cur == '"') {
    if (likely(read_str_con(&cur, end, flg, val, &msg, con))) goto doc_end;
    goto fail_string;
  }
  if (*cur == 't') {
    if (likely(read_true(&cur, val))) goto doc_end;
    goto fail_literal_true;
  }
  if (*cur == 'f') {
    if (likely(read_false(&cur, val))) goto doc_end;
    goto fail_literal_false;
  }
  if (*cur == 'n') {
    if (likely(read_null(&cur, val))) goto doc_end;
    goto fail_literal_null;
  }

  msg = "unexpected character, expected a valid root value";
  if (cur == hdr) {
    if (is_utf8_bom(hdr))
      msg = MSG_ERR_BOM;
    else if (len >= 4 && is_utf32_bom(hdr))
      msg = MSG_ERR_UTF32;
    else if (len >= 2 && is_utf16_bom(hdr))
      msg = MSG_ERR_UTF16;
  }
  return_err(cur, UNEXPECTED_CHARACTER, msg);

arr_begin:
#if YYJSON_READER_DEPTH_LIMIT
  container_depth++;
  if (unlikely(container_depth >= YYJSON_READER_DEPTH_LIMIT)) {
    goto fail_depth;
  }
#endif

  ctn->tag =
      (((u64)ctn_len + 1) << YYJSON_TAG_BIT) | (ctn->tag & YYJSON_TAG_MASK);

  val_incr();
  val->tag = YYJSON_TYPE_ARR;
  val->uni.ofs = (usize)((u8 *)val - (u8 *)ctn);

  ctn = val;
  ctn_len = 0;

arr_val_begin:
  save_incr_state(arr_val_begin);
arr_val_continue:
  if (*cur == '{') {
    cur++;
    goto obj_begin;
  }
  if (*cur == '[') {
    cur++;
    goto arr_begin;
  }
  if (char_is_num(*cur)) {
    val_incr();
    ctn_len++;
    if (likely(read_num(&cur, pre, flg, val, &msg))) goto arr_val_maybe_end;
    goto fail_number;
  }
  if (*cur == '"') {
    val_incr();
    ctn_len++;
    if (likely(read_str_con(&cur, end, flg, val, &msg, con))) goto arr_val_end;
    goto fail_string;
  }
  if (*cur == 't') {
    val_incr();
    ctn_len++;
    if (likely(read_true(&cur, val))) goto arr_val_end;
    goto fail_literal_true;
  }
  if (*cur == 'f') {
    val_incr();
    ctn_len++;
    if (likely(read_false(&cur, val))) goto arr_val_end;
    goto fail_literal_false;
  }
  if (*cur == 'n') {
    val_incr();
    ctn_len++;
    if (likely(read_null(&cur, val))) goto arr_val_end;
    goto fail_literal_null;
  }
  if (*cur == ']') {
    cur++;
    if (likely(ctn_len == 0)) goto arr_end;
    do {
      cur--;
    } while (*cur != ',');
    goto fail_trailing_comma;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto arr_val_continue;
  }
  goto fail_character_val;

arr_val_maybe_end:

  check_maybe_truncated_number();

arr_val_end:
  save_incr_state(arr_val_end);
  if (*cur == ',') {
    cur++;
    goto arr_val_begin;
  }
  if (*cur == ']') {
    cur++;
    goto arr_end;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto arr_val_end;
  }
  goto fail_character_arr_end;

arr_end:
#if YYJSON_READER_DEPTH_LIMIT
  container_depth--;
#endif

  ctn_parent = (yyjson_val *)(void *)((u8 *)ctn - ctn->uni.ofs);

  ctn->uni.ofs = (usize)((u8 *)val - (u8 *)ctn) + sizeof(yyjson_val);
  ctn->tag = ((ctn_len) << YYJSON_TAG_BIT) | YYJSON_TYPE_ARR;
  if (unlikely(ctn == ctn_parent)) goto doc_end;

  ctn = ctn_parent;
  ctn_len = (usize)(ctn->tag >> YYJSON_TAG_BIT);
  if ((ctn->tag & YYJSON_TYPE_MASK) == YYJSON_TYPE_OBJ) {
    goto obj_val_end;
  } else {
    goto arr_val_end;
  }

obj_begin:
#if YYJSON_READER_DEPTH_LIMIT
  container_depth++;
  if (unlikely(container_depth >= YYJSON_READER_DEPTH_LIMIT)) {
    goto fail_depth;
  }
#endif

  ctn->tag =
      (((u64)ctn_len + 1) << YYJSON_TAG_BIT) | (ctn->tag & YYJSON_TAG_MASK);
  val_incr();
  val->tag = YYJSON_TYPE_OBJ;

  val->uni.ofs = (usize)((u8 *)val - (u8 *)ctn);
  ctn = val;
  ctn_len = 0;

obj_key_begin:
  save_incr_state(obj_key_begin);
obj_key_continue:
  if (likely(*cur == '"')) {
    val_incr();
    ctn_len++;
    if (likely(read_str_con(&cur, end, flg, val, &msg, con))) goto obj_key_end;
    goto fail_string;
  }
  if (likely(*cur == '}')) {
    cur++;
    if (likely(ctn_len == 0)) goto obj_end;
    do {
      cur--;
    } while (*cur != ',');
    goto fail_trailing_comma;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto obj_key_continue;
  }
  goto fail_character_obj_key;

obj_key_end:
  save_incr_state(obj_key_end);
  if (*cur == ':') {
    cur++;
    goto obj_val_begin;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto obj_key_end;
  }
  goto fail_character_obj_sep;

obj_val_begin:
  save_incr_state(obj_val_begin);
obj_val_continue:
  if (*cur == '"') {
    val++;
    ctn_len++;
    if (likely(read_str_con(&cur, end, flg, val, &msg, con))) goto obj_val_end;
    goto fail_string;
  }
  if (char_is_num(*cur)) {
    val++;
    ctn_len++;
    if (likely(read_num(&cur, pre, flg, val, &msg))) goto obj_val_maybe_end;
    goto fail_number;
  }
  if (*cur == '{') {
    cur++;
    goto obj_begin;
  }
  if (*cur == '[') {
    cur++;
    goto arr_begin;
  }
  if (*cur == 't') {
    val++;
    ctn_len++;
    if (likely(read_true(&cur, val))) goto obj_val_end;
    goto fail_literal_true;
  }
  if (*cur == 'f') {
    val++;
    ctn_len++;
    if (likely(read_false(&cur, val))) goto obj_val_end;
    goto fail_literal_false;
  }
  if (*cur == 'n') {
    val++;
    ctn_len++;
    if (likely(read_null(&cur, val))) goto obj_val_end;
    goto fail_literal_null;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto obj_val_continue;
  }
  goto fail_character_val;

obj_val_maybe_end:

  check_maybe_truncated_number();

obj_val_end:
  save_incr_state(obj_val_end);
  if (likely(*cur == ',')) {
    cur++;
    goto obj_key_begin;
  }
  if (likely(*cur == '}')) {
    cur++;
    goto obj_end;
  }
  if (char_is_space(*cur)) {
    while (char_is_space(*++cur));
    goto obj_val_end;
  }
  goto fail_character_obj_end;

obj_end:
#if YYJSON_READER_DEPTH_LIMIT
  container_depth--;
#endif

  ctn_parent = (yyjson_val *)(void *)((u8 *)ctn - ctn->uni.ofs);

  ctn->uni.ofs = (usize)((u8 *)val - (u8 *)ctn) + sizeof(yyjson_val);
  ctn->tag = (ctn_len << (YYJSON_TAG_BIT - 1)) | YYJSON_TYPE_OBJ;
  if (unlikely(ctn == ctn_parent)) goto doc_end;
  ctn = ctn_parent;
  ctn_len = (usize)(ctn->tag >> YYJSON_TAG_BIT);
  if ((ctn->tag & YYJSON_TYPE_MASK) == YYJSON_TYPE_OBJ) {
    goto obj_val_end;
  } else {
    goto arr_val_end;
  }

doc_end:

  if (unlikely(cur < end) && !has_flg(STOP_WHEN_DONE)) {
    save_incr_state(doc_end);
    while (char_is_space(*cur)) cur++;
    if (unlikely(cur < end)) goto fail_garbage;
  }

  **pre = '\0';
  doc = (yyjson_doc *)val_hdr;
  doc->root = val_hdr + hdr_len;
  doc->alc = alc;
  doc->dat_read = (usize)(cur - hdr);
  doc->val_read = (usize)((val - doc->root) + 1);
  doc->str_pool = has_flg(INSITU) ? NULL : (char *)hdr;
  state->hdr = NULL;
  state->val_hdr = NULL;
  memset(err, 0, sizeof(yyjson_read_err));
  return doc;

unexpected_end:
  err->pos = len;

  if (unlikely(len >= state->buf_len)) {
    err->code = YYJSON_READ_ERROR_UNEXPECTED_END;
    err->msg = MSG_NOT_END;
    return NULL;
  }

  err->code = YYJSON_READ_ERROR_MORE;
  err->msg = "need more data";
  state->val_end = val_end;
  state->ctn = ctn;
  state->alc_len = alc_len;

  *end = saved_end;
  return NULL;

fail_string:
  return_err(cur, INVALID_STRING, msg);
fail_number:
  return_err(cur, INVALID_NUMBER, msg);
fail_alloc:
  return_err(cur, MEMORY_ALLOCATION, MSG_MALLOC);
fail_trailing_comma:
  return_err(cur, JSON_STRUCTURE, MSG_COMMA);
fail_literal_true:
  return_err(cur, LITERAL, MSG_CHAR_T);
fail_literal_false:
  return_err(cur, LITERAL, MSG_CHAR_F);
fail_literal_null:
  return_err(cur, LITERAL, MSG_CHAR_N);
fail_character_val:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_CHAR);
fail_character_arr_end:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_ARR_END);
fail_character_obj_key:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_OBJ_KEY);
fail_character_obj_sep:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_OBJ_SEP);
fail_character_obj_end:
  return_err(cur, UNEXPECTED_CHARACTER, MSG_OBJ_END);
fail_garbage:
  return_err(cur, UNEXPECTED_CONTENT, MSG_GARBAGE);
fail_depth:
  return_err(cur, DEPTH, MSG_DEPTH);

#undef val_incr
#undef return_err
#undef return_err_inv_param
#undef save_incr_state
#undef check_maybe_truncated_number
}

#endif

#undef has_flg
#undef has_allow
#endif

#if !YYJSON_DISABLE_WRITER

#define has_flg(_flg) unlikely(has_wflag(flg, YYJSON_WRITE_##_flg, 0))
#define has_allow(_flg) unlikely(has_wflag(flg, YYJSON_WRITE_ALLOW_##_flg, 1))
static_inline bool has_wflag(yyjson_write_flag flg, yyjson_write_flag chk,
                             bool non_standard) {
#if YYJSON_DISABLE_NON_STANDARD
  if (non_standard) return false;
#endif
  return (flg & chk) != 0;
}

yyjson_align(2) static const char digit_table[200] = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0',
    '7', '0', '8', '0', '9', '1', '0', '1', '1', '1', '2', '1', '3', '1', '4',
    '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0', '2', '1', '2',
    '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
    '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3',
    '7', '3', '8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4',
    '4', '5', '4', '6', '4', '7', '4', '8', '4', '9', '5', '0', '5', '1', '5',
    '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6',
    '7', '6', '8', '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
    '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', '8', '0', '8', '1', '8',
    '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
    '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9',
    '7', '9', '8', '9', '9'};

static_inline u8 *write_u32_len_8(u32 val, u8 *buf) {
  u32 aa, bb, cc, dd, aabb, ccdd;
  aabb = (u32)(((u64)val * 109951163) >> 40);
  ccdd = val - aabb * 10000;
  aa = (aabb * 5243) >> 19;
  cc = (ccdd * 5243) >> 19;
  bb = aabb - aa * 100;
  dd = ccdd - cc * 100;
  byte_copy_2(buf + 0, digit_table + aa * 2);
  byte_copy_2(buf + 2, digit_table + bb * 2);
  byte_copy_2(buf + 4, digit_table + cc * 2);
  byte_copy_2(buf + 6, digit_table + dd * 2);
  return buf + 8;
}

static_inline u8 *write_u32_len_4(u32 val, u8 *buf) {
  u32 aa, bb;
  aa = (val * 5243) >> 19;
  bb = val - aa * 100;
  byte_copy_2(buf + 0, digit_table + aa * 2);
  byte_copy_2(buf + 2, digit_table + bb * 2);
  return buf + 4;
}

static_inline u8 *write_u32_len_1_to_8(u32 val, u8 *buf) {
  u32 aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

  if (val < 100) {
    lz = val < 10;
    byte_copy_2(buf + 0, digit_table + val * 2 + lz);
    buf -= lz;
    return buf + 2;

  } else if (val < 10000) {
    aa = (val * 5243) >> 19;
    bb = val - aa * 100;
    lz = aa < 10;
    byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
    buf -= lz;
    byte_copy_2(buf + 2, digit_table + bb * 2);
    return buf + 4;

  } else if (val < 1000000) {
    aa = (u32)(((u64)val * 429497) >> 32);
    bbcc = val - aa * 10000;
    bb = (bbcc * 5243) >> 19;
    cc = bbcc - bb * 100;
    lz = aa < 10;
    byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
    buf -= lz;
    byte_copy_2(buf + 2, digit_table + bb * 2);
    byte_copy_2(buf + 4, digit_table + cc * 2);
    return buf + 6;

  } else {
    aabb = (u32)(((u64)val * 109951163) >> 40);
    ccdd = val - aabb * 10000;
    aa = (aabb * 5243) >> 19;
    cc = (ccdd * 5243) >> 19;
    bb = aabb - aa * 100;
    dd = ccdd - cc * 100;
    lz = aa < 10;
    byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
    buf -= lz;
    byte_copy_2(buf + 2, digit_table + bb * 2);
    byte_copy_2(buf + 4, digit_table + cc * 2);
    byte_copy_2(buf + 6, digit_table + dd * 2);
    return buf + 8;
  }
}

static_inline u8 *write_u32_len_5_to_8(u32 val, u8 *buf) {
  u32 aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

  if (val < 1000000) {
    aa = (u32)(((u64)val * 429497) >> 32);
    bbcc = val - aa * 10000;
    bb = (bbcc * 5243) >> 19;
    cc = bbcc - bb * 100;
    lz = aa < 10;
    byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
    buf -= lz;
    byte_copy_2(buf + 2, digit_table + bb * 2);
    byte_copy_2(buf + 4, digit_table + cc * 2);
    return buf + 6;

  } else {
    aabb = (u32)(((u64)val * 109951163) >> 40);
    ccdd = val - aabb * 10000;
    aa = (aabb * 5243) >> 19;
    cc = (ccdd * 5243) >> 19;
    bb = aabb - aa * 100;
    dd = ccdd - cc * 100;
    lz = aa < 10;
    byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
    buf -= lz;
    byte_copy_2(buf + 2, digit_table + bb * 2);
    byte_copy_2(buf + 4, digit_table + cc * 2);
    byte_copy_2(buf + 6, digit_table + dd * 2);
    return buf + 8;
  }
}

static_inline u8 *write_u64(u64 val, u8 *buf) {
  u64 tmp, hgh;
  u32 mid, low;

  if (val < 100000000) {
    buf = write_u32_len_1_to_8((u32)val, buf);
    return buf;

  } else if (val < (u64)100000000 * 100000000) {
    hgh = val / 100000000;
    low = (u32)(val - hgh * 100000000);
    buf = write_u32_len_1_to_8((u32)hgh, buf);
    buf = write_u32_len_8(low, buf);
    return buf;

  } else {
    tmp = val / 100000000;
    low = (u32)(val - tmp * 100000000);
    hgh = (u32)(tmp / 10000);
    mid = (u32)(tmp - hgh * 10000);
    buf = write_u32_len_5_to_8((u32)hgh, buf);
    buf = write_u32_len_4(mid, buf);
    buf = write_u32_len_8(low, buf);
    return buf;
  }
}

#if !YYJSON_DISABLE_FAST_FP_CONV

static const u8 dec_trailing_zero_table[] = {
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static_inline u8 *write_u64_len_1_to_16(u64 val, u8 *buf) {
  u64 hgh;
  u32 low;
  if (val < 100000000) {
    buf = write_u32_len_1_to_8((u32)val, buf);
    return buf;
  } else {
    hgh = val / 100000000;
    low = (u32)(val - hgh * 100000000);
    buf = write_u32_len_1_to_8((u32)hgh, buf);
    buf = write_u32_len_8(low, buf);
    return buf;
  }
}

static_inline u8 *write_u64_len_1_to_17(u64 val, u8 *buf) {
  u64 hgh;
  u32 mid, low, one;
  if (val >= (u64)100000000 * 10000000) {
    hgh = val / 100000000;
    low = (u32)(val - hgh * 100000000);
    one = (u32)(hgh / 100000000);
    mid = (u32)(hgh - (u64)one * 100000000);
    *buf = (u8)((u8)one + (u8)'0');
    buf += one > 0;
    buf = write_u32_len_8(mid, buf);
    buf = write_u32_len_8(low, buf);
    return buf;
  } else if (val >= (u64)100000000) {
    hgh = val / 100000000;
    low = (u32)(val - hgh * 100000000);
    buf = write_u32_len_1_to_8((u32)hgh, buf);
    buf = write_u32_len_8(low, buf);
    return buf;
  } else {
    buf = write_u32_len_1_to_8((u32)val, buf);
    return buf;
  }
}

static_inline u8 *write_u32_len_7_to_9_trim(u32 val, u8 *buf) {
  bool lz;
  u32 tz, tz1, tz2;

  u32 abbcc = val / 10000;
  u32 ddee = val - abbcc * 10000;
  u32 abb = (u32)(((u64)abbcc * 167773) >> 24);
  u32 a = (abb * 41) >> 12;
  u32 bb = abb - a * 100;
  u32 cc = abbcc - abb * 100;

  buf[0] = (u8)(a + '0');
  buf += a > 0;
  lz = bb < 10 && a == 0;
  byte_copy_2(buf + 0, digit_table + bb * 2 + lz);
  buf -= lz;
  byte_copy_2(buf + 2, digit_table + cc * 2);

  if (ddee) {
    u32 dd = (ddee * 5243) >> 19;
    u32 ee = ddee - dd * 100;
    byte_copy_2(buf + 4, digit_table + dd * 2);
    byte_copy_2(buf + 6, digit_table + ee * 2);
    tz1 = dec_trailing_zero_table[dd];
    tz2 = dec_trailing_zero_table[ee];
    tz = ee ? tz2 : (tz1 + 2);
    buf += 8 - tz;
    return buf;
  } else {
    tz1 = dec_trailing_zero_table[bb];
    tz2 = dec_trailing_zero_table[cc];
    tz = cc ? tz2 : (tz1 + tz2);
    buf += 4 - tz;
    return buf;
  }
}

static_inline u8 *write_u64_len_16_to_17_trim(u64 val, u8 *buf) {
  u32 tz, tz1, tz2;

  u32 abbccddee = (u32)(val / 100000000);
  u32 ffgghhii = (u32)(val - (u64)abbccddee * 100000000);
  u32 abbcc = abbccddee / 10000;
  u32 ddee = abbccddee - abbcc * 10000;
  u32 abb = (u32)(((u64)abbcc * 167773) >> 24);
  u32 a = (abb * 41) >> 12;
  u32 bb = abb - a * 100;
  u32 cc = abbcc - abb * 100;
  buf[0] = (u8)(a + '0');
  buf += a > 0;
  byte_copy_2(buf + 0, digit_table + bb * 2);
  byte_copy_2(buf + 2, digit_table + cc * 2);

  if (ffgghhii) {
    u32 dd = (ddee * 5243) >> 19;
    u32 ee = ddee - dd * 100;
    u32 ffgg = (u32)(((u64)ffgghhii * 109951163) >> 40);
    u32 hhii = ffgghhii - ffgg * 10000;
    u32 ff = (ffgg * 5243) >> 19;
    u32 gg = ffgg - ff * 100;
    byte_copy_2(buf + 4, digit_table + dd * 2);
    byte_copy_2(buf + 6, digit_table + ee * 2);
    byte_copy_2(buf + 8, digit_table + ff * 2);
    byte_copy_2(buf + 10, digit_table + gg * 2);
    if (hhii) {
      u32 hh = (hhii * 5243) >> 19;
      u32 ii = hhii - hh * 100;
      byte_copy_2(buf + 12, digit_table + hh * 2);
      byte_copy_2(buf + 14, digit_table + ii * 2);
      tz1 = dec_trailing_zero_table[hh];
      tz2 = dec_trailing_zero_table[ii];
      tz = ii ? tz2 : (tz1 + 2);
      return buf + 16 - tz;
    } else {
      tz1 = dec_trailing_zero_table[ff];
      tz2 = dec_trailing_zero_table[gg];
      tz = gg ? tz2 : (tz1 + 2);
      return buf + 12 - tz;
    }
  } else {
    if (ddee) {
      u32 dd = (ddee * 5243) >> 19;
      u32 ee = ddee - dd * 100;
      byte_copy_2(buf + 4, digit_table + dd * 2);
      byte_copy_2(buf + 6, digit_table + ee * 2);
      tz1 = dec_trailing_zero_table[dd];
      tz2 = dec_trailing_zero_table[ee];
      tz = ee ? tz2 : (tz1 + 2);
      return buf + 8 - tz;
    } else {
      tz1 = dec_trailing_zero_table[bb];
      tz2 = dec_trailing_zero_table[cc];
      tz = cc ? tz2 : (tz1 + tz2);
      return buf + 4 - tz;
    }
  }
}

static_inline u8 *write_f32_exp(i32 exp, u8 *buf) {
  bool lz;
  byte_copy_2(buf, "e-");
  buf += 2 - (exp >= 0);
  exp = exp < 0 ? -exp : exp;
  lz = exp < 10;
  byte_copy_2(buf + 0, digit_table + (u32)exp * 2 + lz);
  return buf + 2 - lz;
}

static_inline u8 *write_f64_exp(i32 exp, u8 *buf) {
  byte_copy_2(buf, "e-");
  buf += 2 - (exp >= 0);
  exp = exp < 0 ? -exp : exp;
  if (exp < 100) {
    bool lz = exp < 10;
    byte_copy_2(buf + 0, digit_table + (u32)exp * 2 + lz);
    return buf + 2 - lz;
  } else {
    u32 hi = ((u32)exp * 656) >> 16;
    u32 lo = (u32)exp - hi * 100;
    buf[0] = (u8)((u8)hi + (u8)'0');
    byte_copy_2(buf + 1, digit_table + lo * 2);
    return buf + 3;
  }
}

typedef struct {
  u64 p10, mul;
  u32 shr1, shr2;
} div_pow10_magic;

static const div_pow10_magic div_pow10_table[] = {
    {U64(0x00000000, 0x00000001), U64(0x00000000, 0x00000000), 0, 0},
    {U64(0x00000000, 0x0000000A), U64(0xCCCCCCCC, 0xCCCCCCCD), 0, 3},
    {U64(0x00000000, 0x00000064), U64(0x28F5C28F, 0x5C28F5C3), 2, 2},
    {U64(0x00000000, 0x000003E8), U64(0x20C49BA5, 0xE353F7CF), 3, 4},
    {U64(0x00000000, 0x00002710), U64(0x346DC5D6, 0x3886594B), 0, 11},
    {U64(0x00000000, 0x000186A0), U64(0x0A7C5AC4, 0x71B47843), 5, 7},
    {U64(0x00000000, 0x000F4240), U64(0x431BDE82, 0xD7B634DB), 0, 18},
    {U64(0x00000000, 0x00989680), U64(0xD6BF94D5, 0xE57A42BD), 0, 23},
    {U64(0x00000000, 0x05F5E100), U64(0xABCC7711, 0x8461CEFD), 0, 26},
    {U64(0x00000000, 0x3B9ACA00), U64(0x0044B82F, 0xA09B5A53), 9, 11},
    {U64(0x00000002, 0x540BE400), U64(0xDBE6FECE, 0xBDEDD5BF), 0, 33},
    {U64(0x00000017, 0x4876E800), U64(0xAFEBFF0B, 0xCB24AAFF), 0, 36},
    {U64(0x000000E8, 0xD4A51000), U64(0x232F3302, 0x5BD42233), 0, 37},
    {U64(0x00000918, 0x4E72A000), U64(0x384B84D0, 0x92ED0385), 0, 41},
    {U64(0x00005AF3, 0x107A4000), U64(0x0B424DC3, 0x5095CD81), 0, 42},
    {U64(0x00038D7E, 0xA4C68000), U64(0x00024075, 0xF3DCEAC3), 15, 20},
    {U64(0x002386F2, 0x6FC10000), U64(0x39A5652F, 0xB1137857), 0, 51},
    {U64(0x01634578, 0x5D8A0000), U64(0x00005C3B, 0xD5191B53), 17, 22},
    {U64(0x0DE0B6B3, 0xA7640000), U64(0x000049C9, 0x7747490F), 18, 24},
    {U64(0x8AC72304, 0x89E80000), U64(0x760F253E, 0xDB4AB0d3), 0, 62},
};

static_inline void div_pow10(u64 num, u32 exp, u64 *div, u64 *mod, u64 *p10) {
  u64 hi, lo;
  div_pow10_magic m = div_pow10_table[exp];
  u128_mul(num >> m.shr1, m.mul, &hi, &lo);
  *div = hi >> m.shr2;
  *mod = num - (*div * m.p10);
  *p10 = m.p10;
}

static_inline u32 u64_round_to_odd(u64 u, u32 cp) {
  u64 hi, lo;
  u32 y_hi, y_lo;
  u128_mul(cp, u, &hi, &lo);
  y_hi = (u32)hi;
  y_lo = (u32)(lo >> 32);
  return y_hi | (y_lo > 1);
}

static_inline u64 u128_round_to_odd(u64 hi, u64 lo, u64 cp) {
  u64 x_hi, x_lo, y_hi, y_lo;
  u128_mul(cp, lo, &x_hi, &x_lo);
  u128_mul_add(cp, hi, x_hi, &y_hi, &y_lo);
  return y_hi | (y_lo > 1);
}

static_inline void f32_bin_to_dec(u32 sig_raw, u32 exp_raw, u32 sig_bin,
                                  i32 exp_bin, u32 *sig_dec, i32 *exp_dec) {
  bool is_even, irregular, round_up, trim;
  bool u0_inside, u1_inside, w0_inside, w1_inside;
  u64 p10_hi, p10_lo, hi, lo;
  u32 s, sp, cb, cbl, cbr, vb, vbl, vbr, upper, lower, mid;
  i32 k, h;

  while (likely(sig_raw)) {
    u32 mod, dec, add_1, add_10, s_hi, s_lo;
    u32 c, half_ulp, t0, t1;

    k = (i32)(exp_bin * 315653) >> 20;
    h = exp_bin + ((-k * 217707) >> 16);
    pow10_table_get_sig(-k, &p10_hi, &p10_lo);

    cb = sig_bin << (h + 1);
    u128_mul(cb, p10_hi, &hi, &lo);
    s_hi = (u32)(hi);
    s_lo = (u32)(lo >> 32);
    mod = s_hi % 10;
    dec = s_hi - mod;

    c = (mod << (32 - 4)) | (s_lo >> 4);
    half_ulp = (u32)(p10_hi >> (32 + 4 - h));

    w1_inside = (s_lo >= ((u32)1 << 31));
    if (unlikely(s_lo == ((u32)1 << 31))) break;
    u0_inside = (half_ulp >= c);
    if (unlikely(half_ulp == c)) break;
    t0 = (u32)10 << (32 - 4);
    t1 = c + half_ulp;
    w0_inside = (t1 >= t0);
    if (unlikely(t0 - t1 <= (u32)1)) break;

    trim = (u0_inside | w0_inside);
    add_10 = (w0_inside ? 10 : 0);
    add_1 = mod + w1_inside;
    *sig_dec = dec + (trim ? add_10 : add_1);
    *exp_dec = k;
    return;
  }

  irregular = (sig_raw == 0 && exp_raw > 1);
  is_even = !(sig_bin & 1);
  cbl = 4 * sig_bin - 2 + irregular;
  cb = 4 * sig_bin;
  cbr = 4 * sig_bin + 2;

  k = (i32)(exp_bin * 315653 - (irregular ? 131237 : 0)) >> 20;
  h = exp_bin + ((-k * 217707) >> 16) + 1;
  pow10_table_get_sig(-k, &p10_hi, &p10_lo);
  p10_hi += 1;

  vbl = u64_round_to_odd(p10_hi, cbl << h);
  vb = u64_round_to_odd(p10_hi, cb << h);
  vbr = u64_round_to_odd(p10_hi, cbr << h);
  lower = vbl + !is_even;
  upper = vbr - !is_even;

  s = vb / 4;
  if (s >= 10) {
    sp = s / 10;
    u0_inside = (lower <= 40 * sp);
    w0_inside = (upper >= 40 * sp + 40);
    if (u0_inside != w0_inside) {
      *sig_dec = sp * 10 + (w0_inside ? 10 : 0);
      *exp_dec = k;
      return;
    }
  }
  u1_inside = (lower <= 4 * s);
  w1_inside = (upper >= 4 * s + 4);
  mid = 4 * s + 2;
  round_up = (vb > mid) || (vb == mid && (s & 1) != 0);
  *sig_dec = s + ((u1_inside != w1_inside) ? w1_inside : round_up);
  *exp_dec = k;
}

static_inline void f64_bin_to_dec(u64 sig_raw, u32 exp_raw, u64 sig_bin,
                                  i32 exp_bin, u64 *sig_dec, i32 *exp_dec) {
  bool is_even, irregular, round_up, trim;
  bool u0_inside, u1_inside, w0_inside, w1_inside;
  u64 s, sp, cb, cbl, cbr, vb, vbl, vbr, p10_hi, p10_lo, upper, lower, mid;
  i32 k, h;

  while (likely(sig_raw)) {
    u64 mod, dec, add_1, add_10, s_hi, s_lo;
    u64 c, half_ulp, t0, t1;

    k = (i32)(exp_bin * 315653) >> 20;
    h = exp_bin + ((-k * 217707) >> 16);
    pow10_table_get_sig(-k, &p10_hi, &p10_lo);

    cb = sig_bin << (h + 1);
    u128_mul(cb, p10_lo, &s_hi, &s_lo);
    u128_mul_add(cb, p10_hi, s_hi, &s_hi, &s_lo);
    mod = s_hi % 10;
    dec = s_hi - mod;

    c = (mod << (64 - 4)) | (s_lo >> 4);
    half_ulp = p10_hi >> (4 - h);

    w1_inside = (s_lo >= ((u64)1 << 63));
    if (unlikely(s_lo == ((u64)1 << 63))) break;
    u0_inside = (half_ulp >= c);
    if (unlikely(half_ulp == c)) break;
    t0 = ((u64)10 << (64 - 4));
    t1 = c + half_ulp;
    w0_inside = (t1 >= t0);
    if (unlikely(t0 - t1 <= (u64)1)) break;

    trim = (u0_inside | w0_inside);
    add_10 = (w0_inside ? 10 : 0);
    add_1 = mod + w1_inside;
    *sig_dec = dec + (trim ? add_10 : add_1);
    *exp_dec = k;
    return;
  }

  irregular = (sig_raw == 0 && exp_raw > 1);
  is_even = !(sig_bin & 1);
  cbl = 4 * sig_bin - 2 + irregular;
  cb = 4 * sig_bin;
  cbr = 4 * sig_bin + 2;

  k = (i32)(exp_bin * 315653 - (irregular ? 131237 : 0)) >> 20;
  h = exp_bin + ((-k * 217707) >> 16) + 1;
  pow10_table_get_sig(-k, &p10_hi, &p10_lo);
  p10_lo += 1;

  vbl = u128_round_to_odd(p10_hi, p10_lo, cbl << h);
  vb = u128_round_to_odd(p10_hi, p10_lo, cb << h);
  vbr = u128_round_to_odd(p10_hi, p10_lo, cbr << h);
  lower = vbl + !is_even;
  upper = vbr - !is_even;

  s = vb / 4;
  if (s >= 10) {
    sp = s / 10;
    u0_inside = (lower <= 40 * sp);
    w0_inside = (upper >= 40 * sp + 40);
    if (u0_inside != w0_inside) {
      *sig_dec = sp * 10 + (w0_inside ? 10 : 0);
      *exp_dec = k;
      return;
    }
  }
  u1_inside = (lower <= 4 * s);
  w1_inside = (upper >= 4 * s + 4);
  mid = 4 * s + 2;
  round_up = (vb > mid) || (vb == mid && (s & 1) != 0);
  *sig_dec = s + ((u1_inside != w1_inside) ? w1_inside : round_up);
  *exp_dec = k;
}

static_inline void f64_bin_to_dec_fast(u64 sig_raw, u32 exp_raw, u64 sig_bin,
                                       i32 exp_bin, u64 *sig_dec, i32 *exp_dec,
                                       bool *round_up) {
  u64 cb, p10_hi, p10_lo, s_hi, s_lo;
  i32 k, h;
  bool irregular, u;

  irregular = (sig_raw == 0 && exp_raw > 1);

  k = (i32)(exp_bin * 315653 - (irregular ? 131237 : 0)) >> 20;
  h = exp_bin + ((-k * 217707) >> 16);
  pow10_table_get_sig(-k, &p10_hi, &p10_lo);

  cb = sig_bin << (h + 1);
  u128_mul(cb, p10_lo, &s_hi, &s_lo);
  u128_mul_add(cb, p10_hi, s_hi, &s_hi, &s_lo);

  u = s_lo >= (irregular ? U64(0x55555555, 0x55555555) : ((u64)1 << 63));

  *sig_dec = s_hi + u;
  *exp_dec = k;
  *round_up = u;
  return;
}

static_inline u8 *write_inf_or_nan(u8 *buf, yyjson_write_flag flg, u64 sig_raw,
                                   bool sign) {
  if (has_flg(INF_AND_NAN_AS_NULL)) {
    byte_copy_4(buf, "null");
    return buf + 4;
  }
  if (has_allow(INF_AND_NAN)) {
    if (sig_raw == 0) {
      buf[0] = '-';
      buf += sign;
      byte_copy_8(buf, "Infinity");
      return buf + 8;
    } else {
      byte_copy_4(buf, "NaN");
      return buf + 3;
    }
  }
  return NULL;
}

static_noinline u8 *write_f32_raw(u8 *buf, u64 raw_f64, yyjson_write_flag flg) {
  u32 sig_bin, sig_dec, sig_raw;
  i32 exp_bin, exp_dec, sig_len, dot_ofs;
  u32 exp_raw, raw;
  u8 *end;
  bool sign;

  raw = f32_to_bits(f64_to_f32(f64_from_bits(raw_f64)));

  sign = (bool)(raw >> (F32_BITS - 1));
  sig_raw = raw & F32_SIG_MASK;
  exp_raw = (raw & F32_EXP_MASK) >> F32_SIG_BITS;

  if (unlikely(exp_raw == ((u32)1 << F32_EXP_BITS) - 1)) {
    return write_inf_or_nan(buf, flg, sig_raw, sign);
  }

  buf[0] = '-';
  buf += sign;

  if ((raw << 1) == 0) {
    byte_copy_4(buf, "0.0");
    return buf + 3;
  }

  if (likely(exp_raw != 0)) {
    sig_bin = sig_raw | ((u32)1 << F32_SIG_BITS);
    exp_bin = (i32)exp_raw - F32_EXP_BIAS - F32_SIG_BITS;

    if ((-F32_SIG_BITS <= exp_bin && exp_bin <= 0) &&
        (u64_tz_bits(sig_bin) >= (u32)-exp_bin)) {
      sig_dec = sig_bin >> -exp_bin;
      buf = write_u32_len_1_to_8(sig_dec, buf);
      byte_copy_2(buf, ".0");
      return buf + 2;
    }

    f32_bin_to_dec(sig_raw, exp_raw, sig_bin, exp_bin, &sig_dec, &exp_dec);

    sig_len = 7 + (sig_dec >= (u32)10000000) + (sig_dec >= (u32)100000000);

    dot_ofs = sig_len + exp_dec;

    if (-6 < dot_ofs && dot_ofs <= 21) {
      i32 num_sep_pos, dot_set_pos, pre_ofs;
      u8 *num_hdr, *num_end, *num_sep, *dot_end;
      bool no_pre_zero;

      memset(buf, '0', 32);

      no_pre_zero = (dot_ofs > 0);

      pre_ofs = no_pre_zero ? 0 : (2 - dot_ofs);
      num_hdr = buf + pre_ofs;
      num_end = write_u32_len_7_to_9_trim(sig_dec, num_hdr);

      num_sep_pos = no_pre_zero ? dot_ofs : 0;
      num_sep = num_hdr + num_sep_pos;
      byte_move_8(num_sep + no_pre_zero, num_sep);
      num_end += no_pre_zero;

      dot_set_pos = yyjson_max(dot_ofs, 1);
      buf[dot_set_pos] = '.';

      dot_end = buf + dot_ofs + 2;
      return yyjson_max(dot_end, num_end);

    } else {
      end = write_u32_len_7_to_9_trim(sig_dec, buf + 1);
      end -= (end == buf + 2);
      exp_dec += sig_len - 1;
      buf[0] = buf[1];
      buf[1] = '.';
      return write_f32_exp(exp_dec, end);
    }

  } else {
    sig_bin = sig_raw;
    exp_bin = 1 - F32_EXP_BIAS - F32_SIG_BITS;

    f32_bin_to_dec(sig_raw, exp_raw, sig_bin, exp_bin, &sig_dec, &exp_dec);

    end = write_u32_len_1_to_8(sig_dec, buf + 1);
    buf[0] = buf[1];
    buf[1] = '.';
    exp_dec += (i32)(end - buf) - 2;

    end -= *(end - 1) == '0';
    end -= *(end - 1) == '0';
    while (*(end - 1) == '0') end--;
    end -= *(end - 1) == '.';

    return write_f32_exp(exp_dec, end);
  }
}

static_noinline u8 *write_f64_raw(u8 *buf, u64 raw, yyjson_write_flag flg) {
  u64 sig_bin, sig_dec, sig_raw;
  i32 exp_bin, exp_dec, sig_len, dot_ofs;
  u32 exp_raw;
  u8 *end;
  bool sign;

  sign = (bool)(raw >> (F64_BITS - 1));
  sig_raw = raw & F64_SIG_MASK;
  exp_raw = (u32)((raw & F64_EXP_MASK) >> F64_SIG_BITS);

  if (unlikely(exp_raw == ((u32)1 << F64_EXP_BITS) - 1)) {
    return write_inf_or_nan(buf, flg, sig_raw, sign);
  }

  buf[0] = '-';
  buf += sign;

  if ((raw << 1) == 0) {
    byte_copy_4(buf, "0.0");
    return buf + 3;
  }

  if (likely(exp_raw != 0)) {
    sig_bin = sig_raw | ((u64)1 << F64_SIG_BITS);
    exp_bin = (i32)exp_raw - F64_EXP_BIAS - F64_SIG_BITS;

    if ((-F64_SIG_BITS <= exp_bin && exp_bin <= 0) &&
        (u64_tz_bits(sig_bin) >= (u32)-exp_bin)) {
      sig_dec = sig_bin >> -exp_bin;
      buf = write_u64_len_1_to_16(sig_dec, buf);
      byte_copy_2(buf, ".0");
      return buf + 2;
    }

    f64_bin_to_dec(sig_raw, exp_raw, sig_bin, exp_bin, &sig_dec, &exp_dec);

    sig_len = 16 + (sig_dec >= (u64)100000000 * 100000000);

    dot_ofs = sig_len + exp_dec;

    if (-6 < dot_ofs && dot_ofs <= 21) {
      i32 num_sep_pos, dot_set_pos, pre_ofs;
      u8 *num_hdr, *num_end, *num_sep, *dot_end;
      bool no_pre_zero;

      memset(buf, '0', 32);

      no_pre_zero = (dot_ofs > 0);

      pre_ofs = no_pre_zero ? 0 : (2 - dot_ofs);
      num_hdr = buf + pre_ofs;
      num_end = write_u64_len_16_to_17_trim(sig_dec, num_hdr);

      num_sep_pos = no_pre_zero ? dot_ofs : 0;
      num_sep = num_hdr + num_sep_pos;
      byte_move_16(num_sep + no_pre_zero, num_sep);
      num_end += no_pre_zero;

      dot_set_pos = yyjson_max(dot_ofs, 1);
      buf[dot_set_pos] = '.';

      dot_end = buf + dot_ofs + 2;
      return yyjson_max(dot_end, num_end);

    } else {
      end = write_u64_len_16_to_17_trim(sig_dec, buf + 1);
      end -= (end == buf + 2);
      exp_dec += sig_len - 1;
      buf[0] = buf[1];
      buf[1] = '.';
      return write_f64_exp(exp_dec, end);
    }

  } else {
    sig_bin = sig_raw;
    exp_bin = 1 - F64_EXP_BIAS - F64_SIG_BITS;

    f64_bin_to_dec(sig_raw, exp_raw, sig_bin, exp_bin, &sig_dec, &exp_dec);

    end = write_u64_len_1_to_17(sig_dec, buf + 1);
    buf[0] = buf[1];
    buf[1] = '.';
    exp_dec += (i32)(end - buf) - 2;

    end -= *(end - 1) == '0';
    end -= *(end - 1) == '0';
    while (*(end - 1) == '0') end--;
    end -= *(end - 1) == '.';

    return write_f64_exp(exp_dec, end);
  }
}

static_noinline u8 *write_f64_raw_fixed(u8 *buf, u64 raw, yyjson_write_flag flg,
                                        u32 prec) {
  u64 sig_bin, sig_dec, sig_raw;
  i32 exp_bin, exp_dec, sig_len, dot_ofs;
  u32 exp_raw;
  u8 *end;
  bool sign;

  sign = (bool)(raw >> (F64_BITS - 1));
  sig_raw = raw & F64_SIG_MASK;
  exp_raw = (u32)((raw & F64_EXP_MASK) >> F64_SIG_BITS);

  if (unlikely(exp_raw == ((u32)1 << F64_EXP_BITS) - 1)) {
    return write_inf_or_nan(buf, flg, sig_raw, sign);
  }

  buf[0] = '-';
  buf += sign;

  if ((raw << 1) == 0) {
    byte_copy_4(buf, "0.0");
    return buf + 3;
  }

  if (likely(exp_raw != 0)) {
    sig_bin = sig_raw | ((u64)1 << F64_SIG_BITS);
    exp_bin = (i32)exp_raw - F64_EXP_BIAS - F64_SIG_BITS;

    if ((-F64_SIG_BITS <= exp_bin && exp_bin <= 0) &&
        (u64_tz_bits(sig_bin) >= (u32)-exp_bin)) {
      sig_dec = sig_bin >> -exp_bin;
      buf = write_u64_len_1_to_16(sig_dec, buf);
      byte_copy_2(buf, ".0");
      return buf + 2;
    }

    if ((raw << 1) < (U64(0x444B1AE4, 0xD6E2EF50) << 1)) {
      i32 num_sep_pos, dot_set_pos, pre_ofs;
      u8 *num_hdr, *num_end, *num_sep;
      bool round_up, no_pre_zero;

      f64_bin_to_dec_fast(sig_raw, exp_raw, sig_bin, exp_bin, &sig_dec,
                          &exp_dec, &round_up);

      sig_len = 16 + (sig_dec >= (u64)100000000 * 100000000);

      if (exp_dec < -1) {
        i32 sig_len_cut = -exp_dec - (i32)prec;
        if (sig_len_cut > sig_len) {
          byte_copy_4(buf, "0.0");
          return buf + 3;
        }
        if (sig_len_cut > 0) {
          u64 div, mod, p10;

          sig_dec -= round_up;
          sig_len = 16 + (sig_dec >= (u64)100000000 * 100000000);

          div_pow10(sig_dec, (u32)sig_len_cut, &div, &mod, &p10);

          sig_dec = div + (mod >= p10 / 2);

          exp_dec += sig_len_cut;
          sig_len -= sig_len_cut;
          sig_len +=
              (sig_len >= 0) && (sig_dec >= div_pow10_table[sig_len].p10);
        }
        if (sig_len <= 0) {
          byte_copy_4(buf, "0.0");
          return buf + 3;
        }
      }

      memset(buf, '0', 32);

      dot_ofs = sig_len + exp_dec;

      no_pre_zero = (dot_ofs > 0);

      pre_ofs = no_pre_zero ? 0 : (1 - dot_ofs);
      num_hdr = buf + pre_ofs;
      num_end = write_u64_len_1_to_17(sig_dec, num_hdr);

      num_sep_pos = no_pre_zero ? dot_ofs : -dot_ofs;
      num_sep = buf + num_sep_pos;
      byte_move_16(num_sep + 1, num_sep);
      num_end += (exp_dec < 0);

      dot_set_pos = yyjson_max(dot_ofs, 1);
      buf[dot_set_pos] = '.';

      buf += dot_set_pos + 2;
      buf = yyjson_max(buf, num_end);
      buf -= *(buf - 1) == '0';
      buf -= *(buf - 1) == '0';
      while (*(buf - 1) == '0') buf--;
      buf += *(buf - 1) == '.';
      return buf;

    } else {
      f64_bin_to_dec(sig_raw, exp_raw, sig_bin, exp_bin, &sig_dec, &exp_dec);

      sig_len = 16 + (sig_dec >= (u64)100000000 * 100000000);

      end = write_u64_len_16_to_17_trim(sig_dec, buf + 1);
      end -= (end == buf + 2);
      exp_dec += sig_len - 1;
      buf[0] = buf[1];
      buf[1] = '.';
      return write_f64_exp(exp_dec, end);
    }
  } else {
    byte_copy_4(buf, "0.0");
    return buf + 3;
  }
}

#else

#if YYJSON_MSC_VER >= 1400
#define snprintf_num(buf, len, fmt, dig, val) \
  sprintf_s((char *)buf, len, fmt, dig, val)
#elif defined(snprintf) || (YYJSON_STDC_VER >= 199901L)
#define snprintf_num(buf, len, fmt, dig, val) \
  snprintf((char *)buf, len, fmt, dig, val)
#else
#define snprintf_num(buf, len, fmt, dig, val) \
  sprintf((char *)buf, fmt, dig, val)
#endif

static_noinline u8 *write_fp_reformat(u8 *buf, int len, yyjson_write_flag flg,
                                      bool fixed) {
  u8 *cur = buf;
  if (unlikely(len < 1)) return NULL;
  cur += (*cur == '-');
  if (unlikely(!char_is_digit(*cur))) {
    if (has_flg(INF_AND_NAN_AS_NULL)) {
      byte_copy_4(buf, "null");
      return buf + 4;
    } else if (has_allow(INF_AND_NAN)) {
      if (*cur == 'i') {
        byte_copy_8(cur, "Infinity");
        return cur + 8;
      } else if (*cur == 'n') {
        byte_copy_4(buf, "NaN");
        return buf + 3;
      }
    }
    return NULL;
  } else {
    u8 *end = buf + len, *dot = NULL, *exp = NULL;

    for (; cur < end; cur++) {
      switch (*cur) {
        case ',':
          *cur = '.';
        case '.':
          dot = cur;
          break;
        case 'e':
          exp = cur;
          break;
        default:
          break;
      }
    }
    if (fixed) {
      while (*(end - 1) == '0') end--;
      end += *(end - 1) == '.';
    } else {
      if (!dot && !exp) {
        byte_copy_2(end, ".0");
        end += 2;
      } else if (exp) {
        cur = exp + 1;

        if (*cur == '+') {
          memmove(cur, cur + 1, (usize)(end - cur - 1));
          end--;
        }
        cur += (*cur == '-');

        if (*cur == '0') {
          u8 *hdr = cur++;
          while (*cur == '0') cur++;
          memmove(hdr, cur, (usize)(end - cur));
          end -= (usize)(cur - hdr);
        }
      }
    }
    return end;
  }
}

static_noinline u8 *write_f64_raw(u8 *buf, u64 raw, yyjson_write_flag flg) {
#if defined(DBL_DECIMAL_DIG) && DBL_DECIMAL_DIG < F64_DEC_DIG
  int dig = DBL_DECIMAL_DIG;
#else
  int dig = F64_DEC_DIG;
#endif
  f64 val = f64_from_bits(raw);
  int len = snprintf_num(buf, FP_BUF_LEN, "%.*g", dig, val);
  return write_fp_reformat(buf, len, flg, false);
}

static_noinline u8 *write_f32_raw(u8 *buf, u64 raw, yyjson_write_flag flg) {
#if defined(FLT_DECIMAL_DIG) && FLT_DECIMAL_DIG < F32_DEC_DIG
  int dig = FLT_DECIMAL_DIG;
#else
  int dig = F32_DEC_DIG;
#endif
  f64 val = (f64)f64_to_f32(f64_from_bits(raw));
  int len = snprintf_num(buf, FP_BUF_LEN, "%.*g", dig, val);
  return write_fp_reformat(buf, len, flg, false);
}

static_noinline u8 *write_f64_raw_fixed(u8 *buf, u64 raw, yyjson_write_flag flg,
                                        u32 prec) {
  f64 val = (f64)f64_from_bits(raw);
  if (-1e21 < val && val < 1e21) {
    int len = snprintf_num(buf, FP_BUF_LEN, "%.*f", (int)prec, val);
    return write_fp_reformat(buf, len, flg, true);
  } else {
    return write_f64_raw(buf, raw, flg);
  }
}

#endif

static_inline u8 *write_num(u8 *cur, yyjson_val *val, yyjson_write_flag flg) {
  if (!(val->tag & YYJSON_SUBTYPE_REAL)) {
    u64 pos = val->uni.u64;
    u64 neg = ~pos + 1;
    usize sign = ((val->tag & YYJSON_SUBTYPE_SINT) > 0) & ((i64)pos < 0);
    *cur = '-';
    return write_u64(sign ? neg : pos, cur + sign);
  } else {
    u64 raw = val->uni.u64;
    u32 val_fmt = (u32)(val->tag >> 32);
    u32 all_fmt = flg;
    u32 fmt = val_fmt | all_fmt;
    if (likely(!(fmt >> (32 - YYJSON_WRITE_FP_FLAG_BITS)))) {
      return write_f64_raw(cur, raw, flg);
    } else if (fmt >> (32 - YYJSON_WRITE_FP_PREC_BITS)) {
      u32 val_prec = val_fmt >> (32 - YYJSON_WRITE_FP_PREC_BITS);
      u32 all_prec = all_fmt >> (32 - YYJSON_WRITE_FP_PREC_BITS);
      u32 prec = val_prec ? val_prec : all_prec;
      return write_f64_raw_fixed(cur, raw, flg, prec);
    } else {
      if (fmt & YYJSON_WRITE_FP_TO_FLOAT) {
        return write_f32_raw(cur, raw, flg);
      } else {
        return write_f64_raw(cur, raw, flg);
      }
    }
  }
}

char *yyjson_write_number(const yyjson_val *val, char *buf) {
  if (unlikely(!val || !buf)) return NULL;
  switch (val->tag & YYJSON_TAG_MASK) {
    case YYJSON_TYPE_NUM | YYJSON_SUBTYPE_UINT: {
      buf = (char *)write_u64(val->uni.u64, (u8 *)buf);
      *buf = '\0';
      return buf;
    }
    case YYJSON_TYPE_NUM | YYJSON_SUBTYPE_SINT: {
      u64 pos = val->uni.u64;
      u64 neg = ~pos + 1;
      usize sign = ((i64)pos < 0);
      *buf = '-';
      buf = (char *)write_u64(sign ? neg : pos, (u8 *)buf + sign);
      *buf = '\0';
      return buf;
    }
    case YYJSON_TYPE_NUM | YYJSON_SUBTYPE_REAL: {
      u64 raw = val->uni.u64;
      u32 fmt = (u32)(val->tag >> 32);
      u32 flg = YYJSON_WRITE_ALLOW_INF_AND_NAN;
      if (likely(!(fmt >> (32 - YYJSON_WRITE_FP_FLAG_BITS)))) {
        buf = (char *)write_f64_raw((u8 *)buf, raw, flg);
      } else if (fmt >> (32 - YYJSON_WRITE_FP_PREC_BITS)) {
        u32 prec = fmt >> (32 - YYJSON_WRITE_FP_PREC_BITS);
        buf = (char *)write_f64_raw_fixed((u8 *)buf, raw, flg, prec);
      } else {
        if (fmt & YYJSON_WRITE_FP_TO_FLOAT) {
          buf = (char *)write_f32_raw((u8 *)buf, raw, flg);
        } else {
          buf = (char *)write_f64_raw((u8 *)buf, raw, flg);
        }
      }
      if (buf) *buf = '\0';
      return buf;
    }
    default:
      return NULL;
  }
}

typedef u8 char_enc_type;
#define CHAR_ENC_CPY_1 0
#define CHAR_ENC_ERR_1 1
#define CHAR_ENC_ESC_A 2
#define CHAR_ENC_ESC_1 3
#define CHAR_ENC_CPY_2 4
#define CHAR_ENC_ESC_2 5
#define CHAR_ENC_CPY_3 6
#define CHAR_ENC_ESC_3 7
#define CHAR_ENC_CPY_4 8
#define CHAR_ENC_ESC_4 9

static const char_enc_type enc_table_cpy[256] = {
    3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 3, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    8, 8, 8, 8, 8, 8, 8, 8, 1, 1, 1, 1, 1, 1, 1, 1};

static const char_enc_type enc_table_cpy_slash[256] = {
    3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 3, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    8, 8, 8, 8, 8, 8, 8, 8, 1, 1, 1, 1, 1, 1, 1, 1};

static const char_enc_type enc_table_esc[256] = {
    3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 3, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    9, 9, 9, 9, 9, 9, 9, 9, 1, 1, 1, 1, 1, 1, 1, 1};

static const char_enc_type enc_table_esc_slash[256] = {
    3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 3, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    9, 9, 9, 9, 9, 9, 9, 9, 1, 1, 1, 1, 1, 1, 1, 1};

yyjson_align(2) static const u8 esc_hex_char_table[512] = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0',
    '7', '0', '8', '0', '9', '0', 'A', '0', 'B', '0', 'C', '0', 'D', '0', 'E',
    '0', 'F', '1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1',
    '6', '1', '7', '1', '8', '1', '9', '1', 'A', '1', 'B', '1', 'C', '1', 'D',
    '1', 'E', '1', 'F', '2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2',
    '5', '2', '6', '2', '7', '2', '8', '2', '9', '2', 'A', '2', 'B', '2', 'C',
    '2', 'D', '2', 'E', '2', 'F', '3', '0', '3', '1', '3', '2', '3', '3', '3',
    '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9', '3', 'A', '3', 'B',
    '3', 'C', '3', 'D', '3', 'E', '3', 'F', '4', '0', '4', '1', '4', '2', '4',
    '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8', '4', '9', '4', 'A',
    '4', 'B', '4', 'C', '4', 'D', '4', 'E', '4', 'F', '5', '0', '5', '1', '5',
    '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '5', 'A', '5', 'B', '5', 'C', '5', 'D', '5', 'E', '5', 'F', '6', '0', '6',
    '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8',
    '6', '9', '6', 'A', '6', 'B', '6', 'C', '6', 'D', '6', 'E', '6', 'F', '7',
    '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7',
    '7', '8', '7', '9', '7', 'A', '7', 'B', '7', 'C', '7', 'D', '7', 'E', '7',
    'F', '8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6',
    '8', '7', '8', '8', '8', '9', '8', 'A', '8', 'B', '8', 'C', '8', 'D', '8',
    'E', '8', 'F', '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5',
    '9', '6', '9', '7', '9', '8', '9', '9', '9', 'A', '9', 'B', '9', 'C', '9',
    'D', '9', 'E', '9', 'F', 'A', '0', 'A', '1', 'A', '2', 'A', '3', 'A', '4',
    'A', '5', 'A', '6', 'A', '7', 'A', '8', 'A', '9', 'A', 'A', 'A', 'B', 'A',
    'C', 'A', 'D', 'A', 'E', 'A', 'F', 'B', '0', 'B', '1', 'B', '2', 'B', '3',
    'B', '4', 'B', '5', 'B', '6', 'B', '7', 'B', '8', 'B', '9', 'B', 'A', 'B',
    'B', 'B', 'C', 'B', 'D', 'B', 'E', 'B', 'F', 'C', '0', 'C', '1', 'C', '2',
    'C', '3', 'C', '4', 'C', '5', 'C', '6', 'C', '7', 'C', '8', 'C', '9', 'C',
    'A', 'C', 'B', 'C', 'C', 'C', 'D', 'C', 'E', 'C', 'F', 'D', '0', 'D', '1',
    'D', '2', 'D', '3', 'D', '4', 'D', '5', 'D', '6', 'D', '7', 'D', '8', 'D',
    '9', 'D', 'A', 'D', 'B', 'D', 'C', 'D', 'D', 'D', 'E', 'D', 'F', 'E', '0',
    'E', '1', 'E', '2', 'E', '3', 'E', '4', 'E', '5', 'E', '6', 'E', '7', 'E',
    '8', 'E', '9', 'E', 'A', 'E', 'B', 'E', 'C', 'E', 'D', 'E', 'E', 'E', 'F',
    'F', '0', 'F', '1', 'F', '2', 'F', '3', 'F', '4', 'F', '5', 'F', '6', 'F',
    '7', 'F', '8', 'F', '9', 'F', 'A', 'F', 'B', 'F', 'C', 'F', 'D', 'F', 'E',
    'F', 'F'};

yyjson_align(2) static const u8 esc_hex_char_table_lower[512] = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0',
    '7', '0', '8', '0', '9', '0', 'a', '0', 'b', '0', 'c', '0', 'd', '0', 'e',
    '0', 'f', '1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1',
    '6', '1', '7', '1', '8', '1', '9', '1', 'a', '1', 'b', '1', 'c', '1', 'd',
    '1', 'e', '1', 'f', '2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2',
    '5', '2', '6', '2', '7', '2', '8', '2', '9', '2', 'a', '2', 'b', '2', 'c',
    '2', 'd', '2', 'e', '2', 'f', '3', '0', '3', '1', '3', '2', '3', '3', '3',
    '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9', '3', 'a', '3', 'b',
    '3', 'c', '3', 'd', '3', 'e', '3', 'f', '4', '0', '4', '1', '4', '2', '4',
    '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8', '4', '9', '4', 'a',
    '4', 'b', '4', 'c', '4', 'd', '4', 'e', '4', 'f', '5', '0', '5', '1', '5',
    '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '5', 'a', '5', 'b', '5', 'c', '5', 'd', '5', 'e', '5', 'f', '6', '0', '6',
    '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8',
    '6', '9', '6', 'a', '6', 'b', '6', 'c', '6', 'd', '6', 'e', '6', 'f', '7',
    '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7',
    '7', '8', '7', '9', '7', 'a', '7', 'b', '7', 'c', '7', 'd', '7', 'e', '7',
    'f', '8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6',
    '8', '7', '8', '8', '8', '9', '8', 'a', '8', 'b', '8', 'c', '8', 'd', '8',
    'e', '8', 'f', '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5',
    '9', '6', '9', '7', '9', '8', '9', '9', '9', 'a', '9', 'b', '9', 'c', '9',
    'd', '9', 'e', '9', 'f', 'a', '0', 'a', '1', 'a', '2', 'a', '3', 'a', '4',
    'a', '5', 'a', '6', 'a', '7', 'a', '8', 'a', '9', 'a', 'a', 'a', 'b', 'a',
    'c', 'a', 'd', 'a', 'e', 'a', 'f', 'b', '0', 'b', '1', 'b', '2', 'b', '3',
    'b', '4', 'b', '5', 'b', '6', 'b', '7', 'b', '8', 'b', '9', 'b', 'a', 'b',
    'b', 'b', 'c', 'b', 'd', 'b', 'e', 'b', 'f', 'c', '0', 'c', '1', 'c', '2',
    'c', '3', 'c', '4', 'c', '5', 'c', '6', 'c', '7', 'c', '8', 'c', '9', 'c',
    'a', 'c', 'b', 'c', 'c', 'c', 'd', 'c', 'e', 'c', 'f', 'd', '0', 'd', '1',
    'd', '2', 'd', '3', 'd', '4', 'd', '5', 'd', '6', 'd', '7', 'd', '8', 'd',
    '9', 'd', 'a', 'd', 'b', 'd', 'c', 'd', 'd', 'd', 'e', 'd', 'f', 'e', '0',
    'e', '1', 'e', '2', 'e', '3', 'e', '4', 'e', '5', 'e', '6', 'e', '7', 'e',
    '8', 'e', '9', 'e', 'a', 'e', 'b', 'e', 'c', 'e', 'd', 'e', 'e', 'e', 'f',
    'f', '0', 'f', '1', 'f', '2', 'f', '3', 'f', '4', 'f', '5', 'f', '6', 'f',
    '7', 'f', '8', 'f', '9', 'f', 'a', 'f', 'b', 'f', 'c', 'f', 'd', 'f', 'e',
    'f', 'f'};

yyjson_align(2) static const u8 esc_single_char_table[512] = {
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', '\\', 'b',  '\\', 't', '\\', 'n', ' ', ' ', '\\', 'f', '\\', 'r',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', '\\', '"',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', '\\', '/', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', '\\', '\\', ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ',
    ' ', ' ', ' ',  ' ',  ' ',  ' ', ' ',  ' '};

static_inline const u8 *get_hex_table_with_flag(yyjson_write_flag flg) {
  return has_flg(LOWERCASE_HEX) ? esc_hex_char_table_lower : esc_hex_char_table;
}

static_inline const char_enc_type *get_enc_table_with_flag(
    yyjson_write_flag flg) {
  if (has_flg(ESCAPE_UNICODE)) {
    if (has_flg(ESCAPE_SLASHES)) {
      return enc_table_esc_slash;
    } else {
      return enc_table_esc;
    }
  } else {
    if (has_flg(ESCAPE_SLASHES)) {
      return enc_table_cpy_slash;
    } else {
      return enc_table_cpy;
    }
  }
}

static_inline u8 *write_raw(u8 *cur, const u8 *raw, usize raw_len) {
  memcpy(cur, raw, raw_len);
  return cur + raw_len;
}

static_inline u8 *write_str_noesc(u8 *cur, const u8 *str, usize str_len) {
  *cur++ = '"';
  while (str_len >= 16) {
    byte_copy_16(cur, str);
    cur += 16;
    str += 16;
    str_len -= 16;
  }
  while (str_len >= 4) {
    byte_copy_4(cur, str);
    cur += 4;
    str += 4;
    str_len -= 4;
  }
  while (str_len) {
    *cur++ = *str++;
    str_len -= 1;
  }
  *cur++ = '"';
  return cur;
}

static_inline u8 *write_str(u8 *cur, bool esc, bool inv, const u8 *str,
                            usize str_len, const char_enc_type *enc_table,
                            const u8 *hex_table) {
  const v32 pre = {{'\\', 'u', '0', '0'}};

  const u8 *src = str;
  const u8 *end = str + str_len;
  *cur++ = '"';

copy_ascii:

#define expr_jump(i) \
  if (unlikely(enc_table[src[i]])) goto stop_char_##i;

#define expr_stop(i)                   \
  stop_char_##i : memcpy(cur, src, i); \
  cur += i;                            \
  src += i;                            \
  goto copy_utf8;

  while (end - src >= 16) {
    repeat16_incr(expr_jump) byte_copy_16(cur, src);
    cur += 16;
    src += 16;
  }

  while (end - src >= 4) {
    repeat4_incr(expr_jump) byte_copy_4(cur, src);
    cur += 4;
    src += 4;
  }

  while (end > src) {
    expr_jump(0) *cur++ = *src++;
  }

  *cur++ = '"';
  return cur;

  repeat16_incr(expr_stop)
#undef expr_jump
#undef expr_stop

      copy_utf8 : if (unlikely(src + 4 > end)) {
    if (end == src) goto copy_end;
    if (end - src < enc_table[*src] / 2) goto err_one;
  }
  switch (enc_table[*src]) {
    case CHAR_ENC_CPY_1: {
      *cur++ = *src++;
      goto copy_ascii;
    }
    case CHAR_ENC_CPY_2: {
#if YYJSON_DISABLE_UTF8_VALIDATION
      byte_copy_2(cur, src);
#else
      u32 uni = 0;
      byte_copy_2(&uni, src);
      if (unlikely(!is_utf8_seq2(uni))) goto err_cpy;
      byte_copy_2(cur, &uni);
#endif
      cur += 2;
      src += 2;
      goto copy_utf8;
    }
    case CHAR_ENC_CPY_3: {
#if YYJSON_DISABLE_UTF8_VALIDATION
      if (likely(src + 4 <= end)) {
        byte_copy_4(cur, src);
      } else {
        byte_copy_2(cur, src);
        cur[2] = src[2];
      }
#else
      u32 uni, tmp;
      if (likely(src + 4 <= end)) {
        uni = byte_load_4(src);
        if (unlikely(!is_utf8_seq3(uni))) goto err_cpy;
        byte_copy_4(cur, src);
      } else {
        uni = byte_load_3(src);
        if (unlikely(!is_utf8_seq3(uni))) goto err_cpy;
        byte_copy_4(cur, &uni);
      }
#endif
      cur += 3;
      src += 3;
      goto copy_utf8;
    }
    case CHAR_ENC_CPY_4: {
#if YYJSON_DISABLE_UTF8_VALIDATION
      byte_copy_4(cur, src);
#else
      u32 uni, tmp;
      uni = byte_load_4(src);
      if (unlikely(!is_utf8_seq4(uni))) goto err_cpy;
      byte_copy_4(cur, src);
#endif
      cur += 4;
      src += 4;
      goto copy_utf8;
    }
    case CHAR_ENC_ESC_A: {
      byte_copy_2(cur, &esc_single_char_table[*src * 2]);
      cur += 2;
      src += 1;
      goto copy_utf8;
    }
    case CHAR_ENC_ESC_1: {
      byte_copy_4(cur + 0, &pre);
      byte_copy_2(cur + 4, &hex_table[*src * 2]);
      cur += 6;
      src += 1;
      goto copy_utf8;
    }
    case CHAR_ENC_ESC_2: {
      u16 u;
#if !YYJSON_DISABLE_UTF8_VALIDATION
      u32 v4 = 0;
      u16 v2 = byte_load_2(src);
      byte_copy_2(&v4, &v2);
      if (unlikely(!is_utf8_seq2(v4))) goto err_esc;
#endif
      u = (u16)(((u16)(src[0] & 0x1F) << 6) | ((u16)(src[1] & 0x3F) << 0));
      byte_copy_2(cur + 0, &pre);
      byte_copy_2(cur + 2, &hex_table[(u >> 8) * 2]);
      byte_copy_2(cur + 4, &hex_table[(u & 0xFF) * 2]);
      cur += 6;
      src += 2;
      goto copy_utf8;
    }
    case CHAR_ENC_ESC_3: {
      u16 u;
      u32 v, tmp;
#if !YYJSON_DISABLE_UTF8_VALIDATION
      v = byte_load_3(src);
      if (unlikely(!is_utf8_seq3(v))) goto err_esc;
#endif
      u = (u16)(((u16)(src[0] & 0x0F) << 12) | ((u16)(src[1] & 0x3F) << 6) |
                ((u16)(src[2] & 0x3F) << 0));
      byte_copy_2(cur + 0, &pre);
      byte_copy_2(cur + 2, &hex_table[(u >> 8) * 2]);
      byte_copy_2(cur + 4, &hex_table[(u & 0xFF) * 2]);
      cur += 6;
      src += 3;
      goto copy_utf8;
    }
    case CHAR_ENC_ESC_4: {
      u32 hi, lo, u, v, tmp;
#if !YYJSON_DISABLE_UTF8_VALIDATION
      v = byte_load_4(src);
      if (unlikely(!is_utf8_seq4(v))) goto err_esc;
#endif
      u = ((u32)(src[0] & 0x07) << 18) | ((u32)(src[1] & 0x3F) << 12) |
          ((u32)(src[2] & 0x3F) << 6) | ((u32)(src[3] & 0x3F) << 0);
      u -= 0x10000;
      hi = (u >> 10) + 0xD800;
      lo = (u & 0x3FF) + 0xDC00;
      byte_copy_2(cur + 0, &pre);
      byte_copy_2(cur + 2, &hex_table[(hi >> 8) * 2]);
      byte_copy_2(cur + 4, &hex_table[(hi & 0xFF) * 2]);
      byte_copy_2(cur + 6, &pre);
      byte_copy_2(cur + 8, &hex_table[(lo >> 8) * 2]);
      byte_copy_2(cur + 10, &hex_table[(lo & 0xFF) * 2]);
      cur += 12;
      src += 4;
      goto copy_utf8;
    }
    case CHAR_ENC_ERR_1: {
      goto err_one;
    }
    default:
      break;
  }

copy_end:
  *cur++ = '"';
  return cur;

err_one:
  if (esc)
    goto err_esc;
  else
    goto err_cpy;

err_cpy:
  if (!inv) return NULL;
  *cur++ = *src++;
  goto copy_utf8;

err_esc:
  if (!inv) return NULL;
  byte_copy_2(cur + 0, &pre);

  byte_copy_2(cur + 2, &hex_table[0xFF * 2]);
  byte_copy_2(cur + 4, &hex_table[0xFD * 2]);
  cur += 6;
  src += 1;
  goto copy_utf8;
}

static_inline u8 *write_null(u8 *cur) {
  v64 v = {{'n', 'u', 'l', 'l', ',', '\n', 0, 0}};
  byte_copy_8(cur, &v);
  return cur + 4;
}

static_inline u8 *write_bool(u8 *cur, bool val) {
  v64 v0 = {{'f', 'a', 'l', 's', 'e', ',', '\n', 0}};
  v64 v1 = {{'t', 'r', 'u', 'e', ',', '\n', 0, 0}};
  if (val) {
    byte_copy_8(cur, &v1);
  } else {
    byte_copy_8(cur, &v0);
  }
  return cur + 5 - val;
}

static_inline u8 *write_indent(u8 *cur, usize level, usize spaces) {
  while (level-- > 0) {
    byte_copy_4(cur, "    ");
    cur += spaces;
  }
  return cur;
}

static bool write_dat_to_fp(FILE *fp, u8 *dat, usize len,
                            yyjson_write_err *err) {
  if (fwrite(dat, len, 1, fp) != 1) {
    err->msg = "file writing failed";
    err->code = YYJSON_WRITE_ERROR_FILE_WRITE;
    return false;
  }
  return true;
}

static bool write_dat_to_file(const char *path, u8 *dat, usize len,
                              yyjson_write_err *err) {
#define return_err(_code, _msg)             \
  do {                                      \
    err->msg = _msg;                        \
    err->code = YYJSON_WRITE_ERROR_##_code; \
    if (file) fclose(file);                 \
    return false;                           \
  } while (false)

  FILE *file = fopen_writeonly(path);
  if (file == NULL) {
    return_err(FILE_OPEN, MSG_FOPEN);
  }
  if (fwrite(dat, len, 1, file) != 1) {
    return_err(FILE_WRITE, MSG_FWRITE);
  }
  if (fclose(file) != 0) {
    file = NULL;
    return_err(FILE_WRITE, MSG_FCLOSE);
  }
  return true;

#undef return_err
}

typedef struct yyjson_write_ctx {
  usize tag;
} yyjson_write_ctx;

static_inline void yyjson_write_ctx_set(yyjson_write_ctx *ctx, usize size,
                                        bool is_obj) {
  ctx->tag = (size << 1) | (usize)is_obj;
}

static_inline void yyjson_write_ctx_get(yyjson_write_ctx *ctx, usize *size,
                                        bool *is_obj) {
  usize tag = ctx->tag;
  *size = tag >> 1;
  *is_obj = (bool)(tag & 1);
}

static_inline u8 *write_root_single(yyjson_val *val, yyjson_write_flag flg,
                                    yyjson_alc alc, char *buf, usize *dat_len,
                                    yyjson_write_err *err) {
#define return_err(_code, _msg)              \
  do {                                       \
    if (hdr) alc.free(alc.ctx, (void *)hdr); \
    *dat_len = 0;                            \
    err->code = YYJSON_WRITE_ERROR_##_code;  \
    err->msg = _msg;                         \
    return NULL;                             \
  } while (false)

#define incr_len(_len)                                 \
  do {                                                 \
    if (buf)                                           \
      hdr = *dat_len >= _len ? (u8 *)buf : (u8 *)NULL; \
    else                                               \
      hdr = (u8 *)alc.malloc(alc.ctx, _len);           \
    if (!hdr) goto fail_alloc;                         \
    cur = hdr;                                         \
  } while (false)

#define check_str_len(_len)                                    \
  do {                                                         \
    if ((sizeof(usize) < 8) && (_len >= (USIZE_MAX - 16) / 6)) \
      goto fail_alloc;                                         \
  } while (false)

  u8 *hdr = NULL, *cur;
  usize str_len;
  const u8 *str_ptr;
  const char_enc_type *enc_table = get_enc_table_with_flag(flg);
  const u8 *hex_table = get_hex_table_with_flag(flg);
  bool cpy = (enc_table == enc_table_cpy);
  bool esc = has_flg(ESCAPE_UNICODE) != 0;
  bool inv = has_allow(INVALID_UNICODE) != 0;
  bool newline = has_flg(NEWLINE_AT_END) != 0;
  const usize end_len = 2;

  switch (unsafe_yyjson_get_type(val)) {
    case YYJSON_TYPE_RAW:
      str_len = unsafe_yyjson_get_len(val);
      str_ptr = (const u8 *)unsafe_yyjson_get_str(val);
      check_str_len(str_len);
      incr_len(str_len + end_len);
      cur = write_raw(cur, str_ptr, str_len);
      break;

    case YYJSON_TYPE_STR:
      str_len = unsafe_yyjson_get_len(val);
      str_ptr = (const u8 *)unsafe_yyjson_get_str(val);
      check_str_len(str_len);
      incr_len(str_len * 6 + 2 + end_len);
      if (likely(cpy) && unsafe_yyjson_get_subtype(val)) {
        cur = write_str_noesc(cur, str_ptr, str_len);
      } else {
        cur = write_str(cur, esc, inv, str_ptr, str_len, enc_table, hex_table);
        if (unlikely(!cur)) goto fail_str;
      }
      break;

    case YYJSON_TYPE_NUM:
      incr_len(FP_BUF_LEN + end_len);
      cur = write_num(cur, val, flg);
      if (unlikely(!cur)) goto fail_num;
      break;

    case YYJSON_TYPE_BOOL:
      incr_len(8);
      cur = write_bool(cur, unsafe_yyjson_get_bool(val));
      break;

    case YYJSON_TYPE_NULL:
      incr_len(8);
      cur = write_null(cur);
      break;

    case YYJSON_TYPE_ARR:
      incr_len(2 + end_len);
      byte_copy_2(cur, "[]");
      cur += 2;
      break;

    case YYJSON_TYPE_OBJ:
      incr_len(2 + end_len);
      byte_copy_2(cur, "{}");
      cur += 2;
      break;

    default:
      goto fail_type;
  }

  if (newline) *cur++ = '\n';
  *cur = '\0';
  *dat_len = (usize)(cur - hdr);
  memset(err, 0, sizeof(yyjson_write_err));
  return hdr;

fail_alloc:
  return_err(MEMORY_ALLOCATION, MSG_MALLOC);
fail_type:
  return_err(INVALID_VALUE_TYPE, MSG_ERR_TYPE);
fail_num:
  return_err(NAN_OR_INF, MSG_NAN_INF);
fail_str:
  return_err(INVALID_STRING, MSG_ERR_UTF8);

#undef return_err
#undef check_str_len
#undef incr_len
}

static_inline u8 *write_root_minify(const yyjson_val *root,
                                    const yyjson_write_flag flg,
                                    const yyjson_alc alc, char *buf,
                                    usize *dat_len, yyjson_write_err *err) {
#define return_err(_code, _msg)             \
  do {                                      \
    *dat_len = 0;                           \
    err->code = YYJSON_WRITE_ERROR_##_code; \
    err->msg = _msg;                        \
    if (hdr) alc.free(alc.ctx, hdr);        \
    return NULL;                            \
  } while (false)

#define incr_len(_len)                                                   \
  do {                                                                   \
    ext_len = (usize)(_len);                                             \
    if (unlikely((u8 *)(cur + ext_len) >= (u8 *)ctx)) {                  \
      usize ctx_pos = (usize)((u8 *)ctx - hdr);                          \
      usize cur_pos = (usize)(cur - hdr);                                \
      ctx_len = (usize)(end - (u8 *)ctx);                                \
      alc_inc = yyjson_max(alc_len / 2, ext_len);                        \
      alc_inc = size_align_up(alc_inc, sizeof(yyjson_write_ctx));        \
      if ((sizeof(usize) < 8) && size_add_is_overflow(alc_len, alc_inc)) \
        goto fail_alloc;                                                 \
      alc_len += alc_inc;                                                \
      tmp = (u8 *)alc.realloc(alc.ctx, hdr, alc_len - alc_inc, alc_len); \
      if (unlikely(!tmp)) goto fail_alloc;                               \
      ctx_tmp = (yyjson_write_ctx *)(void *)(tmp + (alc_len - ctx_len)); \
      memmove((void *)ctx_tmp, (void *)(tmp + ctx_pos), ctx_len);        \
      ctx = ctx_tmp;                                                     \
      cur = tmp + cur_pos;                                               \
      end = tmp + alc_len;                                               \
      hdr = tmp;                                                         \
    }                                                                    \
  } while (false)

#define check_str_len(_len)                                    \
  do {                                                         \
    if ((sizeof(usize) < 8) && (_len >= (USIZE_MAX - 16) / 6)) \
      goto fail_alloc;                                         \
  } while (false)

  yyjson_val *val;
  yyjson_type val_type;
  usize ctn_len, ctn_len_tmp;
  bool ctn_obj, ctn_obj_tmp, is_key;
  u8 *hdr, *cur, *end, *tmp;
  yyjson_write_ctx *ctx, *ctx_tmp;
  usize alc_len, alc_inc, ctx_len, ext_len, str_len;
  const u8 *str_ptr;
  const char_enc_type *enc_table = get_enc_table_with_flag(flg);
  const u8 *hex_table = get_hex_table_with_flag(flg);
  bool cpy = (enc_table == enc_table_cpy);
  bool esc = has_flg(ESCAPE_UNICODE) != 0;
  bool inv = has_allow(INVALID_UNICODE) != 0;
  bool newline = has_flg(NEWLINE_AT_END) != 0;

  if (buf) {
    hdr = (u8 *)buf;
    alc_len = *dat_len;
    alc_len = size_align_down(alc_len, sizeof(yyjson_write_ctx));
    if (alc_len <= sizeof(yyjson_write_ctx)) goto fail_alloc;
  } else {
    alc_len = root->uni.ofs / sizeof(yyjson_val);
    alc_len = alc_len * YYJSON_WRITER_ESTIMATED_MINIFY_RATIO + 64;
    alc_len = size_align_up(alc_len, sizeof(yyjson_write_ctx));
    hdr = (u8 *)alc.malloc(alc.ctx, alc_len);
    if (!hdr) goto fail_alloc;
  }
  cur = hdr;
  end = hdr + alc_len;
  ctx = (yyjson_write_ctx *)(void *)end;

doc_begin:
  val = constcast(yyjson_val *) root;
  val_type = unsafe_yyjson_get_type(val);
  ctn_obj = (val_type == YYJSON_TYPE_OBJ);
  ctn_len = unsafe_yyjson_get_len(val) << (u8)ctn_obj;
  *cur++ = (u8)('[' | ((u8)ctn_obj << 5));
  val++;

val_begin:
  val_type = unsafe_yyjson_get_type(val);
  if (val_type == YYJSON_TYPE_STR) {
    is_key = ((u8)ctn_obj & (u8)~ctn_len);
    str_len = unsafe_yyjson_get_len(val);
    str_ptr = (const u8 *)unsafe_yyjson_get_str(val);
    check_str_len(str_len);
    incr_len(str_len * 6 + 16);
    if (likely(cpy) && unsafe_yyjson_get_subtype(val)) {
      cur = write_str_noesc(cur, str_ptr, str_len);
    } else {
      cur = write_str(cur, esc, inv, str_ptr, str_len, enc_table, hex_table);
      if (unlikely(!cur)) goto fail_str;
    }
    *cur++ = is_key ? ':' : ',';
    goto val_end;
  }
  if (val_type == YYJSON_TYPE_NUM) {
    incr_len(FP_BUF_LEN);
    cur = write_num(cur, val, flg);
    if (unlikely(!cur)) goto fail_num;
    *cur++ = ',';
    goto val_end;
  }
  if ((val_type & (YYJSON_TYPE_ARR & YYJSON_TYPE_OBJ)) ==
      (YYJSON_TYPE_ARR & YYJSON_TYPE_OBJ)) {
    ctn_len_tmp = unsafe_yyjson_get_len(val);
    ctn_obj_tmp = (val_type == YYJSON_TYPE_OBJ);
    incr_len(2 * sizeof(*ctx));
    if (unlikely(ctn_len_tmp == 0)) {
      *cur++ = (u8)('[' | ((u8)ctn_obj_tmp << 5));
      *cur++ = (u8)(']' | ((u8)ctn_obj_tmp << 5));
      *cur++ = ',';
      goto val_end;
    } else {
      yyjson_write_ctx_set(--ctx, ctn_len, ctn_obj);
      ctn_len = ctn_len_tmp << (u8)ctn_obj_tmp;
      ctn_obj = ctn_obj_tmp;
      *cur++ = (u8)('[' | ((u8)ctn_obj << 5));
      val++;
      goto val_begin;
    }
  }
  if (val_type == YYJSON_TYPE_BOOL) {
    incr_len(16);
    cur = write_bool(cur, unsafe_yyjson_get_bool(val));
    cur++;
    goto val_end;
  }
  if (val_type == YYJSON_TYPE_NULL) {
    incr_len(16);
    cur = write_null(cur);
    cur++;
    goto val_end;
  }
  if (val_type == YYJSON_TYPE_RAW) {
    str_len = unsafe_yyjson_get_len(val);
    str_ptr = (const u8 *)unsafe_yyjson_get_str(val);
    check_str_len(str_len);
    incr_len(str_len + 2);
    cur = write_raw(cur, str_ptr, str_len);
    *cur++ = ',';
    goto val_end;
  }
  goto fail_type;

val_end:
  val++;
  ctn_len--;
  if (unlikely(ctn_len == 0)) goto ctn_end;
  goto val_begin;

ctn_end:
  cur--;
  *cur++ = (u8)(']' | ((u8)ctn_obj << 5));
  *cur++ = ',';
  if (unlikely((u8 *)ctx >= end)) goto doc_end;
  yyjson_write_ctx_get(ctx++, &ctn_len, &ctn_obj);
  ctn_len--;
  if (likely(ctn_len > 0)) {
    goto val_begin;
  } else {
    goto ctn_end;
  }

doc_end:
  if (newline) {
    incr_len(2);
    *(cur - 1) = '\n';
    cur++;
  }
  *--cur = '\0';
  *dat_len = (usize)(cur - hdr);
  memset(err, 0, sizeof(yyjson_write_err));
  return hdr;

fail_alloc:
  return_err(MEMORY_ALLOCATION, MSG_MALLOC);
fail_type:
  return_err(INVALID_VALUE_TYPE, MSG_ERR_TYPE);
fail_num:
  return_err(NAN_OR_INF, MSG_NAN_INF);
fail_str:
  return_err(INVALID_STRING, MSG_ERR_UTF8);

#undef return_err
#undef incr_len
#undef check_str_len
}

static_inline u8 *write_root_pretty(const yyjson_val *root,
                                    const yyjson_write_flag flg,
                                    const yyjson_alc alc, char *buf,
                                    usize *dat_len, yyjson_write_err *err) {
#define return_err(_code, _msg)             \
  do {                                      \
    *dat_len = 0;                           \
    err->code = YYJSON_WRITE_ERROR_##_code; \
    err->msg = _msg;                        \
    if (hdr) alc.free(alc.ctx, hdr);        \
    return NULL;                            \
  } while (false)

#define incr_len(_len)                                                   \
  do {                                                                   \
    ext_len = (usize)(_len);                                             \
    if (unlikely((u8 *)(cur + ext_len) >= (u8 *)ctx)) {                  \
      usize ctx_pos = (usize)((u8 *)ctx - hdr);                          \
      usize cur_pos = (usize)(cur - hdr);                                \
      ctx_len = (usize)(end - (u8 *)ctx);                                \
      alc_inc = yyjson_max(alc_len / 2, ext_len);                        \
      alc_inc = size_align_up(alc_inc, sizeof(yyjson_write_ctx));        \
      if ((sizeof(usize) < 8) && size_add_is_overflow(alc_len, alc_inc)) \
        goto fail_alloc;                                                 \
      alc_len += alc_inc;                                                \
      tmp = (u8 *)alc.realloc(alc.ctx, hdr, alc_len - alc_inc, alc_len); \
      if (unlikely(!tmp)) goto fail_alloc;                               \
      ctx_tmp = (yyjson_write_ctx *)(void *)(tmp + (alc_len - ctx_len)); \
      memmove((void *)ctx_tmp, (void *)(tmp + ctx_pos), ctx_len);        \
      ctx = ctx_tmp;                                                     \
      cur = tmp + cur_pos;                                               \
      end = tmp + alc_len;                                               \
      hdr = tmp;                                                         \
    }                                                                    \
  } while (false)

#define check_str_len(_len)                                    \
  do {                                                         \
    if ((sizeof(usize) < 8) && (_len >= (USIZE_MAX - 16) / 6)) \
      goto fail_alloc;                                         \
  } while (false)

  yyjson_val *val;
  yyjson_type val_type;
  usize ctn_len, ctn_len_tmp;
  bool ctn_obj, ctn_obj_tmp, is_key, no_indent;
  u8 *hdr, *cur, *end, *tmp;
  yyjson_write_ctx *ctx, *ctx_tmp;
  usize alc_len, alc_inc, ctx_len, ext_len, str_len, level;
  const u8 *str_ptr;
  const char_enc_type *enc_table = get_enc_table_with_flag(flg);
  const u8 *hex_table = get_hex_table_with_flag(flg);
  bool cpy = (enc_table == enc_table_cpy);
  bool esc = has_flg(ESCAPE_UNICODE) != 0;
  bool inv = has_allow(INVALID_UNICODE) != 0;
  usize spaces = has_flg(PRETTY_TWO_SPACES) ? 2 : 4;
  bool newline = has_flg(NEWLINE_AT_END) != 0;

  if (buf) {
    hdr = (u8 *)buf;
    alc_len = *dat_len;
    alc_len = size_align_down(alc_len, sizeof(yyjson_write_ctx));
    if (alc_len <= sizeof(yyjson_write_ctx)) goto fail_alloc;
  } else {
    alc_len = root->uni.ofs / sizeof(yyjson_val);
    alc_len = alc_len * YYJSON_WRITER_ESTIMATED_PRETTY_RATIO + 64;
    alc_len = size_align_up(alc_len, sizeof(yyjson_write_ctx));
    hdr = (u8 *)alc.malloc(alc.ctx, alc_len);
    if (!hdr) goto fail_alloc;
  }
  cur = hdr;
  end = hdr + alc_len;
  ctx = (yyjson_write_ctx *)(void *)end;

doc_begin:
  val = constcast(yyjson_val *) root;
  val_type = unsafe_yyjson_get_type(val);
  ctn_obj = (val_type == YYJSON_TYPE_OBJ);
  ctn_len = unsafe_yyjson_get_len(val) << (u8)ctn_obj;
  *cur++ = (u8)('[' | ((u8)ctn_obj << 5));
  *cur++ = '\n';
  val++;
  level = 1;

val_begin:
  val_type = unsafe_yyjson_get_type(val);
  if (val_type == YYJSON_TYPE_STR) {
    is_key = (bool)((u8)ctn_obj & (u8)~ctn_len);
    no_indent = (bool)((u8)ctn_obj & (u8)ctn_len);
    str_len = unsafe_yyjson_get_len(val);
    str_ptr = (const u8 *)unsafe_yyjson_get_str(val);
    check_str_len(str_len);
    incr_len(str_len * 6 + 16 + (no_indent ? 0 : level * 4));
    cur = write_indent(cur, no_indent ? 0 : level, spaces);
    if (likely(cpy) && unsafe_yyjson_get_subtype(val)) {
      cur = write_str_noesc(cur, str_ptr, str_len);
    } else {
      cur = write_str(cur, esc, inv, str_ptr, str_len, enc_table, hex_table);
      if (unlikely(!cur)) goto fail_str;
    }
    *cur++ = is_key ? ':' : ',';
    *cur++ = is_key ? ' ' : '\n';
    goto val_end;
  }
  if (val_type == YYJSON_TYPE_NUM) {
    no_indent = (bool)((u8)ctn_obj & (u8)ctn_len);
    incr_len(FP_BUF_LEN + (no_indent ? 0 : level * 4));
    cur = write_indent(cur, no_indent ? 0 : level, spaces);
    cur = write_num(cur, val, flg);
    if (unlikely(!cur)) goto fail_num;
    *cur++ = ',';
    *cur++ = '\n';
    goto val_end;
  }
  if ((val_type & (YYJSON_TYPE_ARR & YYJSON_TYPE_OBJ)) ==
      (YYJSON_TYPE_ARR & YYJSON_TYPE_OBJ)) {
    no_indent = (bool)((u8)ctn_obj & (u8)ctn_len);
    ctn_len_tmp = unsafe_yyjson_get_len(val);
    ctn_obj_tmp = (val_type == YYJSON_TYPE_OBJ);
    incr_len(2 * sizeof(*ctx) + (no_indent ? 0 : level * 4));
    if (unlikely(ctn_len_tmp == 0)) {
      cur = write_indent(cur, no_indent ? 0 : level, spaces);
      *cur++ = (u8)('[' | ((u8)ctn_obj_tmp << 5));
      *cur++ = (u8)(']' | ((u8)ctn_obj_tmp << 5));
      *cur++ = ',';
      *cur++ = '\n';
      goto val_end;
    } else {
      yyjson_write_ctx_set(--ctx, ctn_len, ctn_obj);
      ctn_len = ctn_len_tmp << (u8)ctn_obj_tmp;
      ctn_obj = ctn_obj_tmp;
      cur = write_indent(cur, no_indent ? 0 : level, spaces);
      level++;
      *cur++ = (u8)('[' | ((u8)ctn_obj << 5));
      *cur++ = '\n';
      val++;
      goto val_begin;
    }
  }
  if (val_type == YYJSON_TYPE_BOOL) {
    no_indent = (bool)((u8)ctn_obj & (u8)ctn_len);
    incr_len(16 + (no_indent ? 0 : level * 4));
    cur = write_indent(cur, no_indent ? 0 : level, spaces);
    cur = write_bool(cur, unsafe_yyjson_get_bool(val));
    cur += 2;
    goto val_end;
  }
  if (val_type == YYJSON_TYPE_NULL) {
    no_indent = (bool)((u8)ctn_obj & (u8)ctn_len);
    incr_len(16 + (no_indent ? 0 : level * 4));
    cur = write_indent(cur, no_indent ? 0 : level, spaces);
    cur = write_null(cur);
    cur += 2;
    goto val_end;
  }
  if (val_type == YYJSON_TYPE_RAW) {
    no_indent = (bool)((u8)ctn_obj & (u8)ctn_len);
    str_len = unsafe_yyjson_get_len(val);
    str_ptr = (const u8 *)unsafe_yyjson_get_str(val);
    check_str_len(str_len);
    incr_len(str_len + 3 + (no_indent ? 0 : level * 4));
    cur = write_indent(cur, no_indent ? 0 : level, spaces);
    cur = write_raw(cur, str_ptr, str_len);
    *cur++ = ',';
    *cur++ = '\n';
    goto val_end;
  }
  goto fail_type;

val_end:
  val++;
  ctn_len--;
  if (unlikely(ctn_len == 0)) goto ctn_end;
  goto val_begin;

ctn_end:
  cur -= 2;
  *cur++ = '\n';
  incr_len(level * 4);
  cur = write_indent(cur, --level, spaces);
  *cur++ = (u8)(']' | ((u8)ctn_obj << 5));
  if (unlikely((u8 *)ctx >= end)) goto doc_end;
  yyjson_write_ctx_get(ctx++, &ctn_len, &ctn_obj);
  ctn_len--;
  *cur++ = ',';
  *cur++ = '\n';
  if (likely(ctn_len > 0)) {
    goto val_begin;
  } else {
    goto ctn_end;
  }

doc_end:
  if (newline) {
    incr_len(2);
    *cur++ = '\n';
  }
  *cur = '\0';
  *dat_len = (usize)(cur - hdr);
  memset(err, 0, sizeof(yyjson_write_err));
  return hdr;

fail_alloc:
  return_err(MEMORY_ALLOCATION, MSG_MALLOC);
fail_type:
  return_err(INVALID_VALUE_TYPE, MSG_ERR_TYPE);
fail_num:
  return_err(NAN_OR_INF, MSG_NAN_INF);
fail_str:
  return_err(INVALID_STRING, MSG_ERR_UTF8);

#undef return_err
#undef incr_len
#undef check_str_len
}

static char *write_root(const yyjson_val *val, yyjson_write_flag flg,
                        const yyjson_alc *alc_ptr, char *buf, usize *dat_len,
                        yyjson_write_err *err) {
  yyjson_write_err tmp_err;
  usize tmp_dat_len;
  yyjson_alc alc = alc_ptr ? *alc_ptr : YYJSON_DEFAULT_ALC;
  yyjson_val *root = constcast(yyjson_val *) val;

  if (!err) err = &tmp_err;
  if (!dat_len) dat_len = &tmp_dat_len;

  if (unlikely(!root)) {
    *dat_len = 0;
    err->msg = "input JSON is NULL";
    err->code = YYJSON_READ_ERROR_INVALID_PARAMETER;
    return NULL;
  }

  if (!unsafe_yyjson_is_ctn(root) || unsafe_yyjson_get_len(root) == 0) {
    return (char *)write_root_single(root, flg, alc, buf, dat_len, err);
  } else if (flg & (YYJSON_WRITE_PRETTY | YYJSON_WRITE_PRETTY_TWO_SPACES)) {
    return (char *)write_root_pretty(root, flg, alc, buf, dat_len, err);
  } else {
    return (char *)write_root_minify(root, flg, alc, buf, dat_len, err);
  }
}

char *yyjson_val_write_opts(const yyjson_val *val, yyjson_write_flag flg,
                            const yyjson_alc *alc_ptr, usize *dat_len,
                            yyjson_write_err *err) {
  return write_root(val, flg, alc_ptr, NULL, dat_len, err);
}

char *yyjson_write_opts(const yyjson_doc *doc, yyjson_write_flag flg,
                        const yyjson_alc *alc_ptr, usize *dat_len,
                        yyjson_write_err *err) {
  yyjson_val *root = doc ? doc->root : NULL;
  return write_root(root, flg, alc_ptr, NULL, dat_len, err);
}

bool yyjson_val_write_file(const char *path, const yyjson_val *val,
                           yyjson_write_flag flg, const yyjson_alc *alc_ptr,
                           yyjson_write_err *err) {
  yyjson_write_err tmp_err;
  yyjson_alc alc = alc_ptr ? *alc_ptr : YYJSON_DEFAULT_ALC;
  u8 *dat;
  usize dat_len = 0;
  yyjson_val *root = constcast(yyjson_val *) val;
  bool suc;

  if (!err) err = &tmp_err;
  if (unlikely(!path || !*path)) {
    err->msg = "input path is invalid";
    err->code = YYJSON_READ_ERROR_INVALID_PARAMETER;
    return false;
  }

  dat = (u8 *)write_root(root, flg, &alc, NULL, &dat_len, err);
  if (unlikely(!dat)) return false;
  suc = write_dat_to_file(path, dat, dat_len, err);
  alc.free(alc.ctx, dat);
  return suc;
}

bool yyjson_val_write_fp(FILE *fp, const yyjson_val *val, yyjson_write_flag flg,
                         const yyjson_alc *alc_ptr, yyjson_write_err *err) {
  yyjson_write_err tmp_err;
  yyjson_alc alc = alc_ptr ? *alc_ptr : YYJSON_DEFAULT_ALC;
  u8 *dat;
  usize dat_len = 0;
  yyjson_val *root = constcast(yyjson_val *) val;
  bool suc;

  if (!err) err = &tmp_err;
  if (unlikely(!fp)) {
    err->msg = "input fp is invalid";
    err->code = YYJSON_READ_ERROR_INVALID_PARAMETER;
    return false;
  }

  dat = (u8 *)write_root(root, flg, &alc, NULL, &dat_len, err);
  if (unlikely(!dat)) return false;
  suc = write_dat_to_fp(fp, dat, dat_len, err);
  alc.free(alc.ctx, dat);
  return suc;
}

size_t yyjson_val_write_buf(char *buf, size_t buf_len, const yyjson_val *val,
                            yyjson_write_flag flg, yyjson_write_err *err) {
  if (unlikely(!buf || !buf_len)) {
    if (err) err->code = YYJSON_WRITE_ERROR_INVALID_PARAMETER;
    if (err) err->msg = "input buf or buf_len is invalid";
    return 0;
  } else {
    write_root(val, flg, &YYJSON_NULL_ALC, buf, &buf_len, err);
    return buf_len;
  }
}

bool yyjson_write_file(const char *path, const yyjson_doc *doc,
                       yyjson_write_flag flg, const yyjson_alc *alc_ptr,
                       yyjson_write_err *err) {
  yyjson_val *root = doc ? doc->root : NULL;
  return yyjson_val_write_file(path, root, flg, alc_ptr, err);
}

bool yyjson_write_fp(FILE *fp, const yyjson_doc *doc, yyjson_write_flag flg,
                     const yyjson_alc *alc_ptr, yyjson_write_err *err) {
  yyjson_val *root = doc ? doc->root : NULL;
  return yyjson_val_write_fp(fp, root, flg, alc_ptr, err);
}

size_t yyjson_write_buf(char *buf, size_t buf_len, const yyjson_doc *doc,
                        yyjson_write_flag flg, yyjson_write_err *err) {
  yyjson_val *root = doc ? doc->root : NULL;
  return yyjson_val_write_buf(buf, buf_len, root, flg, err);
}

typedef struct yyjson_mut_write_ctx {
  usize tag;
  yyjson_mut_val *ctn;
} yyjson_mut_write_ctx;

static_inline void yyjson_mut_write_ctx_set(yyjson_mut_write_ctx *ctx,
                                            yyjson_mut_val *ctn, usize size,
                                            bool is_obj) {
  ctx->tag = (size << 1) | (usize)is_obj;
  ctx->ctn = ctn;
}

static_inline void yyjson_mut_write_ctx_get(yyjson_mut_write_ctx *ctx,
                                            yyjson_mut_val **ctn, usize *size,
                                            bool *is_obj) {
  usize tag = ctx->tag;
  *size = tag >> 1;
  *is_obj = (bool)(tag & 1);
  *ctn = ctx->ctn;
}

static_inline usize
yyjson_mut_doc_estimated_val_num(const yyjson_mut_doc *doc) {
  usize sum = 0;
  yyjson_val_chunk *chunk = doc->val_pool.chunks;
  while (chunk) {
    sum += chunk->chunk_size / sizeof(yyjson_mut_val) - 1;
    if (chunk == doc->val_pool.chunks) {
      sum -= (usize)(doc->val_pool.end - doc->val_pool.cur);
    }
    chunk = chunk->next;
  }
  return sum;
}

static_inline u8 *mut_write_root_single(yyjson_mut_val *val,
                                        yyjson_write_flag flg, yyjson_alc alc,
                                        char *buf, usize *dat_len,
                                        yyjson_write_err *err) {
  return write_root_single((yyjson_val *)val, flg, alc, buf, dat_len, err);
}

static_inline u8 *mut_write_root_minify(const yyjson_mut_val *root,
                                        usize estimated_val_num,
                                        yyjson_write_flag flg, yyjson_alc alc,
                                        char *buf, usize *dat_len,
                                        yyjson_write_err *err) {
#define return_err(_code, _msg)             \
  do {                                      \
    *dat_len = 0;                           \
    err->code = YYJSON_WRITE_ERROR_##_code; \
    err->msg = _msg;                        \
    if (hdr) alc.free(alc.ctx, hdr);        \
    return NULL;                            \
  } while (false)

#define incr_len(_len)                                                       \
  do {                                                                       \
    ext_len = (usize)(_len);                                                 \
    if (unlikely((u8 *)(cur + ext_len) >= (u8 *)ctx)) {                      \
      usize ctx_pos = (usize)((u8 *)ctx - hdr);                              \
      usize cur_pos = (usize)(cur - hdr);                                    \
      ctx_len = (usize)(end - (u8 *)ctx);                                    \
      alc_inc = yyjson_max(alc_len / 2, ext_len);                            \
      alc_inc = size_align_up(alc_inc, sizeof(yyjson_mut_write_ctx));        \
      if ((sizeof(usize) < 8) && size_add_is_overflow(alc_len, alc_inc))     \
        goto fail_alloc;                                                     \
      alc_len += alc_inc;                                                    \
      tmp = (u8 *)alc.realloc(alc.ctx, hdr, alc_len - alc_inc, alc_len);     \
      if (unlikely(!tmp)) goto fail_alloc;                                   \
      ctx_tmp = (yyjson_mut_write_ctx *)(void *)(tmp + (alc_len - ctx_len)); \
      memmove((void *)ctx_tmp, (void *)(tmp + ctx_pos), ctx_len);            \
      ctx = ctx_tmp;                                                         \
      cur = tmp + cur_pos;                                                   \
      end = tmp + alc_len;                                                   \
      hdr = tmp;                                                             \
    }                                                                        \
  } while (false)

#define check_str_len(_len)                                    \
  do {                                                         \
    if ((sizeof(usize) < 8) && (_len >= (USIZE_MAX - 16) / 6)) \
      goto fail_alloc;                                         \
  } while (false)

  yyjson_mut_val *val, *ctn;
  yyjson_type val_type;
  usize ctn_len, ctn_len_tmp;
  bool ctn_obj, ctn_obj_tmp, is_key;
  u8 *hdr, *cur, *end, *tmp;
  yyjson_mut_write_ctx *ctx, *ctx_tmp;
  usize alc_len, alc_inc, ctx_len, ext_len, str_len;
  const u8 *str_ptr;
  const char_enc_type *enc_table = get_enc_table_with_flag(flg);
  const u8 *hex_table = get_hex_table_with_flag(flg);
  bool cpy = (enc_table == enc_table_cpy);
  bool esc = has_flg(ESCAPE_UNICODE) != 0;
  bool inv = has_allow(INVALID_UNICODE) != 0;
  bool newline = has_flg(NEWLINE_AT_END) != 0;

  if (buf) {
    hdr = (u8 *)buf;
    alc_len = *dat_len;
    alc_len = size_align_down(alc_len, sizeof(yyjson_mut_write_ctx));
    if (alc_len <= sizeof(yyjson_mut_write_ctx)) goto fail_alloc;
  } else {
    alc_len = estimated_val_num * YYJSON_WRITER_ESTIMATED_MINIFY_RATIO + 64;
    alc_len = size_align_up(alc_len, sizeof(yyjson_mut_write_ctx));
    hdr = (u8 *)alc.malloc(alc.ctx, alc_len);
    if (!hdr) goto fail_alloc;
  }
  cur = hdr;
  end = hdr + alc_len;
  ctx = (yyjson_mut_write_ctx *)(void *)end;

doc_begin:
  val = constcast(yyjson_mut_val *) root;
  val_type = unsafe_yyjson_get_type(val);
  ctn_obj = (val_type == YYJSON_TYPE_OBJ);
  ctn_len = unsafe_yyjson_get_len(val) << (u8)ctn_obj;
  *cur++ = (u8)('[' | ((u8)ctn_obj << 5));
  ctn = val;
  val = (yyjson_mut_val *)val->uni.ptr;
  val = ctn_obj ? val->next->next : val->next;

val_begin:
  val_type = unsafe_yyjson_get_type(val);
  if (val_type == YYJSON_TYPE_STR) {
    is_key = ((u8)ctn_obj & (u8)~ctn_len);
    str_len = unsafe_yyjson_get_len(val);
    str_ptr = (const u8 *)unsafe_yyjson_get_str(val);
    check_str_len(str_len);
    incr_len(str_len * 6 + 16);
    if (likely(cpy) && unsafe_yyjson_get_subtype(val)) {
      cur = write_str_noesc(cur, str_ptr, str_len);
    } else {
      cur = write_str(cur, esc, inv, str_ptr, str_len, enc_table, hex_table);
      if (unlikely(!cur)) goto fail_str;
    }
    *cur++ = is_key ? ':' : ',';
    goto val_end;
  }
  if (val_type == YYJSON_TYPE_NUM) {
    incr_len(FP_BUF_LEN);
    cur = write_num(cur, (yyjson_val *)val, flg);
    if (unlikely(!cur)) goto fail_num;
    *cur++ = ',';
    goto val_end;
  }
  if ((val_type & (YYJSON_TYPE_ARR & YYJSON_TYPE_OBJ)) ==
      (YYJSON_TYPE_ARR & YYJSON_TYPE_OBJ)) {
    ctn_len_tmp = unsafe_yyjson_get_len(val);
    ctn_obj_tmp = (val_type == YYJSON_TYPE_OBJ);
    incr_len(2 * sizeof(*ctx));
    if (unlikely(ctn_len_tmp == 0)) {
      *cur++ = (u8)('[' | ((u8)ctn_obj_tmp << 5));
      *cur++ = (u8)(']' | ((u8)ctn_obj_tmp << 5));
      *cur++ = ',';
      goto val_end;
    } else {
      yyjson_mut_write_ctx_set(--ctx, ctn, ctn_len, ctn_obj);
      ctn_len = ctn_len_tmp << (u8)ctn_obj_tmp;
      ctn_obj = ctn_obj_tmp;
      *cur++ = (u8)('[' | ((u8)ctn_obj << 5));
      ctn = val;
      val = (yyjson_mut_val *)ctn->uni.ptr;
      val = ctn_obj ? val->next->next : val->next;
      goto val_begin;
    }
  }
  if (val_type == YYJSON_TYPE_BOOL) {
    incr_len(16);
    cur = write_bool(cur, unsafe_yyjson_get_bool(val));
    cur++;
    goto val_end;
  }
  if (val_type == YYJSON_TYPE_NULL) {
    incr_len(16);
    cur = write_null(cur);
    cur++;
    goto val_end;
  }
  if (val_type == YYJSON_TYPE_RAW) {
    str_len = unsafe_yyjson_get_len(val);
    str_ptr = (const u8 *)unsafe_yyjson_get_str(val);
    check_str_len(str_len);
    incr_len(str_len + 2);
    cur = write_raw(cur, str_ptr, str_len);
    *cur++ = ',';
    goto val_end;
  }
  goto fail_type;

val_end:
  ctn_len--;
  if (unlikely(ctn_len == 0)) goto ctn_end;
  val = val->next;
  goto val_begin;

ctn_end:
  cur--;
  *cur++ = (u8)(']' | ((u8)ctn_obj << 5));
  *cur++ = ',';
  if (unlikely((u8 *)ctx >= end)) goto doc_end;
  val = ctn->next;
  yyjson_mut_write_ctx_get(ctx++, &ctn, &ctn_len, &ctn_obj);
  ctn_len--;
  if (likely(ctn_len > 0)) {
    goto val_begin;
  } else {
    goto ctn_end;
  }

doc_end:
  if (newline) {
    incr_len(2);
    *(cur - 1) = '\n';
    cur++;
  }
  *--cur = '\0';
  *dat_len = (usize)(cur - hdr);
  err->code = YYJSON_WRITE_SUCCESS;
  err->msg = NULL;
  return hdr;

fail_alloc:
  return_err(MEMORY_ALLOCATION, MSG_MALLOC);
fail_type:
  return_err(INVALID_VALUE_TYPE, MSG_ERR_TYPE);
fail_num:
  return_err(NAN_OR_INF, MSG_NAN_INF);
fail_str:
  return_err(INVALID_STRING, MSG_ERR_UTF8);

#undef return_err
#undef incr_len
#undef check_str_len
}

static_inline u8 *mut_write_root_pretty(const yyjson_mut_val *root,
                                        usize estimated_val_num,
                                        yyjson_write_flag flg, yyjson_alc alc,
                                        char *buf, usize *dat_len,
                                        yyjson_write_err *err) {
#define return_err(_code, _msg)             \
  do {                                      \
    *dat_len = 0;                           \
    err->code = YYJSON_WRITE_ERROR_##_code; \
    err->msg = _msg;                        \
    if (hdr) alc.free(alc.ctx, hdr);        \
    return NULL;                            \
  } while (false)

#define incr_len(_len)                                                       \
  do {                                                                       \
    ext_len = (usize)(_len);                                                 \
    if (unlikely((u8 *)(cur + ext_len) >= (u8 *)ctx)) {                      \
      usize ctx_pos = (usize)((u8 *)ctx - hdr);                              \
      usize cur_pos = (usize)(cur - hdr);                                    \
      ctx_len = (usize)(end - (u8 *)ctx);                                    \
      alc_inc = yyjson_max(alc_len / 2, ext_len);                            \
      alc_inc = size_align_up(alc_inc, sizeof(yyjson_mut_write_ctx));        \
      if ((sizeof(usize) < 8) && size_add_is_overflow(alc_len, alc_inc))     \
        goto fail_alloc;                                                     \
      alc_len += alc_inc;                                                    \
      tmp = (u8 *)alc.realloc(alc.ctx, hdr, alc_len - alc_inc, alc_len);     \
      if (unlikely(!tmp)) goto fail_alloc;                                   \
      ctx_tmp = (yyjson_mut_write_ctx *)(void *)(tmp + (alc_len - ctx_len)); \
      memmove((void *)ctx_tmp, (void *)(tmp + ctx_pos), ctx_len);            \
      ctx = ctx_tmp;                                                         \
      cur = tmp + cur_pos;                                                   \
      end = tmp + alc_len;                                                   \
      hdr = tmp;                                                             \
    }                                                                        \
  } while (false)

#define check_str_len(_len)                                    \
  do {                                                         \
    if ((sizeof(usize) < 8) && (_len >= (USIZE_MAX - 16) / 6)) \
      goto fail_alloc;                                         \
  } while (false)

  yyjson_mut_val *val, *ctn;
  yyjson_type val_type;
  usize ctn_len, ctn_len_tmp;
  bool ctn_obj, ctn_obj_tmp, is_key, no_indent;
  u8 *hdr, *cur, *end, *tmp;
  yyjson_mut_write_ctx *ctx, *ctx_tmp;
  usize alc_len, alc_inc, ctx_len, ext_len, str_len, level;
  const u8 *str_ptr;
  const char_enc_type *enc_table = get_enc_table_with_flag(flg);
  const u8 *hex_table = get_hex_table_with_flag(flg);
  bool cpy = (enc_table == enc_table_cpy);
  bool esc = has_flg(ESCAPE_UNICODE) != 0;
  bool inv = has_allow(INVALID_UNICODE) != 0;
  usize spaces = has_flg(PRETTY_TWO_SPACES) ? 2 : 4;
  bool newline = has_flg(NEWLINE_AT_END) != 0;

  if (buf) {
    hdr = (u8 *)buf;
    alc_len = *dat_len;
    alc_len = size_align_down(alc_len, sizeof(yyjson_mut_write_ctx));
    if (alc_len <= sizeof(yyjson_mut_write_ctx)) goto fail_alloc;
  } else {
    alc_len = estimated_val_num * YYJSON_WRITER_ESTIMATED_PRETTY_RATIO + 64;
    alc_len = size_align_up(alc_len, sizeof(yyjson_mut_write_ctx));
    hdr = (u8 *)alc.malloc(alc.ctx, alc_len);
    if (!hdr) goto fail_alloc;
  }
  cur = hdr;
  end = hdr + alc_len;
  ctx = (yyjson_mut_write_ctx *)(void *)end;

doc_begin:
  val = constcast(yyjson_mut_val *) root;
  val_type = unsafe_yyjson_get_type(val);
  ctn_obj = (val_type == YYJSON_TYPE_OBJ);
  ctn_len = unsafe_yyjson_get_len(val) << (u8)ctn_obj;
  *cur++ = (u8)('[' | ((u8)ctn_obj << 5));
  *cur++ = '\n';
  ctn = val;
  val = (yyjson_mut_val *)val->uni.ptr;
  val = ctn_obj ? val->next->next : val->next;
  level = 1;

val_begin:
  val_type = unsafe_yyjson_get_type(val);
  if (val_type == YYJSON_TYPE_STR) {
    is_key = (bool)((u8)ctn_obj & (u8)~ctn_len);
    no_indent = (bool)((u8)ctn_obj & (u8)ctn_len);
    str_len = unsafe_yyjson_get_len(val);
    str_ptr = (const u8 *)unsafe_yyjson_get_str(val);
    check_str_len(str_len);
    incr_len(str_len * 6 + 16 + (no_indent ? 0 : level * 4));
    cur = write_indent(cur, no_indent ? 0 : level, spaces);
    if (likely(cpy) && unsafe_yyjson_get_subtype(val)) {
      cur = write_str_noesc(cur, str_ptr, str_len);
    } else {
      cur = write_str(cur, esc, inv, str_ptr, str_len, enc_table, hex_table);
      if (unlikely(!cur)) goto fail_str;
    }
    *cur++ = is_key ? ':' : ',';
    *cur++ = is_key ? ' ' : '\n';
    goto val_end;
  }
  if (val_type == YYJSON_TYPE_NUM) {
    no_indent = (bool)((u8)ctn_obj & (u8)ctn_len);
    incr_len(FP_BUF_LEN + (no_indent ? 0 : level * 4));
    cur = write_indent(cur, no_indent ? 0 : level, spaces);
    cur = write_num(cur, (yyjson_val *)val, flg);
    if (unlikely(!cur)) goto fail_num;
    *cur++ = ',';
    *cur++ = '\n';
    goto val_end;
  }
  if ((val_type & (YYJSON_TYPE_ARR & YYJSON_TYPE_OBJ)) ==
      (YYJSON_TYPE_ARR & YYJSON_TYPE_OBJ)) {
    no_indent = (bool)((u8)ctn_obj & (u8)ctn_len);
    ctn_len_tmp = unsafe_yyjson_get_len(val);
    ctn_obj_tmp = (val_type == YYJSON_TYPE_OBJ);
    incr_len(2 * sizeof(*ctx) + (no_indent ? 0 : level * 4));
    if (unlikely(ctn_len_tmp == 0)) {
      cur = write_indent(cur, no_indent ? 0 : level, spaces);
      *cur++ = (u8)('[' | ((u8)ctn_obj_tmp << 5));
      *cur++ = (u8)(']' | ((u8)ctn_obj_tmp << 5));
      *cur++ = ',';
      *cur++ = '\n';
      goto val_end;
    } else {
      yyjson_mut_write_ctx_set(--ctx, ctn, ctn_len, ctn_obj);
      ctn_len = ctn_len_tmp << (u8)ctn_obj_tmp;
      ctn_obj = ctn_obj_tmp;
      cur = write_indent(cur, no_indent ? 0 : level, spaces);
      level++;
      *cur++ = (u8)('[' | ((u8)ctn_obj << 5));
      *cur++ = '\n';
      ctn = val;
      val = (yyjson_mut_val *)ctn->uni.ptr;
      val = ctn_obj ? val->next->next : val->next;
      goto val_begin;
    }
  }
  if (val_type == YYJSON_TYPE_BOOL) {
    no_indent = (bool)((u8)ctn_obj & (u8)ctn_len);
    incr_len(16 + (no_indent ? 0 : level * 4));
    cur = write_indent(cur, no_indent ? 0 : level, spaces);
    cur = write_bool(cur, unsafe_yyjson_get_bool(val));
    cur += 2;
    goto val_end;
  }
  if (val_type == YYJSON_TYPE_NULL) {
    no_indent = (bool)((u8)ctn_obj & (u8)ctn_len);
    incr_len(16 + (no_indent ? 0 : level * 4));
    cur = write_indent(cur, no_indent ? 0 : level, spaces);
    cur = write_null(cur);
    cur += 2;
    goto val_end;
  }
  if (val_type == YYJSON_TYPE_RAW) {
    no_indent = (bool)((u8)ctn_obj & (u8)ctn_len);
    str_len = unsafe_yyjson_get_len(val);
    str_ptr = (const u8 *)unsafe_yyjson_get_str(val);
    check_str_len(str_len);
    incr_len(str_len + 3 + (no_indent ? 0 : level * 4));
    cur = write_indent(cur, no_indent ? 0 : level, spaces);
    cur = write_raw(cur, str_ptr, str_len);
    *cur++ = ',';
    *cur++ = '\n';
    goto val_end;
  }
  goto fail_type;

val_end:
  ctn_len--;
  if (unlikely(ctn_len == 0)) goto ctn_end;
  val = val->next;
  goto val_begin;

ctn_end:
  cur -= 2;
  *cur++ = '\n';
  incr_len(level * 4);
  cur = write_indent(cur, --level, spaces);
  *cur++ = (u8)(']' | ((u8)ctn_obj << 5));
  if (unlikely((u8 *)ctx >= end)) goto doc_end;
  val = ctn->next;
  yyjson_mut_write_ctx_get(ctx++, &ctn, &ctn_len, &ctn_obj);
  ctn_len--;
  *cur++ = ',';
  *cur++ = '\n';
  if (likely(ctn_len > 0)) {
    goto val_begin;
  } else {
    goto ctn_end;
  }

doc_end:
  if (newline) {
    incr_len(2);
    *cur++ = '\n';
  }
  *cur = '\0';
  *dat_len = (usize)(cur - hdr);
  err->code = YYJSON_WRITE_SUCCESS;
  err->msg = NULL;
  return hdr;

fail_alloc:
  return_err(MEMORY_ALLOCATION, MSG_MALLOC);
fail_type:
  return_err(INVALID_VALUE_TYPE, MSG_ERR_TYPE);
fail_num:
  return_err(NAN_OR_INF, MSG_NAN_INF);
fail_str:
  return_err(INVALID_STRING, MSG_ERR_UTF8);

#undef return_err
#undef incr_len
#undef check_str_len
}

static char *mut_write_root(const yyjson_mut_val *val, usize estimated_val_num,
                            yyjson_write_flag flg, const yyjson_alc *alc_ptr,
                            char *buf, usize *dat_len, yyjson_write_err *err) {
  yyjson_write_err tmp_err;
  usize tmp_dat_len;
  yyjson_alc alc = alc_ptr ? *alc_ptr : YYJSON_DEFAULT_ALC;
  yyjson_mut_val *root = constcast(yyjson_mut_val *) val;

  if (!err) err = &tmp_err;
  if (!dat_len) dat_len = &tmp_dat_len;

  if (unlikely(!root)) {
    *dat_len = 0;
    err->msg = "input JSON is NULL";
    err->code = YYJSON_WRITE_ERROR_INVALID_PARAMETER;
    return NULL;
  }

  if (!unsafe_yyjson_is_ctn(root) || unsafe_yyjson_get_len(root) == 0) {
    return (char *)mut_write_root_single(root, flg, alc, buf, dat_len, err);
  } else if (flg & (YYJSON_WRITE_PRETTY | YYJSON_WRITE_PRETTY_TWO_SPACES)) {
    return (char *)mut_write_root_pretty(root, estimated_val_num, flg, alc, buf,
                                         dat_len, err);
  } else {
    return (char *)mut_write_root_minify(root, estimated_val_num, flg, alc, buf,
                                         dat_len, err);
  }
}

char *yyjson_mut_val_write_opts(const yyjson_mut_val *val,
                                yyjson_write_flag flg,
                                const yyjson_alc *alc_ptr, usize *dat_len,
                                yyjson_write_err *err) {
  return mut_write_root(val, 0, flg, alc_ptr, NULL, dat_len, err);
}

char *yyjson_mut_write_opts(const yyjson_mut_doc *doc, yyjson_write_flag flg,
                            const yyjson_alc *alc_ptr, usize *dat_len,
                            yyjson_write_err *err) {
  yyjson_mut_val *root;
  usize estimated_val_num;
  if (likely(doc)) {
    root = doc->root;
    estimated_val_num = yyjson_mut_doc_estimated_val_num(doc);
  } else {
    root = NULL;
    estimated_val_num = 0;
  }
  return mut_write_root(root, estimated_val_num, flg, alc_ptr, NULL, dat_len,
                        err);
}

bool yyjson_mut_val_write_file(const char *path, const yyjson_mut_val *val,
                               yyjson_write_flag flg, const yyjson_alc *alc_ptr,
                               yyjson_write_err *err) {
  yyjson_write_err tmp_err;
  yyjson_alc alc = alc_ptr ? *alc_ptr : YYJSON_DEFAULT_ALC;
  u8 *dat;
  usize dat_len = 0;
  yyjson_mut_val *root = constcast(yyjson_mut_val *) val;
  bool suc;

  if (!err) err = &tmp_err;
  if (unlikely(!path || !*path)) {
    err->msg = "input path is invalid";
    err->code = YYJSON_WRITE_ERROR_INVALID_PARAMETER;
    return false;
  }

  dat = (u8 *)yyjson_mut_val_write_opts(root, flg, &alc, &dat_len, err);
  if (unlikely(!dat)) return false;
  suc = write_dat_to_file(path, dat, dat_len, err);
  alc.free(alc.ctx, dat);
  return suc;
}

bool yyjson_mut_val_write_fp(FILE *fp, const yyjson_mut_val *val,
                             yyjson_write_flag flg, const yyjson_alc *alc_ptr,
                             yyjson_write_err *err) {
  yyjson_write_err tmp_err;
  yyjson_alc alc = alc_ptr ? *alc_ptr : YYJSON_DEFAULT_ALC;
  u8 *dat;
  usize dat_len = 0;
  yyjson_mut_val *root = constcast(yyjson_mut_val *) val;
  bool suc;

  if (!err) err = &tmp_err;
  if (unlikely(!fp)) {
    err->msg = "input fp is invalid";
    err->code = YYJSON_WRITE_ERROR_INVALID_PARAMETER;
    return false;
  }

  dat = (u8 *)yyjson_mut_val_write_opts(root, flg, &alc, &dat_len, err);
  if (unlikely(!dat)) return false;
  suc = write_dat_to_fp(fp, dat, dat_len, err);
  alc.free(alc.ctx, dat);
  return suc;
}

size_t yyjson_mut_val_write_buf(char *buf, size_t buf_len,
                                const yyjson_mut_val *val,
                                yyjson_write_flag flg, yyjson_write_err *err) {
  if (unlikely(!buf || !buf_len)) {
    if (err) err->code = YYJSON_WRITE_ERROR_INVALID_PARAMETER;
    if (err) err->msg = "input buf or buf_len is invalid";
    return 0;
  } else {
    mut_write_root(val, 0, flg, &YYJSON_NULL_ALC, buf, &buf_len, err);
    return buf_len;
  }
}

bool yyjson_mut_write_file(const char *path, const yyjson_mut_doc *doc,
                           yyjson_write_flag flg, const yyjson_alc *alc_ptr,
                           yyjson_write_err *err) {
  yyjson_mut_val *root = doc ? doc->root : NULL;
  return yyjson_mut_val_write_file(path, root, flg, alc_ptr, err);
}

bool yyjson_mut_write_fp(FILE *fp, const yyjson_mut_doc *doc,
                         yyjson_write_flag flg, const yyjson_alc *alc_ptr,
                         yyjson_write_err *err) {
  yyjson_mut_val *root = doc ? doc->root : NULL;
  return yyjson_mut_val_write_fp(fp, root, flg, alc_ptr, err);
}

size_t yyjson_mut_write_buf(char *buf, size_t buf_len,
                            const yyjson_mut_doc *doc, yyjson_write_flag flg,
                            yyjson_write_err *err) {
  yyjson_mut_val *root = doc ? doc->root : NULL;
  return yyjson_mut_val_write_buf(buf, buf_len, root, flg, err);
}

#undef has_flg
#undef has_allow
#endif

#if !YYJSON_DISABLE_UTILS

static_inline const char *ptr_next_token(const char **ptr, const char *end,
                                         usize *len, usize *esc) {
  const char *hdr = *ptr + 1;
  const char *cur = hdr;

  while (cur < end && *cur != '/' && *cur != '~') cur++;
  if (likely(cur == end || *cur != '~')) {
    *ptr = cur;
    *len = (usize)(cur - hdr);
    *esc = 0;
    return hdr;
  } else {
    usize esc_num = 0;
    while (cur < end && *cur != '/') {
      if (*cur++ == '~') {
        if (cur == end || (*cur != '0' && *cur != '1')) {
          *ptr = cur - 1;
          return NULL;
        }
        esc_num++;
      }
    }
    *ptr = cur;
    *len = (usize)(cur - hdr) - esc_num;
    *esc = esc_num;
    return hdr;
  }
}

static_inline bool ptr_token_to_idx(const char *cur, usize len, usize *idx) {
  const char *end = cur + len;
  usize num = 0, add;
  if (unlikely(len == 0 || len > USIZE_SAFE_DIG)) return false;
  if (*cur == '0') {
    if (unlikely(len > 1)) return false;
    *idx = 0;
    return true;
  }
  if (*cur == '-') {
    if (unlikely(len > 1)) return false;
    *idx = USIZE_MAX;
    return true;
  }
  for (; cur < end && (add = (usize)((u8)*cur - (u8)'0')) <= 9; cur++) {
    num = num * 10 + add;
  }
  if (unlikely(num == 0 || cur < end)) return false;
  *idx = num;
  return true;
}

static_inline bool ptr_token_eq(void *key, const char *token, usize len,
                                usize esc) {
  yyjson_val *val = (yyjson_val *)key;
  if (unsafe_yyjson_get_len(val) != len) return false;
  if (likely(!esc)) {
    return memcmp(val->uni.str, token, len) == 0;
  } else {
    const char *str = val->uni.str;
    for (; len-- > 0; token++, str++) {
      if (*token == '~') {
        if (*str != (*++token == '0' ? '~' : '/')) return false;
      } else {
        if (*str != *token) return false;
      }
    }
    return true;
  }
}

static_inline yyjson_val *ptr_arr_get(const yyjson_val *arr, const char *token,
                                      usize len, usize esc) {
  yyjson_val *val = unsafe_yyjson_get_first(arr);
  usize num = unsafe_yyjson_get_len(arr), idx = 0;
  if (unlikely(num == 0)) return NULL;
  if (unlikely(!ptr_token_to_idx(token, len, &idx))) return NULL;
  if (unlikely(idx >= num)) return NULL;
  if (unsafe_yyjson_arr_is_flat(arr)) {
    return val + idx;
  } else {
    while (idx-- > 0) val = unsafe_yyjson_get_next(val);
    return val;
  }
}

static_inline yyjson_val *ptr_obj_get(const yyjson_val *obj, const char *token,
                                      usize len, usize esc) {
  yyjson_val *key = unsafe_yyjson_get_first(obj);
  usize num = unsafe_yyjson_get_len(obj);
  if (unlikely(num == 0)) return NULL;
  for (; num > 0; num--, key = unsafe_yyjson_get_next(key + 1)) {
    if (ptr_token_eq(key, token, len, esc)) return key + 1;
  }
  return NULL;
}

static_inline yyjson_mut_val *ptr_mut_arr_get(const yyjson_mut_val *arr,
                                              const char *token, usize len,
                                              usize esc, yyjson_mut_val **pre,
                                              bool *last) {
  yyjson_mut_val *val = (yyjson_mut_val *)arr->uni.ptr;
  usize num = unsafe_yyjson_get_len(arr), idx;
  if (last) *last = false;
  if (pre) *pre = NULL;
  if (unlikely(num == 0)) {
    if (last && len == 1 && (*token == '0' || *token == '-')) *last = true;
    return NULL;
  }
  if (unlikely(!ptr_token_to_idx(token, len, &idx))) return NULL;
  if (last) *last = (idx == num || idx == USIZE_MAX);
  if (unlikely(idx >= num)) return NULL;
  while (idx-- > 0) val = val->next;
  if (pre) *pre = val;
  return val->next;
}

static_inline yyjson_mut_val *ptr_mut_obj_get(const yyjson_mut_val *obj,
                                              const char *token, usize len,
                                              usize esc, yyjson_mut_val **pre) {
  yyjson_mut_val *pre_key = (yyjson_mut_val *)obj->uni.ptr, *key;
  usize num = unsafe_yyjson_get_len(obj);
  if (pre) *pre = NULL;
  if (unlikely(num == 0)) return NULL;
  for (; num > 0; num--, pre_key = key) {
    key = pre_key->next->next;
    if (ptr_token_eq(key, token, len, esc)) {
      if (pre) *pre = pre_key;
      return key->next;
    }
  }
  return NULL;
}

static_inline yyjson_mut_val *ptr_new_key(const char *token, usize len,
                                          usize esc, yyjson_mut_doc *doc) {
  const char *src = token;
  if (likely(!esc)) {
    return yyjson_mut_strncpy(doc, src, len);
  } else {
    const char *end = src + len + esc;
    char *dst = unsafe_yyjson_mut_str_alc(doc, len + esc);
    char *str = dst;
    if (unlikely(!dst)) return NULL;
    for (; src < end; src++, dst++) {
      if (*src != '~')
        *dst = *src;
      else
        *dst = (*++src == '0' ? '~' : '/');
    }
    *dst = '\0';
    return yyjson_mut_strn(doc, str, len);
  }
}

#define return_err(_ret, _code, _pos, _msg) \
  do {                                      \
    if (err) {                              \
      err->code = YYJSON_PTR_ERR_##_code;   \
      err->msg = _msg;                      \
      err->pos = (usize)(_pos);             \
    }                                       \
    return _ret;                            \
  } while (false)

#define return_err_resolve(_ret, _pos) \
  return_err(_ret, RESOLVE, _pos, "JSON pointer cannot be resolved")
#define return_err_syntax(_ret, _pos) \
  return_err(_ret, SYNTAX, _pos, "invalid escaped character")
#define return_err_alloc(_ret) \
  return_err(_ret, MEMORY_ALLOCATION, 0, "failed to create value")

yyjson_val *unsafe_yyjson_ptr_getx(const yyjson_val *val, const char *ptr,
                                   size_t ptr_len, yyjson_ptr_err *err) {
  const char *hdr = ptr, *end = ptr + ptr_len, *token;
  usize len, esc;
  yyjson_type type;

  while (true) {
    token = ptr_next_token(&ptr, end, &len, &esc);
    if (unlikely(!token)) return_err_syntax(NULL, ptr - hdr);
    type = unsafe_yyjson_get_type(val);
    if (type == YYJSON_TYPE_OBJ) {
      val = ptr_obj_get(val, token, len, esc);
    } else if (type == YYJSON_TYPE_ARR) {
      val = ptr_arr_get(val, token, len, esc);
    } else {
      val = NULL;
    }
    if (!val) return_err_resolve(NULL, token - hdr);
    if (ptr == end) return constcast(yyjson_val *) val;
  }
}

yyjson_mut_val *unsafe_yyjson_mut_ptr_getx(const yyjson_mut_val *val,
                                           const char *ptr, size_t ptr_len,
                                           yyjson_ptr_ctx *ctx,
                                           yyjson_ptr_err *err) {
  const char *hdr = ptr, *end = ptr + ptr_len, *token;
  usize len, esc;
  yyjson_mut_val *ctn, *pre = NULL;
  yyjson_type type;
  bool idx_is_last = false;

  while (true) {
    token = ptr_next_token(&ptr, end, &len, &esc);
    if (unlikely(!token)) return_err_syntax(NULL, ptr - hdr);
    ctn = constcast(yyjson_mut_val *) val;
    type = unsafe_yyjson_get_type(val);
    if (type == YYJSON_TYPE_OBJ) {
      val = ptr_mut_obj_get(val, token, len, esc, &pre);
    } else if (type == YYJSON_TYPE_ARR) {
      val = ptr_mut_arr_get(val, token, len, esc, &pre, &idx_is_last);
    } else {
      val = NULL;
    }
    if (ctx && (ptr == end)) {
      if (type == YYJSON_TYPE_OBJ ||
          (type == YYJSON_TYPE_ARR && (val || idx_is_last))) {
        ctx->ctn = ctn;
        ctx->pre = pre;
      }
    }
    if (!val) return_err_resolve(NULL, token - hdr);
    if (ptr == end) return constcast(yyjson_mut_val *) val;
  }
}

bool unsafe_yyjson_mut_ptr_putx(yyjson_mut_val *val, const char *ptr,
                                size_t ptr_len, yyjson_mut_val *new_val,
                                yyjson_mut_doc *doc, bool create_parent,
                                bool insert_new, yyjson_ptr_ctx *ctx,
                                yyjson_ptr_err *err) {
  const char *hdr = ptr, *end = ptr + ptr_len, *token;
  usize token_len, esc, ctn_len;
  yyjson_mut_val *ctn, *key, *pre = NULL;
  yyjson_mut_val *sep_ctn = NULL, *sep_key = NULL, *sep_val = NULL;
  yyjson_type ctn_type;
  bool idx_is_last = false;

  while (true) {
    token = ptr_next_token(&ptr, end, &token_len, &esc);
    if (unlikely(!token)) return_err_syntax(false, ptr - hdr);
    ctn = val;
    ctn_type = unsafe_yyjson_get_type(ctn);
    if (ctn_type == YYJSON_TYPE_OBJ) {
      val = ptr_mut_obj_get(ctn, token, token_len, esc, &pre);
    } else if (ctn_type == YYJSON_TYPE_ARR) {
      val = ptr_mut_arr_get(ctn, token, token_len, esc, &pre, &idx_is_last);
    } else
      return_err_resolve(false, token - hdr);
    if (!val) break;
    if (ptr == end) break;
  }

  if (unlikely(ptr != end)) {
    if (!create_parent) return_err_resolve(false, token - hdr);

    if (ctn_type == YYJSON_TYPE_ARR) {
      if (!idx_is_last || !insert_new) {
        return_err_resolve(false, token - hdr);
      }
      val = yyjson_mut_obj(doc);
      if (!val) return_err_alloc(false);

      sep_ctn = ctn;
      sep_key = NULL;
      sep_val = val;

      ctn = val;
      val = NULL;
      ctn_type = YYJSON_TYPE_OBJ;
      token = ptr_next_token(&ptr, end, &token_len, &esc);
      if (unlikely(!token)) return_err_resolve(false, token - hdr);
    }

    while (ptr != end) {
      key = ptr_new_key(token, token_len, esc, doc);
      if (!key) return_err_alloc(false);
      val = yyjson_mut_obj(doc);
      if (!val) return_err_alloc(false);

      if (!sep_ctn) {
        sep_ctn = ctn;
        sep_key = key;
        sep_val = val;
      } else {
        yyjson_mut_obj_add(ctn, key, val);
      }

      ctn = val;
      val = NULL;
      token = ptr_next_token(&ptr, end, &token_len, &esc);
      if (unlikely(!token)) return_err_syntax(false, ptr - hdr);
    }
  }

  ctn_len = unsafe_yyjson_get_len(ctn);
  if (ctn_type == YYJSON_TYPE_OBJ) {
    if (ctx) ctx->ctn = ctn;
    if (!val || insert_new) {
      key = ptr_new_key(token, token_len, esc, doc);
      if (unlikely(!key)) return_err_alloc(false);
      if (ctx) ctx->pre = ctn_len ? (yyjson_mut_val *)ctn->uni.ptr : key;
      unsafe_yyjson_mut_obj_add(ctn, key, new_val, ctn_len);
    } else {
      key = pre->next->next;
      if (ctx) ctx->pre = pre;
      if (ctx) ctx->old = val;
      yyjson_mut_obj_put(ctn, key, new_val);
    }
  } else {
    if (ctx && (val || idx_is_last)) ctx->ctn = ctn;
    if (insert_new) {
      if (val) {
        pre->next = new_val;
        new_val->next = val;
        if (ctx) ctx->pre = pre;
        unsafe_yyjson_set_len(ctn, ctn_len + 1);
      } else if (idx_is_last) {
        if (ctx) ctx->pre = ctn_len ? (yyjson_mut_val *)ctn->uni.ptr : new_val;
        yyjson_mut_arr_append(ctn, new_val);
      } else {
        return_err_resolve(false, token - hdr);
      }
    } else {
      if (!val) return_err_resolve(false, token - hdr);
      if (ctn_len > 1) {
        new_val->next = val->next;
        pre->next = new_val;
        if (ctn->uni.ptr == val) ctn->uni.ptr = new_val;
      } else {
        new_val->next = new_val;
        ctn->uni.ptr = new_val;
        pre = new_val;
      }
      if (ctx) ctx->pre = pre;
      if (ctx) ctx->old = val;
    }
  }

  if (unlikely(sep_ctn)) {
    if (sep_key)
      yyjson_mut_obj_add(sep_ctn, sep_key, sep_val);
    else
      yyjson_mut_arr_append(sep_ctn, sep_val);
  }
  return true;
}

yyjson_mut_val *unsafe_yyjson_mut_ptr_replacex(yyjson_mut_val *val,
                                               const char *ptr, size_t len,
                                               yyjson_mut_val *new_val,
                                               yyjson_ptr_ctx *ctx,
                                               yyjson_ptr_err *err) {
  yyjson_mut_val *cur_val;
  yyjson_ptr_ctx cur_ctx;
  memset(&cur_ctx, 0, sizeof(cur_ctx));
  if (!ctx) ctx = &cur_ctx;
  cur_val = unsafe_yyjson_mut_ptr_getx(val, ptr, len, ctx, err);
  if (!cur_val) return NULL;

  if (yyjson_mut_is_obj(ctx->ctn)) {
    yyjson_mut_val *key = ctx->pre->next->next;
    yyjson_mut_obj_put(ctx->ctn, key, new_val);
  } else {
    yyjson_ptr_ctx_replace(ctx, new_val);
  }
  ctx->old = cur_val;
  return cur_val;
}

yyjson_mut_val *unsafe_yyjson_mut_ptr_removex(yyjson_mut_val *val,
                                              const char *ptr, size_t len,
                                              yyjson_ptr_ctx *ctx,
                                              yyjson_ptr_err *err) {
  yyjson_mut_val *cur_val;
  yyjson_ptr_ctx cur_ctx;
  memset(&cur_ctx, 0, sizeof(cur_ctx));
  if (!ctx) ctx = &cur_ctx;
  cur_val = unsafe_yyjson_mut_ptr_getx(val, ptr, len, ctx, err);
  if (cur_val) {
    if (yyjson_mut_is_obj(ctx->ctn)) {
      yyjson_mut_val *key = ctx->pre->next->next;
      yyjson_mut_obj_put(ctx->ctn, key, NULL);
    } else {
      yyjson_ptr_ctx_remove(ctx);
    }
    ctx->pre = NULL;
    ctx->old = cur_val;
  }
  return cur_val;
}

#undef return_err
#undef return_err_resolve
#undef return_err_syntax
#undef return_err_alloc

typedef enum patch_op {
  PATCH_OP_ADD,
  PATCH_OP_REMOVE,
  PATCH_OP_REPLACE,
  PATCH_OP_MOVE,
  PATCH_OP_COPY,
  PATCH_OP_TEST,
  PATCH_OP_NONE
} patch_op;

static patch_op patch_op_get(yyjson_val *op) {
  const char *str = op->uni.str;
  switch (unsafe_yyjson_get_len(op)) {
    case 3:
      if (!memcmp(str, "add", 3)) return PATCH_OP_ADD;
      return PATCH_OP_NONE;
    case 4:
      if (!memcmp(str, "move", 4)) return PATCH_OP_MOVE;
      if (!memcmp(str, "copy", 4)) return PATCH_OP_COPY;
      if (!memcmp(str, "test", 4)) return PATCH_OP_TEST;
      return PATCH_OP_NONE;
    case 6:
      if (!memcmp(str, "remove", 6)) return PATCH_OP_REMOVE;
      return PATCH_OP_NONE;
    case 7:
      if (!memcmp(str, "replace", 7)) return PATCH_OP_REPLACE;
      return PATCH_OP_NONE;
    default:
      return PATCH_OP_NONE;
  }
}

#define return_err(_code, _msg)                              \
  do {                                                       \
    if (err->ptr.code == YYJSON_PTR_ERR_MEMORY_ALLOCATION) { \
      err->code = YYJSON_PATCH_ERROR_MEMORY_ALLOCATION;      \
      err->msg = _msg;                                       \
      memset(&err->ptr, 0, sizeof(yyjson_ptr_err));          \
    } else {                                                 \
      err->code = YYJSON_PATCH_ERROR_##_code;                \
      err->msg = _msg;                                       \
      err->idx = iter.idx ? iter.idx - 1 : 0;                \
    }                                                        \
    return NULL;                                             \
  } while (false)

#define return_err_copy() return_err(MEMORY_ALLOCATION, "failed to copy value")
#define return_err_key(_key) return_err(MISSING_KEY, "missing key " _key)
#define return_err_val(_key) return_err(INVALID_MEMBER, "invalid member " _key)

#define ptr_get(_ptr) \
  yyjson_mut_ptr_getx(root, _ptr->uni.str, _ptr##_len, NULL, &err->ptr)
#define ptr_add(_ptr, _val)                                                    \
  yyjson_mut_ptr_addx(root, _ptr->uni.str, _ptr##_len, _val, doc, false, NULL, \
                      &err->ptr)
#define ptr_remove(_ptr) \
  yyjson_mut_ptr_removex(root, _ptr->uni.str, _ptr##_len, NULL, &err->ptr)
#define ptr_replace(_ptr, _val)                                        \
  yyjson_mut_ptr_replacex(root, _ptr->uni.str, _ptr##_len, _val, NULL, \
                          &err->ptr)

yyjson_mut_val *yyjson_patch(yyjson_mut_doc *doc, const yyjson_val *orig,
                             const yyjson_val *patch, yyjson_patch_err *err) {
  yyjson_mut_val *root;
  yyjson_val *obj;
  yyjson_arr_iter iter;
  yyjson_patch_err err_tmp;
  if (!err) err = &err_tmp;
  memset(err, 0, sizeof(*err));
  memset(&iter, 0, sizeof(iter));

  if (unlikely(!doc || !orig || !patch)) {
    return_err(INVALID_PARAMETER, "input parameter is NULL");
  }
  if (unlikely(!yyjson_is_arr(patch))) {
    return_err(INVALID_PARAMETER, "input patch is not array");
  }
  root = yyjson_val_mut_copy(doc, orig);
  if (unlikely(!root)) return_err_copy();

  yyjson_arr_iter_init(patch, &iter);
  while ((obj = yyjson_arr_iter_next(&iter))) {
    patch_op op_enum;
    yyjson_val *op, *path, *from = NULL, *value;
    yyjson_mut_val *val = NULL, *test;
    usize path_len, from_len = 0;
    if (unlikely(!unsafe_yyjson_is_obj(obj))) {
      return_err(INVALID_OPERATION, "JSON patch operation is not object");
    }

    op = yyjson_obj_get(obj, "op");
    if (unlikely(!op)) return_err_key("`op`");
    if (unlikely(!yyjson_is_str(op))) return_err_val("`op`");
    op_enum = patch_op_get(op);

    path = yyjson_obj_get(obj, "path");
    if (unlikely(!path)) return_err_key("`path`");
    if (unlikely(!yyjson_is_str(path))) return_err_val("`path`");
    path_len = unsafe_yyjson_get_len(path);

    switch ((int)op_enum) {
      case PATCH_OP_ADD:
      case PATCH_OP_REPLACE:
      case PATCH_OP_TEST:
        value = yyjson_obj_get(obj, "value");
        if (unlikely(!value)) return_err_key("`value`");
        val = yyjson_val_mut_copy(doc, value);
        if (unlikely(!val)) return_err_copy();
        break;
      case PATCH_OP_MOVE:
      case PATCH_OP_COPY:
        from = yyjson_obj_get(obj, "from");
        if (unlikely(!from)) return_err_key("`from`");
        if (unlikely(!yyjson_is_str(from))) return_err_val("`from`");
        from_len = unsafe_yyjson_get_len(from);
        break;
      default:
        break;
    }

    switch ((int)op_enum) {
      case PATCH_OP_ADD:
        if (unlikely(path_len == 0)) {
          root = val;
          break;
        }
        if (unlikely(!ptr_add(path, val))) {
          return_err(POINTER, "failed to add `path`");
        }
        break;
      case PATCH_OP_REMOVE:
        if (unlikely(!ptr_remove(path))) {
          return_err(POINTER, "failed to remove `path`");
        }
        break;
      case PATCH_OP_REPLACE:
        if (unlikely(path_len == 0)) {
          root = val;
          break;
        }
        if (unlikely(!ptr_replace(path, val))) {
          return_err(POINTER, "failed to replace `path`");
        }
        break;
      case PATCH_OP_MOVE:
        if (unlikely(from_len == 0 && path_len == 0)) break;
        val = ptr_remove(from);
        if (unlikely(!val)) {
          return_err(POINTER, "failed to remove `from`");
        }
        if (unlikely(path_len == 0)) {
          root = val;
          break;
        }
        if (unlikely(!ptr_add(path, val))) {
          return_err(POINTER, "failed to add `path`");
        }
        break;
      case PATCH_OP_COPY:
        val = ptr_get(from);
        if (unlikely(!val)) {
          return_err(POINTER, "failed to get `from`");
        }
        if (unlikely(path_len == 0)) {
          root = val;
          break;
        }
        val = yyjson_mut_val_mut_copy(doc, val);
        if (unlikely(!val)) return_err_copy();
        if (unlikely(!ptr_add(path, val))) {
          return_err(POINTER, "failed to add `path`");
        }
        break;
      case PATCH_OP_TEST:
        test = ptr_get(path);
        if (unlikely(!test)) {
          return_err(POINTER, "failed to get `path`");
        }
        if (unlikely(!yyjson_mut_equals(val, test))) {
          return_err(EQUAL, "failed to test equal");
        }
        break;
      default:
        return_err(INVALID_MEMBER, "unsupported `op`");
    }
  }
  return root;
}

yyjson_mut_val *yyjson_mut_patch(yyjson_mut_doc *doc,
                                 const yyjson_mut_val *orig,
                                 const yyjson_mut_val *patch,
                                 yyjson_patch_err *err) {
  yyjson_mut_val *root, *obj;
  yyjson_mut_arr_iter iter;
  yyjson_patch_err err_tmp;
  if (!err) err = &err_tmp;
  memset(err, 0, sizeof(*err));
  memset(&iter, 0, sizeof(iter));

  if (unlikely(!doc || !orig || !patch)) {
    return_err(INVALID_PARAMETER, "input parameter is NULL");
  }
  if (unlikely(!yyjson_mut_is_arr(patch))) {
    return_err(INVALID_PARAMETER, "input patch is not array");
  }
  root = yyjson_mut_val_mut_copy(doc, orig);
  if (unlikely(!root)) return_err_copy();

  yyjson_mut_arr_iter_init(constcast(yyjson_mut_val *) patch, &iter);
  while ((obj = yyjson_mut_arr_iter_next(&iter))) {
    patch_op op_enum;
    yyjson_mut_val *op, *path, *from = NULL, *value;
    yyjson_mut_val *val = NULL, *test;
    usize path_len, from_len = 0;
    if (!unsafe_yyjson_is_obj(obj)) {
      return_err(INVALID_OPERATION, "JSON patch operation is not object");
    }

    op = yyjson_mut_obj_get(obj, "op");
    if (unlikely(!op)) return_err_key("`op`");
    if (unlikely(!yyjson_mut_is_str(op))) return_err_val("`op`");
    op_enum = patch_op_get((yyjson_val *)(void *)op);

    path = yyjson_mut_obj_get(obj, "path");
    if (unlikely(!path)) return_err_key("`path`");
    if (unlikely(!yyjson_mut_is_str(path))) return_err_val("`path`");
    path_len = unsafe_yyjson_get_len(path);

    switch ((int)op_enum) {
      case PATCH_OP_ADD:
      case PATCH_OP_REPLACE:
      case PATCH_OP_TEST:
        value = yyjson_mut_obj_get(obj, "value");
        if (unlikely(!value)) return_err_key("`value`");
        val = yyjson_mut_val_mut_copy(doc, value);
        if (unlikely(!val)) return_err_copy();
        break;
      case PATCH_OP_MOVE:
      case PATCH_OP_COPY:
        from = yyjson_mut_obj_get(obj, "from");
        if (unlikely(!from)) return_err_key("`from`");
        if (unlikely(!yyjson_mut_is_str(from))) {
          return_err_val("`from`");
        }
        from_len = unsafe_yyjson_get_len(from);
        break;
      default:
        break;
    }

    switch ((int)op_enum) {
      case PATCH_OP_ADD:
        if (unlikely(path_len == 0)) {
          root = val;
          break;
        }
        if (unlikely(!ptr_add(path, val))) {
          return_err(POINTER, "failed to add `path`");
        }
        break;
      case PATCH_OP_REMOVE:
        if (unlikely(!ptr_remove(path))) {
          return_err(POINTER, "failed to remove `path`");
        }
        break;
      case PATCH_OP_REPLACE:
        if (unlikely(path_len == 0)) {
          root = val;
          break;
        }
        if (unlikely(!ptr_replace(path, val))) {
          return_err(POINTER, "failed to replace `path`");
        }
        break;
      case PATCH_OP_MOVE:
        if (unlikely(from_len == 0 && path_len == 0)) break;
        val = ptr_remove(from);
        if (unlikely(!val)) {
          return_err(POINTER, "failed to remove `from`");
        }
        if (unlikely(path_len == 0)) {
          root = val;
          break;
        }
        if (unlikely(!ptr_add(path, val))) {
          return_err(POINTER, "failed to add `path`");
        }
        break;
      case PATCH_OP_COPY:
        val = ptr_get(from);
        if (unlikely(!val)) {
          return_err(POINTER, "failed to get `from`");
        }
        if (unlikely(path_len == 0)) {
          root = val;
          break;
        }
        val = yyjson_mut_val_mut_copy(doc, val);
        if (unlikely(!val)) return_err_copy();
        if (unlikely(!ptr_add(path, val))) {
          return_err(POINTER, "failed to add `path`");
        }
        break;
      case PATCH_OP_TEST:
        test = ptr_get(path);
        if (unlikely(!test)) {
          return_err(POINTER, "failed to get `path`");
        }
        if (unlikely(!yyjson_mut_equals(val, test))) {
          return_err(EQUAL, "failed to test equal");
        }
        break;
      default:
        return_err(INVALID_MEMBER, "unsupported `op`");
    }
  }
  return root;
}

#undef return_err
#undef return_err_copy
#undef return_err_key
#undef return_err_val
#undef ptr_get
#undef ptr_add
#undef ptr_remove
#undef ptr_replace

yyjson_mut_val *yyjson_merge_patch(yyjson_mut_doc *doc, const yyjson_val *orig,
                                   const yyjson_val *patch) {
  usize idx, max;
  yyjson_val *key, *orig_val, *patch_val, local_orig;
  yyjson_mut_val *builder, *mut_key, *mut_val, *merged_val;

  if (unlikely(!yyjson_is_obj(patch))) {
    return yyjson_val_mut_copy(doc, patch);
  }

  builder = yyjson_mut_obj(doc);
  if (unlikely(!builder)) return NULL;

  memset(&local_orig, 0, sizeof(local_orig));
  if (!yyjson_is_obj(orig)) {
    local_orig.tag = builder->tag;
    local_orig.uni = builder->uni;
    orig = &local_orig;
  }

  if (orig != &local_orig) {
    yyjson_obj_foreach(orig, idx, max, key, orig_val) {
      patch_val = yyjson_obj_getn(patch, unsafe_yyjson_get_str(key),
                                  unsafe_yyjson_get_len(key));
      if (!patch_val) {
        mut_key = yyjson_val_mut_copy(doc, key);
        mut_val = yyjson_val_mut_copy(doc, orig_val);
        if (!yyjson_mut_obj_add(builder, mut_key, mut_val)) return NULL;
      }
    }
  }

  yyjson_obj_foreach(patch, idx, max, key, patch_val) {
    if (unsafe_yyjson_is_null(patch_val)) {
      continue;
    }
    mut_key = yyjson_val_mut_copy(doc, key);
    orig_val = yyjson_obj_getn(orig, unsafe_yyjson_get_str(key),
                               unsafe_yyjson_get_len(key));
    merged_val = yyjson_merge_patch(doc, orig_val, patch_val);
    if (!yyjson_mut_obj_add(builder, mut_key, merged_val)) return NULL;
  }

  return builder;
}

yyjson_mut_val *yyjson_mut_merge_patch(yyjson_mut_doc *doc,
                                       const yyjson_mut_val *orig,
                                       const yyjson_mut_val *patch) {
  usize idx, max;
  yyjson_mut_val *key, *orig_val, *patch_val, local_orig;
  yyjson_mut_val *builder, *mut_key, *mut_val, *merged_val;

  if (unlikely(!yyjson_mut_is_obj(patch))) {
    return yyjson_mut_val_mut_copy(doc, patch);
  }

  builder = yyjson_mut_obj(doc);
  if (unlikely(!builder)) return NULL;

  memset(&local_orig, 0, sizeof(local_orig));
  if (!yyjson_mut_is_obj(orig)) {
    local_orig.tag = builder->tag;
    local_orig.uni = builder->uni;
    orig = &local_orig;
  }

  if (orig != &local_orig) {
    yyjson_mut_obj_foreach(orig, idx, max, key, orig_val) {
      patch_val = yyjson_mut_obj_getn(patch, unsafe_yyjson_get_str(key),
                                      unsafe_yyjson_get_len(key));
      if (!patch_val) {
        mut_key = yyjson_mut_val_mut_copy(doc, key);
        mut_val = yyjson_mut_val_mut_copy(doc, orig_val);
        if (!yyjson_mut_obj_add(builder, mut_key, mut_val)) return NULL;
      }
    }
  }

  yyjson_mut_obj_foreach(patch, idx, max, key, patch_val) {
    if (unsafe_yyjson_is_null(patch_val)) {
      continue;
    }
    mut_key = yyjson_mut_val_mut_copy(doc, key);
    orig_val = yyjson_mut_obj_getn(orig, unsafe_yyjson_get_str(key),
                                   unsafe_yyjson_get_len(key));
    merged_val = yyjson_mut_merge_patch(doc, orig_val, patch_val);
    if (!yyjson_mut_obj_add(builder, mut_key, merged_val)) return NULL;
  }

  return builder;
}

#endif
