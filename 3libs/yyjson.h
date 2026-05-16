#ifndef YYJSON_H
#define YYJSON_H

#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef YYJSON_DISABLE_READER
#endif

#ifndef YYJSON_DISABLE_WRITER
#endif

#ifndef YYJSON_DISABLE_INCR_READER
#endif

#ifndef YYJSON_DISABLE_UTILS
#endif

#ifndef YYJSON_DISABLE_FAST_FP_CONV
#endif

#ifndef YYJSON_DISABLE_NON_STANDARD
#endif

#ifndef YYJSON_DISABLE_UTF8_VALIDATION
#endif

#ifndef YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
#endif

#ifndef YYJSON_EXPORTS
#endif

#ifndef YYJSON_IMPORTS
#endif

#ifndef YYJSON_HAS_STDINT_H
#endif

#ifndef YYJSON_HAS_STDBOOL_H
#endif

#ifndef YYJSON_READER_DEPTH_LIMIT
#endif

#ifdef _MSC_VER
#define YYJSON_MSC_VER _MSC_VER
#else
#define YYJSON_MSC_VER 0
#endif

#ifdef __GNUC__
#define YYJSON_GCC_VER __GNUC__
#if defined(__GNUC_PATCHLEVEL__)
#define yyjson_gcc_available(major, minor, patch)                     \
  ((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= \
   (major * 10000 + minor * 100 + patch))
#else
#define yyjson_gcc_available(major, minor, patch) \
  ((__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >=   \
   (major * 10000 + minor * 100 + patch))
#endif
#else
#define YYJSON_GCC_VER 0
#define yyjson_gcc_available(major, minor, patch) 0
#endif

#if defined(__GNUC__) && defined(__GNUC_MINOR__) && !defined(__clang__) && \
    !defined(__llvm__) && !defined(__INTEL_COMPILER) && !defined(__ICC) && \
    !defined(__NVCC__) && !defined(__PGI) && !defined(__TINYC__)
#define YYJSON_IS_REAL_GCC 1
#else
#define YYJSON_IS_REAL_GCC 0
#endif

#if defined(__STDC__) && (__STDC__ >= 1) && defined(__STDC_VERSION__)
#define YYJSON_STDC_VER __STDC_VERSION__
#else
#define YYJSON_STDC_VER 0
#endif

#if defined(__cplusplus)
#define YYJSON_CPP_VER __cplusplus
#else
#define YYJSON_CPP_VER 0
#endif

#ifndef yyjson_has_builtin
#ifdef __has_builtin
#define yyjson_has_builtin(x) __has_builtin(x)
#else
#define yyjson_has_builtin(x) 0
#endif
#endif

#ifndef yyjson_has_attribute
#ifdef __has_attribute
#define yyjson_has_attribute(x) __has_attribute(x)
#else
#define yyjson_has_attribute(x) 0
#endif
#endif

#ifndef yyjson_has_feature
#ifdef __has_feature
#define yyjson_has_feature(x) __has_feature(x)
#else
#define yyjson_has_feature(x) 0
#endif
#endif

#ifndef yyjson_has_include
#ifdef __has_include
#define yyjson_has_include(x) __has_include(x)
#else
#define yyjson_has_include(x) 0
#endif
#endif

#ifndef yyjson_inline
#if YYJSON_MSC_VER >= 1200
#define yyjson_inline __forceinline
#elif defined(_MSC_VER)
#define yyjson_inline __inline
#elif yyjson_has_attribute(always_inline) || YYJSON_GCC_VER >= 4
#define yyjson_inline __inline__ __attribute__((always_inline))
#elif defined(__clang__) || defined(__GNUC__)
#define yyjson_inline __inline__
#elif defined(__cplusplus) || YYJSON_STDC_VER >= 199901L
#define yyjson_inline inline
#else
#define yyjson_inline
#endif
#endif

#ifndef yyjson_noinline
#if YYJSON_MSC_VER >= 1400
#define yyjson_noinline __declspec(noinline)
#elif yyjson_has_attribute(noinline) || YYJSON_GCC_VER >= 4
#define yyjson_noinline __attribute__((noinline))
#else
#define yyjson_noinline
#endif
#endif

#ifndef yyjson_align
#if YYJSON_MSC_VER >= 1300
#define yyjson_align(x) __declspec(align(x))
#elif yyjson_has_attribute(aligned) || defined(__GNUC__)
#define yyjson_align(x) __attribute__((aligned(x)))
#elif YYJSON_CPP_VER >= 201103L
#define yyjson_align(x) alignas(x)
#else
#define yyjson_align(x)
#endif
#endif

#ifndef yyjson_likely
#if yyjson_has_builtin(__builtin_expect) || \
    (YYJSON_GCC_VER >= 4 && YYJSON_GCC_VER != 5)
#define yyjson_likely(expr) __builtin_expect(!!(expr), 1)
#else
#define yyjson_likely(expr) (expr)
#endif
#endif

#ifndef yyjson_unlikely
#if yyjson_has_builtin(__builtin_expect) || \
    (YYJSON_GCC_VER >= 4 && YYJSON_GCC_VER != 5)
#define yyjson_unlikely(expr) __builtin_expect(!!(expr), 0)
#else
#define yyjson_unlikely(expr) (expr)
#endif
#endif

#ifndef yyjson_constant_p
#if yyjson_has_builtin(__builtin_constant_p) || (YYJSON_GCC_VER >= 3)
#define YYJSON_HAS_CONSTANT_P 1
#define yyjson_constant_p(value) __builtin_constant_p(value)
#else
#define YYJSON_HAS_CONSTANT_P 0
#define yyjson_constant_p(value) 0
#endif
#endif

#ifndef yyjson_deprecated
#if YYJSON_MSC_VER >= 1400
#define yyjson_deprecated(msg) __declspec(deprecated(msg))
#elif yyjson_has_feature(attribute_deprecated_with_message) || \
    (YYJSON_GCC_VER > 4 || (YYJSON_GCC_VER == 4 && __GNUC_MINOR__ >= 5))
#define yyjson_deprecated(msg) __attribute__((deprecated(msg)))
#elif YYJSON_GCC_VER >= 3
#define yyjson_deprecated(msg) __attribute__((deprecated))
#else
#define yyjson_deprecated(msg)
#endif
#endif

#ifndef yyjson_api
#if defined(_WIN32)
#if defined(YYJSON_EXPORTS) && YYJSON_EXPORTS
#define yyjson_api __declspec(dllexport)
#elif defined(YYJSON_IMPORTS) && YYJSON_IMPORTS
#define yyjson_api __declspec(dllimport)
#else
#define yyjson_api
#endif
#elif yyjson_has_attribute(visibility) || YYJSON_GCC_VER >= 4
#define yyjson_api __attribute__((visibility("default")))
#else
#define yyjson_api
#endif
#endif

#ifndef yyjson_api_inline
#define yyjson_api_inline static yyjson_inline
#endif

#ifndef yyjson_constcast
#define yyjson_constcast(type) (type)(void *)(size_t)(const void *)
#endif

#if (defined(YYJSON_HAS_STDINT_H) && YYJSON_HAS_STDINT_H) ||                  \
    YYJSON_MSC_VER >= 1600 || YYJSON_STDC_VER >= 199901L ||                   \
    defined(_STDINT_H) || defined(_STDINT_H_) || defined(__CLANG_STDINT_H) || \
    defined(_STDINT_H_INCLUDED) || yyjson_has_include(<stdint.h>)
#include <stdint.h>
#elif defined(_MSC_VER)
#if _MSC_VER < 1300
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif
#else
#if UCHAR_MAX == 0xFFU
typedef signed char int8_t;
typedef unsigned char uint8_t;
#else
#error cannot find 8-bit integer type
#endif
#if USHRT_MAX == 0xFFFFU
typedef unsigned short uint16_t;
typedef signed short int16_t;
#elif UINT_MAX == 0xFFFFU
typedef unsigned int uint16_t;
typedef signed int int16_t;
#else
#error cannot find 16-bit integer type
#endif
#if UINT_MAX == 0xFFFFFFFFUL
typedef unsigned int uint32_t;
typedef signed int int32_t;
#elif ULONG_MAX == 0xFFFFFFFFUL
typedef unsigned long uint32_t;
typedef signed long int32_t;
#elif USHRT_MAX == 0xFFFFFFFFUL
typedef unsigned short uint32_t;
typedef signed short int32_t;
#else
#error cannot find 32-bit integer type
#endif
#if defined(__INT64_TYPE__) && defined(__UINT64_TYPE__)
typedef __INT64_TYPE__ int64_t;
typedef __UINT64_TYPE__ uint64_t;
#elif defined(__GNUC__) || defined(__clang__)
#if !defined(_SYS_TYPES_H) && !defined(__int8_t_defined)
__extension__ typedef long long int64_t;
#endif
__extension__ typedef unsigned long long uint64_t;
#elif defined(_LONG_LONG) || defined(__MWERKS__) || defined(_CRAYC) || \
    defined(__SUNPRO_C) || defined(__SUNPRO_CC)
typedef long long int64_t;
typedef unsigned long long uint64_t;
#elif (defined(__BORLANDC__) && __BORLANDC__ > 0x460) || \
    defined(__WATCOM_INT64__) || defined(__alpha) || defined(__DECC)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#error cannot find 64-bit integer type
#endif
#endif

#if (defined(YYJSON_HAS_STDBOOL_H) && YYJSON_HAS_STDBOOL_H) || \
    YYJSON_MSC_VER >= 1800 || YYJSON_STDC_VER >= 199901L ||    \
    (yyjson_has_include(<stdbool.h>) && !defined(__STRICT_ANSI__))
#include <stdbool.h>
#elif !defined(__bool_true_false_are_defined)
#define __bool_true_false_are_defined 1
#if defined(__cplusplus)
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#define _Bool bool
#if __cplusplus < 201103L
#define bool bool
#define false false
#define true true
#endif
#endif
#else
#define bool unsigned char
#define true 1
#define false 0
#endif
#endif

#if defined(CHAR_BIT)
#if CHAR_BIT != 8
#error non 8-bit char is not supported
#endif
#endif

#ifndef YYJSON_U64_TO_F64_NO_IMPL
#if (0 < YYJSON_MSC_VER) && (YYJSON_MSC_VER <= 1200)
#define YYJSON_U64_TO_F64_NO_IMPL 1
#else
#define YYJSON_U64_TO_F64_NO_IMPL 0
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-parameter"
#elif YYJSON_IS_REAL_GCC
#if yyjson_gcc_available(4, 6, 0)
#pragma GCC diagnostic push
#endif
#if yyjson_gcc_available(4, 2, 0)
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4800)
#endif

#define YYJSON_VERSION_MAJOR 0

#define YYJSON_VERSION_MINOR 12

#define YYJSON_VERSION_PATCH 0

#define YYJSON_VERSION_HEX 0x000C00

#define YYJSON_VERSION_STRING "0.12.0"

yyjson_api uint32_t yyjson_version(void);

typedef uint8_t yyjson_type;

#define YYJSON_TYPE_NONE ((uint8_t)0)

#define YYJSON_TYPE_RAW ((uint8_t)1)

#define YYJSON_TYPE_NULL ((uint8_t)2)

#define YYJSON_TYPE_BOOL ((uint8_t)3)

#define YYJSON_TYPE_NUM ((uint8_t)4)

#define YYJSON_TYPE_STR ((uint8_t)5)

#define YYJSON_TYPE_ARR ((uint8_t)6)

#define YYJSON_TYPE_OBJ ((uint8_t)7)

typedef uint8_t yyjson_subtype;

#define YYJSON_SUBTYPE_NONE ((uint8_t)(0 << 3))

#define YYJSON_SUBTYPE_FALSE ((uint8_t)(0 << 3))

#define YYJSON_SUBTYPE_TRUE ((uint8_t)(1 << 3))

#define YYJSON_SUBTYPE_UINT ((uint8_t)(0 << 3))

#define YYJSON_SUBTYPE_SINT ((uint8_t)(1 << 3))

#define YYJSON_SUBTYPE_REAL ((uint8_t)(2 << 3))

#define YYJSON_SUBTYPE_NOESC ((uint8_t)(1 << 3))

#define YYJSON_TYPE_MASK ((uint8_t)0x07)

#define YYJSON_TYPE_BIT ((uint8_t)3)

#define YYJSON_SUBTYPE_MASK ((uint8_t)0x18)

#define YYJSON_SUBTYPE_BIT ((uint8_t)2)

#define YYJSON_RESERVED_MASK ((uint8_t)0xE0)

#define YYJSON_RESERVED_BIT ((uint8_t)3)

#define YYJSON_TAG_MASK ((uint8_t)0xFF)

#define YYJSON_TAG_BIT ((uint8_t)8)

#define YYJSON_PADDING_SIZE 4

typedef struct yyjson_alc {
  void *(*malloc)(void *ctx, size_t size);

  void *(*realloc)(void *ctx, void *ptr, size_t old_size, size_t size);

  void (*free)(void *ctx, void *ptr);

  void *ctx;
} yyjson_alc;

yyjson_api bool yyjson_alc_pool_init(yyjson_alc *alc, void *buf, size_t size);

yyjson_api yyjson_alc *yyjson_alc_dyn_new(void);

yyjson_api void yyjson_alc_dyn_free(yyjson_alc *alc);

yyjson_api bool yyjson_locate_pos(const char *str, size_t len, size_t pos,
                                  size_t *line, size_t *col, size_t *chr);

typedef struct yyjson_doc yyjson_doc;

typedef struct yyjson_val yyjson_val;

typedef struct yyjson_mut_doc yyjson_mut_doc;

typedef struct yyjson_mut_val yyjson_mut_val;

typedef uint32_t yyjson_read_flag;

static const yyjson_read_flag YYJSON_READ_NOFLAG = 0;

static const yyjson_read_flag YYJSON_READ_INSITU = 1 << 0;

static const yyjson_read_flag YYJSON_READ_STOP_WHEN_DONE = 1 << 1;

static const yyjson_read_flag YYJSON_READ_ALLOW_TRAILING_COMMAS = 1 << 2;

static const yyjson_read_flag YYJSON_READ_ALLOW_COMMENTS = 1 << 3;

static const yyjson_read_flag YYJSON_READ_ALLOW_INF_AND_NAN = 1 << 4;

static const yyjson_read_flag YYJSON_READ_NUMBER_AS_RAW = 1 << 5;

static const yyjson_read_flag YYJSON_READ_ALLOW_INVALID_UNICODE = 1 << 6;

static const yyjson_read_flag YYJSON_READ_BIGNUM_AS_RAW = 1 << 7;

static const yyjson_read_flag YYJSON_READ_ALLOW_BOM = 1 << 8;

static const yyjson_read_flag YYJSON_READ_ALLOW_EXT_NUMBER = 1 << 9;

static const yyjson_read_flag YYJSON_READ_ALLOW_EXT_ESCAPE = 1 << 10;

static const yyjson_read_flag YYJSON_READ_ALLOW_EXT_WHITESPACE = 1 << 11;

static const yyjson_read_flag YYJSON_READ_ALLOW_SINGLE_QUOTED_STR = 1 << 12;

static const yyjson_read_flag YYJSON_READ_ALLOW_UNQUOTED_KEY = 1 << 13;

static const yyjson_read_flag YYJSON_READ_JSON5 =
    (1 << 2) | (1 << 3) | (1 << 4) | (1 << 9) | (1 << 10) | (1 << 11) |
    (1 << 12) | (1 << 13);

typedef uint32_t yyjson_read_code;

static const yyjson_read_code YYJSON_READ_SUCCESS = 0;

static const yyjson_read_code YYJSON_READ_ERROR_INVALID_PARAMETER = 1;

static const yyjson_read_code YYJSON_READ_ERROR_MEMORY_ALLOCATION = 2;

static const yyjson_read_code YYJSON_READ_ERROR_EMPTY_CONTENT = 3;

static const yyjson_read_code YYJSON_READ_ERROR_UNEXPECTED_CONTENT = 4;

static const yyjson_read_code YYJSON_READ_ERROR_UNEXPECTED_END = 5;

static const yyjson_read_code YYJSON_READ_ERROR_UNEXPECTED_CHARACTER = 6;

static const yyjson_read_code YYJSON_READ_ERROR_JSON_STRUCTURE = 7;

static const yyjson_read_code YYJSON_READ_ERROR_INVALID_COMMENT = 8;

static const yyjson_read_code YYJSON_READ_ERROR_INVALID_NUMBER = 9;

static const yyjson_read_code YYJSON_READ_ERROR_INVALID_STRING = 10;

static const yyjson_read_code YYJSON_READ_ERROR_LITERAL = 11;

static const yyjson_read_code YYJSON_READ_ERROR_FILE_OPEN = 12;

static const yyjson_read_code YYJSON_READ_ERROR_FILE_READ = 13;

static const yyjson_read_code YYJSON_READ_ERROR_MORE = 14;

static const yyjson_read_code YYJSON_READ_ERROR_DEPTH = 15;

typedef struct yyjson_read_err {
  yyjson_read_code code;

  const char *msg;

  size_t pos;
} yyjson_read_err;

#if !defined(YYJSON_DISABLE_READER) || !YYJSON_DISABLE_READER

yyjson_api yyjson_doc *yyjson_read_opts(char *dat, size_t len,
                                        yyjson_read_flag flg,
                                        const yyjson_alc *alc,
                                        yyjson_read_err *err);

yyjson_api yyjson_doc *yyjson_read_file(const char *path, yyjson_read_flag flg,
                                        const yyjson_alc *alc,
                                        yyjson_read_err *err);

yyjson_api yyjson_doc *yyjson_read_fp(FILE *fp, yyjson_read_flag flg,
                                      const yyjson_alc *alc,
                                      yyjson_read_err *err);

yyjson_api_inline yyjson_doc *yyjson_read(const char *dat, size_t len,
                                          yyjson_read_flag flg) {
  flg &= ~YYJSON_READ_INSITU;
  return yyjson_read_opts((char *)(void *)(size_t)(const void *)dat, len, flg,
                          NULL, NULL);
}

#if !defined(YYJSON_DISABLE_INCR_READER) || !YYJSON_DISABLE_INCR_READER

typedef struct yyjson_incr_state yyjson_incr_state;

yyjson_api yyjson_incr_state *yyjson_incr_new(char *buf, size_t buf_len,
                                              yyjson_read_flag flg,
                                              const yyjson_alc *alc);

yyjson_api yyjson_doc *yyjson_incr_read(yyjson_incr_state *state, size_t len,
                                        yyjson_read_err *err);

yyjson_api void yyjson_incr_free(yyjson_incr_state *state);

#endif

yyjson_api_inline size_t yyjson_read_max_memory_usage(size_t len,
                                                      yyjson_read_flag flg) {
  size_t mul = (size_t)12 + !(flg & YYJSON_READ_INSITU);
  size_t pad = 256;
  size_t max = (size_t)(~(size_t)0);
  if (flg & YYJSON_READ_STOP_WHEN_DONE) len = len < 256 ? 256 : len;
  if (len >= (max - pad - mul) / mul) return 0;
  return len * mul + pad;
}

yyjson_api const char *yyjson_read_number(const char *dat, yyjson_val *val,
                                          yyjson_read_flag flg,
                                          const yyjson_alc *alc,
                                          yyjson_read_err *err);

yyjson_api_inline const char *yyjson_mut_read_number(const char *dat,
                                                     yyjson_mut_val *val,
                                                     yyjson_read_flag flg,
                                                     const yyjson_alc *alc,
                                                     yyjson_read_err *err) {
  return yyjson_read_number(dat, (yyjson_val *)val, flg, alc, err);
}

#endif

typedef uint32_t yyjson_write_flag;

static const yyjson_write_flag YYJSON_WRITE_NOFLAG = 0;

static const yyjson_write_flag YYJSON_WRITE_PRETTY = 1 << 0;

static const yyjson_write_flag YYJSON_WRITE_ESCAPE_UNICODE = 1 << 1;

static const yyjson_write_flag YYJSON_WRITE_ESCAPE_SLASHES = 1 << 2;

static const yyjson_write_flag YYJSON_WRITE_ALLOW_INF_AND_NAN = 1 << 3;

static const yyjson_write_flag YYJSON_WRITE_INF_AND_NAN_AS_NULL = 1 << 4;

static const yyjson_write_flag YYJSON_WRITE_ALLOW_INVALID_UNICODE = 1 << 5;

static const yyjson_write_flag YYJSON_WRITE_PRETTY_TWO_SPACES = 1 << 6;

static const yyjson_write_flag YYJSON_WRITE_NEWLINE_AT_END = 1 << 7;

static const yyjson_write_flag YYJSON_WRITE_LOWERCASE_HEX = 1 << 8;

#define YYJSON_WRITE_FP_FLAG_BITS 8

#define YYJSON_WRITE_FP_PREC_BITS 4

#define YYJSON_WRITE_FP_TO_FIXED(prec) \
  ((yyjson_write_flag)((uint32_t)((uint32_t)(prec)) << (32 - 4)))

#define YYJSON_WRITE_FP_TO_FLOAT ((yyjson_write_flag)(1 << (32 - 5)))

typedef uint32_t yyjson_write_code;

static const yyjson_write_code YYJSON_WRITE_SUCCESS = 0;

static const yyjson_write_code YYJSON_WRITE_ERROR_INVALID_PARAMETER = 1;

static const yyjson_write_code YYJSON_WRITE_ERROR_MEMORY_ALLOCATION = 2;

static const yyjson_write_code YYJSON_WRITE_ERROR_INVALID_VALUE_TYPE = 3;

static const yyjson_write_code YYJSON_WRITE_ERROR_NAN_OR_INF = 4;

static const yyjson_write_code YYJSON_WRITE_ERROR_FILE_OPEN = 5;

static const yyjson_write_code YYJSON_WRITE_ERROR_FILE_WRITE = 6;

static const yyjson_write_code YYJSON_WRITE_ERROR_INVALID_STRING = 7;

typedef struct yyjson_write_err {
  yyjson_write_code code;

  const char *msg;
} yyjson_write_err;

#if !defined(YYJSON_DISABLE_WRITER) || !YYJSON_DISABLE_WRITER

yyjson_api char *yyjson_write_opts(const yyjson_doc *doc, yyjson_write_flag flg,
                                   const yyjson_alc *alc, size_t *len,
                                   yyjson_write_err *err);

yyjson_api bool yyjson_write_file(const char *path, const yyjson_doc *doc,
                                  yyjson_write_flag flg, const yyjson_alc *alc,
                                  yyjson_write_err *err);

yyjson_api bool yyjson_write_fp(FILE *fp, const yyjson_doc *doc,
                                yyjson_write_flag flg, const yyjson_alc *alc,
                                yyjson_write_err *err);

yyjson_api size_t yyjson_write_buf(char *buf, size_t buf_len,
                                   const yyjson_doc *doc, yyjson_write_flag flg,
                                   yyjson_write_err *err);

yyjson_api_inline char *yyjson_write(const yyjson_doc *doc,
                                     yyjson_write_flag flg, size_t *len) {
  return yyjson_write_opts(doc, flg, NULL, len, NULL);
}

yyjson_api char *yyjson_mut_write_opts(const yyjson_mut_doc *doc,
                                       yyjson_write_flag flg,
                                       const yyjson_alc *alc, size_t *len,
                                       yyjson_write_err *err);

yyjson_api bool yyjson_mut_write_file(const char *path,
                                      const yyjson_mut_doc *doc,
                                      yyjson_write_flag flg,
                                      const yyjson_alc *alc,
                                      yyjson_write_err *err);

yyjson_api bool yyjson_mut_write_fp(FILE *fp, const yyjson_mut_doc *doc,
                                    yyjson_write_flag flg,
                                    const yyjson_alc *alc,
                                    yyjson_write_err *err);

yyjson_api size_t yyjson_mut_write_buf(char *buf, size_t buf_len,
                                       const yyjson_mut_doc *doc,
                                       yyjson_write_flag flg,
                                       yyjson_write_err *err);

yyjson_api_inline char *yyjson_mut_write(const yyjson_mut_doc *doc,
                                         yyjson_write_flag flg, size_t *len) {
  return yyjson_mut_write_opts(doc, flg, NULL, len, NULL);
}

yyjson_api char *yyjson_val_write_opts(const yyjson_val *val,
                                       yyjson_write_flag flg,
                                       const yyjson_alc *alc, size_t *len,
                                       yyjson_write_err *err);

yyjson_api bool yyjson_val_write_file(const char *path, const yyjson_val *val,
                                      yyjson_write_flag flg,
                                      const yyjson_alc *alc,
                                      yyjson_write_err *err);

yyjson_api bool yyjson_val_write_fp(FILE *fp, const yyjson_val *val,
                                    yyjson_write_flag flg,
                                    const yyjson_alc *alc,
                                    yyjson_write_err *err);

yyjson_api size_t yyjson_val_write_buf(char *buf, size_t buf_len,
                                       const yyjson_val *val,
                                       yyjson_write_flag flg,
                                       yyjson_write_err *err);

yyjson_api_inline char *yyjson_val_write(const yyjson_val *val,
                                         yyjson_write_flag flg, size_t *len) {
  return yyjson_val_write_opts(val, flg, NULL, len, NULL);
}

yyjson_api char *yyjson_mut_val_write_opts(const yyjson_mut_val *val,
                                           yyjson_write_flag flg,
                                           const yyjson_alc *alc, size_t *len,
                                           yyjson_write_err *err);

yyjson_api bool yyjson_mut_val_write_file(const char *path,
                                          const yyjson_mut_val *val,
                                          yyjson_write_flag flg,
                                          const yyjson_alc *alc,
                                          yyjson_write_err *err);

yyjson_api bool yyjson_mut_val_write_fp(FILE *fp, const yyjson_mut_val *val,
                                        yyjson_write_flag flg,
                                        const yyjson_alc *alc,
                                        yyjson_write_err *err);

yyjson_api size_t yyjson_mut_val_write_buf(char *buf, size_t buf_len,
                                           const yyjson_mut_val *val,
                                           yyjson_write_flag flg,
                                           yyjson_write_err *err);

yyjson_api_inline char *yyjson_mut_val_write(const yyjson_mut_val *val,
                                             yyjson_write_flag flg,
                                             size_t *len) {
  return yyjson_mut_val_write_opts(val, flg, NULL, len, NULL);
}

yyjson_api char *yyjson_write_number(const yyjson_val *val, char *buf);

yyjson_api_inline char *yyjson_mut_write_number(const yyjson_mut_val *val,
                                                char *buf) {
  return yyjson_write_number((const yyjson_val *)val, buf);
}

#endif

yyjson_api_inline yyjson_val *yyjson_doc_get_root(const yyjson_doc *doc);

yyjson_api_inline size_t yyjson_doc_get_read_size(const yyjson_doc *doc);

yyjson_api_inline size_t yyjson_doc_get_val_count(const yyjson_doc *doc);

yyjson_api_inline void yyjson_doc_free(yyjson_doc *doc);

yyjson_api_inline bool yyjson_is_raw(const yyjson_val *val);

yyjson_api_inline bool yyjson_is_null(const yyjson_val *val);

yyjson_api_inline bool yyjson_is_true(const yyjson_val *val);

yyjson_api_inline bool yyjson_is_false(const yyjson_val *val);

yyjson_api_inline bool yyjson_is_bool(const yyjson_val *val);

yyjson_api_inline bool yyjson_is_uint(const yyjson_val *val);

yyjson_api_inline bool yyjson_is_sint(const yyjson_val *val);

yyjson_api_inline bool yyjson_is_int(const yyjson_val *val);

yyjson_api_inline bool yyjson_is_real(const yyjson_val *val);

yyjson_api_inline bool yyjson_is_num(const yyjson_val *val);

yyjson_api_inline bool yyjson_is_str(const yyjson_val *val);

yyjson_api_inline bool yyjson_is_arr(const yyjson_val *val);

yyjson_api_inline bool yyjson_is_obj(const yyjson_val *val);

yyjson_api_inline bool yyjson_is_ctn(const yyjson_val *val);

yyjson_api_inline yyjson_type yyjson_get_type(const yyjson_val *val);

yyjson_api_inline yyjson_subtype yyjson_get_subtype(const yyjson_val *val);

yyjson_api_inline uint8_t yyjson_get_tag(const yyjson_val *val);

yyjson_api_inline const char *yyjson_get_type_desc(const yyjson_val *val);

yyjson_api_inline const char *yyjson_get_raw(const yyjson_val *val);

yyjson_api_inline bool yyjson_get_bool(const yyjson_val *val);

yyjson_api_inline uint64_t yyjson_get_uint(const yyjson_val *val);

yyjson_api_inline int64_t yyjson_get_sint(const yyjson_val *val);

yyjson_api_inline int yyjson_get_int(const yyjson_val *val);

yyjson_api_inline double yyjson_get_real(const yyjson_val *val);

yyjson_api_inline double yyjson_get_num(const yyjson_val *val);

yyjson_api_inline const char *yyjson_get_str(const yyjson_val *val);

yyjson_api_inline size_t yyjson_get_len(const yyjson_val *val);

yyjson_api_inline bool yyjson_equals_str(const yyjson_val *val,
                                         const char *str);

yyjson_api_inline bool yyjson_equals_strn(const yyjson_val *val,
                                          const char *str, size_t len);

yyjson_api_inline bool yyjson_equals(const yyjson_val *lhs,
                                     const yyjson_val *rhs);

yyjson_api_inline bool yyjson_set_raw(yyjson_val *val, const char *raw,
                                      size_t len);

yyjson_api_inline bool yyjson_set_null(yyjson_val *val);

yyjson_api_inline bool yyjson_set_bool(yyjson_val *val, bool num);

yyjson_api_inline bool yyjson_set_uint(yyjson_val *val, uint64_t num);

yyjson_api_inline bool yyjson_set_sint(yyjson_val *val, int64_t num);

yyjson_api_inline bool yyjson_set_int(yyjson_val *val, int64_t num);

yyjson_api_inline bool yyjson_set_float(yyjson_val *val, float num);

yyjson_api_inline bool yyjson_set_double(yyjson_val *val, double num);

yyjson_api_inline bool yyjson_set_real(yyjson_val *val, double num);

yyjson_api_inline bool yyjson_set_fp_to_fixed(yyjson_val *val, int prec);

yyjson_api_inline bool yyjson_set_fp_to_float(yyjson_val *val, bool flt);

yyjson_api_inline bool yyjson_set_str(yyjson_val *val, const char *str);

yyjson_api_inline bool yyjson_set_strn(yyjson_val *val, const char *str,
                                       size_t len);

yyjson_api_inline bool yyjson_set_str_noesc(yyjson_val *val, bool noesc);

yyjson_api_inline size_t yyjson_arr_size(const yyjson_val *arr);

yyjson_api_inline yyjson_val *yyjson_arr_get(const yyjson_val *arr, size_t idx);

yyjson_api_inline yyjson_val *yyjson_arr_get_first(const yyjson_val *arr);

yyjson_api_inline yyjson_val *yyjson_arr_get_last(const yyjson_val *arr);

typedef struct yyjson_arr_iter {
  size_t idx;
  size_t max;
  yyjson_val *cur;
} yyjson_arr_iter;

yyjson_api_inline bool yyjson_arr_iter_init(const yyjson_val *arr,
                                            yyjson_arr_iter *iter);

yyjson_api_inline yyjson_arr_iter yyjson_arr_iter_with(const yyjson_val *arr);

yyjson_api_inline bool yyjson_arr_iter_has_next(yyjson_arr_iter *iter);

yyjson_api_inline yyjson_val *yyjson_arr_iter_next(yyjson_arr_iter *iter);

#define yyjson_arr_foreach(arr, idx, max, val)  \
  for ((idx) = 0, (max) = yyjson_arr_size(arr), \
      (val) = yyjson_arr_get_first(arr);        \
       (idx) < (max); (idx)++, (val) = unsafe_yyjson_get_next(val))

yyjson_api_inline size_t yyjson_obj_size(const yyjson_val *obj);

yyjson_api_inline yyjson_val *yyjson_obj_get(const yyjson_val *obj,
                                             const char *key);

yyjson_api_inline yyjson_val *yyjson_obj_getn(const yyjson_val *obj,
                                              const char *key, size_t key_len);

typedef struct yyjson_obj_iter {
  size_t idx;
  size_t max;
  yyjson_val *cur;
  yyjson_val *obj;
} yyjson_obj_iter;

yyjson_api_inline bool yyjson_obj_iter_init(const yyjson_val *obj,
                                            yyjson_obj_iter *iter);

yyjson_api_inline yyjson_obj_iter yyjson_obj_iter_with(const yyjson_val *obj);

yyjson_api_inline bool yyjson_obj_iter_has_next(yyjson_obj_iter *iter);

yyjson_api_inline yyjson_val *yyjson_obj_iter_next(yyjson_obj_iter *iter);

yyjson_api_inline yyjson_val *yyjson_obj_iter_get_val(yyjson_val *key);

yyjson_api_inline yyjson_val *yyjson_obj_iter_get(yyjson_obj_iter *iter,
                                                  const char *key);

yyjson_api_inline yyjson_val *yyjson_obj_iter_getn(yyjson_obj_iter *iter,
                                                   const char *key,
                                                   size_t key_len);

#define yyjson_obj_foreach(obj, idx, max, key, val)                           \
  for ((idx) = 0, (max) = yyjson_obj_size(obj),                               \
      (key) = (obj) ? unsafe_yyjson_get_first(obj) : NULL, (val) = (key) + 1; \
       (idx) < (max);                                                         \
       (idx)++, (key) = unsafe_yyjson_get_next(val), (val) = (key) + 1)

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_get_root(yyjson_mut_doc *doc);

yyjson_api_inline void yyjson_mut_doc_set_root(yyjson_mut_doc *doc,
                                               yyjson_mut_val *root);

yyjson_api bool yyjson_mut_doc_set_str_pool_size(yyjson_mut_doc *doc,
                                                 size_t len);

yyjson_api bool yyjson_mut_doc_set_val_pool_size(yyjson_mut_doc *doc,
                                                 size_t count);

yyjson_api void yyjson_mut_doc_free(yyjson_mut_doc *doc);

yyjson_api yyjson_mut_doc *yyjson_mut_doc_new(const yyjson_alc *alc);

yyjson_api yyjson_mut_doc *yyjson_doc_mut_copy(const yyjson_doc *doc,
                                               const yyjson_alc *alc);

yyjson_api yyjson_mut_doc *yyjson_mut_doc_mut_copy(const yyjson_mut_doc *doc,
                                                   const yyjson_alc *alc);

yyjson_api yyjson_mut_val *yyjson_val_mut_copy(yyjson_mut_doc *doc,
                                               const yyjson_val *val);

yyjson_api yyjson_mut_val *yyjson_mut_val_mut_copy(yyjson_mut_doc *doc,
                                                   const yyjson_mut_val *val);

yyjson_api yyjson_doc *yyjson_mut_doc_imut_copy(const yyjson_mut_doc *doc,
                                                const yyjson_alc *alc);

yyjson_api yyjson_doc *yyjson_mut_val_imut_copy(const yyjson_mut_val *val,
                                                const yyjson_alc *alc);

yyjson_api_inline bool yyjson_mut_is_raw(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_is_null(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_is_true(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_is_false(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_is_bool(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_is_uint(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_is_sint(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_is_int(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_is_real(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_is_num(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_is_str(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_is_arr(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_is_obj(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_is_ctn(const yyjson_mut_val *val);

yyjson_api_inline yyjson_type yyjson_mut_get_type(const yyjson_mut_val *val);

yyjson_api_inline yyjson_subtype
yyjson_mut_get_subtype(const yyjson_mut_val *val);

yyjson_api_inline uint8_t yyjson_mut_get_tag(const yyjson_mut_val *val);

yyjson_api_inline const char *yyjson_mut_get_type_desc(
    const yyjson_mut_val *val);

yyjson_api_inline const char *yyjson_mut_get_raw(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_get_bool(const yyjson_mut_val *val);

yyjson_api_inline uint64_t yyjson_mut_get_uint(const yyjson_mut_val *val);

yyjson_api_inline int64_t yyjson_mut_get_sint(const yyjson_mut_val *val);

yyjson_api_inline int yyjson_mut_get_int(const yyjson_mut_val *val);

yyjson_api_inline double yyjson_mut_get_real(const yyjson_mut_val *val);

yyjson_api_inline double yyjson_mut_get_num(const yyjson_mut_val *val);

yyjson_api_inline const char *yyjson_mut_get_str(const yyjson_mut_val *val);

yyjson_api_inline size_t yyjson_mut_get_len(const yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_equals_str(const yyjson_mut_val *val,
                                             const char *str);

yyjson_api_inline bool yyjson_mut_equals_strn(const yyjson_mut_val *val,
                                              const char *str, size_t len);

yyjson_api_inline bool yyjson_mut_equals(const yyjson_mut_val *lhs,
                                         const yyjson_mut_val *rhs);

yyjson_api_inline bool yyjson_mut_set_raw(yyjson_mut_val *val, const char *raw,
                                          size_t len);

yyjson_api_inline bool yyjson_mut_set_null(yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_set_bool(yyjson_mut_val *val, bool num);

yyjson_api_inline bool yyjson_mut_set_uint(yyjson_mut_val *val, uint64_t num);

yyjson_api_inline bool yyjson_mut_set_sint(yyjson_mut_val *val, int64_t num);

yyjson_api_inline bool yyjson_mut_set_int(yyjson_mut_val *val, int64_t num);

yyjson_api_inline bool yyjson_mut_set_float(yyjson_mut_val *val, float num);

yyjson_api_inline bool yyjson_mut_set_double(yyjson_mut_val *val, double num);

yyjson_api_inline bool yyjson_mut_set_real(yyjson_mut_val *val, double num);

yyjson_api_inline bool yyjson_mut_set_fp_to_fixed(yyjson_mut_val *val,
                                                  int prec);

yyjson_api_inline bool yyjson_mut_set_fp_to_float(yyjson_mut_val *val,
                                                  bool flt);

yyjson_api_inline bool yyjson_mut_set_str(yyjson_mut_val *val, const char *str);

yyjson_api_inline bool yyjson_mut_set_strn(yyjson_mut_val *val, const char *str,
                                           size_t len);

yyjson_api_inline bool yyjson_mut_set_str_noesc(yyjson_mut_val *val,
                                                bool noesc);

yyjson_api_inline bool yyjson_mut_set_arr(yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_set_obj(yyjson_mut_val *val);

yyjson_api_inline yyjson_mut_val *yyjson_mut_raw(yyjson_mut_doc *doc,
                                                 const char *str);

yyjson_api_inline yyjson_mut_val *yyjson_mut_rawn(yyjson_mut_doc *doc,
                                                  const char *str, size_t len);

yyjson_api_inline yyjson_mut_val *yyjson_mut_rawcpy(yyjson_mut_doc *doc,
                                                    const char *str);

yyjson_api_inline yyjson_mut_val *yyjson_mut_rawncpy(yyjson_mut_doc *doc,
                                                     const char *str,
                                                     size_t len);

yyjson_api_inline yyjson_mut_val *yyjson_mut_null(yyjson_mut_doc *doc);

yyjson_api_inline yyjson_mut_val *yyjson_mut_true(yyjson_mut_doc *doc);

yyjson_api_inline yyjson_mut_val *yyjson_mut_false(yyjson_mut_doc *doc);

yyjson_api_inline yyjson_mut_val *yyjson_mut_bool(yyjson_mut_doc *doc,
                                                  bool val);

yyjson_api_inline yyjson_mut_val *yyjson_mut_uint(yyjson_mut_doc *doc,
                                                  uint64_t num);

yyjson_api_inline yyjson_mut_val *yyjson_mut_sint(yyjson_mut_doc *doc,
                                                  int64_t num);

yyjson_api_inline yyjson_mut_val *yyjson_mut_int(yyjson_mut_doc *doc,
                                                 int64_t num);

yyjson_api_inline yyjson_mut_val *yyjson_mut_float(yyjson_mut_doc *doc,
                                                   float num);

yyjson_api_inline yyjson_mut_val *yyjson_mut_double(yyjson_mut_doc *doc,
                                                    double num);

yyjson_api_inline yyjson_mut_val *yyjson_mut_real(yyjson_mut_doc *doc,
                                                  double num);

yyjson_api_inline yyjson_mut_val *yyjson_mut_str(yyjson_mut_doc *doc,
                                                 const char *str);

yyjson_api_inline yyjson_mut_val *yyjson_mut_strn(yyjson_mut_doc *doc,
                                                  const char *str, size_t len);

yyjson_api_inline yyjson_mut_val *yyjson_mut_strcpy(yyjson_mut_doc *doc,
                                                    const char *str);

yyjson_api_inline yyjson_mut_val *yyjson_mut_strncpy(yyjson_mut_doc *doc,
                                                     const char *str,
                                                     size_t len);

yyjson_api_inline size_t yyjson_mut_arr_size(const yyjson_mut_val *arr);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_get(const yyjson_mut_val *arr,
                                                     size_t idx);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_get_first(
    const yyjson_mut_val *arr);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_get_last(
    const yyjson_mut_val *arr);

typedef struct yyjson_mut_arr_iter {
  size_t idx;
  size_t max;
  yyjson_mut_val *cur;
  yyjson_mut_val *pre;
  yyjson_mut_val *arr;
} yyjson_mut_arr_iter;

yyjson_api_inline bool yyjson_mut_arr_iter_init(yyjson_mut_val *arr,
                                                yyjson_mut_arr_iter *iter);

yyjson_api_inline yyjson_mut_arr_iter
yyjson_mut_arr_iter_with(yyjson_mut_val *arr);

yyjson_api_inline bool yyjson_mut_arr_iter_has_next(yyjson_mut_arr_iter *iter);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_iter_next(
    yyjson_mut_arr_iter *iter);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_iter_remove(
    yyjson_mut_arr_iter *iter);

#define yyjson_mut_arr_foreach(arr, idx, max, val)  \
  for ((idx) = 0, (max) = yyjson_mut_arr_size(arr), \
      (val) = yyjson_mut_arr_get_first(arr);        \
       (idx) < (max); (idx)++, (val) = (val)->next)

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr(yyjson_mut_doc *doc);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_bool(yyjson_mut_doc *doc,
                                                           const bool *vals,
                                                           size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_sint(yyjson_mut_doc *doc,
                                                           const int64_t *vals,
                                                           size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_uint(yyjson_mut_doc *doc,
                                                           const uint64_t *vals,
                                                           size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_real(yyjson_mut_doc *doc,
                                                           const double *vals,
                                                           size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_sint8(yyjson_mut_doc *doc,
                                                            const int8_t *vals,
                                                            size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_sint16(
    yyjson_mut_doc *doc, const int16_t *vals, size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_sint32(
    yyjson_mut_doc *doc, const int32_t *vals, size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_sint64(
    yyjson_mut_doc *doc, const int64_t *vals, size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_uint8(yyjson_mut_doc *doc,
                                                            const uint8_t *vals,
                                                            size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_uint16(
    yyjson_mut_doc *doc, const uint16_t *vals, size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_uint32(
    yyjson_mut_doc *doc, const uint32_t *vals, size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_uint64(
    yyjson_mut_doc *doc, const uint64_t *vals, size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_float(yyjson_mut_doc *doc,
                                                            const float *vals,
                                                            size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_double(
    yyjson_mut_doc *doc, const double *vals, size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_str(yyjson_mut_doc *doc,
                                                          const char **vals,
                                                          size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_strn(yyjson_mut_doc *doc,
                                                           const char **vals,
                                                           const size_t *lens,
                                                           size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_strcpy(
    yyjson_mut_doc *doc, const char **vals, size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_strncpy(
    yyjson_mut_doc *doc, const char **vals, const size_t *lens, size_t count);

yyjson_api_inline bool yyjson_mut_arr_insert(yyjson_mut_val *arr,
                                             yyjson_mut_val *val, size_t idx);

yyjson_api_inline bool yyjson_mut_arr_append(yyjson_mut_val *arr,
                                             yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_arr_prepend(yyjson_mut_val *arr,
                                              yyjson_mut_val *val);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_replace(yyjson_mut_val *arr,
                                                         size_t idx,
                                                         yyjson_mut_val *val);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_remove(yyjson_mut_val *arr,
                                                        size_t idx);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_remove_first(
    yyjson_mut_val *arr);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_remove_last(
    yyjson_mut_val *arr);

yyjson_api_inline bool yyjson_mut_arr_remove_range(yyjson_mut_val *arr,
                                                   size_t idx, size_t len);

yyjson_api_inline bool yyjson_mut_arr_clear(yyjson_mut_val *arr);

yyjson_api_inline bool yyjson_mut_arr_rotate(yyjson_mut_val *arr, size_t idx);

yyjson_api_inline bool yyjson_mut_arr_add_val(yyjson_mut_val *arr,
                                              yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_arr_add_null(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr);

yyjson_api_inline bool yyjson_mut_arr_add_true(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr);

yyjson_api_inline bool yyjson_mut_arr_add_false(yyjson_mut_doc *doc,
                                                yyjson_mut_val *arr);

yyjson_api_inline bool yyjson_mut_arr_add_bool(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr, bool val);

yyjson_api_inline bool yyjson_mut_arr_add_uint(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr,
                                               uint64_t num);

yyjson_api_inline bool yyjson_mut_arr_add_sint(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr,
                                               int64_t num);

yyjson_api_inline bool yyjson_mut_arr_add_int(yyjson_mut_doc *doc,
                                              yyjson_mut_val *arr, int64_t num);

yyjson_api_inline bool yyjson_mut_arr_add_float(yyjson_mut_doc *doc,
                                                yyjson_mut_val *arr, float num);

yyjson_api_inline bool yyjson_mut_arr_add_double(yyjson_mut_doc *doc,
                                                 yyjson_mut_val *arr,
                                                 double num);

yyjson_api_inline bool yyjson_mut_arr_add_real(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr, double num);

yyjson_api_inline bool yyjson_mut_arr_add_str(yyjson_mut_doc *doc,
                                              yyjson_mut_val *arr,
                                              const char *str);

yyjson_api_inline bool yyjson_mut_arr_add_strn(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr,
                                               const char *str, size_t len);

yyjson_api_inline bool yyjson_mut_arr_add_strcpy(yyjson_mut_doc *doc,
                                                 yyjson_mut_val *arr,
                                                 const char *str);

yyjson_api_inline bool yyjson_mut_arr_add_strncpy(yyjson_mut_doc *doc,
                                                  yyjson_mut_val *arr,
                                                  const char *str, size_t len);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_add_arr(yyjson_mut_doc *doc,
                                                         yyjson_mut_val *arr);

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_add_obj(yyjson_mut_doc *doc,
                                                         yyjson_mut_val *arr);

yyjson_api_inline size_t yyjson_mut_obj_size(const yyjson_mut_val *obj);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_get(const yyjson_mut_val *obj,
                                                     const char *key);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_getn(const yyjson_mut_val *obj,
                                                      const char *key,
                                                      size_t key_len);

typedef struct yyjson_mut_obj_iter {
  size_t idx;
  size_t max;
  yyjson_mut_val *cur;
  yyjson_mut_val *pre;
  yyjson_mut_val *obj;
} yyjson_mut_obj_iter;

yyjson_api_inline bool yyjson_mut_obj_iter_init(yyjson_mut_val *obj,
                                                yyjson_mut_obj_iter *iter);

yyjson_api_inline yyjson_mut_obj_iter
yyjson_mut_obj_iter_with(yyjson_mut_val *obj);

yyjson_api_inline bool yyjson_mut_obj_iter_has_next(yyjson_mut_obj_iter *iter);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_iter_next(
    yyjson_mut_obj_iter *iter);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_iter_get_val(
    yyjson_mut_val *key);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_iter_remove(
    yyjson_mut_obj_iter *iter);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_iter_get(
    yyjson_mut_obj_iter *iter, const char *key);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_iter_getn(
    yyjson_mut_obj_iter *iter, const char *key, size_t key_len);

#define yyjson_mut_obj_foreach(obj, idx, max, key, val)                      \
  for ((idx) = 0, (max) = yyjson_mut_obj_size(obj),                          \
      (key) = (max) ? ((yyjson_mut_val *)(obj)->uni.ptr)->next->next : NULL, \
      (val) = (key) ? (key)->next : NULL;                                    \
       (idx) < (max); (idx)++, (key) = (val)->next, (val) = (key)->next)

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj(yyjson_mut_doc *doc);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_with_str(yyjson_mut_doc *doc,
                                                          const char **keys,
                                                          const char **vals,
                                                          size_t count);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_with_kv(yyjson_mut_doc *doc,
                                                         const char **kv_pairs,
                                                         size_t pair_count);

yyjson_api_inline bool yyjson_mut_obj_add(yyjson_mut_val *obj,
                                          yyjson_mut_val *key,
                                          yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_obj_put(yyjson_mut_val *obj,
                                          yyjson_mut_val *key,
                                          yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_obj_insert(yyjson_mut_val *obj,
                                             yyjson_mut_val *key,
                                             yyjson_mut_val *val, size_t idx);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_remove(yyjson_mut_val *obj,
                                                        yyjson_mut_val *key);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_remove_key(yyjson_mut_val *obj,
                                                            const char *key);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_remove_keyn(
    yyjson_mut_val *obj, const char *key, size_t key_len);

yyjson_api_inline bool yyjson_mut_obj_clear(yyjson_mut_val *obj);

yyjson_api_inline bool yyjson_mut_obj_replace(yyjson_mut_val *obj,
                                              yyjson_mut_val *key,
                                              yyjson_mut_val *val);

yyjson_api_inline bool yyjson_mut_obj_rotate(yyjson_mut_val *obj, size_t idx);

yyjson_api_inline bool yyjson_mut_obj_add_null(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *key);

yyjson_api_inline bool yyjson_mut_obj_add_true(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *key);

yyjson_api_inline bool yyjson_mut_obj_add_false(yyjson_mut_doc *doc,
                                                yyjson_mut_val *obj,
                                                const char *key);

yyjson_api_inline bool yyjson_mut_obj_add_bool(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *key, bool val);

yyjson_api_inline bool yyjson_mut_obj_add_uint(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *key, uint64_t val);

yyjson_api_inline bool yyjson_mut_obj_add_sint(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *key, int64_t val);

yyjson_api_inline bool yyjson_mut_obj_add_int(yyjson_mut_doc *doc,
                                              yyjson_mut_val *obj,
                                              const char *key, int64_t val);

yyjson_api_inline bool yyjson_mut_obj_add_float(yyjson_mut_doc *doc,
                                                yyjson_mut_val *obj,
                                                const char *key, float val);

yyjson_api_inline bool yyjson_mut_obj_add_double(yyjson_mut_doc *doc,
                                                 yyjson_mut_val *obj,
                                                 const char *key, double val);

yyjson_api_inline bool yyjson_mut_obj_add_real(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *key, double val);

yyjson_api_inline bool yyjson_mut_obj_add_str(yyjson_mut_doc *doc,
                                              yyjson_mut_val *obj,
                                              const char *key, const char *val);

yyjson_api_inline bool yyjson_mut_obj_add_strn(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *key, const char *val,
                                               size_t len);

yyjson_api_inline bool yyjson_mut_obj_add_strcpy(yyjson_mut_doc *doc,
                                                 yyjson_mut_val *obj,
                                                 const char *key,
                                                 const char *val);

yyjson_api_inline bool yyjson_mut_obj_add_strncpy(yyjson_mut_doc *doc,
                                                  yyjson_mut_val *obj,
                                                  const char *key,
                                                  const char *val, size_t len);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_add_arr(yyjson_mut_doc *doc,
                                                         yyjson_mut_val *obj,
                                                         const char *key);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_add_obj(yyjson_mut_doc *doc,
                                                         yyjson_mut_val *obj,
                                                         const char *key);

yyjson_api_inline bool yyjson_mut_obj_add_val(yyjson_mut_doc *doc,
                                              yyjson_mut_val *obj,
                                              const char *key,
                                              yyjson_mut_val *val);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_remove_str(yyjson_mut_val *obj,
                                                            const char *key);

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_remove_strn(
    yyjson_mut_val *obj, const char *key, size_t len);

yyjson_api_inline bool yyjson_mut_obj_rename_key(yyjson_mut_doc *doc,
                                                 yyjson_mut_val *obj,
                                                 const char *key,
                                                 const char *new_key);

yyjson_api_inline bool yyjson_mut_obj_rename_keyn(yyjson_mut_doc *doc,
                                                  yyjson_mut_val *obj,
                                                  const char *key, size_t len,
                                                  const char *new_key,
                                                  size_t new_len);

#if !defined(YYJSON_DISABLE_UTILS) || !YYJSON_DISABLE_UTILS

typedef uint32_t yyjson_ptr_code;

static const yyjson_ptr_code YYJSON_PTR_ERR_NONE = 0;

static const yyjson_ptr_code YYJSON_PTR_ERR_PARAMETER = 1;

static const yyjson_ptr_code YYJSON_PTR_ERR_SYNTAX = 2;

static const yyjson_ptr_code YYJSON_PTR_ERR_RESOLVE = 3;

static const yyjson_ptr_code YYJSON_PTR_ERR_NULL_ROOT = 4;

static const yyjson_ptr_code YYJSON_PTR_ERR_SET_ROOT = 5;

static const yyjson_ptr_code YYJSON_PTR_ERR_MEMORY_ALLOCATION = 6;

typedef struct yyjson_ptr_err {
  yyjson_ptr_code code;

  const char *msg;

  size_t pos;
} yyjson_ptr_err;

typedef struct yyjson_ptr_ctx {
  yyjson_mut_val *ctn;

  yyjson_mut_val *pre;

  yyjson_mut_val *old;
} yyjson_ptr_ctx;

yyjson_api_inline yyjson_val *yyjson_doc_ptr_get(const yyjson_doc *doc,
                                                 const char *ptr);

yyjson_api_inline yyjson_val *yyjson_doc_ptr_getn(const yyjson_doc *doc,
                                                  const char *ptr, size_t len);

yyjson_api_inline yyjson_val *yyjson_doc_ptr_getx(const yyjson_doc *doc,
                                                  const char *ptr, size_t len,
                                                  yyjson_ptr_err *err);

yyjson_api_inline yyjson_val *yyjson_ptr_get(const yyjson_val *val,
                                             const char *ptr);

yyjson_api_inline yyjson_val *yyjson_ptr_getn(const yyjson_val *val,
                                              const char *ptr, size_t len);

yyjson_api_inline yyjson_val *yyjson_ptr_getx(const yyjson_val *val,
                                              const char *ptr, size_t len,
                                              yyjson_ptr_err *err);

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_get(
    const yyjson_mut_doc *doc, const char *ptr);

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_getn(
    const yyjson_mut_doc *doc, const char *ptr, size_t len);

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_getx(
    const yyjson_mut_doc *doc, const char *ptr, size_t len, yyjson_ptr_ctx *ctx,
    yyjson_ptr_err *err);

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_get(const yyjson_mut_val *val,
                                                     const char *ptr);

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_getn(const yyjson_mut_val *val,
                                                      const char *ptr,
                                                      size_t len);

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_getx(const yyjson_mut_val *val,
                                                      const char *ptr,
                                                      size_t len,
                                                      yyjson_ptr_ctx *ctx,
                                                      yyjson_ptr_err *err);

yyjson_api_inline bool yyjson_mut_doc_ptr_add(yyjson_mut_doc *doc,
                                              const char *ptr,
                                              yyjson_mut_val *new_val);

yyjson_api_inline bool yyjson_mut_doc_ptr_addn(yyjson_mut_doc *doc,
                                               const char *ptr, size_t len,
                                               yyjson_mut_val *new_val);

yyjson_api_inline bool yyjson_mut_doc_ptr_addx(
    yyjson_mut_doc *doc, const char *ptr, size_t len, yyjson_mut_val *new_val,
    bool create_parent, yyjson_ptr_ctx *ctx, yyjson_ptr_err *err);

yyjson_api_inline bool yyjson_mut_ptr_add(yyjson_mut_val *val, const char *ptr,
                                          yyjson_mut_val *new_val,
                                          yyjson_mut_doc *doc);

yyjson_api_inline bool yyjson_mut_ptr_addn(yyjson_mut_val *val, const char *ptr,
                                           size_t len, yyjson_mut_val *new_val,
                                           yyjson_mut_doc *doc);

yyjson_api_inline bool yyjson_mut_ptr_addx(yyjson_mut_val *val, const char *ptr,
                                           size_t len, yyjson_mut_val *new_val,
                                           yyjson_mut_doc *doc,
                                           bool create_parent,
                                           yyjson_ptr_ctx *ctx,
                                           yyjson_ptr_err *err);

yyjson_api_inline bool yyjson_mut_doc_ptr_set(yyjson_mut_doc *doc,
                                              const char *ptr,
                                              yyjson_mut_val *new_val);

yyjson_api_inline bool yyjson_mut_doc_ptr_setn(yyjson_mut_doc *doc,
                                               const char *ptr, size_t len,
                                               yyjson_mut_val *new_val);

yyjson_api_inline bool yyjson_mut_doc_ptr_setx(
    yyjson_mut_doc *doc, const char *ptr, size_t len, yyjson_mut_val *new_val,
    bool create_parent, yyjson_ptr_ctx *ctx, yyjson_ptr_err *err);

yyjson_api_inline bool yyjson_mut_ptr_set(yyjson_mut_val *val, const char *ptr,
                                          yyjson_mut_val *new_val,
                                          yyjson_mut_doc *doc);

yyjson_api_inline bool yyjson_mut_ptr_setn(yyjson_mut_val *val, const char *ptr,
                                           size_t len, yyjson_mut_val *new_val,
                                           yyjson_mut_doc *doc);

yyjson_api_inline bool yyjson_mut_ptr_setx(yyjson_mut_val *val, const char *ptr,
                                           size_t len, yyjson_mut_val *new_val,
                                           yyjson_mut_doc *doc,
                                           bool create_parent,
                                           yyjson_ptr_ctx *ctx,
                                           yyjson_ptr_err *err);

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_replace(
    yyjson_mut_doc *doc, const char *ptr, yyjson_mut_val *new_val);

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_replacen(
    yyjson_mut_doc *doc, const char *ptr, size_t len, yyjson_mut_val *new_val);

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_replacex(
    yyjson_mut_doc *doc, const char *ptr, size_t len, yyjson_mut_val *new_val,
    yyjson_ptr_ctx *ctx, yyjson_ptr_err *err);

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_replace(
    yyjson_mut_val *val, const char *ptr, yyjson_mut_val *new_val);

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_replacen(
    yyjson_mut_val *val, const char *ptr, size_t len, yyjson_mut_val *new_val);

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_replacex(
    yyjson_mut_val *val, const char *ptr, size_t len, yyjson_mut_val *new_val,
    yyjson_ptr_ctx *ctx, yyjson_ptr_err *err);

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_remove(yyjson_mut_doc *doc,
                                                            const char *ptr);

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_removen(
    yyjson_mut_doc *doc, const char *ptr, size_t len);

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_removex(
    yyjson_mut_doc *doc, const char *ptr, size_t len, yyjson_ptr_ctx *ctx,
    yyjson_ptr_err *err);

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_remove(yyjson_mut_val *val,
                                                        const char *ptr);

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_removen(yyjson_mut_val *val,
                                                         const char *ptr,
                                                         size_t len);

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_removex(yyjson_mut_val *val,
                                                         const char *ptr,
                                                         size_t len,
                                                         yyjson_ptr_ctx *ctx,
                                                         yyjson_ptr_err *err);

yyjson_api_inline bool yyjson_ptr_ctx_append(yyjson_ptr_ctx *ctx,
                                             yyjson_mut_val *key,
                                             yyjson_mut_val *val);

yyjson_api_inline bool yyjson_ptr_ctx_replace(yyjson_ptr_ctx *ctx,
                                              yyjson_mut_val *val);

yyjson_api_inline bool yyjson_ptr_ctx_remove(yyjson_ptr_ctx *ctx);

typedef uint32_t yyjson_patch_code;

static const yyjson_patch_code YYJSON_PATCH_SUCCESS = 0;

static const yyjson_patch_code YYJSON_PATCH_ERROR_INVALID_PARAMETER = 1;

static const yyjson_patch_code YYJSON_PATCH_ERROR_MEMORY_ALLOCATION = 2;

static const yyjson_patch_code YYJSON_PATCH_ERROR_INVALID_OPERATION = 3;

static const yyjson_patch_code YYJSON_PATCH_ERROR_MISSING_KEY = 4;

static const yyjson_patch_code YYJSON_PATCH_ERROR_INVALID_MEMBER = 5;

static const yyjson_patch_code YYJSON_PATCH_ERROR_EQUAL = 6;

static const yyjson_patch_code YYJSON_PATCH_ERROR_POINTER = 7;

typedef struct yyjson_patch_err {
  yyjson_patch_code code;

  size_t idx;

  const char *msg;

  yyjson_ptr_err ptr;
} yyjson_patch_err;

yyjson_api yyjson_mut_val *yyjson_patch(yyjson_mut_doc *doc,
                                        const yyjson_val *orig,
                                        const yyjson_val *patch,
                                        yyjson_patch_err *err);

yyjson_api yyjson_mut_val *yyjson_mut_patch(yyjson_mut_doc *doc,
                                            const yyjson_mut_val *orig,
                                            const yyjson_mut_val *patch,
                                            yyjson_patch_err *err);

yyjson_api yyjson_mut_val *yyjson_merge_patch(yyjson_mut_doc *doc,
                                              const yyjson_val *orig,
                                              const yyjson_val *patch);

yyjson_api yyjson_mut_val *yyjson_mut_merge_patch(yyjson_mut_doc *doc,
                                                  const yyjson_mut_val *orig,
                                                  const yyjson_mut_val *patch);

#endif

typedef union yyjson_val_uni {
  uint64_t u64;
  int64_t i64;
  double f64;
  const char *str;
  void *ptr;
  size_t ofs;
} yyjson_val_uni;

struct yyjson_val {
  uint64_t tag;
  yyjson_val_uni uni;
};

struct yyjson_doc {
  yyjson_val *root;

  yyjson_alc alc;

  size_t dat_read;

  size_t val_read;

  char *str_pool;
};

yyjson_api_inline bool unsafe_yyjson_is_str_noesc(const char *str, size_t len) {
#if YYJSON_HAS_CONSTANT_P && \
    (!YYJSON_IS_REAL_GCC || yyjson_gcc_available(4, 4, 0))
  if (yyjson_constant_p(len) && len <= 32) {
#define yyjson_repeat32_incr(x)                                               \
  x(0) x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11) x(12) x(13)   \
      x(14) x(15) x(16) x(17) x(18) x(19) x(20) x(21) x(22) x(23) x(24) x(25) \
          x(26) x(27) x(28) x(29) x(30) x(31)
#define yyjson_check_char_noesc(i)                                 \
  if (i < len) {                                                   \
    char c = str[i];                                               \
    if (c < ' ' || c > '~' || c == '"' || c == '\\') return false; \
  }
    yyjson_repeat32_incr(yyjson_check_char_noesc)
#undef yyjson_repeat32_incr
#undef yyjson_check_char_noesc
        return true;
  }
#else
  (void)str;
  (void)len;
#endif
  return false;
}

yyjson_api_inline double unsafe_yyjson_u64_to_f64(uint64_t num) {
#if YYJSON_U64_TO_F64_NO_IMPL
  uint64_t msb = ((uint64_t)1) << 63;
  if ((num & msb) == 0) {
    return (double)(int64_t)num;
  } else {
    return ((double)(int64_t)((num >> 1) | (num & 1))) * (double)2.0;
  }
#else
  return (double)num;
#endif
}

yyjson_api_inline yyjson_type unsafe_yyjson_get_type(const void *val) {
  uint8_t tag = (uint8_t)((const yyjson_val *)val)->tag;
  return (yyjson_type)(tag & YYJSON_TYPE_MASK);
}

yyjson_api_inline yyjson_subtype unsafe_yyjson_get_subtype(const void *val) {
  uint8_t tag = (uint8_t)((const yyjson_val *)val)->tag;
  return (yyjson_subtype)(tag & YYJSON_SUBTYPE_MASK);
}

yyjson_api_inline uint8_t unsafe_yyjson_get_tag(const void *val) {
  uint8_t tag = (uint8_t)((const yyjson_val *)val)->tag;
  return (uint8_t)(tag & YYJSON_TAG_MASK);
}

yyjson_api_inline bool unsafe_yyjson_is_raw(const void *val) {
  return unsafe_yyjson_get_type(val) == YYJSON_TYPE_RAW;
}

yyjson_api_inline bool unsafe_yyjson_is_null(const void *val) {
  return unsafe_yyjson_get_type(val) == YYJSON_TYPE_NULL;
}

yyjson_api_inline bool unsafe_yyjson_is_bool(const void *val) {
  return unsafe_yyjson_get_type(val) == YYJSON_TYPE_BOOL;
}

yyjson_api_inline bool unsafe_yyjson_is_num(const void *val) {
  return unsafe_yyjson_get_type(val) == YYJSON_TYPE_NUM;
}

yyjson_api_inline bool unsafe_yyjson_is_str(const void *val) {
  return unsafe_yyjson_get_type(val) == YYJSON_TYPE_STR;
}

yyjson_api_inline bool unsafe_yyjson_is_arr(const void *val) {
  return unsafe_yyjson_get_type(val) == YYJSON_TYPE_ARR;
}

yyjson_api_inline bool unsafe_yyjson_is_obj(const void *val) {
  return unsafe_yyjson_get_type(val) == YYJSON_TYPE_OBJ;
}

yyjson_api_inline bool unsafe_yyjson_is_ctn(const void *val) {
  uint8_t mask = YYJSON_TYPE_ARR & YYJSON_TYPE_OBJ;
  return (unsafe_yyjson_get_tag(val) & mask) == mask;
}

yyjson_api_inline bool unsafe_yyjson_is_uint(const void *val) {
  const uint8_t patt = YYJSON_TYPE_NUM | YYJSON_SUBTYPE_UINT;
  return unsafe_yyjson_get_tag(val) == patt;
}

yyjson_api_inline bool unsafe_yyjson_is_sint(const void *val) {
  const uint8_t patt = YYJSON_TYPE_NUM | YYJSON_SUBTYPE_SINT;
  return unsafe_yyjson_get_tag(val) == patt;
}

yyjson_api_inline bool unsafe_yyjson_is_int(const void *val) {
  const uint8_t mask = YYJSON_TAG_MASK & (~YYJSON_SUBTYPE_SINT);
  const uint8_t patt = YYJSON_TYPE_NUM | YYJSON_SUBTYPE_UINT;
  return (unsafe_yyjson_get_tag(val) & mask) == patt;
}

yyjson_api_inline bool unsafe_yyjson_is_real(const void *val) {
  const uint8_t patt = YYJSON_TYPE_NUM | YYJSON_SUBTYPE_REAL;
  return unsafe_yyjson_get_tag(val) == patt;
}

yyjson_api_inline bool unsafe_yyjson_is_true(const void *val) {
  const uint8_t patt = YYJSON_TYPE_BOOL | YYJSON_SUBTYPE_TRUE;
  return unsafe_yyjson_get_tag(val) == patt;
}

yyjson_api_inline bool unsafe_yyjson_is_false(const void *val) {
  const uint8_t patt = YYJSON_TYPE_BOOL | YYJSON_SUBTYPE_FALSE;
  return unsafe_yyjson_get_tag(val) == patt;
}

yyjson_api_inline bool unsafe_yyjson_arr_is_flat(const yyjson_val *val) {
  size_t ofs = val->uni.ofs;
  size_t len = (size_t)(val->tag >> YYJSON_TAG_BIT);
  return len * sizeof(yyjson_val) + sizeof(yyjson_val) == ofs;
}

yyjson_api_inline const char *unsafe_yyjson_get_raw(const void *val) {
  return ((const yyjson_val *)val)->uni.str;
}

yyjson_api_inline bool unsafe_yyjson_get_bool(const void *val) {
  uint8_t tag = unsafe_yyjson_get_tag(val);
  return (bool)((tag & YYJSON_SUBTYPE_MASK) >> YYJSON_TYPE_BIT);
}

yyjson_api_inline uint64_t unsafe_yyjson_get_uint(const void *val) {
  return ((const yyjson_val *)val)->uni.u64;
}

yyjson_api_inline int64_t unsafe_yyjson_get_sint(const void *val) {
  return ((const yyjson_val *)val)->uni.i64;
}

yyjson_api_inline int unsafe_yyjson_get_int(const void *val) {
  return (int)((const yyjson_val *)val)->uni.i64;
}

yyjson_api_inline double unsafe_yyjson_get_real(const void *val) {
  return ((const yyjson_val *)val)->uni.f64;
}

yyjson_api_inline double unsafe_yyjson_get_num(const void *val) {
  uint8_t tag = unsafe_yyjson_get_tag(val);
  if (tag == (YYJSON_TYPE_NUM | YYJSON_SUBTYPE_REAL)) {
    return ((const yyjson_val *)val)->uni.f64;
  } else if (tag == (YYJSON_TYPE_NUM | YYJSON_SUBTYPE_SINT)) {
    return (double)((const yyjson_val *)val)->uni.i64;
  } else if (tag == (YYJSON_TYPE_NUM | YYJSON_SUBTYPE_UINT)) {
    return unsafe_yyjson_u64_to_f64(((const yyjson_val *)val)->uni.u64);
  }
  return 0.0;
}

yyjson_api_inline const char *unsafe_yyjson_get_str(const void *val) {
  return ((const yyjson_val *)val)->uni.str;
}

yyjson_api_inline size_t unsafe_yyjson_get_len(const void *val) {
  return (size_t)(((const yyjson_val *)val)->tag >> YYJSON_TAG_BIT);
}

yyjson_api_inline yyjson_val *unsafe_yyjson_get_first(const yyjson_val *ctn) {
  return yyjson_constcast(yyjson_val *) ctn + 1;
}

yyjson_api_inline yyjson_val *unsafe_yyjson_get_next(const yyjson_val *val) {
  bool is_ctn = unsafe_yyjson_is_ctn(val);
  size_t ctn_ofs = val->uni.ofs;
  size_t ofs = (is_ctn ? ctn_ofs : sizeof(yyjson_val));
  uint8_t *ptr = yyjson_constcast(uint8_t *) val;
  return (yyjson_val *)(void *)(ptr + ofs);
}

yyjson_api_inline bool unsafe_yyjson_equals_strn(const void *val,
                                                 const char *str, size_t len) {
  return unsafe_yyjson_get_len(val) == len &&
         memcmp(((const yyjson_val *)val)->uni.str, str, len) == 0;
}

yyjson_api_inline bool unsafe_yyjson_equals_str(const void *val,
                                                const char *str) {
  return unsafe_yyjson_equals_strn(val, str, strlen(str));
}

yyjson_api_inline void unsafe_yyjson_set_type(void *val, yyjson_type type,
                                              yyjson_subtype subtype) {
  uint8_t tag = (type | subtype);
  uint64_t new_tag = ((yyjson_val *)val)->tag;
  new_tag = (new_tag & (~(uint64_t)YYJSON_TAG_MASK)) | (uint64_t)tag;
  ((yyjson_val *)val)->tag = new_tag;
}

yyjson_api_inline void unsafe_yyjson_set_len(void *val, size_t len) {
  uint64_t tag = ((yyjson_val *)val)->tag & YYJSON_TAG_MASK;
  tag |= (uint64_t)len << YYJSON_TAG_BIT;
  ((yyjson_val *)val)->tag = tag;
}

yyjson_api_inline void unsafe_yyjson_set_tag(void *val, yyjson_type type,
                                             yyjson_subtype subtype,
                                             size_t len) {
  uint64_t tag = (uint64_t)len << YYJSON_TAG_BIT;
  tag |= (type | subtype);
  ((yyjson_val *)val)->tag = tag;
}

yyjson_api_inline void unsafe_yyjson_inc_len(void *val) {
  uint64_t tag = ((yyjson_val *)val)->tag;
  tag += (uint64_t)(1 << YYJSON_TAG_BIT);
  ((yyjson_val *)val)->tag = tag;
}

yyjson_api_inline void unsafe_yyjson_set_raw(void *val, const char *raw,
                                             size_t len) {
  unsafe_yyjson_set_tag(val, YYJSON_TYPE_RAW, YYJSON_SUBTYPE_NONE, len);
  ((yyjson_val *)val)->uni.str = raw;
}

yyjson_api_inline void unsafe_yyjson_set_null(void *val) {
  unsafe_yyjson_set_tag(val, YYJSON_TYPE_NULL, YYJSON_SUBTYPE_NONE, 0);
}

yyjson_api_inline void unsafe_yyjson_set_bool(void *val, bool num) {
  yyjson_subtype subtype = num ? YYJSON_SUBTYPE_TRUE : YYJSON_SUBTYPE_FALSE;
  unsafe_yyjson_set_tag(val, YYJSON_TYPE_BOOL, subtype, 0);
}

yyjson_api_inline void unsafe_yyjson_set_uint(void *val, uint64_t num) {
  unsafe_yyjson_set_tag(val, YYJSON_TYPE_NUM, YYJSON_SUBTYPE_UINT, 0);
  ((yyjson_val *)val)->uni.u64 = num;
}

yyjson_api_inline void unsafe_yyjson_set_sint(void *val, int64_t num) {
  unsafe_yyjson_set_tag(val, YYJSON_TYPE_NUM, YYJSON_SUBTYPE_SINT, 0);
  ((yyjson_val *)val)->uni.i64 = num;
}

yyjson_api_inline void unsafe_yyjson_set_fp_to_fixed(void *val, int prec) {
  ((yyjson_val *)val)->tag &= ~((uint64_t)YYJSON_WRITE_FP_TO_FIXED(15) << 32);
  ((yyjson_val *)val)->tag |= (uint64_t)YYJSON_WRITE_FP_TO_FIXED(prec) << 32;
}

yyjson_api_inline void unsafe_yyjson_set_fp_to_float(void *val, bool flt) {
  uint64_t flag = (uint64_t)YYJSON_WRITE_FP_TO_FLOAT << 32;
  if (flt)
    ((yyjson_val *)val)->tag |= flag;
  else
    ((yyjson_val *)val)->tag &= ~flag;
}

yyjson_api_inline void unsafe_yyjson_set_float(void *val, float num) {
  unsafe_yyjson_set_tag(val, YYJSON_TYPE_NUM, YYJSON_SUBTYPE_REAL, 0);
  ((yyjson_val *)val)->tag |= (uint64_t)YYJSON_WRITE_FP_TO_FLOAT << 32;
  ((yyjson_val *)val)->uni.f64 = (double)num;
}

yyjson_api_inline void unsafe_yyjson_set_double(void *val, double num) {
  unsafe_yyjson_set_tag(val, YYJSON_TYPE_NUM, YYJSON_SUBTYPE_REAL, 0);
  ((yyjson_val *)val)->uni.f64 = num;
}

yyjson_api_inline void unsafe_yyjson_set_real(void *val, double num) {
  unsafe_yyjson_set_tag(val, YYJSON_TYPE_NUM, YYJSON_SUBTYPE_REAL, 0);
  ((yyjson_val *)val)->uni.f64 = num;
}

yyjson_api_inline void unsafe_yyjson_set_str_noesc(void *val, bool noesc) {
  ((yyjson_val *)val)->tag &= ~(uint64_t)YYJSON_SUBTYPE_MASK;
  if (noesc) ((yyjson_val *)val)->tag |= (uint64_t)YYJSON_SUBTYPE_NOESC;
}

yyjson_api_inline void unsafe_yyjson_set_strn(void *val, const char *str,
                                              size_t len) {
  unsafe_yyjson_set_tag(val, YYJSON_TYPE_STR, YYJSON_SUBTYPE_NONE, len);
  ((yyjson_val *)val)->uni.str = str;
}

yyjson_api_inline void unsafe_yyjson_set_str(void *val, const char *str) {
  size_t len = strlen(str);
  bool noesc = unsafe_yyjson_is_str_noesc(str, len);
  yyjson_subtype subtype = noesc ? YYJSON_SUBTYPE_NOESC : YYJSON_SUBTYPE_NONE;
  unsafe_yyjson_set_tag(val, YYJSON_TYPE_STR, subtype, len);
  ((yyjson_val *)val)->uni.str = str;
}

yyjson_api_inline void unsafe_yyjson_set_arr(void *val, size_t size) {
  unsafe_yyjson_set_tag(val, YYJSON_TYPE_ARR, YYJSON_SUBTYPE_NONE, size);
}

yyjson_api_inline void unsafe_yyjson_set_obj(void *val, size_t size) {
  unsafe_yyjson_set_tag(val, YYJSON_TYPE_OBJ, YYJSON_SUBTYPE_NONE, size);
}

yyjson_api_inline yyjson_val *yyjson_doc_get_root(const yyjson_doc *doc) {
  return doc ? doc->root : NULL;
}

yyjson_api_inline size_t yyjson_doc_get_read_size(const yyjson_doc *doc) {
  return doc ? doc->dat_read : 0;
}

yyjson_api_inline size_t yyjson_doc_get_val_count(const yyjson_doc *doc) {
  return doc ? doc->val_read : 0;
}

yyjson_api_inline void yyjson_doc_free(yyjson_doc *doc) {
  if (doc) {
    yyjson_alc alc = doc->alc;
    memset(&doc->alc, 0, sizeof(alc));
    if (doc->str_pool) alc.free(alc.ctx, doc->str_pool);
    alc.free(alc.ctx, doc);
  }
}

yyjson_api_inline bool yyjson_is_raw(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_raw(val) : false;
}

yyjson_api_inline bool yyjson_is_null(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_null(val) : false;
}

yyjson_api_inline bool yyjson_is_true(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_true(val) : false;
}

yyjson_api_inline bool yyjson_is_false(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_false(val) : false;
}

yyjson_api_inline bool yyjson_is_bool(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_bool(val) : false;
}

yyjson_api_inline bool yyjson_is_uint(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_uint(val) : false;
}

yyjson_api_inline bool yyjson_is_sint(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_sint(val) : false;
}

yyjson_api_inline bool yyjson_is_int(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_int(val) : false;
}

yyjson_api_inline bool yyjson_is_real(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_real(val) : false;
}

yyjson_api_inline bool yyjson_is_num(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_num(val) : false;
}

yyjson_api_inline bool yyjson_is_str(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_str(val) : false;
}

yyjson_api_inline bool yyjson_is_arr(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_arr(val) : false;
}

yyjson_api_inline bool yyjson_is_obj(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_obj(val) : false;
}

yyjson_api_inline bool yyjson_is_ctn(const yyjson_val *val) {
  return val ? unsafe_yyjson_is_ctn(val) : false;
}

yyjson_api_inline yyjson_type yyjson_get_type(const yyjson_val *val) {
  return val ? unsafe_yyjson_get_type(val) : YYJSON_TYPE_NONE;
}

yyjson_api_inline yyjson_subtype yyjson_get_subtype(const yyjson_val *val) {
  return val ? unsafe_yyjson_get_subtype(val) : YYJSON_SUBTYPE_NONE;
}

yyjson_api_inline uint8_t yyjson_get_tag(const yyjson_val *val) {
  return val ? unsafe_yyjson_get_tag(val) : 0;
}

yyjson_api_inline const char *yyjson_get_type_desc(const yyjson_val *val) {
  switch (yyjson_get_tag(val)) {
    case YYJSON_TYPE_RAW | YYJSON_SUBTYPE_NONE:
      return "raw";
    case YYJSON_TYPE_NULL | YYJSON_SUBTYPE_NONE:
      return "null";
    case YYJSON_TYPE_STR | YYJSON_SUBTYPE_NONE:
      return "string";
    case YYJSON_TYPE_STR | YYJSON_SUBTYPE_NOESC:
      return "string";
    case YYJSON_TYPE_ARR | YYJSON_SUBTYPE_NONE:
      return "array";
    case YYJSON_TYPE_OBJ | YYJSON_SUBTYPE_NONE:
      return "object";
    case YYJSON_TYPE_BOOL | YYJSON_SUBTYPE_TRUE:
      return "true";
    case YYJSON_TYPE_BOOL | YYJSON_SUBTYPE_FALSE:
      return "false";
    case YYJSON_TYPE_NUM | YYJSON_SUBTYPE_UINT:
      return "uint";
    case YYJSON_TYPE_NUM | YYJSON_SUBTYPE_SINT:
      return "sint";
    case YYJSON_TYPE_NUM | YYJSON_SUBTYPE_REAL:
      return "real";
    default:
      return "unknown";
  }
}

yyjson_api_inline const char *yyjson_get_raw(const yyjson_val *val) {
  return yyjson_is_raw(val) ? unsafe_yyjson_get_raw(val) : NULL;
}

yyjson_api_inline bool yyjson_get_bool(const yyjson_val *val) {
  return yyjson_is_bool(val) ? unsafe_yyjson_get_bool(val) : false;
}

yyjson_api_inline uint64_t yyjson_get_uint(const yyjson_val *val) {
  return yyjson_is_int(val) ? unsafe_yyjson_get_uint(val) : 0;
}

yyjson_api_inline int64_t yyjson_get_sint(const yyjson_val *val) {
  return yyjson_is_int(val) ? unsafe_yyjson_get_sint(val) : 0;
}

yyjson_api_inline int yyjson_get_int(const yyjson_val *val) {
  return yyjson_is_int(val) ? unsafe_yyjson_get_int(val) : 0;
}

yyjson_api_inline double yyjson_get_real(const yyjson_val *val) {
  return yyjson_is_real(val) ? unsafe_yyjson_get_real(val) : 0.0;
}

yyjson_api_inline double yyjson_get_num(const yyjson_val *val) {
  return val ? unsafe_yyjson_get_num(val) : 0.0;
}

yyjson_api_inline const char *yyjson_get_str(const yyjson_val *val) {
  return yyjson_is_str(val) ? unsafe_yyjson_get_str(val) : NULL;
}

yyjson_api_inline size_t yyjson_get_len(const yyjson_val *val) {
  return val ? unsafe_yyjson_get_len(val) : 0;
}

yyjson_api_inline bool yyjson_equals_str(const yyjson_val *val,
                                         const char *str) {
  if (yyjson_likely(val && str)) {
    return unsafe_yyjson_is_str(val) && unsafe_yyjson_equals_str(val, str);
  }
  return false;
}

yyjson_api_inline bool yyjson_equals_strn(const yyjson_val *val,
                                          const char *str, size_t len) {
  if (yyjson_likely(val && str)) {
    return unsafe_yyjson_is_str(val) &&
           unsafe_yyjson_equals_strn(val, str, len);
  }
  return false;
}

yyjson_api bool unsafe_yyjson_equals(const yyjson_val *lhs,
                                     const yyjson_val *rhs);

yyjson_api_inline bool yyjson_equals(const yyjson_val *lhs,
                                     const yyjson_val *rhs) {
  if (yyjson_unlikely(!lhs || !rhs)) return false;
  return unsafe_yyjson_equals(lhs, rhs);
}

yyjson_api_inline bool yyjson_set_raw(yyjson_val *val, const char *raw,
                                      size_t len) {
  if (yyjson_unlikely(!val || unsafe_yyjson_is_ctn(val))) return false;
  unsafe_yyjson_set_raw(val, raw, len);
  return true;
}

yyjson_api_inline bool yyjson_set_null(yyjson_val *val) {
  if (yyjson_unlikely(!val || unsafe_yyjson_is_ctn(val))) return false;
  unsafe_yyjson_set_null(val);
  return true;
}

yyjson_api_inline bool yyjson_set_bool(yyjson_val *val, bool num) {
  if (yyjson_unlikely(!val || unsafe_yyjson_is_ctn(val))) return false;
  unsafe_yyjson_set_bool(val, num);
  return true;
}

yyjson_api_inline bool yyjson_set_uint(yyjson_val *val, uint64_t num) {
  if (yyjson_unlikely(!val || unsafe_yyjson_is_ctn(val))) return false;
  unsafe_yyjson_set_uint(val, num);
  return true;
}

yyjson_api_inline bool yyjson_set_sint(yyjson_val *val, int64_t num) {
  if (yyjson_unlikely(!val || unsafe_yyjson_is_ctn(val))) return false;
  unsafe_yyjson_set_sint(val, num);
  return true;
}

yyjson_api_inline bool yyjson_set_int(yyjson_val *val, int64_t num) {
  if (yyjson_unlikely(!val || unsafe_yyjson_is_ctn(val))) return false;
  unsafe_yyjson_set_sint(val, num);
  return true;
}

yyjson_api_inline bool yyjson_set_float(yyjson_val *val, float num) {
  if (yyjson_unlikely(!val || unsafe_yyjson_is_ctn(val))) return false;
  unsafe_yyjson_set_float(val, num);
  return true;
}

yyjson_api_inline bool yyjson_set_double(yyjson_val *val, double num) {
  if (yyjson_unlikely(!val || unsafe_yyjson_is_ctn(val))) return false;
  unsafe_yyjson_set_double(val, num);
  return true;
}

yyjson_api_inline bool yyjson_set_real(yyjson_val *val, double num) {
  if (yyjson_unlikely(!val || unsafe_yyjson_is_ctn(val))) return false;
  unsafe_yyjson_set_real(val, num);
  return true;
}

yyjson_api_inline bool yyjson_set_fp_to_fixed(yyjson_val *val, int prec) {
  if (yyjson_unlikely(!yyjson_is_real(val))) return false;
  unsafe_yyjson_set_fp_to_fixed(val, prec);
  return true;
}

yyjson_api_inline bool yyjson_set_fp_to_float(yyjson_val *val, bool flt) {
  if (yyjson_unlikely(!yyjson_is_real(val))) return false;
  unsafe_yyjson_set_fp_to_float(val, flt);
  return true;
}

yyjson_api_inline bool yyjson_set_str(yyjson_val *val, const char *str) {
  if (yyjson_unlikely(!val || unsafe_yyjson_is_ctn(val))) return false;
  if (yyjson_unlikely(!str)) return false;
  unsafe_yyjson_set_str(val, str);
  return true;
}

yyjson_api_inline bool yyjson_set_strn(yyjson_val *val, const char *str,
                                       size_t len) {
  if (yyjson_unlikely(!val || unsafe_yyjson_is_ctn(val))) return false;
  if (yyjson_unlikely(!str)) return false;
  unsafe_yyjson_set_strn(val, str, len);
  return true;
}

yyjson_api_inline bool yyjson_set_str_noesc(yyjson_val *val, bool noesc) {
  if (yyjson_unlikely(!yyjson_is_str(val))) return false;
  unsafe_yyjson_set_str_noesc(val, noesc);
  return true;
}

yyjson_api_inline size_t yyjson_arr_size(const yyjson_val *arr) {
  return yyjson_is_arr(arr) ? unsafe_yyjson_get_len(arr) : 0;
}

yyjson_api_inline yyjson_val *yyjson_arr_get(const yyjson_val *arr,
                                             size_t idx) {
  if (yyjson_likely(yyjson_is_arr(arr))) {
    if (yyjson_likely(unsafe_yyjson_get_len(arr) > idx)) {
      yyjson_val *val = unsafe_yyjson_get_first(arr);
      if (unsafe_yyjson_arr_is_flat(arr)) {
        return val + idx;
      } else {
        while (idx-- > 0) val = unsafe_yyjson_get_next(val);
        return val;
      }
    }
  }
  return NULL;
}

yyjson_api_inline yyjson_val *yyjson_arr_get_first(const yyjson_val *arr) {
  if (yyjson_likely(yyjson_is_arr(arr))) {
    if (yyjson_likely(unsafe_yyjson_get_len(arr) > 0)) {
      return unsafe_yyjson_get_first(arr);
    }
  }
  return NULL;
}

yyjson_api_inline yyjson_val *yyjson_arr_get_last(const yyjson_val *arr) {
  if (yyjson_likely(yyjson_is_arr(arr))) {
    size_t len = unsafe_yyjson_get_len(arr);
    if (yyjson_likely(len > 0)) {
      yyjson_val *val = unsafe_yyjson_get_first(arr);
      if (unsafe_yyjson_arr_is_flat(arr)) {
        return val + (len - 1);
      } else {
        while (len-- > 1) val = unsafe_yyjson_get_next(val);
        return val;
      }
    }
  }
  return NULL;
}

yyjson_api_inline bool yyjson_arr_iter_init(const yyjson_val *arr,
                                            yyjson_arr_iter *iter) {
  if (yyjson_likely(yyjson_is_arr(arr) && iter)) {
    iter->idx = 0;
    iter->max = unsafe_yyjson_get_len(arr);
    iter->cur = unsafe_yyjson_get_first(arr);
    return true;
  }
  if (iter) memset(iter, 0, sizeof(yyjson_arr_iter));
  return false;
}

yyjson_api_inline yyjson_arr_iter yyjson_arr_iter_with(const yyjson_val *arr) {
  yyjson_arr_iter iter;
  yyjson_arr_iter_init(arr, &iter);
  return iter;
}

yyjson_api_inline bool yyjson_arr_iter_has_next(yyjson_arr_iter *iter) {
  return iter ? iter->idx < iter->max : false;
}

yyjson_api_inline yyjson_val *yyjson_arr_iter_next(yyjson_arr_iter *iter) {
  yyjson_val *val;
  if (iter && iter->idx < iter->max) {
    val = iter->cur;
    iter->cur = unsafe_yyjson_get_next(val);
    iter->idx++;
    return val;
  }
  return NULL;
}

yyjson_api_inline size_t yyjson_obj_size(const yyjson_val *obj) {
  return yyjson_is_obj(obj) ? unsafe_yyjson_get_len(obj) : 0;
}

yyjson_api_inline yyjson_val *yyjson_obj_get(const yyjson_val *obj,
                                             const char *key) {
  return yyjson_obj_getn(obj, key, key ? strlen(key) : 0);
}

yyjson_api_inline yyjson_val *yyjson_obj_getn(const yyjson_val *obj,
                                              const char *_key,
                                              size_t key_len) {
  if (yyjson_likely(yyjson_is_obj(obj) && _key)) {
    size_t len = unsafe_yyjson_get_len(obj);
    yyjson_val *key = unsafe_yyjson_get_first(obj);
    while (len-- > 0) {
      if (unsafe_yyjson_equals_strn(key, _key, key_len)) return key + 1;
      key = unsafe_yyjson_get_next(key + 1);
    }
  }
  return NULL;
}

yyjson_api_inline bool yyjson_obj_iter_init(const yyjson_val *obj,
                                            yyjson_obj_iter *iter) {
  if (yyjson_likely(yyjson_is_obj(obj) && iter)) {
    iter->idx = 0;
    iter->max = unsafe_yyjson_get_len(obj);
    iter->cur = unsafe_yyjson_get_first(obj);
    iter->obj = yyjson_constcast(yyjson_val *) obj;
    return true;
  }
  if (iter) memset(iter, 0, sizeof(yyjson_obj_iter));
  return false;
}

yyjson_api_inline yyjson_obj_iter yyjson_obj_iter_with(const yyjson_val *obj) {
  yyjson_obj_iter iter;
  yyjson_obj_iter_init(obj, &iter);
  return iter;
}

yyjson_api_inline bool yyjson_obj_iter_has_next(yyjson_obj_iter *iter) {
  return iter ? iter->idx < iter->max : false;
}

yyjson_api_inline yyjson_val *yyjson_obj_iter_next(yyjson_obj_iter *iter) {
  if (iter && iter->idx < iter->max) {
    yyjson_val *key = iter->cur;
    iter->idx++;
    iter->cur = unsafe_yyjson_get_next(key + 1);
    return key;
  }
  return NULL;
}

yyjson_api_inline yyjson_val *yyjson_obj_iter_get_val(yyjson_val *key) {
  return key ? key + 1 : NULL;
}

yyjson_api_inline yyjson_val *yyjson_obj_iter_get(yyjson_obj_iter *iter,
                                                  const char *key) {
  return yyjson_obj_iter_getn(iter, key, key ? strlen(key) : 0);
}

yyjson_api_inline yyjson_val *yyjson_obj_iter_getn(yyjson_obj_iter *iter,
                                                   const char *key,
                                                   size_t key_len) {
  if (iter && key) {
    size_t idx = iter->idx;
    size_t max = iter->max;
    yyjson_val *cur = iter->cur;
    if (yyjson_unlikely(idx == max)) {
      idx = 0;
      cur = unsafe_yyjson_get_first(iter->obj);
    }
    while (idx++ < max) {
      yyjson_val *next = unsafe_yyjson_get_next(cur + 1);
      if (unsafe_yyjson_equals_strn(cur, key, key_len)) {
        iter->idx = idx;
        iter->cur = next;
        return cur + 1;
      }
      cur = next;
      if (idx == iter->max && iter->idx < iter->max) {
        idx = 0;
        max = iter->idx;
        cur = unsafe_yyjson_get_first(iter->obj);
      }
    }
  }
  return NULL;
}

struct yyjson_mut_val {
  uint64_t tag;
  yyjson_val_uni uni;
  yyjson_mut_val *next;
};

typedef struct yyjson_str_chunk {
  struct yyjson_str_chunk *next;
  size_t chunk_size;

} yyjson_str_chunk;

typedef struct yyjson_str_pool {
  char *cur;
  char *end;
  size_t chunk_size;
  size_t chunk_size_max;
  yyjson_str_chunk *chunks;
} yyjson_str_pool;

typedef struct yyjson_val_chunk {
  struct yyjson_val_chunk *next;
  size_t chunk_size;

} yyjson_val_chunk;

typedef struct yyjson_val_pool {
  yyjson_mut_val *cur;
  yyjson_mut_val *end;
  size_t chunk_size;
  size_t chunk_size_max;
  yyjson_val_chunk *chunks;
} yyjson_val_pool;

struct yyjson_mut_doc {
  yyjson_mut_val *root;
  yyjson_alc alc;
  yyjson_str_pool str_pool;
  yyjson_val_pool val_pool;
};

yyjson_api bool unsafe_yyjson_str_pool_grow(yyjson_str_pool *pool,
                                            const yyjson_alc *alc, size_t len);

yyjson_api bool unsafe_yyjson_val_pool_grow(yyjson_val_pool *pool,
                                            const yyjson_alc *alc,
                                            size_t count);

yyjson_api_inline char *unsafe_yyjson_mut_str_alc(yyjson_mut_doc *doc,
                                                  size_t len) {
  char *mem;
  const yyjson_alc *alc = &doc->alc;
  yyjson_str_pool *pool = &doc->str_pool;
  if (yyjson_unlikely((size_t)(pool->end - pool->cur) <= len)) {
    if (yyjson_unlikely(!unsafe_yyjson_str_pool_grow(pool, alc, len + 1))) {
      return NULL;
    }
  }
  mem = pool->cur;
  pool->cur = mem + len + 1;
  return mem;
}

yyjson_api_inline char *unsafe_yyjson_mut_strncpy(yyjson_mut_doc *doc,
                                                  const char *str, size_t len) {
  char *mem = unsafe_yyjson_mut_str_alc(doc, len);
  if (yyjson_unlikely(!mem)) return NULL;
  memcpy((void *)mem, (const void *)str, len);
  mem[len] = '\0';
  return mem;
}

yyjson_api_inline yyjson_mut_val *unsafe_yyjson_mut_val(yyjson_mut_doc *doc,
                                                        size_t count) {
  yyjson_mut_val *val;
  yyjson_alc *alc = &doc->alc;
  yyjson_val_pool *pool = &doc->val_pool;
  if (yyjson_unlikely((size_t)(pool->end - pool->cur) < count)) {
    if (yyjson_unlikely(!unsafe_yyjson_val_pool_grow(pool, alc, count))) {
      return NULL;
    }
  }
  val = pool->cur;
  pool->cur += count;
  return val;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_get_root(yyjson_mut_doc *doc) {
  return doc ? doc->root : NULL;
}

yyjson_api_inline void yyjson_mut_doc_set_root(yyjson_mut_doc *doc,
                                               yyjson_mut_val *root) {
  if (doc) doc->root = root;
}

yyjson_api_inline bool yyjson_mut_is_raw(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_raw(val) : false;
}

yyjson_api_inline bool yyjson_mut_is_null(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_null(val) : false;
}

yyjson_api_inline bool yyjson_mut_is_true(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_true(val) : false;
}

yyjson_api_inline bool yyjson_mut_is_false(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_false(val) : false;
}

yyjson_api_inline bool yyjson_mut_is_bool(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_bool(val) : false;
}

yyjson_api_inline bool yyjson_mut_is_uint(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_uint(val) : false;
}

yyjson_api_inline bool yyjson_mut_is_sint(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_sint(val) : false;
}

yyjson_api_inline bool yyjson_mut_is_int(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_int(val) : false;
}

yyjson_api_inline bool yyjson_mut_is_real(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_real(val) : false;
}

yyjson_api_inline bool yyjson_mut_is_num(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_num(val) : false;
}

yyjson_api_inline bool yyjson_mut_is_str(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_str(val) : false;
}

yyjson_api_inline bool yyjson_mut_is_arr(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_arr(val) : false;
}

yyjson_api_inline bool yyjson_mut_is_obj(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_obj(val) : false;
}

yyjson_api_inline bool yyjson_mut_is_ctn(const yyjson_mut_val *val) {
  return val ? unsafe_yyjson_is_ctn(val) : false;
}

yyjson_api_inline yyjson_type yyjson_mut_get_type(const yyjson_mut_val *val) {
  return yyjson_get_type((const yyjson_val *)val);
}

yyjson_api_inline yyjson_subtype
yyjson_mut_get_subtype(const yyjson_mut_val *val) {
  return yyjson_get_subtype((const yyjson_val *)val);
}

yyjson_api_inline uint8_t yyjson_mut_get_tag(const yyjson_mut_val *val) {
  return yyjson_get_tag((const yyjson_val *)val);
}

yyjson_api_inline const char *yyjson_mut_get_type_desc(
    const yyjson_mut_val *val) {
  return yyjson_get_type_desc((const yyjson_val *)val);
}

yyjson_api_inline const char *yyjson_mut_get_raw(const yyjson_mut_val *val) {
  return yyjson_get_raw((const yyjson_val *)val);
}

yyjson_api_inline bool yyjson_mut_get_bool(const yyjson_mut_val *val) {
  return yyjson_get_bool((const yyjson_val *)val);
}

yyjson_api_inline uint64_t yyjson_mut_get_uint(const yyjson_mut_val *val) {
  return yyjson_get_uint((const yyjson_val *)val);
}

yyjson_api_inline int64_t yyjson_mut_get_sint(const yyjson_mut_val *val) {
  return yyjson_get_sint((const yyjson_val *)val);
}

yyjson_api_inline int yyjson_mut_get_int(const yyjson_mut_val *val) {
  return yyjson_get_int((const yyjson_val *)val);
}

yyjson_api_inline double yyjson_mut_get_real(const yyjson_mut_val *val) {
  return yyjson_get_real((const yyjson_val *)val);
}

yyjson_api_inline double yyjson_mut_get_num(const yyjson_mut_val *val) {
  return yyjson_get_num((const yyjson_val *)val);
}

yyjson_api_inline const char *yyjson_mut_get_str(const yyjson_mut_val *val) {
  return yyjson_get_str((const yyjson_val *)val);
}

yyjson_api_inline size_t yyjson_mut_get_len(const yyjson_mut_val *val) {
  return yyjson_get_len((const yyjson_val *)val);
}

yyjson_api_inline bool yyjson_mut_equals_str(const yyjson_mut_val *val,
                                             const char *str) {
  return yyjson_equals_str((const yyjson_val *)val, str);
}

yyjson_api_inline bool yyjson_mut_equals_strn(const yyjson_mut_val *val,
                                              const char *str, size_t len) {
  return yyjson_equals_strn((const yyjson_val *)val, str, len);
}

yyjson_api bool unsafe_yyjson_mut_equals(const yyjson_mut_val *lhs,
                                         const yyjson_mut_val *rhs);

yyjson_api_inline bool yyjson_mut_equals(const yyjson_mut_val *lhs,
                                         const yyjson_mut_val *rhs) {
  if (yyjson_unlikely(!lhs || !rhs)) return false;
  return unsafe_yyjson_mut_equals(lhs, rhs);
}

yyjson_api_inline bool yyjson_mut_set_raw(yyjson_mut_val *val, const char *raw,
                                          size_t len) {
  if (yyjson_unlikely(!val || !raw)) return false;
  unsafe_yyjson_set_raw(val, raw, len);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_null(yyjson_mut_val *val) {
  if (yyjson_unlikely(!val)) return false;
  unsafe_yyjson_set_null(val);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_bool(yyjson_mut_val *val, bool num) {
  if (yyjson_unlikely(!val)) return false;
  unsafe_yyjson_set_bool(val, num);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_uint(yyjson_mut_val *val, uint64_t num) {
  if (yyjson_unlikely(!val)) return false;
  unsafe_yyjson_set_uint(val, num);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_sint(yyjson_mut_val *val, int64_t num) {
  if (yyjson_unlikely(!val)) return false;
  unsafe_yyjson_set_sint(val, num);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_int(yyjson_mut_val *val, int64_t num) {
  if (yyjson_unlikely(!val)) return false;
  unsafe_yyjson_set_sint(val, num);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_float(yyjson_mut_val *val, float num) {
  if (yyjson_unlikely(!val)) return false;
  unsafe_yyjson_set_float(val, num);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_double(yyjson_mut_val *val, double num) {
  if (yyjson_unlikely(!val)) return false;
  unsafe_yyjson_set_double(val, num);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_real(yyjson_mut_val *val, double num) {
  if (yyjson_unlikely(!val)) return false;
  unsafe_yyjson_set_real(val, num);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_fp_to_fixed(yyjson_mut_val *val,
                                                  int prec) {
  if (yyjson_unlikely(!yyjson_mut_is_real(val))) return false;
  unsafe_yyjson_set_fp_to_fixed(val, prec);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_fp_to_float(yyjson_mut_val *val,
                                                  bool flt) {
  if (yyjson_unlikely(!yyjson_mut_is_real(val))) return false;
  unsafe_yyjson_set_fp_to_float(val, flt);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_str(yyjson_mut_val *val,
                                          const char *str) {
  if (yyjson_unlikely(!val || !str)) return false;
  unsafe_yyjson_set_str(val, str);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_strn(yyjson_mut_val *val, const char *str,
                                           size_t len) {
  if (yyjson_unlikely(!val || !str)) return false;
  unsafe_yyjson_set_strn(val, str, len);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_str_noesc(yyjson_mut_val *val,
                                                bool noesc) {
  if (yyjson_unlikely(!yyjson_mut_is_str(val))) return false;
  unsafe_yyjson_set_str_noesc(val, noesc);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_arr(yyjson_mut_val *val) {
  if (yyjson_unlikely(!val)) return false;
  unsafe_yyjson_set_arr(val, 0);
  return true;
}

yyjson_api_inline bool yyjson_mut_set_obj(yyjson_mut_val *val) {
  if (yyjson_unlikely(!val)) return false;
  unsafe_yyjson_set_obj(val, 0);
  return true;
}

#define yyjson_mut_val_one(func)                         \
  if (yyjson_likely(doc)) {                              \
    yyjson_mut_val *val = unsafe_yyjson_mut_val(doc, 1); \
    if (yyjson_likely(val)) {                            \
      func return val;                                   \
    }                                                    \
  }                                                      \
  return NULL

#define yyjson_mut_val_one_str(func)                     \
  if (yyjson_likely(doc && str)) {                       \
    yyjson_mut_val *val = unsafe_yyjson_mut_val(doc, 1); \
    if (yyjson_likely(val)) {                            \
      func return val;                                   \
    }                                                    \
  }                                                      \
  return NULL

yyjson_api_inline yyjson_mut_val *yyjson_mut_raw(yyjson_mut_doc *doc,
                                                 const char *str) {
  yyjson_mut_val_one_str({ unsafe_yyjson_set_raw(val, str, strlen(str)); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_rawn(yyjson_mut_doc *doc,
                                                  const char *str, size_t len) {
  yyjson_mut_val_one_str({ unsafe_yyjson_set_raw(val, str, len); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_rawcpy(yyjson_mut_doc *doc,
                                                    const char *str) {
  yyjson_mut_val_one_str({
    size_t len = strlen(str);
    char *new_str = unsafe_yyjson_mut_strncpy(doc, str, len);
    if (yyjson_unlikely(!new_str)) return NULL;
    unsafe_yyjson_set_raw(val, new_str, len);
  });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_rawncpy(yyjson_mut_doc *doc,
                                                     const char *str,
                                                     size_t len) {
  yyjson_mut_val_one_str({
    char *new_str = unsafe_yyjson_mut_strncpy(doc, str, len);
    if (yyjson_unlikely(!new_str)) return NULL;
    unsafe_yyjson_set_raw(val, new_str, len);
  });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_null(yyjson_mut_doc *doc) {
  yyjson_mut_val_one({ unsafe_yyjson_set_null(val); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_true(yyjson_mut_doc *doc) {
  yyjson_mut_val_one({ unsafe_yyjson_set_bool(val, true); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_false(yyjson_mut_doc *doc) {
  yyjson_mut_val_one({ unsafe_yyjson_set_bool(val, false); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_bool(yyjson_mut_doc *doc,
                                                  bool _val) {
  yyjson_mut_val_one({ unsafe_yyjson_set_bool(val, _val); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_uint(yyjson_mut_doc *doc,
                                                  uint64_t num) {
  yyjson_mut_val_one({ unsafe_yyjson_set_uint(val, num); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_sint(yyjson_mut_doc *doc,
                                                  int64_t num) {
  yyjson_mut_val_one({ unsafe_yyjson_set_sint(val, num); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_int(yyjson_mut_doc *doc,
                                                 int64_t num) {
  yyjson_mut_val_one({ unsafe_yyjson_set_sint(val, num); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_float(yyjson_mut_doc *doc,
                                                   float num) {
  yyjson_mut_val_one({ unsafe_yyjson_set_float(val, num); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_double(yyjson_mut_doc *doc,
                                                    double num) {
  yyjson_mut_val_one({ unsafe_yyjson_set_double(val, num); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_real(yyjson_mut_doc *doc,
                                                  double num) {
  yyjson_mut_val_one({ unsafe_yyjson_set_real(val, num); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_str(yyjson_mut_doc *doc,
                                                 const char *str) {
  yyjson_mut_val_one_str({ unsafe_yyjson_set_str(val, str); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_strn(yyjson_mut_doc *doc,
                                                  const char *str, size_t len) {
  yyjson_mut_val_one_str({ unsafe_yyjson_set_strn(val, str, len); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_strcpy(yyjson_mut_doc *doc,
                                                    const char *str) {
  yyjson_mut_val_one_str({
    size_t len = strlen(str);
    bool noesc = unsafe_yyjson_is_str_noesc(str, len);
    yyjson_subtype sub = noesc ? YYJSON_SUBTYPE_NOESC : YYJSON_SUBTYPE_NONE;
    char *new_str = unsafe_yyjson_mut_strncpy(doc, str, len);
    if (yyjson_unlikely(!new_str)) return NULL;
    unsafe_yyjson_set_tag(val, YYJSON_TYPE_STR, sub, len);
    val->uni.str = new_str;
  });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_strncpy(yyjson_mut_doc *doc,
                                                     const char *str,
                                                     size_t len) {
  yyjson_mut_val_one_str({
    char *new_str = unsafe_yyjson_mut_strncpy(doc, str, len);
    if (yyjson_unlikely(!new_str)) return NULL;
    unsafe_yyjson_set_strn(val, new_str, len);
  });
}

#undef yyjson_mut_val_one
#undef yyjson_mut_val_one_str

yyjson_api_inline size_t yyjson_mut_arr_size(const yyjson_mut_val *arr) {
  return yyjson_mut_is_arr(arr) ? unsafe_yyjson_get_len(arr) : 0;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_get(const yyjson_mut_val *arr,
                                                     size_t idx) {
  if (yyjson_likely(idx < yyjson_mut_arr_size(arr))) {
    yyjson_mut_val *val = (yyjson_mut_val *)arr->uni.ptr;
    while (idx-- > 0) val = val->next;
    return val->next;
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_get_first(
    const yyjson_mut_val *arr) {
  if (yyjson_likely(yyjson_mut_arr_size(arr) > 0)) {
    return ((yyjson_mut_val *)arr->uni.ptr)->next;
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_get_last(
    const yyjson_mut_val *arr) {
  if (yyjson_likely(yyjson_mut_arr_size(arr) > 0)) {
    return ((yyjson_mut_val *)arr->uni.ptr);
  }
  return NULL;
}

yyjson_api_inline bool yyjson_mut_arr_iter_init(yyjson_mut_val *arr,
                                                yyjson_mut_arr_iter *iter) {
  if (yyjson_likely(yyjson_mut_is_arr(arr) && iter)) {
    iter->idx = 0;
    iter->max = unsafe_yyjson_get_len(arr);
    iter->cur = iter->max ? (yyjson_mut_val *)arr->uni.ptr : NULL;
    iter->pre = NULL;
    iter->arr = arr;
    return true;
  }
  if (iter) memset(iter, 0, sizeof(yyjson_mut_arr_iter));
  return false;
}

yyjson_api_inline yyjson_mut_arr_iter
yyjson_mut_arr_iter_with(yyjson_mut_val *arr) {
  yyjson_mut_arr_iter iter;
  yyjson_mut_arr_iter_init(arr, &iter);
  return iter;
}

yyjson_api_inline bool yyjson_mut_arr_iter_has_next(yyjson_mut_arr_iter *iter) {
  return iter ? iter->idx < iter->max : false;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_iter_next(
    yyjson_mut_arr_iter *iter) {
  if (iter && iter->idx < iter->max) {
    yyjson_mut_val *val = iter->cur;
    iter->pre = val;
    iter->cur = val->next;
    iter->idx++;
    return iter->cur;
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_iter_remove(
    yyjson_mut_arr_iter *iter) {
  if (yyjson_likely(iter && 0 < iter->idx && iter->idx <= iter->max)) {
    yyjson_mut_val *prev = iter->pre;
    yyjson_mut_val *cur = iter->cur;
    yyjson_mut_val *next = cur->next;
    if (yyjson_unlikely(iter->idx == iter->max)) iter->arr->uni.ptr = prev;
    iter->idx--;
    iter->max--;
    unsafe_yyjson_set_len(iter->arr, iter->max);
    prev->next = next;
    iter->cur = prev;
    return cur;
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr(yyjson_mut_doc *doc) {
  if (yyjson_likely(doc)) {
    yyjson_mut_val *val = unsafe_yyjson_mut_val(doc, 1);
    if (yyjson_likely(val)) {
      val->tag = YYJSON_TYPE_ARR | YYJSON_SUBTYPE_NONE;
      return val;
    }
  }
  return NULL;
}

#define yyjson_mut_arr_with_func(func)                                       \
  if (yyjson_likely(                                                         \
          doc && ((0 < count &&                                              \
                   count < (~(size_t)0) / sizeof(yyjson_mut_val) && vals) || \
                  count == 0))) {                                            \
    yyjson_mut_val *arr = unsafe_yyjson_mut_val(doc, 1 + count);             \
    if (yyjson_likely(arr)) {                                                \
      arr->tag = ((uint64_t)count << YYJSON_TAG_BIT) | YYJSON_TYPE_ARR;      \
      if (count > 0) {                                                       \
        size_t i;                                                            \
        for (i = 0; i < count; i++) {                                        \
          yyjson_mut_val *val = arr + i + 1;                                 \
          func val->next = val + 1;                                          \
        }                                                                    \
        arr[count].next = arr + 1;                                           \
        arr->uni.ptr = arr + count;                                          \
      }                                                                      \
      return arr;                                                            \
    }                                                                        \
  }                                                                          \
  return NULL

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_bool(yyjson_mut_doc *doc,
                                                           const bool *vals,
                                                           size_t count) {
  yyjson_mut_arr_with_func({ unsafe_yyjson_set_bool(val, vals[i]); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_sint(yyjson_mut_doc *doc,
                                                           const int64_t *vals,
                                                           size_t count) {
  return yyjson_mut_arr_with_sint64(doc, vals, count);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_uint(yyjson_mut_doc *doc,
                                                           const uint64_t *vals,
                                                           size_t count) {
  return yyjson_mut_arr_with_uint64(doc, vals, count);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_real(yyjson_mut_doc *doc,
                                                           const double *vals,
                                                           size_t count) {
  yyjson_mut_arr_with_func({ unsafe_yyjson_set_real(val, vals[i]); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_sint8(yyjson_mut_doc *doc,
                                                            const int8_t *vals,
                                                            size_t count) {
  yyjson_mut_arr_with_func({ unsafe_yyjson_set_sint(val, vals[i]); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_sint16(
    yyjson_mut_doc *doc, const int16_t *vals, size_t count) {
  yyjson_mut_arr_with_func({ unsafe_yyjson_set_sint(val, vals[i]); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_sint32(
    yyjson_mut_doc *doc, const int32_t *vals, size_t count) {
  yyjson_mut_arr_with_func({ unsafe_yyjson_set_sint(val, vals[i]); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_sint64(
    yyjson_mut_doc *doc, const int64_t *vals, size_t count) {
  yyjson_mut_arr_with_func({ unsafe_yyjson_set_sint(val, vals[i]); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_uint8(yyjson_mut_doc *doc,
                                                            const uint8_t *vals,
                                                            size_t count) {
  yyjson_mut_arr_with_func({ unsafe_yyjson_set_uint(val, vals[i]); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_uint16(
    yyjson_mut_doc *doc, const uint16_t *vals, size_t count) {
  yyjson_mut_arr_with_func({ unsafe_yyjson_set_uint(val, vals[i]); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_uint32(
    yyjson_mut_doc *doc, const uint32_t *vals, size_t count) {
  yyjson_mut_arr_with_func({ unsafe_yyjson_set_uint(val, vals[i]); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_uint64(
    yyjson_mut_doc *doc, const uint64_t *vals, size_t count) {
  yyjson_mut_arr_with_func({ unsafe_yyjson_set_uint(val, vals[i]); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_float(yyjson_mut_doc *doc,
                                                            const float *vals,
                                                            size_t count) {
  yyjson_mut_arr_with_func({ unsafe_yyjson_set_float(val, vals[i]); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_double(
    yyjson_mut_doc *doc, const double *vals, size_t count) {
  yyjson_mut_arr_with_func({ unsafe_yyjson_set_double(val, vals[i]); });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_str(yyjson_mut_doc *doc,
                                                          const char **vals,
                                                          size_t count) {
  yyjson_mut_arr_with_func({
    if (yyjson_unlikely(!vals[i])) return NULL;
    unsafe_yyjson_set_str(val, vals[i]);
  });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_strn(yyjson_mut_doc *doc,
                                                           const char **vals,
                                                           const size_t *lens,
                                                           size_t count) {
  if (yyjson_unlikely(count > 0 && !lens)) return NULL;
  yyjson_mut_arr_with_func({
    if (yyjson_unlikely(!vals[i])) return NULL;
    unsafe_yyjson_set_strn(val, vals[i], lens[i]);
  });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_strcpy(
    yyjson_mut_doc *doc, const char **vals, size_t count) {
  size_t len;
  const char *str, *new_str;
  yyjson_mut_arr_with_func({
    str = vals[i];
    if (yyjson_unlikely(!str)) return NULL;
    len = strlen(str);
    new_str = unsafe_yyjson_mut_strncpy(doc, str, len);
    if (yyjson_unlikely(!new_str)) return NULL;
    unsafe_yyjson_set_strn(val, new_str, len);
  });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_with_strncpy(
    yyjson_mut_doc *doc, const char **vals, const size_t *lens, size_t count) {
  size_t len;
  const char *str, *new_str;
  if (yyjson_unlikely(count > 0 && !lens)) return NULL;
  yyjson_mut_arr_with_func({
    str = vals[i];
    if (yyjson_unlikely(!str)) return NULL;
    len = lens[i];
    new_str = unsafe_yyjson_mut_strncpy(doc, str, len);
    if (yyjson_unlikely(!new_str)) return NULL;
    unsafe_yyjson_set_strn(val, new_str, len);
  });
}

#undef yyjson_mut_arr_with_func

yyjson_api_inline bool yyjson_mut_arr_insert(yyjson_mut_val *arr,
                                             yyjson_mut_val *val, size_t idx) {
  if (yyjson_likely(yyjson_mut_is_arr(arr) && val)) {
    size_t len = unsafe_yyjson_get_len(arr);
    if (yyjson_likely(idx <= len)) {
      unsafe_yyjson_set_len(arr, len + 1);
      if (len == 0) {
        val->next = val;
        arr->uni.ptr = val;
      } else {
        yyjson_mut_val *prev = ((yyjson_mut_val *)arr->uni.ptr);
        yyjson_mut_val *next = prev->next;
        if (idx == len) {
          prev->next = val;
          val->next = next;
          arr->uni.ptr = val;
        } else {
          while (idx-- > 0) {
            prev = next;
            next = next->next;
          }
          prev->next = val;
          val->next = next;
        }
      }
      return true;
    }
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_append(yyjson_mut_val *arr,
                                             yyjson_mut_val *val) {
  if (yyjson_likely(yyjson_mut_is_arr(arr) && val)) {
    size_t len = unsafe_yyjson_get_len(arr);
    unsafe_yyjson_set_len(arr, len + 1);
    if (len == 0) {
      val->next = val;
    } else {
      yyjson_mut_val *prev = ((yyjson_mut_val *)arr->uni.ptr);
      yyjson_mut_val *next = prev->next;
      prev->next = val;
      val->next = next;
    }
    arr->uni.ptr = val;
    return true;
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_prepend(yyjson_mut_val *arr,
                                              yyjson_mut_val *val) {
  if (yyjson_likely(yyjson_mut_is_arr(arr) && val)) {
    size_t len = unsafe_yyjson_get_len(arr);
    unsafe_yyjson_set_len(arr, len + 1);
    if (len == 0) {
      val->next = val;
      arr->uni.ptr = val;
    } else {
      yyjson_mut_val *prev = ((yyjson_mut_val *)arr->uni.ptr);
      yyjson_mut_val *next = prev->next;
      prev->next = val;
      val->next = next;
    }
    return true;
  }
  return false;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_replace(yyjson_mut_val *arr,
                                                         size_t idx,
                                                         yyjson_mut_val *val) {
  if (yyjson_likely(yyjson_mut_is_arr(arr) && val)) {
    size_t len = unsafe_yyjson_get_len(arr);
    if (yyjson_likely(idx < len)) {
      if (yyjson_likely(len > 1)) {
        yyjson_mut_val *prev = ((yyjson_mut_val *)arr->uni.ptr);
        yyjson_mut_val *next = prev->next;
        while (idx-- > 0) {
          prev = next;
          next = next->next;
        }
        prev->next = val;
        val->next = next->next;
        if ((void *)next == arr->uni.ptr) arr->uni.ptr = val;
        return next;
      } else {
        yyjson_mut_val *prev = ((yyjson_mut_val *)arr->uni.ptr);
        val->next = val;
        arr->uni.ptr = val;
        return prev;
      }
    }
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_remove(yyjson_mut_val *arr,
                                                        size_t idx) {
  if (yyjson_likely(yyjson_mut_is_arr(arr))) {
    size_t len = unsafe_yyjson_get_len(arr);
    if (yyjson_likely(idx < len)) {
      unsafe_yyjson_set_len(arr, len - 1);
      if (yyjson_likely(len > 1)) {
        yyjson_mut_val *prev = ((yyjson_mut_val *)arr->uni.ptr);
        yyjson_mut_val *next = prev->next;
        while (idx-- > 0) {
          prev = next;
          next = next->next;
        }
        prev->next = next->next;
        if ((void *)next == arr->uni.ptr) arr->uni.ptr = prev;
        return next;
      } else {
        return ((yyjson_mut_val *)arr->uni.ptr);
      }
    }
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_remove_first(
    yyjson_mut_val *arr) {
  if (yyjson_likely(yyjson_mut_is_arr(arr))) {
    size_t len = unsafe_yyjson_get_len(arr);
    if (len > 1) {
      yyjson_mut_val *prev = ((yyjson_mut_val *)arr->uni.ptr);
      yyjson_mut_val *next = prev->next;
      prev->next = next->next;
      unsafe_yyjson_set_len(arr, len - 1);
      return next;
    } else if (len == 1) {
      yyjson_mut_val *prev = ((yyjson_mut_val *)arr->uni.ptr);
      unsafe_yyjson_set_len(arr, 0);
      return prev;
    }
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_remove_last(
    yyjson_mut_val *arr) {
  if (yyjson_likely(yyjson_mut_is_arr(arr))) {
    size_t len = unsafe_yyjson_get_len(arr);
    if (yyjson_likely(len > 1)) {
      yyjson_mut_val *prev = ((yyjson_mut_val *)arr->uni.ptr);
      yyjson_mut_val *next = prev->next;
      unsafe_yyjson_set_len(arr, len - 1);
      while (--len > 0) prev = prev->next;
      prev->next = next;
      next = (yyjson_mut_val *)arr->uni.ptr;
      arr->uni.ptr = prev;
      return next;
    } else if (len == 1) {
      yyjson_mut_val *prev = ((yyjson_mut_val *)arr->uni.ptr);
      unsafe_yyjson_set_len(arr, 0);
      return prev;
    }
  }
  return NULL;
}

yyjson_api_inline bool yyjson_mut_arr_remove_range(yyjson_mut_val *arr,
                                                   size_t _idx, size_t _len) {
  if (yyjson_likely(yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *prev, *next;
    bool tail_removed;
    size_t len = unsafe_yyjson_get_len(arr);
    if (yyjson_unlikely(_idx + _len > len)) return false;
    if (yyjson_unlikely(_len == 0)) return true;
    unsafe_yyjson_set_len(arr, len - _len);
    if (yyjson_unlikely(len == _len)) return true;
    tail_removed = (_idx + _len == len);
    prev = ((yyjson_mut_val *)arr->uni.ptr);
    while (_idx-- > 0) prev = prev->next;
    next = prev->next;
    while (_len-- > 0) next = next->next;
    prev->next = next;
    if (yyjson_unlikely(tail_removed)) arr->uni.ptr = prev;
    return true;
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_clear(yyjson_mut_val *arr) {
  if (yyjson_likely(yyjson_mut_is_arr(arr))) {
    unsafe_yyjson_set_len(arr, 0);
    return true;
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_rotate(yyjson_mut_val *arr, size_t idx) {
  if (yyjson_likely(yyjson_mut_is_arr(arr) &&
                    unsafe_yyjson_get_len(arr) > idx)) {
    yyjson_mut_val *val = (yyjson_mut_val *)arr->uni.ptr;
    while (idx-- > 0) val = val->next;
    arr->uni.ptr = (void *)val;
    return true;
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_val(yyjson_mut_val *arr,
                                              yyjson_mut_val *val) {
  return yyjson_mut_arr_append(arr, val);
}

yyjson_api_inline bool yyjson_mut_arr_add_null(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_null(doc);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_true(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_true(doc);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_false(yyjson_mut_doc *doc,
                                                yyjson_mut_val *arr) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_false(doc);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_bool(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr, bool _val) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_bool(doc, _val);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_uint(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr,
                                               uint64_t num) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_uint(doc, num);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_sint(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr,
                                               int64_t num) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_sint(doc, num);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_int(yyjson_mut_doc *doc,
                                              yyjson_mut_val *arr,
                                              int64_t num) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_sint(doc, num);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_float(yyjson_mut_doc *doc,
                                                yyjson_mut_val *arr,
                                                float num) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_float(doc, num);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_double(yyjson_mut_doc *doc,
                                                 yyjson_mut_val *arr,
                                                 double num) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_double(doc, num);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_real(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr,
                                               double num) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_real(doc, num);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_str(yyjson_mut_doc *doc,
                                              yyjson_mut_val *arr,
                                              const char *str) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_str(doc, str);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_strn(yyjson_mut_doc *doc,
                                               yyjson_mut_val *arr,
                                               const char *str, size_t len) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_strn(doc, str, len);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_strcpy(yyjson_mut_doc *doc,
                                                 yyjson_mut_val *arr,
                                                 const char *str) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_strcpy(doc, str);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_arr_add_strncpy(yyjson_mut_doc *doc,
                                                  yyjson_mut_val *arr,
                                                  const char *str, size_t len) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_strncpy(doc, str, len);
    return yyjson_mut_arr_append(arr, val);
  }
  return false;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_add_arr(yyjson_mut_doc *doc,
                                                         yyjson_mut_val *arr) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_arr(doc);
    return yyjson_mut_arr_append(arr, val) ? val : NULL;
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_arr_add_obj(yyjson_mut_doc *doc,
                                                         yyjson_mut_val *arr) {
  if (yyjson_likely(doc && yyjson_mut_is_arr(arr))) {
    yyjson_mut_val *val = yyjson_mut_obj(doc);
    return yyjson_mut_arr_append(arr, val) ? val : NULL;
  }
  return NULL;
}

yyjson_api_inline size_t yyjson_mut_obj_size(const yyjson_mut_val *obj) {
  return yyjson_mut_is_obj(obj) ? unsafe_yyjson_get_len(obj) : 0;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_get(const yyjson_mut_val *obj,
                                                     const char *key) {
  return yyjson_mut_obj_getn(obj, key, key ? strlen(key) : 0);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_getn(const yyjson_mut_val *obj,
                                                      const char *_key,
                                                      size_t key_len) {
  size_t len = yyjson_mut_obj_size(obj);
  if (yyjson_likely(len && _key)) {
    yyjson_mut_val *key = ((yyjson_mut_val *)obj->uni.ptr)->next->next;
    while (len-- > 0) {
      if (unsafe_yyjson_equals_strn(key, _key, key_len)) return key->next;
      key = key->next->next;
    }
  }
  return NULL;
}

yyjson_api_inline bool yyjson_mut_obj_iter_init(yyjson_mut_val *obj,
                                                yyjson_mut_obj_iter *iter) {
  if (yyjson_likely(yyjson_mut_is_obj(obj) && iter)) {
    iter->idx = 0;
    iter->max = unsafe_yyjson_get_len(obj);
    iter->cur = iter->max ? (yyjson_mut_val *)obj->uni.ptr : NULL;
    iter->pre = NULL;
    iter->obj = obj;
    return true;
  }
  if (iter) memset(iter, 0, sizeof(yyjson_mut_obj_iter));
  return false;
}

yyjson_api_inline yyjson_mut_obj_iter
yyjson_mut_obj_iter_with(yyjson_mut_val *obj) {
  yyjson_mut_obj_iter iter;
  yyjson_mut_obj_iter_init(obj, &iter);
  return iter;
}

yyjson_api_inline bool yyjson_mut_obj_iter_has_next(yyjson_mut_obj_iter *iter) {
  return iter ? iter->idx < iter->max : false;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_iter_next(
    yyjson_mut_obj_iter *iter) {
  if (iter && iter->idx < iter->max) {
    yyjson_mut_val *key = iter->cur;
    iter->pre = key;
    iter->cur = key->next->next;
    iter->idx++;
    return iter->cur;
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_iter_get_val(
    yyjson_mut_val *key) {
  return key ? key->next : NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_iter_remove(
    yyjson_mut_obj_iter *iter) {
  if (yyjson_likely(iter && 0 < iter->idx && iter->idx <= iter->max)) {
    yyjson_mut_val *prev = iter->pre;
    yyjson_mut_val *cur = iter->cur;
    yyjson_mut_val *next = cur->next->next;
    if (yyjson_unlikely(iter->idx == iter->max)) iter->obj->uni.ptr = prev;
    iter->idx--;
    iter->max--;
    unsafe_yyjson_set_len(iter->obj, iter->max);
    prev->next->next = next;
    iter->cur = prev;
    return cur->next;
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_iter_get(
    yyjson_mut_obj_iter *iter, const char *key) {
  return yyjson_mut_obj_iter_getn(iter, key, key ? strlen(key) : 0);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_iter_getn(
    yyjson_mut_obj_iter *iter, const char *key, size_t key_len) {
  if (iter && key) {
    size_t idx = 0;
    size_t max = iter->max;
    yyjson_mut_val *pre, *cur = iter->cur;
    while (idx++ < max) {
      pre = cur;
      cur = cur->next->next;
      if (unsafe_yyjson_equals_strn(cur, key, key_len)) {
        iter->idx += idx;
        if (iter->idx > max) iter->idx -= max + 1;
        iter->pre = pre;
        iter->cur = cur;
        return cur->next;
      }
    }
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj(yyjson_mut_doc *doc) {
  if (yyjson_likely(doc)) {
    yyjson_mut_val *val = unsafe_yyjson_mut_val(doc, 1);
    if (yyjson_likely(val)) {
      val->tag = YYJSON_TYPE_OBJ | YYJSON_SUBTYPE_NONE;
      return val;
    }
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_with_str(yyjson_mut_doc *doc,
                                                          const char **keys,
                                                          const char **vals,
                                                          size_t count) {
  if (yyjson_likely(doc && ((count > 0 && keys && vals) || (count == 0)))) {
    yyjson_mut_val *obj = unsafe_yyjson_mut_val(doc, 1 + count * 2);
    if (yyjson_likely(obj)) {
      obj->tag = ((uint64_t)count << YYJSON_TAG_BIT) | YYJSON_TYPE_OBJ;
      if (count > 0) {
        size_t i;
        for (i = 0; i < count; i++) {
          yyjson_mut_val *key = obj + (i * 2 + 1);
          yyjson_mut_val *val = obj + (i * 2 + 2);
          uint64_t key_len = (uint64_t)strlen(keys[i]);
          uint64_t val_len = (uint64_t)strlen(vals[i]);
          key->tag = (key_len << YYJSON_TAG_BIT) | YYJSON_TYPE_STR;
          val->tag = (val_len << YYJSON_TAG_BIT) | YYJSON_TYPE_STR;
          key->uni.str = keys[i];
          val->uni.str = vals[i];
          key->next = val;
          val->next = val + 1;
        }
        obj[count * 2].next = obj + 1;
        obj->uni.ptr = obj + (count * 2 - 1);
      }
      return obj;
    }
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_with_kv(yyjson_mut_doc *doc,
                                                         const char **pairs,
                                                         size_t count) {
  if (yyjson_likely(doc && ((count > 0 && pairs) || (count == 0)))) {
    yyjson_mut_val *obj = unsafe_yyjson_mut_val(doc, 1 + count * 2);
    if (yyjson_likely(obj)) {
      obj->tag = ((uint64_t)count << YYJSON_TAG_BIT) | YYJSON_TYPE_OBJ;
      if (count > 0) {
        size_t i;
        for (i = 0; i < count; i++) {
          yyjson_mut_val *key = obj + (i * 2 + 1);
          yyjson_mut_val *val = obj + (i * 2 + 2);
          const char *key_str = pairs[i * 2 + 0];
          const char *val_str = pairs[i * 2 + 1];
          uint64_t key_len = (uint64_t)strlen(key_str);
          uint64_t val_len = (uint64_t)strlen(val_str);
          key->tag = (key_len << YYJSON_TAG_BIT) | YYJSON_TYPE_STR;
          val->tag = (val_len << YYJSON_TAG_BIT) | YYJSON_TYPE_STR;
          key->uni.str = key_str;
          val->uni.str = val_str;
          key->next = val;
          val->next = val + 1;
        }
        obj[count * 2].next = obj + 1;
        obj->uni.ptr = obj + (count * 2 - 1);
      }
      return obj;
    }
  }
  return NULL;
}

yyjson_api_inline void unsafe_yyjson_mut_obj_add(yyjson_mut_val *obj,
                                                 yyjson_mut_val *key,
                                                 yyjson_mut_val *val,
                                                 size_t len) {
  if (yyjson_likely(len)) {
    yyjson_mut_val *prev_val = ((yyjson_mut_val *)obj->uni.ptr)->next;
    yyjson_mut_val *next_key = prev_val->next;
    prev_val->next = key;
    val->next = next_key;
  } else {
    val->next = key;
  }
  key->next = val;
  obj->uni.ptr = (void *)key;
  unsafe_yyjson_set_len(obj, len + 1);
}

yyjson_api_inline yyjson_mut_val *unsafe_yyjson_mut_obj_remove(
    yyjson_mut_val *obj, const char *key, size_t key_len) {
  size_t obj_len = unsafe_yyjson_get_len(obj);
  if (obj_len) {
    yyjson_mut_val *pre_key = (yyjson_mut_val *)obj->uni.ptr;
    yyjson_mut_val *cur_key = pre_key->next->next;
    yyjson_mut_val *removed_item = NULL;
    size_t i;
    for (i = 0; i < obj_len; i++) {
      if (unsafe_yyjson_equals_strn(cur_key, key, key_len)) {
        if (!removed_item) removed_item = cur_key->next;
        cur_key = cur_key->next->next;
        pre_key->next->next = cur_key;
        if (i + 1 == obj_len) obj->uni.ptr = pre_key;
        i--;
        obj_len--;
      } else {
        pre_key = cur_key;
        cur_key = cur_key->next->next;
      }
    }
    unsafe_yyjson_set_len(obj, obj_len);
    return removed_item;
  } else {
    return NULL;
  }
}

yyjson_api_inline bool unsafe_yyjson_mut_obj_replace(yyjson_mut_val *obj,
                                                     yyjson_mut_val *key,
                                                     yyjson_mut_val *val) {
  size_t key_len = unsafe_yyjson_get_len(key);
  size_t obj_len = unsafe_yyjson_get_len(obj);
  if (obj_len) {
    yyjson_mut_val *pre_key = (yyjson_mut_val *)obj->uni.ptr;
    yyjson_mut_val *cur_key = pre_key->next->next;
    size_t i;
    for (i = 0; i < obj_len; i++) {
      if (unsafe_yyjson_equals_strn(cur_key, key->uni.str, key_len)) {
        cur_key->next->tag = val->tag;
        cur_key->next->uni.u64 = val->uni.u64;
        return true;
      } else {
        cur_key = cur_key->next->next;
      }
    }
  }
  return false;
}

yyjson_api_inline void unsafe_yyjson_mut_obj_rotate(yyjson_mut_val *obj,
                                                    size_t idx) {
  yyjson_mut_val *key = (yyjson_mut_val *)obj->uni.ptr;
  while (idx-- > 0) key = key->next->next;
  obj->uni.ptr = (void *)key;
}

yyjson_api_inline bool yyjson_mut_obj_add(yyjson_mut_val *obj,
                                          yyjson_mut_val *key,
                                          yyjson_mut_val *val) {
  if (yyjson_likely(yyjson_mut_is_obj(obj) && yyjson_mut_is_str(key) && val)) {
    unsafe_yyjson_mut_obj_add(obj, key, val, unsafe_yyjson_get_len(obj));
    return true;
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_obj_put(yyjson_mut_val *obj,
                                          yyjson_mut_val *key,
                                          yyjson_mut_val *val) {
  bool replaced = false;
  size_t key_len;
  yyjson_mut_obj_iter iter;
  yyjson_mut_val *cur_key;
  if (yyjson_unlikely(!yyjson_mut_is_obj(obj) || !yyjson_mut_is_str(key)))
    return false;
  key_len = unsafe_yyjson_get_len(key);
  yyjson_mut_obj_iter_init(obj, &iter);
  while ((cur_key = yyjson_mut_obj_iter_next(&iter)) != 0) {
    if (unsafe_yyjson_equals_strn(cur_key, key->uni.str, key_len)) {
      if (!replaced && val) {
        replaced = true;
        val->next = cur_key->next->next;
        cur_key->next = val;
      } else {
        yyjson_mut_obj_iter_remove(&iter);
      }
    }
  }
  if (!replaced && val) unsafe_yyjson_mut_obj_add(obj, key, val, iter.max);
  return true;
}

yyjson_api_inline bool yyjson_mut_obj_insert(yyjson_mut_val *obj,
                                             yyjson_mut_val *key,
                                             yyjson_mut_val *val, size_t idx) {
  if (yyjson_likely(yyjson_mut_is_obj(obj) && yyjson_mut_is_str(key) && val)) {
    size_t len = unsafe_yyjson_get_len(obj);
    if (yyjson_likely(len >= idx)) {
      if (len > idx) {
        void *ptr = obj->uni.ptr;
        unsafe_yyjson_mut_obj_rotate(obj, idx);
        unsafe_yyjson_mut_obj_add(obj, key, val, len);
        obj->uni.ptr = ptr;
      } else {
        unsafe_yyjson_mut_obj_add(obj, key, val, len);
      }
      return true;
    }
  }
  return false;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_remove(yyjson_mut_val *obj,
                                                        yyjson_mut_val *key) {
  if (yyjson_likely(yyjson_mut_is_obj(obj) && yyjson_mut_is_str(key))) {
    return unsafe_yyjson_mut_obj_remove(obj, key->uni.str,
                                        unsafe_yyjson_get_len(key));
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_remove_key(yyjson_mut_val *obj,
                                                            const char *key) {
  if (yyjson_likely(yyjson_mut_is_obj(obj) && key)) {
    size_t key_len = strlen(key);
    return unsafe_yyjson_mut_obj_remove(obj, key, key_len);
  }
  return NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_remove_keyn(
    yyjson_mut_val *obj, const char *key, size_t key_len) {
  if (yyjson_likely(yyjson_mut_is_obj(obj) && key)) {
    return unsafe_yyjson_mut_obj_remove(obj, key, key_len);
  }
  return NULL;
}

yyjson_api_inline bool yyjson_mut_obj_clear(yyjson_mut_val *obj) {
  if (yyjson_likely(yyjson_mut_is_obj(obj))) {
    unsafe_yyjson_set_len(obj, 0);
    return true;
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_obj_replace(yyjson_mut_val *obj,
                                              yyjson_mut_val *key,
                                              yyjson_mut_val *val) {
  if (yyjson_likely(yyjson_mut_is_obj(obj) && yyjson_mut_is_str(key) && val)) {
    return unsafe_yyjson_mut_obj_replace(obj, key, val);
  }
  return false;
}

yyjson_api_inline bool yyjson_mut_obj_rotate(yyjson_mut_val *obj, size_t idx) {
  if (yyjson_likely(yyjson_mut_is_obj(obj) &&
                    unsafe_yyjson_get_len(obj) > idx)) {
    unsafe_yyjson_mut_obj_rotate(obj, idx);
    return true;
  }
  return false;
}

#define yyjson_mut_obj_add_func(func)                                 \
  if (yyjson_likely(doc && yyjson_mut_is_obj(obj) && _key)) {         \
    yyjson_mut_val *key = unsafe_yyjson_mut_val(doc, 2);              \
    if (yyjson_likely(key)) {                                         \
      size_t len = unsafe_yyjson_get_len(obj);                        \
      yyjson_mut_val *val = key + 1;                                  \
      size_t key_len = strlen(_key);                                  \
      bool noesc = unsafe_yyjson_is_str_noesc(_key, key_len);         \
      key->tag = YYJSON_TYPE_STR;                                     \
      key->tag |= noesc ? YYJSON_SUBTYPE_NOESC : YYJSON_SUBTYPE_NONE; \
      key->tag |= (uint64_t)strlen(_key) << YYJSON_TAG_BIT;           \
      key->uni.str = _key;                                            \
      func unsafe_yyjson_mut_obj_add(obj, key, val, len);             \
      return true;                                                    \
    }                                                                 \
  }                                                                   \
  return false

yyjson_api_inline bool yyjson_mut_obj_add_null(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *_key) {
  yyjson_mut_obj_add_func({ unsafe_yyjson_set_null(val); });
}

yyjson_api_inline bool yyjson_mut_obj_add_true(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *_key) {
  yyjson_mut_obj_add_func({ unsafe_yyjson_set_bool(val, true); });
}

yyjson_api_inline bool yyjson_mut_obj_add_false(yyjson_mut_doc *doc,
                                                yyjson_mut_val *obj,
                                                const char *_key) {
  yyjson_mut_obj_add_func({ unsafe_yyjson_set_bool(val, false); });
}

yyjson_api_inline bool yyjson_mut_obj_add_bool(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *_key, bool _val) {
  yyjson_mut_obj_add_func({ unsafe_yyjson_set_bool(val, _val); });
}

yyjson_api_inline bool yyjson_mut_obj_add_uint(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *_key,
                                               uint64_t _val) {
  yyjson_mut_obj_add_func({ unsafe_yyjson_set_uint(val, _val); });
}

yyjson_api_inline bool yyjson_mut_obj_add_sint(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *_key, int64_t _val) {
  yyjson_mut_obj_add_func({ unsafe_yyjson_set_sint(val, _val); });
}

yyjson_api_inline bool yyjson_mut_obj_add_int(yyjson_mut_doc *doc,
                                              yyjson_mut_val *obj,
                                              const char *_key, int64_t _val) {
  yyjson_mut_obj_add_func({ unsafe_yyjson_set_sint(val, _val); });
}

yyjson_api_inline bool yyjson_mut_obj_add_float(yyjson_mut_doc *doc,
                                                yyjson_mut_val *obj,
                                                const char *_key, float _val) {
  yyjson_mut_obj_add_func({ unsafe_yyjson_set_float(val, _val); });
}

yyjson_api_inline bool yyjson_mut_obj_add_double(yyjson_mut_doc *doc,
                                                 yyjson_mut_val *obj,
                                                 const char *_key,
                                                 double _val) {
  yyjson_mut_obj_add_func({ unsafe_yyjson_set_double(val, _val); });
}

yyjson_api_inline bool yyjson_mut_obj_add_real(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *_key, double _val) {
  yyjson_mut_obj_add_func({ unsafe_yyjson_set_real(val, _val); });
}

yyjson_api_inline bool yyjson_mut_obj_add_str(yyjson_mut_doc *doc,
                                              yyjson_mut_val *obj,
                                              const char *_key,
                                              const char *_val) {
  if (yyjson_unlikely(!_val)) return false;
  yyjson_mut_obj_add_func({
    size_t val_len = strlen(_val);
    bool val_noesc = unsafe_yyjson_is_str_noesc(_val, val_len);
    val->tag = ((uint64_t)strlen(_val) << YYJSON_TAG_BIT) | YYJSON_TYPE_STR;
    val->tag |= val_noesc ? YYJSON_SUBTYPE_NOESC : YYJSON_SUBTYPE_NONE;
    val->uni.str = _val;
  });
}

yyjson_api_inline bool yyjson_mut_obj_add_strn(yyjson_mut_doc *doc,
                                               yyjson_mut_val *obj,
                                               const char *_key,
                                               const char *_val, size_t _len) {
  if (yyjson_unlikely(!_val)) return false;
  yyjson_mut_obj_add_func({
    val->tag = ((uint64_t)_len << YYJSON_TAG_BIT) | YYJSON_TYPE_STR;
    val->uni.str = _val;
  });
}

yyjson_api_inline bool yyjson_mut_obj_add_strcpy(yyjson_mut_doc *doc,
                                                 yyjson_mut_val *obj,
                                                 const char *_key,
                                                 const char *_val) {
  if (yyjson_unlikely(!_val)) return false;
  yyjson_mut_obj_add_func({
    size_t _len = strlen(_val);
    val->uni.str = unsafe_yyjson_mut_strncpy(doc, _val, _len);
    if (yyjson_unlikely(!val->uni.str)) return false;
    val->tag = ((uint64_t)_len << YYJSON_TAG_BIT) | YYJSON_TYPE_STR;
  });
}

yyjson_api_inline bool yyjson_mut_obj_add_strncpy(yyjson_mut_doc *doc,
                                                  yyjson_mut_val *obj,
                                                  const char *_key,
                                                  const char *_val,
                                                  size_t _len) {
  if (yyjson_unlikely(!_val)) return false;
  yyjson_mut_obj_add_func({
    val->uni.str = unsafe_yyjson_mut_strncpy(doc, _val, _len);
    if (yyjson_unlikely(!val->uni.str)) return false;
    val->tag = ((uint64_t)_len << YYJSON_TAG_BIT) | YYJSON_TYPE_STR;
  });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_add_arr(yyjson_mut_doc *doc,
                                                         yyjson_mut_val *obj,
                                                         const char *_key) {
  yyjson_mut_val *key = yyjson_mut_str(doc, _key);
  yyjson_mut_val *val = yyjson_mut_arr(doc);
  return yyjson_mut_obj_add(obj, key, val) ? val : NULL;
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_add_obj(yyjson_mut_doc *doc,
                                                         yyjson_mut_val *obj,
                                                         const char *_key) {
  yyjson_mut_val *key = yyjson_mut_str(doc, _key);
  yyjson_mut_val *val = yyjson_mut_obj(doc);
  return yyjson_mut_obj_add(obj, key, val) ? val : NULL;
}

yyjson_api_inline bool yyjson_mut_obj_add_val(yyjson_mut_doc *doc,
                                              yyjson_mut_val *obj,
                                              const char *_key,
                                              yyjson_mut_val *_val) {
  if (yyjson_unlikely(!_val)) return false;
  yyjson_mut_obj_add_func({ val = _val; });
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_remove_str(yyjson_mut_val *obj,
                                                            const char *key) {
  return yyjson_mut_obj_remove_strn(obj, key, key ? strlen(key) : 0);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_obj_remove_strn(
    yyjson_mut_val *obj, const char *_key, size_t _len) {
  if (yyjson_likely(yyjson_mut_is_obj(obj) && _key)) {
    yyjson_mut_val *key;
    yyjson_mut_obj_iter iter;
    yyjson_mut_val *val_removed = NULL;
    yyjson_mut_obj_iter_init(obj, &iter);
    while ((key = yyjson_mut_obj_iter_next(&iter)) != NULL) {
      if (unsafe_yyjson_equals_strn(key, _key, _len)) {
        if (!val_removed) val_removed = key->next;
        yyjson_mut_obj_iter_remove(&iter);
      }
    }
    return val_removed;
  }
  return NULL;
}

yyjson_api_inline bool yyjson_mut_obj_rename_key(yyjson_mut_doc *doc,
                                                 yyjson_mut_val *obj,
                                                 const char *key,
                                                 const char *new_key) {
  if (!key || !new_key) return false;
  return yyjson_mut_obj_rename_keyn(doc, obj, key, strlen(key), new_key,
                                    strlen(new_key));
}

yyjson_api_inline bool yyjson_mut_obj_rename_keyn(yyjson_mut_doc *doc,
                                                  yyjson_mut_val *obj,
                                                  const char *key, size_t len,
                                                  const char *new_key,
                                                  size_t new_len) {
  char *cpy_key = NULL;
  yyjson_mut_val *old_key;
  yyjson_mut_obj_iter iter;
  if (!doc || !obj || !key || !new_key) return false;
  yyjson_mut_obj_iter_init(obj, &iter);
  while ((old_key = yyjson_mut_obj_iter_next(&iter))) {
    if (unsafe_yyjson_equals_strn((void *)old_key, key, len)) {
      if (!cpy_key) {
        cpy_key = unsafe_yyjson_mut_strncpy(doc, new_key, new_len);
        if (!cpy_key) return false;
      }
      yyjson_mut_set_strn(old_key, cpy_key, new_len);
    }
  }
  return cpy_key != NULL;
}

#if !defined(YYJSON_DISABLE_UTILS) || !YYJSON_DISABLE_UTILS

#define yyjson_ptr_set_err(_code, _msg)   \
  do {                                    \
    if (err) {                            \
      err->code = YYJSON_PTR_ERR_##_code; \
      err->msg = _msg;                    \
      err->pos = 0;                       \
    }                                     \
  } while (false)

yyjson_api yyjson_val *unsafe_yyjson_ptr_getx(const yyjson_val *val,
                                              const char *ptr, size_t len,
                                              yyjson_ptr_err *err);

yyjson_api yyjson_mut_val *unsafe_yyjson_mut_ptr_getx(const yyjson_mut_val *val,
                                                      const char *ptr,
                                                      size_t len,
                                                      yyjson_ptr_ctx *ctx,
                                                      yyjson_ptr_err *err);

yyjson_api bool unsafe_yyjson_mut_ptr_putx(yyjson_mut_val *val, const char *ptr,
                                           size_t len, yyjson_mut_val *new_val,
                                           yyjson_mut_doc *doc,
                                           bool create_parent, bool insert_new,
                                           yyjson_ptr_ctx *ctx,
                                           yyjson_ptr_err *err);

yyjson_api yyjson_mut_val *unsafe_yyjson_mut_ptr_replacex(
    yyjson_mut_val *val, const char *ptr, size_t len, yyjson_mut_val *new_val,
    yyjson_ptr_ctx *ctx, yyjson_ptr_err *err);

yyjson_api yyjson_mut_val *unsafe_yyjson_mut_ptr_removex(yyjson_mut_val *val,
                                                         const char *ptr,
                                                         size_t len,
                                                         yyjson_ptr_ctx *ctx,
                                                         yyjson_ptr_err *err);

yyjson_api_inline yyjson_val *yyjson_doc_ptr_get(const yyjson_doc *doc,
                                                 const char *ptr) {
  if (yyjson_unlikely(!ptr)) return NULL;
  return yyjson_doc_ptr_getn(doc, ptr, strlen(ptr));
}

yyjson_api_inline yyjson_val *yyjson_doc_ptr_getn(const yyjson_doc *doc,
                                                  const char *ptr, size_t len) {
  return yyjson_doc_ptr_getx(doc, ptr, len, NULL);
}

yyjson_api_inline yyjson_val *yyjson_doc_ptr_getx(const yyjson_doc *doc,
                                                  const char *ptr, size_t len,
                                                  yyjson_ptr_err *err) {
  yyjson_ptr_set_err(NONE, NULL);
  if (yyjson_unlikely(!doc || !ptr)) {
    yyjson_ptr_set_err(PARAMETER, "input parameter is NULL");
    return NULL;
  }
  if (yyjson_unlikely(!doc->root)) {
    yyjson_ptr_set_err(NULL_ROOT, "document's root is NULL");
    return NULL;
  }
  if (yyjson_unlikely(len == 0)) {
    return doc->root;
  }
  if (yyjson_unlikely(*ptr != '/')) {
    yyjson_ptr_set_err(SYNTAX, "no prefix '/'");
    return NULL;
  }
  return unsafe_yyjson_ptr_getx(doc->root, ptr, len, err);
}

yyjson_api_inline yyjson_val *yyjson_ptr_get(const yyjson_val *val,
                                             const char *ptr) {
  if (yyjson_unlikely(!ptr)) return NULL;
  return yyjson_ptr_getn(val, ptr, strlen(ptr));
}

yyjson_api_inline yyjson_val *yyjson_ptr_getn(const yyjson_val *val,
                                              const char *ptr, size_t len) {
  return yyjson_ptr_getx(val, ptr, len, NULL);
}

yyjson_api_inline yyjson_val *yyjson_ptr_getx(const yyjson_val *val,
                                              const char *ptr, size_t len,
                                              yyjson_ptr_err *err) {
  yyjson_ptr_set_err(NONE, NULL);
  if (yyjson_unlikely(!val || !ptr)) {
    yyjson_ptr_set_err(PARAMETER, "input parameter is NULL");
    return NULL;
  }
  if (yyjson_unlikely(len == 0)) {
    return yyjson_constcast(yyjson_val *) val;
  }
  if (yyjson_unlikely(*ptr != '/')) {
    yyjson_ptr_set_err(SYNTAX, "no prefix '/'");
    return NULL;
  }
  return unsafe_yyjson_ptr_getx(val, ptr, len, err);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_get(
    const yyjson_mut_doc *doc, const char *ptr) {
  if (!ptr) return NULL;
  return yyjson_mut_doc_ptr_getn(doc, ptr, strlen(ptr));
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_getn(
    const yyjson_mut_doc *doc, const char *ptr, size_t len) {
  return yyjson_mut_doc_ptr_getx(doc, ptr, len, NULL, NULL);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_getx(
    const yyjson_mut_doc *doc, const char *ptr, size_t len, yyjson_ptr_ctx *ctx,
    yyjson_ptr_err *err) {
  yyjson_ptr_set_err(NONE, NULL);
  if (ctx) memset(ctx, 0, sizeof(*ctx));

  if (yyjson_unlikely(!doc || !ptr)) {
    yyjson_ptr_set_err(PARAMETER, "input parameter is NULL");
    return NULL;
  }
  if (yyjson_unlikely(!doc->root)) {
    yyjson_ptr_set_err(NULL_ROOT, "document's root is NULL");
    return NULL;
  }
  if (yyjson_unlikely(len == 0)) {
    return doc->root;
  }
  if (yyjson_unlikely(*ptr != '/')) {
    yyjson_ptr_set_err(SYNTAX, "no prefix '/'");
    return NULL;
  }
  return unsafe_yyjson_mut_ptr_getx(doc->root, ptr, len, ctx, err);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_get(const yyjson_mut_val *val,
                                                     const char *ptr) {
  if (!ptr) return NULL;
  return yyjson_mut_ptr_getn(val, ptr, strlen(ptr));
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_getn(const yyjson_mut_val *val,
                                                      const char *ptr,
                                                      size_t len) {
  return yyjson_mut_ptr_getx(val, ptr, len, NULL, NULL);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_getx(const yyjson_mut_val *val,
                                                      const char *ptr,
                                                      size_t len,
                                                      yyjson_ptr_ctx *ctx,
                                                      yyjson_ptr_err *err) {
  yyjson_ptr_set_err(NONE, NULL);
  if (ctx) memset(ctx, 0, sizeof(*ctx));

  if (yyjson_unlikely(!val || !ptr)) {
    yyjson_ptr_set_err(PARAMETER, "input parameter is NULL");
    return NULL;
  }
  if (yyjson_unlikely(len == 0)) {
    return yyjson_constcast(yyjson_mut_val *) val;
  }
  if (yyjson_unlikely(*ptr != '/')) {
    yyjson_ptr_set_err(SYNTAX, "no prefix '/'");
    return NULL;
  }
  return unsafe_yyjson_mut_ptr_getx(val, ptr, len, ctx, err);
}

yyjson_api_inline bool yyjson_mut_doc_ptr_add(yyjson_mut_doc *doc,
                                              const char *ptr,
                                              yyjson_mut_val *new_val) {
  if (yyjson_unlikely(!ptr)) return false;
  return yyjson_mut_doc_ptr_addn(doc, ptr, strlen(ptr), new_val);
}

yyjson_api_inline bool yyjson_mut_doc_ptr_addn(yyjson_mut_doc *doc,
                                               const char *ptr, size_t len,
                                               yyjson_mut_val *new_val) {
  return yyjson_mut_doc_ptr_addx(doc, ptr, len, new_val, true, NULL, NULL);
}

yyjson_api_inline bool yyjson_mut_doc_ptr_addx(
    yyjson_mut_doc *doc, const char *ptr, size_t len, yyjson_mut_val *new_val,
    bool create_parent, yyjson_ptr_ctx *ctx, yyjson_ptr_err *err) {
  yyjson_ptr_set_err(NONE, NULL);
  if (ctx) memset(ctx, 0, sizeof(*ctx));

  if (yyjson_unlikely(!doc || !ptr || !new_val)) {
    yyjson_ptr_set_err(PARAMETER, "input parameter is NULL");
    return false;
  }
  if (yyjson_unlikely(len == 0)) {
    if (doc->root) {
      yyjson_ptr_set_err(SET_ROOT, "cannot set document's root");
      return false;
    } else {
      doc->root = new_val;
      return true;
    }
  }
  if (yyjson_unlikely(*ptr != '/')) {
    yyjson_ptr_set_err(SYNTAX, "no prefix '/'");
    return false;
  }
  if (yyjson_unlikely(!doc->root && !create_parent)) {
    yyjson_ptr_set_err(NULL_ROOT, "document's root is NULL");
    return false;
  }
  if (yyjson_unlikely(!doc->root)) {
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    if (yyjson_unlikely(!root)) {
      yyjson_ptr_set_err(MEMORY_ALLOCATION, "failed to create value");
      return false;
    }
    if (unsafe_yyjson_mut_ptr_putx(root, ptr, len, new_val, doc, create_parent,
                                   true, ctx, err)) {
      doc->root = root;
      return true;
    }
    return false;
  }
  return unsafe_yyjson_mut_ptr_putx(doc->root, ptr, len, new_val, doc,
                                    create_parent, true, ctx, err);
}

yyjson_api_inline bool yyjson_mut_ptr_add(yyjson_mut_val *val, const char *ptr,
                                          yyjson_mut_val *new_val,
                                          yyjson_mut_doc *doc) {
  if (yyjson_unlikely(!ptr)) return false;
  return yyjson_mut_ptr_addn(val, ptr, strlen(ptr), new_val, doc);
}

yyjson_api_inline bool yyjson_mut_ptr_addn(yyjson_mut_val *val, const char *ptr,
                                           size_t len, yyjson_mut_val *new_val,
                                           yyjson_mut_doc *doc) {
  return yyjson_mut_ptr_addx(val, ptr, len, new_val, doc, true, NULL, NULL);
}

yyjson_api_inline bool yyjson_mut_ptr_addx(yyjson_mut_val *val, const char *ptr,
                                           size_t len, yyjson_mut_val *new_val,
                                           yyjson_mut_doc *doc,
                                           bool create_parent,
                                           yyjson_ptr_ctx *ctx,
                                           yyjson_ptr_err *err) {
  yyjson_ptr_set_err(NONE, NULL);
  if (ctx) memset(ctx, 0, sizeof(*ctx));

  if (yyjson_unlikely(!val || !ptr || !new_val || !doc)) {
    yyjson_ptr_set_err(PARAMETER, "input parameter is NULL");
    return false;
  }
  if (yyjson_unlikely(len == 0)) {
    yyjson_ptr_set_err(SET_ROOT, "cannot set root");
    return false;
  }
  if (yyjson_unlikely(*ptr != '/')) {
    yyjson_ptr_set_err(SYNTAX, "no prefix '/'");
    return false;
  }
  return unsafe_yyjson_mut_ptr_putx(val, ptr, len, new_val, doc, create_parent,
                                    true, ctx, err);
}

yyjson_api_inline bool yyjson_mut_doc_ptr_set(yyjson_mut_doc *doc,
                                              const char *ptr,
                                              yyjson_mut_val *new_val) {
  if (yyjson_unlikely(!ptr)) return false;
  return yyjson_mut_doc_ptr_setn(doc, ptr, strlen(ptr), new_val);
}

yyjson_api_inline bool yyjson_mut_doc_ptr_setn(yyjson_mut_doc *doc,
                                               const char *ptr, size_t len,
                                               yyjson_mut_val *new_val) {
  return yyjson_mut_doc_ptr_setx(doc, ptr, len, new_val, true, NULL, NULL);
}

yyjson_api_inline bool yyjson_mut_doc_ptr_setx(
    yyjson_mut_doc *doc, const char *ptr, size_t len, yyjson_mut_val *new_val,
    bool create_parent, yyjson_ptr_ctx *ctx, yyjson_ptr_err *err) {
  yyjson_ptr_set_err(NONE, NULL);
  if (ctx) memset(ctx, 0, sizeof(*ctx));

  if (yyjson_unlikely(!doc || !ptr)) {
    yyjson_ptr_set_err(PARAMETER, "input parameter is NULL");
    return false;
  }
  if (yyjson_unlikely(len == 0)) {
    if (ctx) ctx->old = doc->root;
    doc->root = new_val;
    return true;
  }
  if (yyjson_unlikely(*ptr != '/')) {
    yyjson_ptr_set_err(SYNTAX, "no prefix '/'");
    return false;
  }
  if (!new_val) {
    if (!doc->root) {
      yyjson_ptr_set_err(RESOLVE, "JSON pointer cannot be resolved");
      return false;
    }
    return !!unsafe_yyjson_mut_ptr_removex(doc->root, ptr, len, ctx, err);
  }
  if (yyjson_unlikely(!doc->root && !create_parent)) {
    yyjson_ptr_set_err(NULL_ROOT, "document's root is NULL");
    return false;
  }
  if (yyjson_unlikely(!doc->root)) {
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    if (yyjson_unlikely(!root)) {
      yyjson_ptr_set_err(MEMORY_ALLOCATION, "failed to create value");
      return false;
    }
    if (unsafe_yyjson_mut_ptr_putx(root, ptr, len, new_val, doc, create_parent,
                                   false, ctx, err)) {
      doc->root = root;
      return true;
    }
    return false;
  }
  return unsafe_yyjson_mut_ptr_putx(doc->root, ptr, len, new_val, doc,
                                    create_parent, false, ctx, err);
}

yyjson_api_inline bool yyjson_mut_ptr_set(yyjson_mut_val *val, const char *ptr,
                                          yyjson_mut_val *new_val,
                                          yyjson_mut_doc *doc) {
  if (yyjson_unlikely(!ptr)) return false;
  return yyjson_mut_ptr_setn(val, ptr, strlen(ptr), new_val, doc);
}

yyjson_api_inline bool yyjson_mut_ptr_setn(yyjson_mut_val *val, const char *ptr,
                                           size_t len, yyjson_mut_val *new_val,
                                           yyjson_mut_doc *doc) {
  return yyjson_mut_ptr_setx(val, ptr, len, new_val, doc, true, NULL, NULL);
}

yyjson_api_inline bool yyjson_mut_ptr_setx(yyjson_mut_val *val, const char *ptr,
                                           size_t len, yyjson_mut_val *new_val,
                                           yyjson_mut_doc *doc,
                                           bool create_parent,
                                           yyjson_ptr_ctx *ctx,
                                           yyjson_ptr_err *err) {
  yyjson_ptr_set_err(NONE, NULL);
  if (ctx) memset(ctx, 0, sizeof(*ctx));

  if (yyjson_unlikely(!val || !ptr || !doc)) {
    yyjson_ptr_set_err(PARAMETER, "input parameter is NULL");
    return false;
  }
  if (yyjson_unlikely(len == 0)) {
    yyjson_ptr_set_err(SET_ROOT, "cannot set root");
    return false;
  }
  if (yyjson_unlikely(*ptr != '/')) {
    yyjson_ptr_set_err(SYNTAX, "no prefix '/'");
    return false;
  }
  if (!new_val) {
    return !!unsafe_yyjson_mut_ptr_removex(val, ptr, len, ctx, err);
  }
  return unsafe_yyjson_mut_ptr_putx(val, ptr, len, new_val, doc, create_parent,
                                    false, ctx, err);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_replace(
    yyjson_mut_doc *doc, const char *ptr, yyjson_mut_val *new_val) {
  if (!ptr) return NULL;
  return yyjson_mut_doc_ptr_replacen(doc, ptr, strlen(ptr), new_val);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_replacen(
    yyjson_mut_doc *doc, const char *ptr, size_t len, yyjson_mut_val *new_val) {
  return yyjson_mut_doc_ptr_replacex(doc, ptr, len, new_val, NULL, NULL);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_replacex(
    yyjson_mut_doc *doc, const char *ptr, size_t len, yyjson_mut_val *new_val,
    yyjson_ptr_ctx *ctx, yyjson_ptr_err *err) {
  yyjson_ptr_set_err(NONE, NULL);
  if (ctx) memset(ctx, 0, sizeof(*ctx));

  if (yyjson_unlikely(!doc || !ptr || !new_val)) {
    yyjson_ptr_set_err(PARAMETER, "input parameter is NULL");
    return NULL;
  }
  if (yyjson_unlikely(len == 0)) {
    yyjson_mut_val *root = doc->root;
    if (yyjson_unlikely(!root)) {
      yyjson_ptr_set_err(RESOLVE, "JSON pointer cannot be resolved");
      return NULL;
    }
    if (ctx) ctx->old = root;
    doc->root = new_val;
    return root;
  }
  if (yyjson_unlikely(!doc->root)) {
    yyjson_ptr_set_err(NULL_ROOT, "document's root is NULL");
    return NULL;
  }
  if (yyjson_unlikely(*ptr != '/')) {
    yyjson_ptr_set_err(SYNTAX, "no prefix '/'");
    return NULL;
  }
  return unsafe_yyjson_mut_ptr_replacex(doc->root, ptr, len, new_val, ctx, err);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_replace(
    yyjson_mut_val *val, const char *ptr, yyjson_mut_val *new_val) {
  if (!ptr) return NULL;
  return yyjson_mut_ptr_replacen(val, ptr, strlen(ptr), new_val);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_replacen(
    yyjson_mut_val *val, const char *ptr, size_t len, yyjson_mut_val *new_val) {
  return yyjson_mut_ptr_replacex(val, ptr, len, new_val, NULL, NULL);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_replacex(
    yyjson_mut_val *val, const char *ptr, size_t len, yyjson_mut_val *new_val,
    yyjson_ptr_ctx *ctx, yyjson_ptr_err *err) {
  yyjson_ptr_set_err(NONE, NULL);
  if (ctx) memset(ctx, 0, sizeof(*ctx));

  if (yyjson_unlikely(!val || !ptr || !new_val)) {
    yyjson_ptr_set_err(PARAMETER, "input parameter is NULL");
    return NULL;
  }
  if (yyjson_unlikely(len == 0)) {
    yyjson_ptr_set_err(SET_ROOT, "cannot set root");
    return NULL;
  }
  if (yyjson_unlikely(*ptr != '/')) {
    yyjson_ptr_set_err(SYNTAX, "no prefix '/'");
    return NULL;
  }
  return unsafe_yyjson_mut_ptr_replacex(val, ptr, len, new_val, ctx, err);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_remove(yyjson_mut_doc *doc,
                                                            const char *ptr) {
  if (!ptr) return NULL;
  return yyjson_mut_doc_ptr_removen(doc, ptr, strlen(ptr));
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_removen(
    yyjson_mut_doc *doc, const char *ptr, size_t len) {
  return yyjson_mut_doc_ptr_removex(doc, ptr, len, NULL, NULL);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_doc_ptr_removex(
    yyjson_mut_doc *doc, const char *ptr, size_t len, yyjson_ptr_ctx *ctx,
    yyjson_ptr_err *err) {
  yyjson_ptr_set_err(NONE, NULL);
  if (ctx) memset(ctx, 0, sizeof(*ctx));

  if (yyjson_unlikely(!doc || !ptr)) {
    yyjson_ptr_set_err(PARAMETER, "input parameter is NULL");
    return NULL;
  }
  if (yyjson_unlikely(!doc->root)) {
    yyjson_ptr_set_err(NULL_ROOT, "document's root is NULL");
    return NULL;
  }
  if (yyjson_unlikely(len == 0)) {
    yyjson_mut_val *root = doc->root;
    if (ctx) ctx->old = root;
    doc->root = NULL;
    return root;
  }
  if (yyjson_unlikely(*ptr != '/')) {
    yyjson_ptr_set_err(SYNTAX, "no prefix '/'");
    return NULL;
  }
  return unsafe_yyjson_mut_ptr_removex(doc->root, ptr, len, ctx, err);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_remove(yyjson_mut_val *val,
                                                        const char *ptr) {
  if (!ptr) return NULL;
  return yyjson_mut_ptr_removen(val, ptr, strlen(ptr));
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_removen(yyjson_mut_val *val,
                                                         const char *ptr,
                                                         size_t len) {
  return yyjson_mut_ptr_removex(val, ptr, len, NULL, NULL);
}

yyjson_api_inline yyjson_mut_val *yyjson_mut_ptr_removex(yyjson_mut_val *val,
                                                         const char *ptr,
                                                         size_t len,
                                                         yyjson_ptr_ctx *ctx,
                                                         yyjson_ptr_err *err) {
  yyjson_ptr_set_err(NONE, NULL);
  if (ctx) memset(ctx, 0, sizeof(*ctx));

  if (yyjson_unlikely(!val || !ptr)) {
    yyjson_ptr_set_err(PARAMETER, "input parameter is NULL");
    return NULL;
  }
  if (yyjson_unlikely(len == 0)) {
    yyjson_ptr_set_err(SET_ROOT, "cannot set root");
    return NULL;
  }
  if (yyjson_unlikely(*ptr != '/')) {
    yyjson_ptr_set_err(SYNTAX, "no prefix '/'");
    return NULL;
  }
  return unsafe_yyjson_mut_ptr_removex(val, ptr, len, ctx, err);
}

yyjson_api_inline bool yyjson_ptr_ctx_append(yyjson_ptr_ctx *ctx,
                                             yyjson_mut_val *key,
                                             yyjson_mut_val *val) {
  yyjson_mut_val *ctn, *pre_key, *pre_val, *cur_key, *cur_val;
  if (!ctx || !ctx->ctn || !val) return false;
  ctn = ctx->ctn;

  if (yyjson_mut_is_obj(ctn)) {
    if (!key) return false;
    key->next = val;
    pre_key = ctx->pre;
    if (unsafe_yyjson_get_len(ctn) == 0) {
      val->next = key;
      ctn->uni.ptr = key;
      ctx->pre = key;
    } else if (!pre_key) {
      pre_key = (yyjson_mut_val *)ctn->uni.ptr;
      pre_val = pre_key->next;
      val->next = pre_val->next;
      pre_val->next = key;
      ctn->uni.ptr = key;
      ctx->pre = pre_key;
    } else {
      cur_key = pre_key->next->next;
      cur_val = cur_key->next;
      val->next = cur_val->next;
      cur_val->next = key;
      if (ctn->uni.ptr == cur_key) ctn->uni.ptr = key;
      ctx->pre = cur_key;
    }
  } else {
    pre_val = ctx->pre;
    if (unsafe_yyjson_get_len(ctn) == 0) {
      val->next = val;
      ctn->uni.ptr = val;
      ctx->pre = val;
    } else if (!pre_val) {
      pre_val = (yyjson_mut_val *)ctn->uni.ptr;
      val->next = pre_val->next;
      pre_val->next = val;
      ctn->uni.ptr = val;
      ctx->pre = pre_val;
    } else {
      cur_val = pre_val->next;
      val->next = cur_val->next;
      cur_val->next = val;
      if (ctn->uni.ptr == cur_val) ctn->uni.ptr = val;
      ctx->pre = cur_val;
    }
  }
  unsafe_yyjson_inc_len(ctn);
  return true;
}

yyjson_api_inline bool yyjson_ptr_ctx_replace(yyjson_ptr_ctx *ctx,
                                              yyjson_mut_val *val) {
  yyjson_mut_val *ctn, *pre_key, *cur_key, *pre_val, *cur_val;
  if (!ctx || !ctx->ctn || !ctx->pre || !val) return false;
  ctn = ctx->ctn;
  if (yyjson_mut_is_obj(ctn)) {
    pre_key = ctx->pre;
    pre_val = pre_key->next;
    cur_key = pre_val->next;
    cur_val = cur_key->next;

    cur_key->next = val;
    val->next = cur_val->next;
    ctx->old = cur_val;
  } else {
    pre_val = ctx->pre;
    cur_val = pre_val->next;

    if (pre_val != cur_val) {
      val->next = cur_val->next;
      pre_val->next = val;
      if (ctn->uni.ptr == cur_val) ctn->uni.ptr = val;
    } else {
      val->next = val;
      ctn->uni.ptr = val;
      ctx->pre = val;
    }
    ctx->old = cur_val;
  }
  return true;
}

yyjson_api_inline bool yyjson_ptr_ctx_remove(yyjson_ptr_ctx *ctx) {
  yyjson_mut_val *ctn, *pre_key, *pre_val, *cur_key, *cur_val;
  size_t len;
  if (!ctx || !ctx->ctn || !ctx->pre) return false;
  ctn = ctx->ctn;
  if (yyjson_mut_is_obj(ctn)) {
    pre_key = ctx->pre;
    pre_val = pre_key->next;
    cur_key = pre_val->next;
    cur_val = cur_key->next;

    pre_val->next = cur_val->next;
    if (ctn->uni.ptr == cur_key) ctn->uni.ptr = pre_key;
    ctx->pre = NULL;
    ctx->old = cur_val;
  } else {
    pre_val = ctx->pre;
    cur_val = pre_val->next;

    pre_val->next = cur_val->next;
    if (ctn->uni.ptr == cur_val) ctn->uni.ptr = pre_val;
    ctx->pre = NULL;
    ctx->old = cur_val;
  }
  len = unsafe_yyjson_get_len(ctn) - 1;
  if (len == 0) ctn->uni.ptr = NULL;
  unsafe_yyjson_set_len(ctn, len);
  return true;
}

#undef yyjson_ptr_set_err

yyjson_api_inline bool yyjson_ptr_get_bool(const yyjson_val *root,
                                           const char *ptr, bool *value) {
  yyjson_val *val = yyjson_ptr_get(root, ptr);
  if (value && yyjson_is_bool(val)) {
    *value = unsafe_yyjson_get_bool(val);
    return true;
  } else {
    return false;
  }
}

yyjson_api_inline bool yyjson_ptr_get_uint(const yyjson_val *root,
                                           const char *ptr, uint64_t *value) {
  yyjson_val *val = yyjson_ptr_get(root, ptr);
  if (value && val) {
    uint64_t ret = val->uni.u64;
    if (unsafe_yyjson_is_uint(val) ||
        (unsafe_yyjson_is_sint(val) && !(ret >> 63))) {
      *value = ret;
      return true;
    }
  }
  return false;
}

yyjson_api_inline bool yyjson_ptr_get_sint(const yyjson_val *root,
                                           const char *ptr, int64_t *value) {
  yyjson_val *val = yyjson_ptr_get(root, ptr);
  if (value && val) {
    int64_t ret = val->uni.i64;
    if (unsafe_yyjson_is_sint(val) ||
        (unsafe_yyjson_is_uint(val) && ret >= 0)) {
      *value = ret;
      return true;
    }
  }
  return false;
}

yyjson_api_inline bool yyjson_ptr_get_real(const yyjson_val *root,
                                           const char *ptr, double *value) {
  yyjson_val *val = yyjson_ptr_get(root, ptr);
  if (value && yyjson_is_real(val)) {
    *value = unsafe_yyjson_get_real(val);
    return true;
  } else {
    return false;
  }
}

yyjson_api_inline bool yyjson_ptr_get_num(const yyjson_val *root,
                                          const char *ptr, double *value) {
  yyjson_val *val = yyjson_ptr_get(root, ptr);
  if (value && yyjson_is_num(val)) {
    *value = unsafe_yyjson_get_num(val);
    return true;
  } else {
    return false;
  }
}

yyjson_api_inline bool yyjson_ptr_get_str(const yyjson_val *root,
                                          const char *ptr, const char **value) {
  yyjson_val *val = yyjson_ptr_get(root, ptr);
  if (value && yyjson_is_str(val)) {
    *value = unsafe_yyjson_get_str(val);
    return true;
  } else {
    return false;
  }
}

yyjson_deprecated("renamed to yyjson_doc_ptr_get") yyjson_api_inline
    yyjson_val *yyjson_doc_get_pointer(yyjson_doc *doc, const char *ptr) {
  return yyjson_doc_ptr_get(doc, ptr);
}

yyjson_deprecated("renamed to yyjson_doc_ptr_getn") yyjson_api_inline
    yyjson_val *yyjson_doc_get_pointern(yyjson_doc *doc, const char *ptr,
                                        size_t len) {
  return yyjson_doc_ptr_getn(doc, ptr, len);
}

yyjson_deprecated("renamed to yyjson_mut_doc_ptr_get") yyjson_api_inline
    yyjson_mut_val *yyjson_mut_doc_get_pointer(yyjson_mut_doc *doc,
                                               const char *ptr) {
  return yyjson_mut_doc_ptr_get(doc, ptr);
}

yyjson_deprecated("renamed to yyjson_mut_doc_ptr_getn") yyjson_api_inline
    yyjson_mut_val *yyjson_mut_doc_get_pointern(yyjson_mut_doc *doc,
                                                const char *ptr, size_t len) {
  return yyjson_mut_doc_ptr_getn(doc, ptr, len);
}

yyjson_deprecated("renamed to yyjson_ptr_get") yyjson_api_inline
    yyjson_val *yyjson_get_pointer(yyjson_val *val, const char *ptr) {
  return yyjson_ptr_get(val, ptr);
}

yyjson_deprecated("renamed to yyjson_ptr_getn") yyjson_api_inline
    yyjson_val *yyjson_get_pointern(yyjson_val *val, const char *ptr,
                                    size_t len) {
  return yyjson_ptr_getn(val, ptr, len);
}

yyjson_deprecated("renamed to yyjson_mut_ptr_get") yyjson_api_inline
    yyjson_mut_val *yyjson_mut_get_pointer(yyjson_mut_val *val,
                                           const char *ptr) {
  return yyjson_mut_ptr_get(val, ptr);
}

yyjson_deprecated("renamed to yyjson_mut_ptr_getn") yyjson_api_inline
    yyjson_mut_val *yyjson_mut_get_pointern(yyjson_mut_val *val,
                                            const char *ptr, size_t len) {
  return yyjson_mut_ptr_getn(val, ptr, len);
}

yyjson_deprecated("renamed to unsafe_yyjson_ptr_getn") yyjson_api_inline
    yyjson_val *unsafe_yyjson_get_pointer(yyjson_val *val, const char *ptr,
                                          size_t len) {
  yyjson_ptr_err err;
  return unsafe_yyjson_ptr_getx(val, ptr, len, &err);
}

yyjson_deprecated("renamed to unsafe_yyjson_mut_ptr_getx") yyjson_api_inline
    yyjson_mut_val *unsafe_yyjson_mut_get_pointer(yyjson_mut_val *val,
                                                  const char *ptr, size_t len) {
  yyjson_ptr_err err;
  return unsafe_yyjson_mut_ptr_getx(val, ptr, len, NULL, &err);
}

#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#elif YYJSON_IS_REAL_GCC
#if yyjson_gcc_available(4, 6, 0)
#pragma GCC diagnostic pop
#endif
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif
