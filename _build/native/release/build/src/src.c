#ifdef __cplusplus
extern "C" {
#endif

#include "moonbit.h"

#ifdef _MSC_VER
#define _Noreturn __declspec(noreturn)
#endif

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wshift-op-parentheses"
#pragma clang diagnostic ignored "-Wtautological-compare"
#endif

MOONBIT_EXPORT _Noreturn void moonbit_panic(void);
MOONBIT_EXPORT void *moonbit_malloc_array(enum moonbit_block_kind kind,
                                          int elem_size_shift, int32_t len);
MOONBIT_EXPORT int moonbit_val_array_equal(const void *lhs, const void *rhs);
MOONBIT_EXPORT moonbit_string_t moonbit_add_string(moonbit_string_t s1,
                                                   moonbit_string_t s2);
MOONBIT_EXPORT void moonbit_unsafe_bytes_blit(moonbit_bytes_t dst,
                                              int32_t dst_start,
                                              moonbit_bytes_t src,
                                              int32_t src_offset, int32_t len);
MOONBIT_EXPORT moonbit_string_t moonbit_unsafe_bytes_sub_string(
    moonbit_bytes_t bytes, int32_t start, int32_t len);
MOONBIT_EXPORT void moonbit_println(moonbit_string_t str);
MOONBIT_EXPORT moonbit_bytes_t *moonbit_get_cli_args(void);
MOONBIT_EXPORT void moonbit_runtime_init(int argc, char **argv);
MOONBIT_EXPORT void moonbit_drop_object(void *);

#define Moonbit_make_regular_object_header(ptr_field_offset, ptr_field_count,  \
                                           tag)                                \
  (((uint32_t)moonbit_BLOCK_KIND_REGULAR << 30) |                              \
   (((uint32_t)(ptr_field_offset) & (((uint32_t)1 << 11) - 1)) << 19) |        \
   (((uint32_t)(ptr_field_count) & (((uint32_t)1 << 11) - 1)) << 8) |          \
   ((tag) & 0xFF))

// header manipulation macros
#define Moonbit_object_ptr_field_offset(obj)                                   \
  ((Moonbit_object_header(obj)->meta >> 19) & (((uint32_t)1 << 11) - 1))

#define Moonbit_object_ptr_field_count(obj)                                    \
  ((Moonbit_object_header(obj)->meta >> 8) & (((uint32_t)1 << 11) - 1))

#if !defined(_WIN64) && !defined(_WIN32)
void *malloc(size_t size);
void free(void *ptr);
#define libc_malloc malloc
#define libc_free free
#endif

// several important runtime functions are inlined
static void *moonbit_malloc_inlined(size_t size) {
  struct moonbit_object *ptr = (struct moonbit_object *)libc_malloc(
      sizeof(struct moonbit_object) + size);
  ptr->rc = 1;
  return ptr + 1;
}

#define moonbit_malloc(obj) moonbit_malloc_inlined(obj)
#define moonbit_free(obj) libc_free(Moonbit_object_header(obj))

static void moonbit_incref_inlined(void *ptr) {
  struct moonbit_object *header = Moonbit_object_header(ptr);
  int32_t const count = header->rc;
  if (count > 0) {
    header->rc = count + 1;
  }
}

#define moonbit_incref moonbit_incref_inlined

static void moonbit_decref_inlined(void *ptr) {
  struct moonbit_object *header = Moonbit_object_header(ptr);
  int32_t const count = header->rc;
  if (count > 1) {
    header->rc = count - 1;
  } else if (count == 1) {
    moonbit_drop_object(ptr);
  }
}

#define moonbit_decref moonbit_decref_inlined

#define moonbit_unsafe_make_string moonbit_make_string

// detect whether compiler builtins exist for advanced bitwise operations
#ifdef __has_builtin

#if __has_builtin(__builtin_clz)
#define HAS_BUILTIN_CLZ
#endif

#if __has_builtin(__builtin_ctz)
#define HAS_BUILTIN_CTZ
#endif

#if __has_builtin(__builtin_popcount)
#define HAS_BUILTIN_POPCNT
#endif

#if __has_builtin(__builtin_sqrt)
#define HAS_BUILTIN_SQRT
#endif

#if __has_builtin(__builtin_sqrtf)
#define HAS_BUILTIN_SQRTF
#endif

#if __has_builtin(__builtin_fabs)
#define HAS_BUILTIN_FABS
#endif

#if __has_builtin(__builtin_fabsf)
#define HAS_BUILTIN_FABSF
#endif

#endif

// if there is no builtin operators, use software implementation
#ifdef HAS_BUILTIN_CLZ
static inline int32_t moonbit_clz32(int32_t x) {
  return x == 0 ? 32 : __builtin_clz(x);
}

static inline int32_t moonbit_clz64(int64_t x) {
  return x == 0 ? 64 : __builtin_clzll(x);
}

#undef HAS_BUILTIN_CLZ
#else
// table for [clz] value of 4bit integer.
static const uint8_t moonbit_clz4[] = {4, 3, 2, 2, 1, 1, 1, 1,
                                       0, 0, 0, 0, 0, 0, 0, 0};

int32_t moonbit_clz32(uint32_t x) {
  /* The ideas is to:

     1. narrow down the 4bit block where the most signficant "1" bit lies,
        using binary search
     2. find the number of leading zeros in that 4bit block via table lookup

     Different time/space tradeoff can be made here by enlarging the table
     and do less binary search.
     One benefit of the 4bit lookup table is that it can fit into a single cache
     line.
  */
  int32_t result = 0;
  if (x > 0xffff) {
    x >>= 16;
  } else {
    result += 16;
  }
  if (x > 0xff) {
    x >>= 8;
  } else {
    result += 8;
  }
  if (x > 0xf) {
    x >>= 4;
  } else {
    result += 4;
  }
  return result + moonbit_clz4[x];
}

int32_t moonbit_clz64(uint64_t x) {
  int32_t result = 0;
  if (x > 0xffffffff) {
    x >>= 32;
  } else {
    result += 32;
  }
  return result + moonbit_clz32((uint32_t)x);
}
#endif

#ifdef HAS_BUILTIN_CTZ
static inline int32_t moonbit_ctz32(int32_t x) {
  return x == 0 ? 32 : __builtin_ctz(x);
}

static inline int32_t moonbit_ctz64(int64_t x) {
  return x == 0 ? 64 : __builtin_ctzll(x);
}

#undef HAS_BUILTIN_CTZ
#else
int32_t moonbit_ctz32(int32_t x) {
  /* The algorithm comes from:

       Leiserson, Charles E. et al. “Using de Bruijn Sequences to Index a 1 in a
     Computer Word.” (1998).

     The ideas is:

     1. leave only the least significant "1" bit in the input,
        set all other bits to "0". This is achieved via [x & -x]
     2. now we have [x * n == n << ctz(x)], if [n] is a de bruijn sequence
        (every 5bit pattern occurn exactly once when you cycle through the bit
     string), we can find [ctz(x)] from the most significant 5 bits of [x * n]
 */
  static const uint32_t de_bruijn_32 = 0x077CB531;
  static const uint8_t index32[] = {0,  1,  28, 2,  29, 14, 24, 3,  30, 22, 20,
                                    15, 25, 17, 4,  8,  31, 27, 13, 23, 21, 19,
                                    16, 7,  26, 12, 18, 6,  11, 5,  10, 9};
  return (x == 0) * 32 + index32[(de_bruijn_32 * (x & -x)) >> 27];
}

int32_t moonbit_ctz64(int64_t x) {
  static const uint64_t de_bruijn_64 = 0x0218A392CD3D5DBF;
  static const uint8_t index64[] = {
      0,  1,  2,  7,  3,  13, 8,  19, 4,  25, 14, 28, 9,  34, 20, 40,
      5,  17, 26, 38, 15, 46, 29, 48, 10, 31, 35, 54, 21, 50, 41, 57,
      63, 6,  12, 18, 24, 27, 33, 39, 16, 37, 45, 47, 30, 53, 49, 56,
      62, 11, 23, 32, 36, 44, 52, 55, 61, 22, 43, 51, 60, 42, 59, 58};
  return (x == 0) * 64 + index64[(de_bruijn_64 * (x & -x)) >> 58];
}
#endif

#ifdef HAS_BUILTIN_POPCNT

#define moonbit_popcnt32 __builtin_popcount
#define moonbit_popcnt64 __builtin_popcountll
#undef HAS_BUILTIN_POPCNT

#else
int32_t moonbit_popcnt32(uint32_t x) {
  /* The classic SIMD Within A Register algorithm.
     ref: [https://nimrod.blog/posts/algorithms-behind-popcount/]
 */
  x = x - ((x >> 1) & 0x55555555);
  x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
  x = (x + (x >> 4)) & 0x0F0F0F0F;
  return (x * 0x01010101) >> 24;
}

int32_t moonbit_popcnt64(uint64_t x) {
  x = x - ((x >> 1) & 0x5555555555555555);
  x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
  x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0F;
  return (x * 0x0101010101010101) >> 56;
}
#endif

/* The following sqrt implementation comes from
   [musl](https://git.musl-libc.org/cgit/musl),
   with some helpers inlined to make it zero dependency.
 */
#ifdef MOONBIT_NATIVE_NO_SYS_HEADER
const uint16_t __rsqrt_tab[128] = {
    0xb451, 0xb2f0, 0xb196, 0xb044, 0xaef9, 0xadb6, 0xac79, 0xab43, 0xaa14,
    0xa8eb, 0xa7c8, 0xa6aa, 0xa592, 0xa480, 0xa373, 0xa26b, 0xa168, 0xa06a,
    0x9f70, 0x9e7b, 0x9d8a, 0x9c9d, 0x9bb5, 0x9ad1, 0x99f0, 0x9913, 0x983a,
    0x9765, 0x9693, 0x95c4, 0x94f8, 0x9430, 0x936b, 0x92a9, 0x91ea, 0x912e,
    0x9075, 0x8fbe, 0x8f0a, 0x8e59, 0x8daa, 0x8cfe, 0x8c54, 0x8bac, 0x8b07,
    0x8a64, 0x89c4, 0x8925, 0x8889, 0x87ee, 0x8756, 0x86c0, 0x862b, 0x8599,
    0x8508, 0x8479, 0x83ec, 0x8361, 0x82d8, 0x8250, 0x81c9, 0x8145, 0x80c2,
    0x8040, 0xff02, 0xfd0e, 0xfb25, 0xf947, 0xf773, 0xf5aa, 0xf3ea, 0xf234,
    0xf087, 0xeee3, 0xed47, 0xebb3, 0xea27, 0xe8a3, 0xe727, 0xe5b2, 0xe443,
    0xe2dc, 0xe17a, 0xe020, 0xdecb, 0xdd7d, 0xdc34, 0xdaf1, 0xd9b3, 0xd87b,
    0xd748, 0xd61a, 0xd4f1, 0xd3cd, 0xd2ad, 0xd192, 0xd07b, 0xcf69, 0xce5b,
    0xcd51, 0xcc4a, 0xcb48, 0xca4a, 0xc94f, 0xc858, 0xc764, 0xc674, 0xc587,
    0xc49d, 0xc3b7, 0xc2d4, 0xc1f4, 0xc116, 0xc03c, 0xbf65, 0xbe90, 0xbdbe,
    0xbcef, 0xbc23, 0xbb59, 0xba91, 0xb9cc, 0xb90a, 0xb84a, 0xb78c, 0xb6d0,
    0xb617, 0xb560,
};

/* returns a*b*2^-32 - e, with error 0 <= e < 1.  */
static inline uint32_t mul32(uint32_t a, uint32_t b) {
  return (uint64_t)a * b >> 32;
}
#endif

#ifdef MOONBIT_NATIVE_NO_SYS_HEADER
float sqrtf(float x) {
  uint32_t ix, m, m1, m0, even, ey;

  ix = *(uint32_t *)&x;
  if (ix - 0x00800000 >= 0x7f800000 - 0x00800000) {
    /* x < 0x1p-126 or inf or nan.  */
    if (ix * 2 == 0)
      return x;
    if (ix == 0x7f800000)
      return x;
    if (ix > 0x7f800000)
      return (x - x) / (x - x);
    /* x is subnormal, normalize it.  */
    x *= 0x1p23f;
    ix = *(uint32_t *)&x;
    ix -= 23 << 23;
  }

  /* x = 4^e m; with int e and m in [1, 4).  */
  even = ix & 0x00800000;
  m1 = (ix << 8) | 0x80000000;
  m0 = (ix << 7) & 0x7fffffff;
  m = even ? m0 : m1;

  /* 2^e is the exponent part of the return value.  */
  ey = ix >> 1;
  ey += 0x3f800000 >> 1;
  ey &= 0x7f800000;

  /* compute r ~ 1/sqrt(m), s ~ sqrt(m) with 2 goldschmidt iterations.  */
  static const uint32_t three = 0xc0000000;
  uint32_t r, s, d, u, i;
  i = (ix >> 17) % 128;
  r = (uint32_t)__rsqrt_tab[i] << 16;
  /* |r*sqrt(m) - 1| < 0x1p-8 */
  s = mul32(m, r);
  /* |s/sqrt(m) - 1| < 0x1p-8 */
  d = mul32(s, r);
  u = three - d;
  r = mul32(r, u) << 1;
  /* |r*sqrt(m) - 1| < 0x1.7bp-16 */
  s = mul32(s, u) << 1;
  /* |s/sqrt(m) - 1| < 0x1.7bp-16 */
  d = mul32(s, r);
  u = three - d;
  s = mul32(s, u);
  /* -0x1.03p-28 < s/sqrt(m) - 1 < 0x1.fp-31 */
  s = (s - 1) >> 6;
  /* s < sqrt(m) < s + 0x1.08p-23 */

  /* compute nearest rounded result.  */
  uint32_t d0, d1, d2;
  float y, t;
  d0 = (m << 16) - s * s;
  d1 = s - d0;
  d2 = d1 + s + 1;
  s += d1 >> 31;
  s &= 0x007fffff;
  s |= ey;
  y = *(float *)&s;
  /* handle rounding and inexact exception. */
  uint32_t tiny = d2 == 0 ? 0 : 0x01000000;
  tiny |= (d1 ^ d2) & 0x80000000;
  t = *(float *)&tiny;
  y = y + t;
  return y;
}
#endif

#ifdef MOONBIT_NATIVE_NO_SYS_HEADER
/* returns a*b*2^-64 - e, with error 0 <= e < 3.  */
static inline uint64_t mul64(uint64_t a, uint64_t b) {
  uint64_t ahi = a >> 32;
  uint64_t alo = a & 0xffffffff;
  uint64_t bhi = b >> 32;
  uint64_t blo = b & 0xffffffff;
  return ahi * bhi + (ahi * blo >> 32) + (alo * bhi >> 32);
}

double sqrt(double x) {
  uint64_t ix, top, m;

  /* special case handling.  */
  ix = *(uint64_t *)&x;
  top = ix >> 52;
  if (top - 0x001 >= 0x7ff - 0x001) {
    /* x < 0x1p-1022 or inf or nan.  */
    if (ix * 2 == 0)
      return x;
    if (ix == 0x7ff0000000000000)
      return x;
    if (ix > 0x7ff0000000000000)
      return (x - x) / (x - x);
    /* x is subnormal, normalize it.  */
    x *= 0x1p52;
    ix = *(uint64_t *)&x;
    top = ix >> 52;
    top -= 52;
  }

  /* argument reduction:
     x = 4^e m; with integer e, and m in [1, 4)
     m: fixed point representation [2.62]
     2^e is the exponent part of the result.  */
  int even = top & 1;
  m = (ix << 11) | 0x8000000000000000;
  if (even)
    m >>= 1;
  top = (top + 0x3ff) >> 1;

  /* approximate r ~ 1/sqrt(m) and s ~ sqrt(m) when m in [1,4)

     initial estimate:
     7bit table lookup (1bit exponent and 6bit significand).

     iterative approximation:
     using 2 goldschmidt iterations with 32bit int arithmetics
     and a final iteration with 64bit int arithmetics.

     details:

     the relative error (e = r0 sqrt(m)-1) of a linear estimate
     (r0 = a m + b) is |e| < 0.085955 ~ 0x1.6p-4 at best,
     a table lookup is faster and needs one less iteration
     6 bit lookup table (128b) gives |e| < 0x1.f9p-8
     7 bit lookup table (256b) gives |e| < 0x1.fdp-9
     for single and double prec 6bit is enough but for quad
     prec 7bit is needed (or modified iterations). to avoid
     one more iteration >=13bit table would be needed (16k).

     a newton-raphson iteration for r is
       w = r*r
       u = 3 - m*w
       r = r*u/2
     can use a goldschmidt iteration for s at the end or
       s = m*r

     first goldschmidt iteration is
       s = m*r
       u = 3 - s*r
       r = r*u/2
       s = s*u/2
     next goldschmidt iteration is
       u = 3 - s*r
       r = r*u/2
       s = s*u/2
     and at the end r is not computed only s.

     they use the same amount of operations and converge at the
     same quadratic rate, i.e. if
       r1 sqrt(m) - 1 = e, then
       r2 sqrt(m) - 1 = -3/2 e^2 - 1/2 e^3
     the advantage of goldschmidt is that the mul for s and r
     are independent (computed in parallel), however it is not
     "self synchronizing": it only uses the input m in the
     first iteration so rounding errors accumulate. at the end
     or when switching to larger precision arithmetics rounding
     errors dominate so the first iteration should be used.

     the fixed point representations are
       m: 2.30 r: 0.32, s: 2.30, d: 2.30, u: 2.30, three: 2.30
     and after switching to 64 bit
       m: 2.62 r: 0.64, s: 2.62, d: 2.62, u: 2.62, three: 2.62  */

  static const uint64_t three = 0xc0000000;
  uint64_t r, s, d, u, i;

  i = (ix >> 46) % 128;
  r = (uint32_t)__rsqrt_tab[i] << 16;
  /* |r sqrt(m) - 1| < 0x1.fdp-9 */
  s = mul32(m >> 32, r);
  /* |s/sqrt(m) - 1| < 0x1.fdp-9 */
  d = mul32(s, r);
  u = three - d;
  r = mul32(r, u) << 1;
  /* |r sqrt(m) - 1| < 0x1.7bp-16 */
  s = mul32(s, u) << 1;
  /* |s/sqrt(m) - 1| < 0x1.7bp-16 */
  d = mul32(s, r);
  u = three - d;
  r = mul32(r, u) << 1;
  /* |r sqrt(m) - 1| < 0x1.3704p-29 (measured worst-case) */
  r = r << 32;
  s = mul64(m, r);
  d = mul64(s, r);
  u = (three << 32) - d;
  s = mul64(s, u); /* repr: 3.61 */
  /* -0x1p-57 < s - sqrt(m) < 0x1.8001p-61 */
  s = (s - 2) >> 9; /* repr: 12.52 */
  /* -0x1.09p-52 < s - sqrt(m) < -0x1.fffcp-63 */

  /* s < sqrt(m) < s + 0x1.09p-52,
     compute nearest rounded result:
     the nearest result to 52 bits is either s or s+0x1p-52,
     we can decide by comparing (2^52 s + 0.5)^2 to 2^104 m.  */
  uint64_t d0, d1, d2;
  double y, t;
  d0 = (m << 42) - s * s;
  d1 = s - d0;
  d2 = d1 + s + 1;
  s += d1 >> 63;
  s &= 0x000fffffffffffff;
  s |= top << 52;
  y = *(double *)&s;
  return y;
}
#endif

#ifdef MOONBIT_NATIVE_NO_SYS_HEADER
double fabs(double x) {
  union {
    double f;
    uint64_t i;
  } u = {x};
  u.i &= 0x7fffffffffffffffULL;
  return u.f;
}
#endif

#ifdef MOONBIT_NATIVE_NO_SYS_HEADER
float fabsf(float x) {
  union {
    float f;
    uint32_t i;
  } u = {x};
  u.i &= 0x7fffffff;
  return u.f;
}
#endif

#ifdef _MSC_VER
/* MSVC treats syntactic division by zero as fatal error,
   even for float point numbers,
   so we have to use a constant variable to work around this */
static const int MOONBIT_ZERO = 0;
#else
#define MOONBIT_ZERO 0
#endif

#ifdef __cplusplus
}
#endif
struct $$3c$String$2a$Int$3e$;

struct $Result$3c$Unit$2a$$moonbitlang$x$fs$IOError$3e$$Err;

struct $$moonbitlang$core$builtin$Array$3c$String$3e$;

struct $String$$iter$$2a$p$fn$1$2d$cap;

struct $Result$3c$Unit$2a$String$3e$$Err;

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$;

struct $Ref$3c$Int$3e$;

struct $$3c$Bytes$3e$$3d$$3e$String;

struct $StringView;

struct $$ZSeanYves$Doclint$src$core$Page;

struct $Result$3c$Byte$2a$$moonbitlang$core$builtin$NoError$3e$$Err;

struct $$ZSeanYves$Doclint$src$core$Issue;

struct $$moonbitlang$core$builtin$ArrayView$3c$String$3e$;

struct $$3c$Int$3e$$3d$$3e$Byte;

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule;

struct $$moonbitlang$core$builtin$ArrayView$3c$Byte$3e$;

struct $Result$3c$FixedArray$3c$String$3e$$2a$$moonbitlang$core$builtin$NoError$3e$$Err;

struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$;

struct $$ZSeanYves$Doclint$src$core$Rule;

struct $$3c$StringView$2a$Option$3c$StringView$3e$$3e$;

struct $$ZSeanYves$Doclint$src$core$Rule$static_method_table;

struct $Result$3c$Bool$2a$$moonbitlang$x$fs$IOError$3e$$Ok;

struct $Result$3c$FixedArray$3c$String$3e$$2a$$moonbitlang$core$builtin$NoError$3e$$Ok;

struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Ok;

struct $Error$moonbitlang$x$fs$IOError$IOError;

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$;

struct $$moonbitlang$core$builtin$Logger;

struct $Result$3c$Bytes$2a$$moonbitlang$core$builtin$NoError$3e$$Err;

struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$;

struct $$moonbitlang$core$builtin$ArrayView$3c$$ZSeanYves$Doclint$src$core$Issue$3e$;

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule;

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$ThesisRuleset;

struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$;

struct $$moonbitlang$core$builtin$StringBuilder$as_$moonbitlang$core$builtin$Logger;

struct $Result$3c$String$2a$String$3e$$Ok;

struct $$ZSeanYves$Doclint$src$doclint_rules_contract$ContractRuleset;

struct $Result$3c$Unit$2a$$moonbitlang$x$fs$IOError$3e$$Ok;

struct $$ZSeanYves$Doclint$src$core$DocMeta;

struct $$moonbitlang$core$builtin$ArrayView$3c$Char$3e$;

struct $Result$3c$StringView$2a$$moonbitlang$core$builtin$CreatingViewError$3e$$Err;

struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Err;

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$;

struct $Result$3c$String$2a$String$3e$$Err;

struct $ArrayView$$iter$7c$$ZSeanYves$Doclint$src$core$Issue$7c$$$2a$p$fn$2$2d$cap;

struct $Result$3c$String$2a$$moonbitlang$core$builtin$NoError$3e$$Err;

struct $$moonbitlang$core$builtin$Array$3c$Char$3e$;

struct $Result$3c$String$2a$$moonbitlang$core$builtin$NoError$3e$$Ok;

struct $Result$3c$Bool$2a$$moonbitlang$x$fs$IOError$3e$$Err;

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$as_$ZSeanYves$Doclint$src$core$Rule;

struct $Result$3c$Unit$2a$String$3e$$Ok;

struct $Result$3c$String$2a$$moonbitlang$x$fs$IOError$3e$$Err;

struct $Result$3c$Bytes$2a$$moonbitlang$x$fs$IOError$3e$$Err;

struct $Bytes$$from_array$fn$3$2d$cap;

struct $Result$3c$Bytes$2a$$moonbitlang$x$fs$IOError$3e$$Ok;

struct $Result$3c$StringView$2a$$moonbitlang$core$builtin$CreatingViewError$3e$$Ok;

struct $$ZSeanYves$Doclint$src$core$Location;

struct $$moonbitlang$core$builtin$SourceLocRepr;

struct $Result$3c$Bytes$2a$$moonbitlang$core$builtin$NoError$3e$$Ok;

struct $Result$3c$String$2a$$moonbitlang$x$fs$IOError$3e$$Ok;

struct $Option$3c$StringView$3e$$Some;

struct $Result$3c$Byte$2a$$moonbitlang$core$builtin$NoError$3e$$Ok;

struct $$moonbitlang$core$builtin$Logger$static_method_table;

struct $$moonbitlang$core$builtin$StringBuilder;

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$as_$ZSeanYves$Doclint$src$core$Rule;

struct $$ZSeanYves$Doclint$src$core$Document;

struct $$3c$String$2a$Int$3e$ {
  int32_t $1;
  moonbit_string_t $0;
  
};

struct $Result$3c$Unit$2a$$moonbitlang$x$fs$IOError$3e$$Err {
  void* $0;
  
};

struct $$moonbitlang$core$builtin$Array$3c$String$3e$ {
  int32_t $1;
  moonbit_string_t* $0;
  
};

struct $String$$iter$$2a$p$fn$1$2d$cap {
  int32_t(* code)(struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$*);
  int32_t $2;
  struct $Ref$3c$Int$3e$* $0;
  moonbit_string_t $1;
  
};

struct $Result$3c$Unit$2a$String$3e$$Err {
  moonbit_string_t $0;
  
};

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$ {
  int32_t $1;
  struct $$ZSeanYves$Doclint$src$core$Rule* $0;
  
};

struct $Ref$3c$Int$3e$ {
  int32_t $0;
  
};

struct $$3c$Bytes$3e$$3d$$3e$String {
  moonbit_string_t(* code)(
    struct $$3c$Bytes$3e$$3d$$3e$String*,
    moonbit_bytes_t
  );
  
};

struct $StringView {
  int32_t $1;
  int32_t $2;
  moonbit_string_t $0;
  
};

struct $$ZSeanYves$Doclint$src$core$Page {
  int32_t $0;
  moonbit_string_t $1;
  
};

struct $Result$3c$Byte$2a$$moonbitlang$core$builtin$NoError$3e$$Err {
  int32_t $0;
  
};

struct $$ZSeanYves$Doclint$src$core$Issue {
  int32_t $1;
  moonbit_string_t $0;
  moonbit_string_t $2;
  struct $$ZSeanYves$Doclint$src$core$Location* $3;
  
};

struct $$moonbitlang$core$builtin$ArrayView$3c$String$3e$ {
  int32_t $1;
  int32_t $2;
  moonbit_string_t* $0;
  
};

struct $$3c$Int$3e$$3d$$3e$Byte {
  int32_t(* code)(struct $$3c$Int$3e$$3d$$3e$Byte*, int32_t);
  
};

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule {
  int32_t $2;
  moonbit_string_t $0;
  moonbit_string_t $1;
  
};

struct $$moonbitlang$core$builtin$ArrayView$3c$Byte$3e$ {
  int32_t $1;
  int32_t $2;
  moonbit_bytes_t $0;
  
};

struct $Result$3c$FixedArray$3c$String$3e$$2a$$moonbitlang$core$builtin$NoError$3e$$Err {
  int32_t $0;
  
};

struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$ {
  struct $$ZSeanYves$Doclint$src$core$Issue*(* code)(
    struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$*
  );
  
};

struct $$ZSeanYves$Doclint$src$core$Rule {
  struct $$ZSeanYves$Doclint$src$core$Rule$static_method_table* $0;
  void* $1;
  
};

struct $$3c$StringView$2a$Option$3c$StringView$3e$$3e$ {
  int32_t $0_1;
  int32_t $0_2;
  moonbit_string_t $0_0;
  void* $1;
  
};

struct $$ZSeanYves$Doclint$src$core$Rule$static_method_table {
  moonbit_string_t(* $method_0)(void*);
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$*(* $method_1)(
    void*,
    int32_t,
    struct $$ZSeanYves$Doclint$src$core$Document*
  );
  
};

struct $Result$3c$Bool$2a$$moonbitlang$x$fs$IOError$3e$$Ok {
  int32_t $0;
  
};

struct $Result$3c$FixedArray$3c$String$3e$$2a$$moonbitlang$core$builtin$NoError$3e$$Ok {
  moonbit_string_t* $0;
  
};

struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Ok {
  struct $$ZSeanYves$Doclint$src$core$Document* $0;
  
};

struct $Error$moonbitlang$x$fs$IOError$IOError {
  moonbit_string_t $0;
  
};

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$ {
  int32_t $1;
  struct $$ZSeanYves$Doclint$src$core$Page** $0;
  
};

struct $$moonbitlang$core$builtin$Logger {
  struct $$moonbitlang$core$builtin$Logger$static_method_table* $0;
  void* $1;
  
};

struct $Result$3c$Bytes$2a$$moonbitlang$core$builtin$NoError$3e$$Err {
  int32_t $0;
  
};

struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$ {
  int32_t(* code)(struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$*);
  
};

struct $$moonbitlang$core$builtin$ArrayView$3c$$ZSeanYves$Doclint$src$core$Issue$3e$ {
  int32_t $1;
  int32_t $2;
  struct $$ZSeanYves$Doclint$src$core$Issue** $0;
  
};

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule {
  int32_t $0;
  int32_t $1;
  
};

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$ThesisRuleset {
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* $0;
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* $1;
  
};

struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$ {
  int32_t $1;
  moonbit_bytes_t $0;
  
};

struct $$moonbitlang$core$builtin$StringBuilder$as_$moonbitlang$core$builtin$Logger {
  struct $$moonbitlang$core$builtin$Logger$static_method_table* $0;
  void* $1;
  
};

struct $Result$3c$String$2a$String$3e$$Ok {
  moonbit_string_t $0;
  
};

struct $$ZSeanYves$Doclint$src$doclint_rules_contract$ContractRuleset {
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* $0;
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* $1;
  
};

struct $Result$3c$Unit$2a$$moonbitlang$x$fs$IOError$3e$$Ok {
  int32_t $0;
  
};

struct $$ZSeanYves$Doclint$src$core$DocMeta {
  moonbit_string_t $0;
  moonbit_string_t $1;
  moonbit_string_t $2;
  
};

struct $$moonbitlang$core$builtin$ArrayView$3c$Char$3e$ {
  int32_t $1;
  int32_t $2;
  int32_t* $0;
  
};

struct $Result$3c$StringView$2a$$moonbitlang$core$builtin$CreatingViewError$3e$$Err {
  void* $0;
  
};

struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Err {
  moonbit_string_t $0;
  
};

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$ {
  int32_t $1;
  struct $$ZSeanYves$Doclint$src$core$Issue** $0;
  
};

struct $Result$3c$String$2a$String$3e$$Err {
  moonbit_string_t $0;
  
};

struct $ArrayView$$iter$7c$$ZSeanYves$Doclint$src$core$Issue$7c$$$2a$p$fn$2$2d$cap {
  struct $$ZSeanYves$Doclint$src$core$Issue*(* code)(
    struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$*
  );
  int32_t $0_1;
  int32_t $0_2;
  struct $$ZSeanYves$Doclint$src$core$Issue** $0_0;
  struct $Ref$3c$Int$3e$* $1;
  
};

struct $Result$3c$String$2a$$moonbitlang$core$builtin$NoError$3e$$Err {
  int32_t $0;
  
};

struct $$moonbitlang$core$builtin$Array$3c$Char$3e$ {
  int32_t $1;
  int32_t* $0;
  
};

struct $Result$3c$String$2a$$moonbitlang$core$builtin$NoError$3e$$Ok {
  moonbit_string_t $0;
  
};

struct $Result$3c$Bool$2a$$moonbitlang$x$fs$IOError$3e$$Err {
  void* $0;
  
};

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$as_$ZSeanYves$Doclint$src$core$Rule {
  struct $$ZSeanYves$Doclint$src$core$Rule$static_method_table* $0;
  void* $1;
  
};

struct $Result$3c$Unit$2a$String$3e$$Ok {
  int32_t $0;
  
};

struct $Result$3c$String$2a$$moonbitlang$x$fs$IOError$3e$$Err {
  void* $0;
  
};

struct $Result$3c$Bytes$2a$$moonbitlang$x$fs$IOError$3e$$Err {
  void* $0;
  
};

struct $Bytes$$from_array$fn$3$2d$cap {
  int32_t(* code)(struct $$3c$Int$3e$$3d$$3e$Byte*, int32_t);
  int32_t $0_1;
  int32_t $0_2;
  moonbit_bytes_t $0_0;
  
};

struct $Result$3c$Bytes$2a$$moonbitlang$x$fs$IOError$3e$$Ok {
  moonbit_bytes_t $0;
  
};

struct $Result$3c$StringView$2a$$moonbitlang$core$builtin$CreatingViewError$3e$$Ok {
  int32_t $0_1;
  int32_t $0_2;
  moonbit_string_t $0_0;
  
};

struct $$ZSeanYves$Doclint$src$core$Location {
  int32_t $0;
  int32_t $1;
  int32_t $2;
  
};

struct $$moonbitlang$core$builtin$SourceLocRepr {
  int32_t $0_1;
  int32_t $0_2;
  int32_t $1_1;
  int32_t $1_2;
  int32_t $2_1;
  int32_t $2_2;
  int32_t $3_1;
  int32_t $3_2;
  int32_t $4_1;
  int32_t $4_2;
  int32_t $5_1;
  int32_t $5_2;
  moonbit_string_t $0_0;
  moonbit_string_t $1_0;
  moonbit_string_t $2_0;
  moonbit_string_t $3_0;
  moonbit_string_t $4_0;
  moonbit_string_t $5_0;
  
};

struct $Result$3c$Bytes$2a$$moonbitlang$core$builtin$NoError$3e$$Ok {
  moonbit_bytes_t $0;
  
};

struct $Result$3c$String$2a$$moonbitlang$x$fs$IOError$3e$$Ok {
  moonbit_string_t $0;
  
};

struct $Option$3c$StringView$3e$$Some {
  int32_t $0_1;
  int32_t $0_2;
  moonbit_string_t $0_0;
  
};

struct $Result$3c$Byte$2a$$moonbitlang$core$builtin$NoError$3e$$Ok {
  int32_t $0;
  
};

struct $$moonbitlang$core$builtin$Logger$static_method_table {
  int32_t(* $method_0)(void*, moonbit_string_t);
  int32_t(* $method_1)(void*, moonbit_string_t, int32_t, int32_t);
  int32_t(* $method_2)(void*, struct $StringView);
  int32_t(* $method_3)(void*, int32_t);
  
};

struct $$moonbitlang$core$builtin$StringBuilder {
  int32_t $1;
  moonbit_bytes_t $0;
  
};

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$as_$ZSeanYves$Doclint$src$core$Rule {
  struct $$ZSeanYves$Doclint$src$core$Rule$static_method_table* $0;
  void* $1;
  
};

struct $$ZSeanYves$Doclint$src$core$Document {
  struct $$ZSeanYves$Doclint$src$core$DocMeta* $0;
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* $1;
  
};

struct moonbit_result_0 {
  int tag;
  union { int32_t ok; void* err;  } data;
  
};

struct moonbit_result_2 {
  int tag;
  union { moonbit_bytes_t ok; void* err;  } data;
  
};

struct moonbit_result_3 {
  int tag;
  union { struct $StringView ok; void* err;  } data;
  
};

struct moonbit_result_1 {
  int tag;
  union { moonbit_string_t ok; void* err;  } data;
  
};

int32_t $ZSeanYves$Doclint$src$write_out(
  moonbit_string_t out_json_path$885,
  moonbit_string_t out_html_path$891,
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* issues$886,
  int32_t emit_html$889
);

int32_t $ZSeanYves$Doclint$src$print_help();

struct $$moonbitlang$core$builtin$Array$3c$String$3e$* $ZSeanYves$Doclint$src$drop1(
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* xs$880
);

moonbit_string_t $ZSeanYves$Doclint$src$join_path(
  moonbit_string_t dir$875,
  moonbit_string_t file$876
);

int32_t $ZSeanYves$Doclint$src$ensure_dir(moonbit_string_t path$870);

void* $ZSeanYves$Doclint$src$write_utf8(
  moonbit_string_t path$868,
  moonbit_string_t content$869
);

void* $ZSeanYves$Doclint$src$read_utf8(moonbit_string_t path$865);

int32_t $moonbitlang$x$sys$exit(int32_t code$861);

struct $$moonbitlang$core$builtin$Array$3c$String$3e$* $moonbitlang$x$sys$get_cli_args(
  
);

int32_t $moonbitlang$x$sys$internal$ffi$exit(int32_t _param$1201);

struct $$moonbitlang$core$builtin$Array$3c$String$3e$* $moonbitlang$x$sys$internal$ffi$get_cli_args(
  
);

moonbit_string_t $$moonbitlang$x$sys$internal$ffi$get_cli_args$fn$4(
  struct $$3c$Bytes$3e$$3d$$3e$String* _env$2411,
  moonbit_bytes_t arg$860
);

#define $moonbitlang$x$sys$internal$ffi$internal_get_cli_args moonbit_get_cli_args

struct moonbit_result_0 $moonbitlang$x$fs$is_dir(moonbit_string_t path$859);

struct moonbit_result_0 $moonbitlang$x$fs$create_dir(
  moonbit_string_t path$858
);

int32_t $moonbitlang$x$fs$path_exists(moonbit_string_t path$857);

struct moonbit_result_0 $moonbitlang$x$fs$write_string_to_file$inner(
  moonbit_string_t path$854,
  moonbit_string_t content$855,
  moonbit_string_t encoding$856
);

struct moonbit_result_1 $moonbitlang$x$fs$read_file_to_string$inner(
  moonbit_string_t path$852,
  moonbit_string_t encoding$853
);

struct moonbit_result_0 $moonbitlang$x$fs$is_dir_internal(
  moonbit_string_t path$851
);

#define $moonbitlang$x$fs$is_dir_ffi moonbitlang_x_fs_is_dir_ffi

struct moonbit_result_0 $moonbitlang$x$fs$create_dir_internal(
  moonbit_string_t path$849
);

#define $moonbitlang$x$fs$create_dir_ffi moonbitlang_x_fs_create_dir_ffi

int32_t $moonbitlang$x$fs$path_exists_internal(moonbit_string_t path$848);

#define $moonbitlang$x$fs$stat_ffi moonbitlang_x_fs_stat_ffi

struct moonbit_result_0 $moonbitlang$x$fs$write_string_to_file_internal$inner(
  moonbit_string_t path$847,
  moonbit_string_t content$846,
  moonbit_string_t encoding$844
);

struct moonbit_result_0 $moonbitlang$x$fs$write_bytes_to_file_internal(
  moonbit_string_t path$841,
  moonbit_bytes_t content$843
);

struct moonbit_result_1 $moonbitlang$x$fs$read_file_to_string_internal$inner(
  moonbit_string_t path$839,
  moonbit_string_t encoding$838
);

struct moonbit_result_2 $moonbitlang$x$fs$read_file_to_bytes_internal(
  moonbit_string_t path$834
);

moonbit_string_t $moonbitlang$x$fs$get_error_message();

#define $moonbitlang$x$fs$fclose_ffi moonbitlang_x_fs_fclose_ffi

#define $moonbitlang$x$fs$fflush_ffi moonbitlang_x_fs_fflush_ffi

#define $moonbitlang$x$fs$ftell_ffi moonbitlang_x_fs_ftell_ffi

#define $moonbitlang$x$fs$fseek_ffi moonbitlang_x_fs_fseek_ffi

#define $moonbitlang$x$fs$get_error_message_ffi moonbitlang_x_fs_get_error_message

#define $moonbitlang$x$fs$fwrite_ffi moonbitlang_x_fs_fwrite_ffi

#define $moonbitlang$x$fs$fread_ffi moonbitlang_x_fs_fread_ffi

#define $moonbitlang$x$fs$is_null moonbitlang_x_fs_is_null

#define $moonbitlang$x$fs$fopen_ffi moonbitlang_x_fs_fopen_ffi

moonbit_string_t $moonbitlang$x$internal$ffi$utf8_bytes_to_mbt_string(
  moonbit_bytes_t bytes$829
);

moonbit_bytes_t $moonbitlang$x$internal$ffi$mbt_string_to_utf8_bytes(
  moonbit_string_t str$821,
  int32_t is_filename$826
);

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$* $ZSeanYves$Doclint$src$doclint_rules_contract$as_rules_ref(
  struct $$ZSeanYves$Doclint$src$doclint_rules_contract$ContractRuleset* rs$818
);

struct $$ZSeanYves$Doclint$src$doclint_rules_contract$ContractRuleset* $ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset(
  
);

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$* $ZSeanYves$Doclint$src$doclint_rules_thesis$as_rules_ref(
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$ThesisRuleset* rs$814
);

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$ThesisRuleset* $ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset(
  
);

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$$check(
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* self$809,
  int32_t _ctx$810,
  struct $$ZSeanYves$Doclint$src$core$Document* doc$803
);

moonbit_string_t $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$$id(
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* self$800
);

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$$check(
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* self$798,
  int32_t _ctx$799,
  struct $$ZSeanYves$Doclint$src$core$Document* doc$797
);

moonbit_string_t $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$$id(
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* _self$795
);

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $ZSeanYves$Doclint$src$core$check_doc(
  int32_t ctx$792,
  struct $$ZSeanYves$Doclint$src$core$Document* doc$793,
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$* rules$794
);

struct $$ZSeanYves$Doclint$src$core$Issue* $ZSeanYves$Doclint$src$core$mk_issue(
  moonbit_string_t rule_id$788,
  int32_t severity$789,
  moonbit_string_t message$790,
  struct $$ZSeanYves$Doclint$src$core$Location* location$791
);

int32_t $ZSeanYves$Doclint$src$core$page_count(
  struct $$ZSeanYves$Doclint$src$core$Document* doc$787
);

void* $ZSeanYves$Doclint$src$core$parse_doc_json(moonbit_string_t s$784);

struct $$ZSeanYves$Doclint$src$core$Document* $ZSeanYves$Doclint$src$core$parse_meta_and_pages(
  moonbit_string_t s$770
);

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* $ZSeanYves$Doclint$src$core$parse_pages(
  moonbit_string_t s$745
);

int32_t $ZSeanYves$Doclint$src$core$skip_ws_comma(
  moonbit_string_t s$741,
  int32_t i$740
);

moonbit_string_t $ZSeanYves$Doclint$src$core$find_string_field_from(
  moonbit_string_t s$725,
  moonbit_string_t key$723,
  int32_t start$726
);

int64_t $ZSeanYves$Doclint$src$core$find_int_field_from(
  moonbit_string_t s$705,
  moonbit_string_t key$703,
  int32_t start$706
);

int32_t $ZSeanYves$Doclint$src$core$skip_ws(
  moonbit_string_t s$699,
  int32_t i$698
);

int64_t $ZSeanYves$Doclint$src$core$parse_int_dec(moonbit_string_t s$692);

moonbit_string_t $ZSeanYves$Doclint$src$core$find_string_field(
  moonbit_string_t s$676,
  moonbit_string_t key$674
);

struct $$3c$String$2a$Int$3e$* $ZSeanYves$Doclint$src$core$read_json_string(
  moonbit_string_t s$669,
  int32_t start$668
);

int64_t $ZSeanYves$Doclint$src$core$find_sub(
  moonbit_string_t s$660,
  moonbit_string_t pat$662,
  int32_t start$663
);

int64_t $ZSeanYves$Doclint$src$core$find_char(
  moonbit_string_t s$656,
  int32_t ch$657,
  int32_t start$655
);

moonbit_string_t $ZSeanYves$Doclint$src$core$slice(
  moonbit_string_t s$647,
  int32_t start$651,
  int32_t end_$652
);

moonbit_string_t $ZSeanYves$Doclint$src$core$issues_to_html(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* issues$629
);

moonbit_string_t $ZSeanYves$Doclint$src$core$section_table(
  moonbit_string_t title$624,
  moonbit_string_t rows$623
);

moonbit_string_t $ZSeanYves$Doclint$src$core$issue_row_html(
  struct $$ZSeanYves$Doclint$src$core$Issue* it$618
);

moonbit_string_t $ZSeanYves$Doclint$src$core$severity_badge(int32_t sev$616);

moonbit_string_t $ZSeanYves$Doclint$src$core$escape_html(
  moonbit_string_t s$611
);

moonbit_string_t $ZSeanYves$Doclint$src$core$issues_to_json(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* issues$605
);

moonbit_string_t $ZSeanYves$Doclint$src$core$issue_to_json(
  struct $$ZSeanYves$Doclint$src$core$Issue* it$599
);

moonbit_string_t $ZSeanYves$Doclint$src$core$location_to_json(
  struct $$ZSeanYves$Doclint$src$core$Location* loc$597
);

moonbit_string_t $ZSeanYves$Doclint$src$core$quote(moonbit_string_t s$596);

moonbit_string_t $ZSeanYves$Doclint$src$core$escape_str(
  moonbit_string_t s$591
);

moonbit_string_t $ZSeanYves$Doclint$src$core$severity_to_string(
  int32_t s$588
);

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $ZSeanYves$Doclint$src$core$run_rules(
  int32_t ctx$585,
  struct $$ZSeanYves$Doclint$src$core$Document* doc$586,
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$* rules$581
);

moonbit_string_t $$moonbitlang$core$builtin$Array$$join$0(
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* self$577,
  struct $StringView separator$578
);

int32_t $$moonbitlang$core$builtin$Array$$push_iter$0(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$575,
  struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* iter$572
);

struct $$moonbitlang$core$builtin$Array$3c$String$3e$* $$moonbitlang$core$builtin$Array$$from_fixed_array$0(
  moonbit_string_t* arr$569
);

moonbit_string_t $$moonbitlang$core$builtin$ArrayView$$join$0(
  struct $$moonbitlang$core$builtin$ArrayView$3c$String$3e$ self$544,
  struct $StringView separator$555
);

int32_t $$moonbitlang$core$builtin$Show$$$moonbitlang$core$builtin$SourceLoc$$output(
  moonbit_string_t self$542,
  struct $$moonbitlang$core$builtin$Logger logger$543
);

int32_t $$moonbitlang$core$builtin$Show$$$moonbitlang$core$builtin$SourceLocRepr$$output(
  struct $$moonbitlang$core$builtin$SourceLocRepr* self$505,
  struct $$moonbitlang$core$builtin$Logger logger$541
);

moonbit_bytes_t $Bytes$$from_array(
  struct $$moonbitlang$core$builtin$ArrayView$3c$Byte$3e$ arr$502
);

int32_t $Bytes$$from_array$fn$3(
  struct $$3c$Int$3e$$3d$$3e$Byte* _env$1806,
  int32_t i$503
);

int32_t $moonbitlang$core$builtin$println$0(moonbit_string_t input$501);

moonbit_bytes_t $Bytes$$makei$0(
  int32_t length$496,
  struct $$3c$Int$3e$$3d$$3e$Byte* value$498
);

int32_t $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit_fixed$0(
  moonbit_string_t* dst$490,
  int32_t dst_offset$491,
  moonbit_string_t* src$492,
  int32_t src_offset$493,
  int32_t len$495
);

int32_t $String$$unsafe_charcode_at(
  moonbit_string_t self$487,
  int32_t idx$488
);

moonbit_bytes_t $Bytes$$make(int32_t len$485, int32_t init$486);

struct $$moonbitlang$core$builtin$Array$3c$String$3e$* $$moonbitlang$core$builtin$Array$$make_uninit$0(
  int32_t len$484
);

int32_t $$moonbitlang$core$builtin$ArrayView$$at$0(
  struct $$moonbitlang$core$builtin$ArrayView$3c$Byte$3e$ self$483,
  int32_t index$482
);

moonbit_string_t* $FixedArray$$map$0(
  moonbit_bytes_t* self$476,
  struct $$3c$Bytes$3e$$3d$$3e$String* f$478
);

struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$moonbitlang$core$builtin$Array$$iter$0(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$475
);

struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$moonbitlang$core$builtin$ArrayView$$iter$0(
  struct $$moonbitlang$core$builtin$ArrayView$3c$$ZSeanYves$Doclint$src$core$Issue$3e$ self$473
);

struct $$ZSeanYves$Doclint$src$core$Issue* $ArrayView$$iter$7c$$ZSeanYves$Doclint$src$core$Issue$7c$$$2a$p$fn$2(
  struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* _env$1759
);

moonbit_string_t $$moonbitlang$core$builtin$Show$$String$$to_string(
  moonbit_string_t self$471
);

int32_t $$moonbitlang$core$builtin$Show$$Int$$output(
  int32_t self$470,
  struct $$moonbitlang$core$builtin$Logger logger$469
);

int32_t $$moonbitlang$core$builtin$Compare$$String$$compare(
  moonbit_string_t self$463,
  moonbit_string_t other$465
);

struct $StringView $$moonbitlang$core$builtin$ToStringView$$String$$to_string_view(
  moonbit_string_t self$461
);

moonbit_string_t $$moonbitlang$core$builtin$Show$$Char$$to_string(
  int32_t self$460
);

moonbit_string_t $moonbitlang$core$builtin$char_to_string(int32_t char$459);

struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* $String$$iter(
  moonbit_string_t self$453
);

int32_t $String$$iter$$2a$p$fn$1(
  struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* _env$1736
);

int32_t $String$$contains(
  moonbit_string_t self$450,
  struct $StringView str$451
);

int32_t $StringView$$contains(
  struct $StringView self$448,
  struct $StringView str$449
);

int32_t $$moonbitlang$core$builtin$Iter$$next$1(
  struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* self$446
);

struct $$ZSeanYves$Doclint$src$core$Issue* $$moonbitlang$core$builtin$Iter$$next$0(
  struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$444
);

int32_t $$moonbitlang$core$builtin$Array$$push$4(
  struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$* self$440,
  int32_t value$442
);

int32_t $$moonbitlang$core$builtin$Array$$push$3(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$437,
  struct $$ZSeanYves$Doclint$src$core$Issue* value$439
);

int32_t $$moonbitlang$core$builtin$Array$$push$2(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* self$434,
  struct $$ZSeanYves$Doclint$src$core$Page* value$436
);

int32_t $$moonbitlang$core$builtin$Array$$push$1(
  struct $$moonbitlang$core$builtin$Array$3c$Char$3e$* self$431,
  int32_t value$433
);

int32_t $$moonbitlang$core$builtin$Array$$push$0(
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* self$428,
  moonbit_string_t value$430
);

int32_t $$moonbitlang$core$builtin$Array$$realloc$4(
  struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$* self$426
);

int32_t $$moonbitlang$core$builtin$Array$$realloc$3(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$423
);

int32_t $$moonbitlang$core$builtin$Array$$realloc$2(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* self$420
);

int32_t $$moonbitlang$core$builtin$Array$$realloc$1(
  struct $$moonbitlang$core$builtin$Array$3c$Char$3e$* self$417
);

int32_t $$moonbitlang$core$builtin$Array$$realloc$0(
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* self$414
);

int32_t $$moonbitlang$core$builtin$Array$$resize_buffer$4(
  struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$* self$410,
  int32_t new_capacity$408
);

int32_t $$moonbitlang$core$builtin$Array$$resize_buffer$3(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$404,
  int32_t new_capacity$402
);

int32_t $$moonbitlang$core$builtin$Array$$resize_buffer$2(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* self$398,
  int32_t new_capacity$396
);

int32_t $$moonbitlang$core$builtin$Array$$resize_buffer$1(
  struct $$moonbitlang$core$builtin$Array$3c$Char$3e$* self$392,
  int32_t new_capacity$390
);

int32_t $$moonbitlang$core$builtin$Array$$resize_buffer$0(
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* self$386,
  int32_t new_capacity$384
);

int64_t $StringView$$find(
  struct $StringView self$382,
  struct $StringView str$381
);

int64_t $moonbitlang$core$builtin$brute_force_find(
  struct $StringView haystack$371,
  struct $StringView needle$373
);

int64_t $moonbitlang$core$builtin$boyer_moore_horspool_find(
  struct $StringView haystack$357,
  struct $StringView needle$359
);

int32_t $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_view(
  struct $$moonbitlang$core$builtin$StringBuilder* self$353,
  struct $StringView str$354
);

int32_t $String$$char_length_eq$inner(
  moonbit_string_t self$345,
  int32_t len$348,
  int32_t start_offset$352,
  int64_t end_offset$343
);

moonbit_string_t $String$$from_array(
  struct $$moonbitlang$core$builtin$ArrayView$3c$Char$3e$ chars$337
);

int32_t $$moonbitlang$core$builtin$ArrayView$$length$3(
  struct $$moonbitlang$core$builtin$ArrayView$3c$Byte$3e$ self$335
);

int32_t $$moonbitlang$core$builtin$ArrayView$$length$2(
  struct $$moonbitlang$core$builtin$ArrayView$3c$$ZSeanYves$Doclint$src$core$Issue$3e$ self$334
);

int32_t $$moonbitlang$core$builtin$ArrayView$$length$1(
  struct $$moonbitlang$core$builtin$ArrayView$3c$String$3e$ self$333
);

int32_t $$moonbitlang$core$builtin$ArrayView$$length$0(
  struct $$moonbitlang$core$builtin$ArrayView$3c$Char$3e$ self$332
);

struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* $$moonbitlang$core$builtin$Iter$$new$1(
  struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* f$331
);

struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$moonbitlang$core$builtin$Iter$$new$0(
  struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* f$330
);

int32_t $StringView$$unsafe_get(
  struct $StringView self$328,
  int32_t index$329
);

moonbit_string_t $Int$$to_string$inner(int32_t self$312, int32_t radix$311);

int32_t $moonbitlang$core$builtin$radix_count32(
  uint32_t value$305,
  int32_t radix$308
);

int32_t $moonbitlang$core$builtin$hex_count32(uint32_t value$303);

int32_t $moonbitlang$core$builtin$dec_count32(uint32_t value$302);

int32_t $moonbitlang$core$builtin$int_to_string_dec(
  uint16_t* buffer$292,
  uint32_t num$280,
  int32_t digit_start$283,
  int32_t total_len$282
);

int32_t $moonbitlang$core$builtin$int_to_string_generic(
  uint16_t* buffer$274,
  uint32_t num$268,
  int32_t digit_start$266,
  int32_t total_len$265,
  int32_t radix$270
);

int32_t $moonbitlang$core$builtin$int_to_string_hex(
  uint16_t* buffer$261,
  uint32_t num$257,
  int32_t digit_start$255,
  int32_t total_len$254
);

moonbit_string_t $$moonbitlang$core$builtin$Show$$$default_impl$$to_string$1(
  int32_t self$252
);

moonbit_string_t $$moonbitlang$core$builtin$Show$$$default_impl$$to_string$0(
  moonbit_string_t self$250
);

int32_t $StringView$$start_offset(struct $StringView self$248);

int32_t $StringView$$length(struct $StringView self$247);

moonbit_string_t $StringView$$data(struct $StringView self$246);

int32_t $$moonbitlang$core$builtin$Logger$$$default_impl$$write_substring$0(
  struct $$moonbitlang$core$builtin$StringBuilder* self$240,
  moonbit_string_t value$243,
  int32_t start$244,
  int32_t len$245
);

struct moonbit_result_3 $String$$sub(
  moonbit_string_t self$238,
  int64_t start$opt$236,
  int64_t end$239
);

struct moonbit_result_3 $String$$sub$inner(
  moonbit_string_t self$228,
  int32_t start$234,
  int64_t end$230
);

int32_t $$moonbitlang$core$builtin$Compare$$$default_impl$$op_ge$0(
  moonbit_string_t x$225,
  moonbit_string_t y$226
);

int32_t $$moonbitlang$core$builtin$Compare$$$default_impl$$op_le$0(
  moonbit_string_t x$223,
  moonbit_string_t y$224
);

int32_t $$moonbitlang$core$builtin$Compare$$UInt16$$compare(
  int32_t self$221,
  int32_t that$222
);

int32_t $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_string(
  struct $$moonbitlang$core$builtin$StringBuilder* self$219,
  moonbit_string_t str$220
);

int32_t $FixedArray$$blit_from_string(
  moonbit_bytes_t self$211,
  int32_t bytes_offset$206,
  moonbit_string_t str$213,
  int32_t str_offset$209,
  int32_t length$207
);

struct $$moonbitlang$core$builtin$SourceLocRepr* $$moonbitlang$core$builtin$SourceLocRepr$$parse(
  moonbit_string_t repr$128
);

moonbit_bytes_t $$moonbitlang$core$builtin$Array$$buffer$4(
  struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$* self$126
);

struct $$ZSeanYves$Doclint$src$core$Issue** $$moonbitlang$core$builtin$Array$$buffer$3(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$125
);

struct $$ZSeanYves$Doclint$src$core$Page** $$moonbitlang$core$builtin$Array$$buffer$2(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* self$124
);

int32_t* $$moonbitlang$core$builtin$Array$$buffer$1(
  struct $$moonbitlang$core$builtin$Array$3c$Char$3e$* self$123
);

moonbit_string_t* $$moonbitlang$core$builtin$Array$$buffer$0(
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* self$122
);

int32_t $UInt16$$unsafe_to_char(int32_t self$121);

int32_t $moonbitlang$core$builtin$code_point_of_surrogate_pair(
  int32_t leading$119,
  int32_t trailing$120
);

int32_t $UInt16$$is_trailing_surrogate(int32_t self$118);

int32_t $UInt16$$is_leading_surrogate(int32_t self$117);

int32_t $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_char(
  struct $$moonbitlang$core$builtin$StringBuilder* self$114,
  int32_t ch$116
);

int32_t $$moonbitlang$core$builtin$StringBuilder$$grow_if_necessary(
  struct $$moonbitlang$core$builtin$StringBuilder* self$109,
  int32_t required$110
);

int32_t $$moonbitlang$core$builtin$Default$$Byte$$default();

int32_t $FixedArray$$set_utf16le_char(
  moonbit_bytes_t self$103,
  int32_t offset$104,
  int32_t value$102
);

int32_t $UInt$$to_byte(uint32_t self$100);

uint32_t $Char$$to_uint(int32_t self$99);

moonbit_string_t $$moonbitlang$core$builtin$StringBuilder$$to_string(
  struct $$moonbitlang$core$builtin$StringBuilder* self$98
);

moonbit_string_t $Bytes$$to_unchecked_string$inner(
  moonbit_bytes_t self$93,
  int32_t offset$97,
  int64_t length$95
);

#define $moonbitlang$core$builtin$unsafe_sub_string moonbit_unsafe_bytes_sub_string

struct $$moonbitlang$core$builtin$StringBuilder* $$moonbitlang$core$builtin$StringBuilder$$new$inner(
  int32_t size_hint$90
);

int32_t $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$4(
  moonbit_bytes_t dst$84,
  int32_t dst_offset$85,
  moonbit_bytes_t src$86,
  int32_t src_offset$87,
  int32_t len$88
);

int32_t $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$3(
  struct $$ZSeanYves$Doclint$src$core$Issue** dst$79,
  int32_t dst_offset$80,
  struct $$ZSeanYves$Doclint$src$core$Issue** src$81,
  int32_t src_offset$82,
  int32_t len$83
);

int32_t $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$2(
  struct $$ZSeanYves$Doclint$src$core$Page** dst$74,
  int32_t dst_offset$75,
  struct $$ZSeanYves$Doclint$src$core$Page** src$76,
  int32_t src_offset$77,
  int32_t len$78
);

int32_t $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$1(
  int32_t* dst$69,
  int32_t dst_offset$70,
  int32_t* src$71,
  int32_t src_offset$72,
  int32_t len$73
);

int32_t $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$0(
  moonbit_string_t* dst$64,
  int32_t dst_offset$65,
  moonbit_string_t* src$66,
  int32_t src_offset$67,
  int32_t len$68
);

int32_t $FixedArray$$unsafe_blit$5(
  moonbit_bytes_t dst$55,
  int32_t dst_offset$57,
  moonbit_bytes_t src$56,
  int32_t src_offset$58,
  int32_t len$60
);

int32_t $FixedArray$$unsafe_blit$4(
  struct $$ZSeanYves$Doclint$src$core$Issue** dst$46,
  int32_t dst_offset$48,
  struct $$ZSeanYves$Doclint$src$core$Issue** src$47,
  int32_t src_offset$49,
  int32_t len$51
);

int32_t $FixedArray$$unsafe_blit$3(
  struct $$ZSeanYves$Doclint$src$core$Page** dst$37,
  int32_t dst_offset$39,
  struct $$ZSeanYves$Doclint$src$core$Page** src$38,
  int32_t src_offset$40,
  int32_t len$42
);

int32_t $FixedArray$$unsafe_blit$2(
  int32_t* dst$28,
  int32_t dst_offset$30,
  int32_t* src$29,
  int32_t src_offset$31,
  int32_t len$33
);

int32_t $FixedArray$$unsafe_blit$1(
  moonbit_bytes_t dst$19,
  int32_t dst_offset$21,
  moonbit_bytes_t src$20,
  int32_t src_offset$22,
  int32_t len$24
);

int32_t $FixedArray$$unsafe_blit$0(
  moonbit_string_t* dst$10,
  int32_t dst_offset$12,
  moonbit_string_t* src$11,
  int32_t src_offset$13,
  int32_t len$15
);

int32_t $moonbitlang$core$builtin$abort$2(
  moonbit_string_t string$8,
  moonbit_string_t loc$9
);

int32_t $moonbitlang$core$builtin$abort$1(
  moonbit_string_t string$6,
  moonbit_string_t loc$7
);

int32_t $moonbitlang$core$builtin$abort$0(
  moonbit_string_t string$4,
  moonbit_string_t loc$5
);

int32_t $moonbitlang$core$abort$abort$2(moonbit_string_t msg$3);

int32_t $moonbitlang$core$abort$abort$1(moonbit_string_t msg$2);

int32_t $moonbitlang$core$abort$abort$0(moonbit_string_t msg$1);

int32_t $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_char$dyncall_as_$moonbitlang$core$builtin$Logger(
  void* _obj_ptr$1227,
  int32_t _param$1226
);

int32_t $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_view$dyncall_as_$moonbitlang$core$builtin$Logger(
  void* _obj_ptr$1224,
  struct $StringView _param$1223
);

int32_t $$moonbitlang$core$builtin$Logger$$$default_impl$$write_substring$dyncall_as_$moonbitlang$core$builtin$Logger$0(
  void* _obj_ptr$1221,
  moonbit_string_t _param$1218,
  int32_t _param$1219,
  int32_t _param$1220
);

int32_t $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_string$dyncall_as_$moonbitlang$core$builtin$Logger(
  void* _obj_ptr$1216,
  moonbit_string_t _param$1215
);

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$$check$dyncall_as_$ZSeanYves$Doclint$src$core$Rule(
  void* _obj_ptr$1213,
  int32_t _param$1211,
  struct $$ZSeanYves$Doclint$src$core$Document* _param$1212
);

moonbit_string_t $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$$id$dyncall_as_$ZSeanYves$Doclint$src$core$Rule(
  void* _obj_ptr$1209
);

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$$check$dyncall_as_$ZSeanYves$Doclint$src$core$Rule(
  void* _obj_ptr$1207,
  int32_t _param$1205,
  struct $$ZSeanYves$Doclint$src$core$Document* _param$1206
);

moonbit_string_t $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$$id$dyncall_as_$ZSeanYves$Doclint$src$core$Rule(
  void* _obj_ptr$1203
);

int32_t moonbitlang_x_fs_fwrite_ffi(
  moonbit_bytes_t $0,
  int32_t $1,
  int32_t $2,
  void* $3
);

moonbit_bytes_t moonbitlang_x_fs_get_error_message();

int32_t moonbitlang_x_fs_fflush_ffi(void* $0);

void* moonbitlang_x_fs_fopen_ffi(moonbit_bytes_t $0, moonbit_bytes_t $1);

int32_t moonbitlang_x_fs_fread_ffi(
  moonbit_bytes_t $0,
  int32_t $1,
  int32_t $2,
  void* $3
);

int32_t moonbitlang_x_fs_create_dir_ffi(moonbit_bytes_t $0);

int32_t moonbitlang_x_fs_ftell_ffi(void* $0);

int32_t moonbitlang_x_fs_is_dir_ffi(moonbit_bytes_t $0);

int32_t moonbitlang_x_fs_stat_ffi(moonbit_bytes_t $0);

int32_t moonbitlang_x_fs_fseek_ffi(void* $0, int32_t $1, int32_t $2);

int32_t moonbitlang_x_fs_fclose_ffi(void* $0);

void exit(int32_t $0);

int32_t moonbitlang_x_fs_is_null(void* $0);

struct { int32_t rc; uint32_t meta; uint16_t const data[52]; 
} const moonbit_string_literal_130 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 51), 
    64, 109, 111, 111, 110, 98, 105, 116, 108, 97, 110, 103, 47, 99, 
    111, 114, 101, 47, 98, 117, 105, 108, 116, 105, 110, 58, 97, 114, 
    114, 97, 121, 118, 105, 101, 119, 46, 109, 98, 116, 58, 49, 50, 52, 
    58, 53, 45, 49, 50, 54, 58, 54, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[1]; 
} const moonbit_string_literal_26 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 0), 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[6]; 
} const moonbit_string_literal_108 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 5), 
    38, 35, 51, 57, 59, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[10]; 
} const moonbit_string_literal_7 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 9), 
    119, 114, 105, 116, 116, 101, 110, 58, 32, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[56]; 
} const moonbit_string_literal_63 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 55), 
    46, 98, 97, 100, 103, 101, 46, 101, 114, 114, 111, 114, 123, 98, 
    111, 114, 100, 101, 114, 45, 99, 111, 108, 111, 114, 58, 35, 102, 
    51, 99, 50, 99, 50, 59, 32, 98, 97, 99, 107, 103, 114, 111, 117, 
    110, 100, 58, 35, 102, 102, 102, 53, 102, 53, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[5]; 
} const moonbit_string_literal_126 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 4), 
    87, 97, 114, 110, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[3]; 
} const moonbit_string_literal_122 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 2), 
    92, 110, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[53]; 
} const moonbit_string_literal_74 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 52), 
    60, 100, 105, 118, 32, 99, 108, 97, 115, 115, 61, 34, 98, 111, 120, 
    34, 62, 60, 100, 105, 118, 32, 99, 108, 97, 115, 115, 61, 34, 109, 
    117, 116, 101, 100, 34, 62, 69, 114, 114, 111, 114, 60, 47, 100, 
    105, 118, 62, 60, 100, 105, 118, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[5]; 
} const moonbit_string_literal_146 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 4), 
    45, 45, 105, 110, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[8]; 
} const moonbit_string_literal_29 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 7), 
    39029, 25968, 19981, 31526, 21512, 35201, 27714, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[15]; 
} const moonbit_string_literal_154 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 14),
    25991, 26723, 32, 74, 83, 79, 78, 32, 35299, 26512, 22833, 36133, 
    58, 32, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_41 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 48, 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[124]; 
} const moonbit_string_literal_62 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 123), 
    46, 98, 97, 100, 103, 101, 123, 100, 105, 115, 112, 108, 97, 121, 
    58, 105, 110, 108, 105, 110, 101, 45, 98, 108, 111, 99, 107, 59, 
    32, 112, 97, 100, 100, 105, 110, 103, 58, 50, 112, 120, 32, 56, 112, 
    120, 59, 32, 98, 111, 114, 100, 101, 114, 45, 114, 97, 100, 105, 
    117, 115, 58, 57, 57, 57, 112, 120, 59, 32, 102, 111, 110, 116, 45, 
    115, 105, 122, 101, 58, 49, 50, 112, 120, 59, 32, 98, 111, 114, 100, 
    101, 114, 58, 49, 112, 120, 32, 115, 111, 108, 105, 100, 32, 35, 
    101, 101, 101, 59, 32, 98, 97, 99, 107, 103, 114, 111, 117, 110, 
    100, 58, 35, 102, 102, 102, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[50]; 
} const moonbit_string_literal_132 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 49), 
    64, 109, 111, 111, 110, 98, 105, 116, 108, 97, 110, 103, 47, 99, 
    111, 114, 101, 47, 98, 117, 105, 108, 116, 105, 110, 58, 115, 116, 
    114, 105, 110, 103, 46, 109, 98, 116, 58, 52, 50, 52, 58, 57, 45, 
    52, 50, 52, 58, 52, 48, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[11]; 
} const moonbit_string_literal_32 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 10), 
    99, 114, 101, 97, 116, 101, 100, 95, 97, 116, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[11]; 
} const moonbit_string_literal_110 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 10), 
    34, 105, 115, 115, 117, 101, 115, 34, 58, 91, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[6]; 
} const moonbit_string_literal_92 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 5), 
    112, 97, 103, 101, 32, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[23]; 
} const moonbit_string_literal_20 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 22), 
    102, 97, 105, 108, 101, 100, 32, 116, 111, 32, 99, 114, 101, 97, 
    116, 101, 32, 100, 105, 114, 58, 32, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[16]; 
} const moonbit_string_literal_17 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 15), 
    32, 32, 45, 45, 111, 117, 116, 100, 105, 114, 32, 111, 117, 116, 
    10, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[18]; 
} const moonbit_string_literal_71 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 17), 
    60, 100, 105, 118, 32, 99, 108, 97, 115, 115, 61, 34, 107, 112, 105, 
    34, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[109]; 
} const moonbit_string_literal_52 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 108), 
    98, 111, 100, 121, 123, 102, 111, 110, 116, 45, 102, 97, 109, 105, 
    108, 121, 58, 115, 121, 115, 116, 101, 109, 45, 117, 105, 44, 45, 
    97, 112, 112, 108, 101, 45, 115, 121, 115, 116, 101, 109, 44, 83, 
    101, 103, 111, 101, 32, 85, 73, 44, 82, 111, 98, 111, 116, 111, 44, 
    65, 114, 105, 97, 108, 59, 32, 109, 97, 114, 103, 105, 110, 58, 48, 
    59, 32, 112, 97, 100, 100, 105, 110, 103, 58, 50, 52, 112, 120, 59, 
    32, 98, 97, 99, 107, 103, 114, 111, 117, 110, 100, 58, 35, 102, 97, 
    102, 97, 102, 97, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[8]; 
} const moonbit_string_literal_144 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 7), 
    45, 45, 114, 117, 108, 101, 115, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[5]; 
} const moonbit_string_literal_125 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 4), 
    73, 110, 102, 111, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[5]; 
} const moonbit_string_literal_112 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 4), 
    110, 117, 108, 108, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[7]; 
} const moonbit_string_literal_77 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 6), 
    60, 47, 100, 105, 118, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_94 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 41, 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[32]; 
} const moonbit_string_literal_84 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 31), 
    60, 100, 105, 118, 32, 99, 108, 97, 115, 115, 61, 34, 109, 117, 116, 
    101, 100, 34, 62, 40, 110, 111, 110, 101, 41, 60, 47, 100, 105, 118, 
    62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[52]; 
} const moonbit_string_literal_75 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 51), 
    60, 100, 105, 118, 32, 99, 108, 97, 115, 115, 61, 34, 98, 111, 120, 
    34, 62, 60, 100, 105, 118, 32, 99, 108, 97, 115, 115, 61, 34, 109, 
    117, 116, 101, 100, 34, 62, 87, 97, 114, 110, 60, 47, 100, 105, 118, 
    62, 60, 100, 105, 118, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[14]; 
} const moonbit_string_literal_68 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 13), 
    60, 47, 104, 101, 97, 100, 62, 60, 98, 111, 100, 121, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_36 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 32, 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[46]; 
} const moonbit_string_literal_54 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 45), 
    46, 107, 112, 105, 123, 100, 105, 115, 112, 108, 97, 121, 58, 102, 
    108, 101, 120, 59, 32, 103, 97, 112, 58, 49, 50, 112, 120, 59, 32, 
    102, 108, 101, 120, 45, 119, 114, 97, 112, 58, 119, 114, 97, 112, 
    59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[37]; 
} const moonbit_string_literal_103 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 36), 
    60, 115, 112, 97, 110, 32, 99, 108, 97, 115, 115, 61, 34, 98, 97, 
    100, 103, 101, 32, 105, 110, 102, 111, 34, 62, 73, 110, 102, 111, 
    60, 47, 115, 112, 97, 110, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[8]; 
} const moonbit_string_literal_89 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 7), 
    60, 116, 98, 111, 100, 121, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[3]; 
} const moonbit_string_literal_120 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 2), 
    92, 34, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_18 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 47, 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[67]; 
} const moonbit_string_literal_87 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 66), 
    60, 116, 104, 62, 114, 117, 108, 101, 95, 105, 100, 60, 47, 116, 
    104, 62, 60, 116, 104, 62, 115, 101, 118, 101, 114, 105, 116, 121, 
    60, 47, 116, 104, 62, 60, 116, 104, 62, 109, 101, 115, 115, 97, 103, 
    101, 60, 47, 116, 104, 62, 60, 116, 104, 62, 108, 111, 99, 97, 116, 
    105, 111, 110, 60, 47, 116, 104, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[24]; 
} const moonbit_string_literal_70 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 23), 
    60, 104, 49, 62, 68, 111, 99, 108, 105, 110, 116, 32, 82, 101, 112, 
    111, 114, 116, 60, 47, 104, 49, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[93]; 
} const moonbit_string_literal_59 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 92), 
    116, 104, 44, 116, 100, 123, 112, 97, 100, 100, 105, 110, 103, 58, 
    49, 48, 112, 120, 32, 49, 50, 112, 120, 59, 32, 98, 111, 114, 100, 
    101, 114, 45, 98, 111, 116, 116, 111, 109, 58, 49, 112, 120, 32, 
    115, 111, 108, 105, 100, 32, 35, 101, 101, 101, 59, 32, 116, 101, 
    120, 116, 45, 97, 108, 105, 103, 110, 58, 108, 101, 102, 116, 59, 
    32, 118, 101, 114, 116, 105, 99, 97, 108, 45, 97, 108, 105, 103, 
    110, 58, 116, 111, 112, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[26]; 
} const moonbit_string_literal_145 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 25), 
    109, 105, 115, 115, 105, 110, 103, 32, 118, 97, 108, 117, 101, 32, 
    102, 111, 114, 32, 45, 45, 114, 117, 108, 101, 115, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_43 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 92, 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[11]; 
} const moonbit_string_literal_96 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 10), 
    60, 116, 100, 62, 60, 99, 111, 100, 101, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[6]; 
} const moonbit_string_literal_80 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 5), 
    73, 110, 102, 111, 115, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[3]; 
} const moonbit_string_literal_124 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 2), 
    92, 116, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[3]; 
} const moonbit_string_literal_123 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 2), 
    92, 114, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[6]; 
} const moonbit_string_literal_99 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 5), 
    60, 47, 116, 100, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_46 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 110, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_40 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 45, 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[28]; 
} const moonbit_string_literal_19 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 27), 
    112, 97, 116, 104, 32, 101, 120, 105, 115, 116, 115, 32, 98, 117, 
    116, 32, 110, 111, 116, 32, 97, 32, 100, 105, 114, 58, 32, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[7]; 
} const moonbit_string_literal_142 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 6), 
    45, 45, 104, 101, 108, 112, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[5]; 
} const moonbit_string_literal_104 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 4), 
    38, 108, 116, 59, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[5]; 
} const moonbit_string_literal_35 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 4), 
    116, 101, 120, 116, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[18]; 
} const moonbit_string_literal_28 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 17), 
    116, 104, 101, 115, 105, 115, 46, 112, 97, 103, 101, 95, 108, 105, 
    109, 105, 116, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[39]; 
} const moonbit_string_literal_101 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 38), 
    60, 115, 112, 97, 110, 32, 99, 108, 97, 115, 115, 61, 34, 98, 97, 
    100, 103, 101, 32, 101, 114, 114, 111, 114, 34, 62, 69, 114, 114, 
    111, 114, 60, 47, 115, 112, 97, 110, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[8]; 
} const moonbit_string_literal_27 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 7), 
    32570, 23569, 24517, 38656, 20869, 23481, 65306, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[68]; 
} const moonbit_string_literal_49 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 67), 
    60, 109, 101, 116, 97, 32, 110, 97, 109, 101, 61, 34, 118, 105, 101, 
    119, 112, 111, 114, 116, 34, 32, 99, 111, 110, 116, 101, 110, 116, 
    61, 34, 119, 105, 100, 116, 104, 61, 100, 101, 118, 105, 99, 101, 
    45, 119, 105, 100, 116, 104, 44, 105, 110, 105, 116, 105, 97, 108, 
    45, 115, 99, 97, 108, 101, 61, 49, 34, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[23]; 
} const moonbit_string_literal_24 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 22), 
    85, 110, 115, 117, 112, 112, 111, 114, 116, 101, 100, 32, 101, 110, 
    99, 111, 100, 105, 110, 103, 58, 32, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[3]; 
} const moonbit_string_literal_5 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 2), 
    21512, 21516, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[34]; 
} const moonbit_string_literal_16 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 33), 
    32, 32, 45, 45, 105, 110, 32, 116, 101, 115, 116, 95, 103, 111, 108, 
    100, 101, 110, 47, 99, 97, 115, 101, 50, 46, 105, 110, 46, 106, 115, 
    111, 110, 10, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[7]; 
} const moonbit_string_literal_139 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 6), 
    116, 104, 101, 115, 105, 115, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[12]; 
} const moonbit_string_literal_119 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 11), 
    34, 115, 112, 97, 110, 95, 101, 110, 100, 34, 58, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[14]; 
} const moonbit_string_literal_23 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 13), 
    114, 101, 97, 100, 32, 102, 97, 105, 108, 101, 100, 58, 32, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[8]; 
} const moonbit_string_literal_8 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 7), 
    85, 115, 97, 103, 101, 58, 10, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[8]; 
} const moonbit_string_literal_117 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 7), 
    34, 112, 97, 103, 101, 34, 58, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[9]; 
} const moonbit_string_literal_90 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 8), 
    60, 47, 116, 98, 111, 100, 121, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[41]; 
} const moonbit_string_literal_57 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 40), 
    104, 50, 123, 102, 111, 110, 116, 45, 115, 105, 122, 101, 58, 49, 
    54, 112, 120, 59, 32, 109, 97, 114, 103, 105, 110, 58, 49, 54, 112, 
    120, 32, 48, 32, 56, 112, 120, 32, 48, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[6]; 
} const moonbit_string_literal_30 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 5), 
    116, 105, 116, 108, 101, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_109 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 123, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[8]; 
} const moonbit_string_literal_85 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 7), 
    60, 116, 97, 98, 108, 101, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[37]; 
} const moonbit_string_literal_102 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 36), 
    60, 115, 112, 97, 110, 32, 99, 108, 97, 115, 115, 61, 34, 98, 97, 
    100, 103, 101, 32, 119, 97, 114, 110, 34, 62, 87, 97, 114, 110, 60, 
    47, 115, 112, 97, 110, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[53]; 
} const moonbit_string_literal_72 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 52), 
    60, 100, 105, 118, 32, 99, 108, 97, 115, 115, 61, 34, 98, 111, 120, 
    34, 62, 60, 100, 105, 118, 32, 99, 108, 97, 115, 115, 61, 34, 109, 
    117, 116, 101, 100, 34, 62, 84, 111, 116, 97, 108, 60, 47, 100, 105, 
    118, 62, 60, 100, 105, 118, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[6]; 
} const moonbit_string_literal_127 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 5), 
    69, 114, 114, 111, 114, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[122]; 
} const moonbit_string_literal_58 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 121), 
    116, 97, 98, 108, 101, 123, 119, 105, 100, 116, 104, 58, 49, 48, 
    48, 37, 59, 32, 98, 111, 114, 100, 101, 114, 45, 99, 111, 108, 108, 
    97, 112, 115, 101, 58, 99, 111, 108, 108, 97, 112, 115, 101, 59, 
    32, 98, 97, 99, 107, 103, 114, 111, 117, 110, 100, 58, 35, 102, 102, 
    102, 59, 32, 98, 111, 114, 100, 101, 114, 58, 49, 112, 120, 32, 115, 
    111, 108, 105, 100, 32, 35, 101, 101, 101, 59, 32, 98, 111, 114, 
    100, 101, 114, 45, 114, 97, 100, 105, 117, 115, 58, 49, 50, 112, 
    120, 59, 32, 111, 118, 101, 114, 102, 108, 111, 119, 58, 104, 105, 
    100, 100, 101, 110, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[12]; 
} const moonbit_string_literal_152 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 11), 
    114, 101, 112, 111, 114, 116, 46, 106, 115, 111, 110, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_45 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 114, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[9]; 
} const moonbit_string_literal_6 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 8), 
    98, 97, 100, 32, 106, 115, 111, 110, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[3]; 
} const moonbit_string_literal_93 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 2), 
    32, 91, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_0 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 44, 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[23]; 
} const moonbit_string_literal_131 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 22), 
    105, 110, 118, 97, 108, 105, 100, 32, 115, 117, 114, 114, 111, 103, 
    97, 116, 101, 32, 112, 97, 105, 114, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[36]; 
} const moonbit_string_literal_66 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 35), 
    46, 109, 117, 116, 101, 100, 123, 99, 111, 108, 111, 114, 58, 35, 
    54, 54, 54, 59, 32, 102, 111, 110, 116, 45, 115, 105, 122, 101, 58, 
    49, 50, 112, 120, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_44 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 116, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[7]; 
} const moonbit_string_literal_138 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 6), 
    10, 32, 32, 97, 116, 32, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[30]; 
} const moonbit_string_literal_50 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 29), 
    60, 116, 105, 116, 108, 101, 62, 68, 111, 99, 108, 105, 110, 116, 
    32, 82, 101, 112, 111, 114, 116, 60, 47, 116, 105, 116, 108, 101, 
    62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[31]; 
} const moonbit_string_literal_133 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 30), 
    114, 97, 100, 105, 120, 32, 109, 117, 115, 116, 32, 98, 101, 32, 
    98, 101, 116, 119, 101, 101, 110, 32, 50, 32, 97, 110, 100, 32, 51, 
    54, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[5]; 
} const moonbit_string_literal_95 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 4), 
    60, 116, 114, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[16]; 
} const moonbit_string_literal_47 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 15), 
    60, 33, 100, 111, 99, 116, 121, 112, 101, 32, 104, 116, 109, 108, 
    62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[27]; 
} const moonbit_string_literal_149 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 26), 
    109, 105, 115, 115, 105, 110, 103, 32, 118, 97, 108, 117, 101, 32, 
    102, 111, 114, 32, 45, 45, 111, 117, 116, 100, 105, 114, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[23]; 
} const moonbit_string_literal_147 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 22), 
    109, 105, 115, 115, 105, 110, 103, 32, 118, 97, 108, 117, 101, 32, 
    102, 111, 114, 32, 45, 45, 105, 110, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[139]; 
} const moonbit_string_literal_53 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 138), 
    46, 99, 97, 114, 100, 123, 98, 97, 99, 107, 103, 114, 111, 117, 110, 
    100, 58, 35, 102, 102, 102, 59, 32, 98, 111, 114, 100, 101, 114, 
    58, 49, 112, 120, 32, 115, 111, 108, 105, 100, 32, 35, 101, 101, 
    101, 59, 32, 98, 111, 114, 100, 101, 114, 45, 114, 97, 100, 105, 
    117, 115, 58, 49, 50, 112, 120, 59, 32, 112, 97, 100, 100, 105, 110, 
    103, 58, 49, 54, 112, 120, 59, 32, 109, 97, 114, 103, 105, 110, 45, 
    98, 111, 116, 116, 111, 109, 58, 49, 54, 112, 120, 59, 32, 98, 111, 
    120, 45, 115, 104, 97, 100, 111, 119, 58, 48, 32, 49, 112, 120, 32, 
    50, 112, 120, 32, 114, 103, 98, 97, 40, 48, 44, 48, 44, 48, 44, 46, 
    48, 52, 41, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[7]; 
} const moonbit_string_literal_107 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 6), 
    38, 113, 117, 111, 116, 59, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[9]; 
} const moonbit_string_literal_79 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 8), 
    87, 97, 114, 110, 105, 110, 103, 115, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[4]; 
} const moonbit_string_literal_141 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 3), 
    111, 117, 116, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[6]; 
} const moonbit_string_literal_100 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 5), 
    60, 47, 116, 114, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[8]; 
} const moonbit_string_literal_1 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 7), 
    34, 112, 97, 103, 101, 115, 34, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[14]; 
} const moonbit_string_literal_118 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 13), 
    34, 115, 112, 97, 110, 95, 115, 116, 97, 114, 116, 34, 58, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[8]; 
} const moonbit_string_literal_51 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 7), 
    60, 115, 116, 121, 108, 101, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[9]; 
} const moonbit_string_literal_148 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 8), 
    45, 45, 111, 117, 116, 100, 105, 114, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[5]; 
} const moonbit_string_literal_21 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 4), 
    117, 116, 102, 56, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[49]; 
} const moonbit_string_literal_137 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 48), 
    64, 109, 111, 111, 110, 98, 105, 116, 108, 97, 110, 103, 47, 99, 
    111, 114, 101, 47, 98, 117, 105, 108, 116, 105, 110, 58, 98, 121, 
    116, 101, 115, 46, 109, 98, 116, 58, 50, 57, 56, 58, 53, 45, 50, 
    57, 56, 58, 51, 49, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[26]; 
} const moonbit_string_literal_140 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 25), 
    116, 101, 115, 116, 95, 103, 111, 108, 100, 101, 110, 47, 99, 97, 
    115, 101, 50, 46, 105, 110, 46, 106, 115, 111, 110, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[37]; 
} const moonbit_string_literal_135 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 36), 
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 97, 98, 99, 100, 101, 102, 
    103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 
    116, 117, 118, 119, 120, 121, 122, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[11]; 
} const moonbit_string_literal_11 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 10), 
    69, 120, 97, 109, 112, 108, 101, 115, 58, 10, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[14]; 
} const moonbit_string_literal_151 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 13), 
    117, 110, 107, 110, 111, 119, 110, 32, 97, 114, 103, 58, 32, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[43]; 
} const moonbit_string_literal_128 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 42), 
    105, 110, 100, 101, 120, 32, 111, 117, 116, 32, 111, 102, 32, 98, 
    111, 117, 110, 100, 115, 58, 32, 116, 104, 101, 32, 108, 101, 110, 
    32, 105, 115, 32, 102, 114, 111, 109, 32, 48, 32, 116, 111, 32, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[5]; 
} const moonbit_string_literal_105 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 4), 
    38, 103, 116, 59, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[9]; 
} const moonbit_string_literal_156 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 8), 
    99, 111, 110, 116, 114, 97, 99, 116, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_10 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 10, 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[6]; 
} const moonbit_string_literal_34 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 5), 
    105, 110, 100, 101, 120, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[3]; 
} const moonbit_string_literal_3 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 2), 
    25688, 35201, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[18]; 
} const moonbit_string_literal_15 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 17), 
    32, 32, 45, 45, 114, 117, 108, 101, 115, 32, 116, 104, 101, 115, 
    105, 115, 10, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[7]; 
} const moonbit_string_literal_78 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 6), 
    69, 114, 114, 111, 114, 115, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[39]; 
} const moonbit_string_literal_56 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 38), 
    104, 49, 123, 102, 111, 110, 116, 45, 115, 105, 122, 101, 58, 50, 
    48, 112, 120, 59, 32, 109, 97, 114, 103, 105, 110, 58, 48, 32, 48, 
    32, 49, 50, 112, 120, 32, 48, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[57]; 
} const moonbit_string_literal_12 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 56), 
    32, 32, 100, 111, 99, 108, 105, 110, 116, 32, 45, 45, 114, 117, 108, 
    101, 115, 32, 116, 104, 101, 115, 105, 115, 32, 45, 45, 105, 110, 
    32, 116, 101, 115, 116, 95, 103, 111, 108, 100, 101, 110, 47, 99, 
    97, 115, 101, 49, 46, 105, 110, 46, 106, 115, 111, 110, 10, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[19]; 
} const moonbit_string_literal_129 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 18), 
    32, 98, 117, 116, 32, 116, 104, 101, 32, 105, 110, 100, 101, 120, 
    32, 105, 115, 32, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_111 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[55]; 
} const moonbit_string_literal_64 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 54), 
    46, 98, 97, 100, 103, 101, 46, 119, 97, 114, 110, 123, 98, 111, 114, 
    100, 101, 114, 45, 99, 111, 108, 111, 114, 58, 35, 102, 50, 100, 
    54, 97, 53, 59, 32, 98, 97, 99, 107, 103, 114, 111, 117, 110, 100, 
    58, 35, 102, 102, 102, 97, 102, 50, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[11]; 
} const moonbit_string_literal_115 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 10), 
    34, 109, 101, 115, 115, 97, 103, 101, 34, 58, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[55]; 
} const moonbit_string_literal_65 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 54), 
    46, 98, 97, 100, 103, 101, 46, 105, 110, 102, 111, 123, 98, 111, 
    114, 100, 101, 114, 45, 99, 111, 108, 111, 114, 58, 35, 98, 56, 100, 
    52, 102, 102, 59, 32, 98, 97, 99, 107, 103, 114, 111, 117, 110, 100, 
    58, 35, 102, 51, 102, 56, 102, 102, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[5]; 
} const moonbit_string_literal_98 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 4), 
    60, 116, 100, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[9]; 
} const moonbit_string_literal_67 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 8), 
    60, 47, 115, 116, 121, 108, 101, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[33]; 
} const moonbit_string_literal_25 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 32), 
    44, 32, 111, 110, 108, 121, 32, 117, 116, 102, 56, 32, 105, 115, 
    32, 115, 117, 112, 112, 111, 114, 116, 101, 100, 32, 102, 111, 114, 
    32, 110, 111, 119, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[13]; 
} const moonbit_string_literal_97 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 12), 
    60, 47, 99, 111, 100, 101, 62, 60, 47, 116, 100, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[11]; 
} const moonbit_string_literal_113 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 10), 
    34, 114, 117, 108, 101, 95, 105, 100, 34, 58, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_38 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 9, 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[23]; 
} const moonbit_string_literal_2 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 22), 
    98, 97, 115, 105, 99, 46, 114, 101, 113, 117, 105, 114, 101, 95, 
    97, 98, 115, 116, 114, 97, 99, 116, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[11]; 
} const moonbit_string_literal_155 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 10), 
    99, 111, 114, 101, 46, 112, 97, 114, 115, 101, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[15]; 
} const moonbit_string_literal_22 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 14), 
    119, 114, 105, 116, 101, 32, 102, 97, 105, 108, 101, 100, 58, 32, 
    0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[14]; 
} const moonbit_string_literal_88 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 13), 
    60, 47, 116, 114, 62, 60, 47, 116, 104, 101, 97, 100, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[6]; 
} const moonbit_string_literal_83 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 5), 
    60, 47, 104, 50, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_42 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 57, 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[13]; 
} const moonbit_string_literal_73 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 12), 
    60, 47, 100, 105, 118, 62, 60, 47, 100, 105, 118, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[12]; 
} const moonbit_string_literal_116 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 11), 
    34, 108, 111, 99, 97, 116, 105, 111, 110, 34, 58, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[38]; 
} const moonbit_string_literal_61 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 37), 
    116, 114, 58, 108, 97, 115, 116, 45, 99, 104, 105, 108, 100, 32, 
    116, 100, 123, 98, 111, 114, 100, 101, 114, 45, 98, 111, 116, 116, 
    111, 109, 58, 110, 111, 110, 101, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[18]; 
} const moonbit_string_literal_136 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 17), 
    67, 104, 97, 114, 32, 111, 117, 116, 32, 111, 102, 32, 114, 97, 110, 
    103, 101, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[9]; 
} const moonbit_string_literal_91 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 8), 
    60, 47, 116, 97, 98, 108, 101, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[12]; 
} const moonbit_string_literal_153 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 11), 
    114, 101, 112, 111, 114, 116, 46, 104, 116, 109, 108, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[7]; 
} const moonbit_string_literal_31 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 6), 
    97, 117, 116, 104, 111, 114, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[41]; 
} const moonbit_string_literal_60 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 40), 
    116, 104, 123, 98, 97, 99, 107, 103, 114, 111, 117, 110, 100, 58, 
    35, 102, 54, 102, 55, 102, 57, 59, 32, 102, 111, 110, 116, 45, 119, 
    101, 105, 103, 104, 116, 58, 54, 48, 48, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[53]; 
} const moonbit_string_literal_134 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 52), 
    64, 109, 111, 111, 110, 98, 105, 116, 108, 97, 110, 103, 47, 99, 
    111, 114, 101, 47, 98, 117, 105, 108, 116, 105, 110, 58, 116, 111, 
    95, 115, 116, 114, 105, 110, 103, 46, 109, 98, 116, 58, 50, 50, 52, 
    58, 53, 45, 50, 50, 52, 58, 52, 52, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[35]; 
} const moonbit_string_literal_48 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 34), 
    60, 104, 116, 109, 108, 62, 60, 104, 101, 97, 100, 62, 60, 109, 101, 
    116, 97, 32, 99, 104, 97, 114, 115, 101, 116, 61, 34, 117, 116, 102, 
    45, 56, 34, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_37 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 13, 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_33 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 93, 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[72]; 
} const moonbit_string_literal_13 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 71), 
    32, 32, 100, 111, 99, 108, 105, 110, 116, 32, 45, 45, 114, 117, 108, 
    101, 115, 32, 99, 111, 110, 116, 114, 97, 99, 116, 32, 45, 45, 105, 
    110, 32, 116, 101, 115, 116, 95, 103, 111, 108, 100, 101, 110, 47, 
    99, 97, 115, 101, 50, 46, 105, 110, 46, 106, 115, 111, 110, 32, 45, 
    45, 111, 117, 116, 100, 105, 114, 32, 111, 117, 116, 10, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[19]; 
} const moonbit_string_literal_69 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 18), 
    60, 100, 105, 118, 32, 99, 108, 97, 115, 115, 61, 34, 99, 97, 114, 
    100, 34, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[11]; 
} const moonbit_string_literal_14 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 10), 
    68, 101, 102, 97, 117, 108, 116, 115, 58, 10, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[6]; 
} const moonbit_string_literal_106 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 5), 
    38, 97, 109, 112, 59, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[5]; 
} const moonbit_string_literal_82 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 4), 
    60, 104, 50, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[2]; 
} const moonbit_string_literal_39 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 1), 34, 0};

struct { int32_t rc; uint32_t meta; uint16_t const data[23]; 
} const moonbit_string_literal_4 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 22), 
    99, 111, 110, 116, 114, 97, 99, 116, 46, 114, 101, 113, 117, 105, 
    114, 101, 95, 116, 105, 116, 108, 101, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[3]; 
} const moonbit_string_literal_143 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 2), 
    45, 104, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[3]; 
} const moonbit_string_literal_121 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 2), 
    92, 92, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[15]; 
} const moonbit_string_literal_81 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 14), 
    60, 47, 98, 111, 100, 121, 62, 60, 47, 104, 116, 109, 108, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[52]; 
} const moonbit_string_literal_76 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 51), 
    60, 100, 105, 118, 32, 99, 108, 97, 115, 115, 61, 34, 98, 111, 120, 
    34, 62, 60, 100, 105, 118, 32, 99, 108, 97, 115, 115, 61, 34, 109, 
    117, 116, 101, 100, 34, 62, 73, 110, 102, 111, 60, 47, 100, 105, 
    118, 62, 60, 100, 105, 118, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[74]; 
} const moonbit_string_literal_9 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 73), 
    32, 32, 100, 111, 99, 108, 105, 110, 116, 32, 45, 45, 114, 117, 108, 
    101, 115, 32, 116, 104, 101, 115, 105, 115, 124, 99, 111, 110, 116, 
    114, 97, 99, 116, 32, 45, 45, 105, 110, 32, 60, 102, 105, 108, 101, 
    62, 32, 45, 45, 111, 117, 116, 100, 105, 114, 32, 60, 100, 105, 114, 
    62, 32, 91, 45, 45, 110, 111, 45, 104, 116, 109, 108, 93, 10, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[12]; 
} const moonbit_string_literal_114 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 11), 
    34, 115, 101, 118, 101, 114, 105, 116, 121, 34, 58, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[12]; 
} const moonbit_string_literal_86 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 11), 
    60, 116, 104, 101, 97, 100, 62, 60, 116, 114, 62, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[10]; 
} const moonbit_string_literal_150 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 9), 
    45, 45, 110, 111, 45, 104, 116, 109, 108, 0
  };

struct { int32_t rc; uint32_t meta; uint16_t const data[90]; 
} const moonbit_string_literal_55 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 1, 89), 
    46, 107, 112, 105, 32, 46, 98, 111, 120, 123, 112, 97, 100, 100, 
    105, 110, 103, 58, 49, 48, 112, 120, 32, 49, 50, 112, 120, 59, 32, 
    98, 111, 114, 100, 101, 114, 58, 49, 112, 120, 32, 115, 111, 108, 
    105, 100, 32, 35, 101, 101, 101, 59, 32, 98, 111, 114, 100, 101, 
    114, 45, 114, 97, 100, 105, 117, 115, 58, 49, 48, 112, 120, 59, 32, 
    98, 97, 99, 107, 103, 114, 111, 117, 110, 100, 58, 35, 102, 102, 
    102, 59, 125, 0
  };

struct { int32_t rc; uint32_t meta; uint8_t const data[1]; 
} const moonbit_bytes_literal_2 =
  { -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 0, 0), 0};

struct { int32_t rc; uint32_t meta; uint8_t const data[4]; 
} const moonbit_bytes_literal_1 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 0, 3), 
    114, 98, 0, 0
  };

struct { int32_t rc; uint32_t meta; uint8_t const data[4]; 
} const moonbit_bytes_literal_0 =
  {
    -1, Moonbit_make_array_header(moonbit_BLOCK_KIND_VAL_ARRAY, 0, 3), 
    119, 98, 0, 0
  };

struct moonbit_object const moonbit_constant_constructor_0 =
  { -1, Moonbit_make_regular_object_header(2, 0, 0)};

struct moonbit_object const moonbit_constant_constructor_1 =
  { -1, Moonbit_make_regular_object_header(2, 0, 1)};

struct { int32_t rc; uint32_t meta; struct $$3c$Bytes$3e$$3d$$3e$String data; 
} const $$moonbitlang$x$sys$internal$ffi$get_cli_args$fn$4$closure =
  {
    -1, Moonbit_make_regular_object_header(2, 0, 0),
    $$moonbitlang$x$sys$internal$ffi$get_cli_args$fn$4
  };

struct {
  int32_t rc;
  uint32_t meta;
  struct $$ZSeanYves$Doclint$src$core$Rule$static_method_table data;
  
} $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$as_$ZSeanYves$Doclint$src$core$Rule$static_method_table_id$object =
  {
    -1,
    Moonbit_make_regular_object_header(
      sizeof(
        struct $$ZSeanYves$Doclint$src$core$Rule$static_method_table
      )
      >> 2,
        0,
        0
    ),
    {
      .$method_0 = $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$$id$dyncall_as_$ZSeanYves$Doclint$src$core$Rule,
        .$method_1 = $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$$check$dyncall_as_$ZSeanYves$Doclint$src$core$Rule
    }
  };

struct $$ZSeanYves$Doclint$src$core$Rule$static_method_table* $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$as_$ZSeanYves$Doclint$src$core$Rule$static_method_table_id =
  &$$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$as_$ZSeanYves$Doclint$src$core$Rule$static_method_table_id$object.data;

struct {
  int32_t rc;
  uint32_t meta;
  struct $$ZSeanYves$Doclint$src$core$Rule$static_method_table data;
  
} $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$as_$ZSeanYves$Doclint$src$core$Rule$static_method_table_id$object =
  {
    -1,
    Moonbit_make_regular_object_header(
      sizeof(
        struct $$ZSeanYves$Doclint$src$core$Rule$static_method_table
      )
      >> 2,
        0,
        0
    ),
    {
      .$method_0 = $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$$id$dyncall_as_$ZSeanYves$Doclint$src$core$Rule,
        .$method_1 = $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$$check$dyncall_as_$ZSeanYves$Doclint$src$core$Rule
    }
  };

struct $$ZSeanYves$Doclint$src$core$Rule$static_method_table* $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$as_$ZSeanYves$Doclint$src$core$Rule$static_method_table_id =
  &$$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$as_$ZSeanYves$Doclint$src$core$Rule$static_method_table_id$object.data;

struct {
  int32_t rc;
  uint32_t meta;
  struct $$moonbitlang$core$builtin$Logger$static_method_table data;
  
} $$moonbitlang$core$builtin$StringBuilder$as_$moonbitlang$core$builtin$Logger$static_method_table_id$object =
  {
    -1,
    Moonbit_make_regular_object_header(
      sizeof(
        struct $$moonbitlang$core$builtin$Logger$static_method_table
      )
      >> 2,
        0,
        0
    ),
    {
      .$method_0 = $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_string$dyncall_as_$moonbitlang$core$builtin$Logger,
        .$method_1 = $$moonbitlang$core$builtin$Logger$$$default_impl$$write_substring$dyncall_as_$moonbitlang$core$builtin$Logger$0,
        .$method_2 = $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_view$dyncall_as_$moonbitlang$core$builtin$Logger,
        .$method_3 = $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_char$dyncall_as_$moonbitlang$core$builtin$Logger
    }
  };

struct $$moonbitlang$core$builtin$Logger$static_method_table* $$moonbitlang$core$builtin$StringBuilder$as_$moonbitlang$core$builtin$Logger$static_method_table_id =
  &$$moonbitlang$core$builtin$StringBuilder$as_$moonbitlang$core$builtin$Logger$static_method_table_id$object.data;

moonbit_string_t $ZSeanYves$Doclint$src$core$issues_to_json$$2a$bind$7c$202 =
  (moonbit_string_t)moonbit_string_literal_0.data;

moonbit_string_t $ZSeanYves$Doclint$src$core$parse_pages$pat$7c$82 =
  (moonbit_string_t)moonbit_string_literal_1.data;

struct {
  int32_t rc;
  uint32_t meta;
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule data;
  
} $ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$811$object =
  {
    -1,
    Moonbit_make_regular_object_header(
      sizeof(
        struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule
      )
      >> 2,
        0,
        0
    ), {.$0 = 2, .$1 = 50}
  };

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* $ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$811 =
  &$ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$811$object.data;

struct {
  int32_t rc;
  uint32_t meta;
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule data;
  
} $ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$812$object =
  {
    -1,
    Moonbit_make_regular_object_header(
      offsetof(
        struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule,
          $0
      )
      >> 2,
        2,
        0
    ),
    {
      .$0 = (moonbit_string_t)moonbit_string_literal_2.data,
        .$1 = (moonbit_string_t)moonbit_string_literal_3.data,
        .$2 = 2
    }
  };

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* $ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$812 =
  &$ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$812$object.data;

struct {
  int32_t rc;
  uint32_t meta;
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$ThesisRuleset data;
  
} $ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$813$object =
  {
    -1,
    Moonbit_make_regular_object_header(
      offsetof(
        struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$ThesisRuleset, $0
      )
      >> 2,
        2,
        0
    ),
    {
      .$0 = &$ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$811$object.data,
        .$1 = &$ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$812$object.data
    }
  };

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$ThesisRuleset* $ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$813 =
  &$ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$813$object.data;

struct {
  int32_t rc;
  uint32_t meta;
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule data;
  
} $ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$815$object =
  {
    -1,
    Moonbit_make_regular_object_header(
      sizeof(
        struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule
      )
      >> 2,
        0,
        0
    ), {.$0 = 1, .$1 = 200}
  };

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* $ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$815 =
  &$ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$815$object.data;

struct {
  int32_t rc;
  uint32_t meta;
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule data;
  
} $ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$816$object =
  {
    -1,
    Moonbit_make_regular_object_header(
      offsetof(
        struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule,
          $0
      )
      >> 2,
        2,
        0
    ),
    {
      .$0 = (moonbit_string_t)moonbit_string_literal_4.data,
        .$1 = (moonbit_string_t)moonbit_string_literal_5.data,
        .$2 = 1
    }
  };

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* $ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$816 =
  &$ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$816$object.data;

struct {
  int32_t rc;
  uint32_t meta;
  struct $$ZSeanYves$Doclint$src$doclint_rules_contract$ContractRuleset data;
  
} $ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$817$object =
  {
    -1,
    Moonbit_make_regular_object_header(
      offsetof(
        struct $$ZSeanYves$Doclint$src$doclint_rules_contract$ContractRuleset,
          $0
      )
      >> 2,
        2,
        0
    ),
    {
      .$0 = &$ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$815$object.data,
        .$1 = &$ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$816$object.data
    }
  };

struct $$ZSeanYves$Doclint$src$doclint_rules_contract$ContractRuleset* $ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$817 =
  &$ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$817$object.data;

struct {
  int32_t rc;
  uint32_t meta;
  struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Err data;
  
} $ZSeanYves$Doclint$src$core$parse_doc_json$constr$782$object =
  {
    -1,
    Moonbit_make_regular_object_header(
      offsetof(
        struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Err,
          $0
      )
      >> 2,
        1,
        0
    ), {.$0 = (moonbit_string_t)moonbit_string_literal_6.data}
  };

struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Err* $ZSeanYves$Doclint$src$core$parse_doc_json$constr$782 =
  &$ZSeanYves$Doclint$src$core$parse_doc_json$constr$782$object.data;

int64_t $moonbitlang$core$builtin$brute_force_find$constr$369;

int64_t $moonbitlang$core$builtin$boyer_moore_horspool_find$constr$355;

int32_t $ZSeanYves$Doclint$src$write_out(
  moonbit_string_t out_json_path$885,
  moonbit_string_t out_html_path$891,
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* issues$886,
  int32_t emit_html$889
) {
  moonbit_string_t _tmp$2443;
  void* _bind$884;
  moonbit_incref(issues$886);
  _tmp$2443 = $ZSeanYves$Doclint$src$core$issues_to_json(issues$886);
  moonbit_incref(out_json_path$885);
  _bind$884 = $ZSeanYves$Doclint$src$write_utf8(out_json_path$885, _tmp$2443);
  switch (Moonbit_object_tag(_bind$884)) {
    case 1: {
      moonbit_string_t _tmp$2442;
      moonbit_decref(_bind$884);
      _tmp$2442
      = moonbit_add_string(
        (moonbit_string_t)moonbit_string_literal_7.data, out_json_path$885
      );
      $moonbitlang$core$builtin$println$0(_tmp$2442);
      break;
    }
    default: {
      struct $Result$3c$Unit$2a$String$3e$$Err* _Err$887;
      moonbit_string_t _field$2447;
      int32_t _cnt$2652;
      moonbit_string_t _msg$888;
      moonbit_decref(out_json_path$885);
      _Err$887 = (struct $Result$3c$Unit$2a$String$3e$$Err*)_bind$884;
      _field$2447 = _Err$887->$0;
      _cnt$2652 = Moonbit_object_header(_Err$887)->rc;
      if (_cnt$2652 > 1) {
        int32_t _new_cnt$2653 = _cnt$2652 - 1;
        Moonbit_object_header(_Err$887)->rc = _new_cnt$2653;
        moonbit_incref(_field$2447);
      } else if (_cnt$2652 == 1) {
        moonbit_free(_Err$887);
      }
      _msg$888 = _field$2447;
      $moonbitlang$core$builtin$println$0(_msg$888);
      break;
    }
  }
  if (emit_html$889) {
    moonbit_string_t _tmp$2445 =
      $ZSeanYves$Doclint$src$core$issues_to_html(issues$886);
    void* _bind$890;
    moonbit_incref(out_html_path$891);
    _bind$890
    = $ZSeanYves$Doclint$src$write_utf8(
      out_html_path$891, _tmp$2445
    );
    switch (Moonbit_object_tag(_bind$890)) {
      case 1: {
        moonbit_string_t _tmp$2444;
        moonbit_decref(_bind$890);
        _tmp$2444
        = moonbit_add_string(
          (moonbit_string_t)moonbit_string_literal_7.data, out_html_path$891
        );
        $moonbitlang$core$builtin$println$0(_tmp$2444);
        break;
      }
      default: {
        struct $Result$3c$Unit$2a$String$3e$$Err* _Err$892;
        moonbit_string_t _field$2446;
        int32_t _cnt$2654;
        moonbit_string_t _msg$893;
        moonbit_decref(out_html_path$891);
        _Err$892 = (struct $Result$3c$Unit$2a$String$3e$$Err*)_bind$890;
        _field$2446 = _Err$892->$0;
        _cnt$2654 = Moonbit_object_header(_Err$892)->rc;
        if (_cnt$2654 > 1) {
          int32_t _new_cnt$2655 = _cnt$2654 - 1;
          Moonbit_object_header(_Err$892)->rc = _new_cnt$2655;
          moonbit_incref(_field$2446);
        } else if (_cnt$2654 == 1) {
          moonbit_free(_Err$892);
        }
        _msg$893 = _field$2446;
        $moonbitlang$core$builtin$println$0(_msg$893);
        break;
      }
    }
  } else {
    moonbit_decref(out_html_path$891);
    moonbit_decref(issues$886);
  }
  return 0;
}

int32_t $ZSeanYves$Doclint$src$print_help() {
  moonbit_string_t _tmp$2441 =
    moonbit_add_string(
      (moonbit_string_t)moonbit_string_literal_8.data,
        (moonbit_string_t)moonbit_string_literal_9.data
    );
  moonbit_string_t _tmp$2440 =
    moonbit_add_string(
      _tmp$2441, (moonbit_string_t)moonbit_string_literal_10.data
    );
  moonbit_string_t _tmp$2439 =
    moonbit_add_string(
      _tmp$2440, (moonbit_string_t)moonbit_string_literal_11.data
    );
  moonbit_string_t _tmp$2438 =
    moonbit_add_string(
      _tmp$2439, (moonbit_string_t)moonbit_string_literal_12.data
    );
  moonbit_string_t _tmp$2437 =
    moonbit_add_string(
      _tmp$2438, (moonbit_string_t)moonbit_string_literal_13.data
    );
  moonbit_string_t _tmp$2436 =
    moonbit_add_string(
      _tmp$2437, (moonbit_string_t)moonbit_string_literal_10.data
    );
  moonbit_string_t _tmp$2435 =
    moonbit_add_string(
      _tmp$2436, (moonbit_string_t)moonbit_string_literal_14.data
    );
  moonbit_string_t _tmp$2434 =
    moonbit_add_string(
      _tmp$2435, (moonbit_string_t)moonbit_string_literal_15.data
    );
  moonbit_string_t _tmp$2433 =
    moonbit_add_string(
      _tmp$2434, (moonbit_string_t)moonbit_string_literal_16.data
    );
  moonbit_string_t _tmp$2432 =
    moonbit_add_string(
      _tmp$2433, (moonbit_string_t)moonbit_string_literal_17.data
    );
  $moonbitlang$core$builtin$println$0(_tmp$2432);
  return 0;
}

struct $$moonbitlang$core$builtin$Array$3c$String$3e$* $ZSeanYves$Doclint$src$drop1(
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* xs$880
) {
  moonbit_string_t* _tmp$2431 = (moonbit_string_t*)moonbit_empty_ref_array;
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* out$877 =
    (struct $$moonbitlang$core$builtin$Array$3c$String$3e$*)moonbit_malloc(
      sizeof(struct $$moonbitlang$core$builtin$Array$3c$String$3e$)
    );
  int32_t i$878;
  int32_t _len$879;
  int32_t _i$881;
  Moonbit_object_header(out$877)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $$moonbitlang$core$builtin$Array$3c$String$3e$, $0) >> 2,
      1,
      0
  );
  out$877->$0 = _tmp$2431;
  out$877->$1 = 0;
  i$878 = 0;
  _len$879 = xs$880->$1;
  _i$881 = 0;
  while (1) {
    if (_i$881 < _len$879) {
      moonbit_string_t* _field$2449 = xs$880->$0;
      moonbit_string_t* buf$2429 = _field$2449;
      moonbit_string_t _tmp$2448 = (moonbit_string_t)buf$2429[_i$881];
      moonbit_string_t x$882 = _tmp$2448;
      int32_t _tmp$2427 = i$878;
      int32_t _tmp$2428;
      int32_t _tmp$2430;
      if (_tmp$2427 > 0) {
        moonbit_incref(x$882);
        moonbit_incref(out$877);
        $$moonbitlang$core$builtin$Array$$push$0(out$877, x$882);
      }
      _tmp$2428 = i$878;
      i$878 = _tmp$2428 + 1;
      _tmp$2430 = _i$881 + 1;
      _i$881 = _tmp$2430;
      continue;
    } else {
      moonbit_decref(xs$880);
    }
    break;
  }
  return out$877;
}

moonbit_string_t $ZSeanYves$Doclint$src$join_path(
  moonbit_string_t dir$875,
  moonbit_string_t file$876
) {
  moonbit_string_t _tmp$2426 =
    moonbit_add_string(
      dir$875, (moonbit_string_t)moonbit_string_literal_18.data
    );
  return moonbit_add_string(_tmp$2426, file$876);
}

int32_t $ZSeanYves$Doclint$src$ensure_dir(moonbit_string_t path$870) {
  moonbit_incref(path$870);
  if ($moonbitlang$x$fs$path_exists(path$870)) {
    void* _try_err$872;
    int32_t _tmp$2419;
    struct moonbit_result_0 _tmp$2733;
    moonbit_incref(path$870);
    _tmp$2733 = $moonbitlang$x$fs$is_dir(path$870);
    if (_tmp$2733.tag) {
      int32_t const _ok$2420 = _tmp$2733.data.ok;
      _tmp$2419 = _ok$2420;
    } else {
      void* const _err$2421 = _tmp$2733.data.err;
      _try_err$872 = _err$2421;
      goto $join$871;
    }
    goto $joinlet$2732;
    $join$871:;
    moonbit_decref(_try_err$872);
    _tmp$2419 = 0;
    $joinlet$2732:;
    if (!_tmp$2419) {
      moonbit_string_t _tmp$2422 =
        moonbit_add_string(
          (moonbit_string_t)moonbit_string_literal_19.data, path$870
        );
      $moonbitlang$core$builtin$println$0(_tmp$2422);
    } else {
      moonbit_decref(path$870);
    }
  } else {
    void* _try_err$874;
    struct moonbit_result_0 _tmp$2735;
    moonbit_string_t _tmp$2423;
    moonbit_incref(path$870);
    _tmp$2735 = $moonbitlang$x$fs$create_dir(path$870);
    if (_tmp$2735.tag) {
      int32_t const _ok$2424 = _tmp$2735.data.ok;
      moonbit_decref(path$870);
    } else {
      void* const _err$2425 = _tmp$2735.data.err;
      _try_err$874 = _err$2425;
      goto $join$873;
    }
    goto $joinlet$2734;
    $join$873:;
    moonbit_decref(_try_err$874);
    _tmp$2423
    = moonbit_add_string(
      (moonbit_string_t)moonbit_string_literal_20.data, path$870
    );
    $moonbitlang$core$builtin$println$0(_tmp$2423);
    $joinlet$2734:;
  }
  return 0;
}

void* $ZSeanYves$Doclint$src$write_utf8(
  moonbit_string_t path$868,
  moonbit_string_t content$869
) {
  void* _try_err$867;
  struct moonbit_result_0 _tmp$2737;
  moonbit_string_t _tmp$2415;
  void* _block$2738;
  int32_t _tmp$2418;
  void* _block$2739;
  moonbit_incref(path$868);
  _tmp$2737
  = $moonbitlang$x$fs$write_string_to_file$inner(
    path$868, content$869, (moonbit_string_t)moonbit_string_literal_21.data
  );
  if (_tmp$2737.tag) {
    int32_t const _ok$2416 = _tmp$2737.data.ok;
    moonbit_decref(path$868);
  } else {
    void* const _err$2417 = _tmp$2737.data.err;
    _try_err$867 = _err$2417;
    goto $join$866;
  }
  goto $joinlet$2736;
  $join$866:;
  moonbit_decref(_try_err$867);
  _tmp$2415
  = moonbit_add_string(
    (moonbit_string_t)moonbit_string_literal_22.data, path$868
  );
  _block$2738
  = (void*)moonbit_malloc(sizeof(struct $Result$3c$Unit$2a$String$3e$$Err));
  Moonbit_object_header(_block$2738)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $Result$3c$Unit$2a$String$3e$$Err, $0) >> 2, 1, 0
  );
  ((struct $Result$3c$Unit$2a$String$3e$$Err*)_block$2738)->$0 = _tmp$2415;
  return _block$2738;
  $joinlet$2736:;
  _tmp$2418 = 0;
  _block$2739
  = (void*)moonbit_malloc(sizeof(struct $Result$3c$Unit$2a$String$3e$$Ok));
  Moonbit_object_header(_block$2739)->meta
  = Moonbit_make_regular_object_header(
    sizeof(struct $Result$3c$Unit$2a$String$3e$$Ok) >> 2, 0, 1
  );
  ((struct $Result$3c$Unit$2a$String$3e$$Ok*)_block$2739)->$0 = _tmp$2418;
  return _block$2739;
}

void* $ZSeanYves$Doclint$src$read_utf8(moonbit_string_t path$865) {
  void* _try_err$864;
  moonbit_string_t s$862;
  struct moonbit_result_1 _tmp$2741;
  moonbit_string_t _tmp$2412;
  void* _block$2742;
  void* _block$2743;
  moonbit_incref(path$865);
  _tmp$2741
  = $moonbitlang$x$fs$read_file_to_string$inner(
    path$865, (moonbit_string_t)moonbit_string_literal_21.data
  );
  if (_tmp$2741.tag) {
    moonbit_string_t const _ok$2413 = _tmp$2741.data.ok;
    moonbit_decref(path$865);
    s$862 = _ok$2413;
  } else {
    void* const _err$2414 = _tmp$2741.data.err;
    _try_err$864 = _err$2414;
    goto $join$863;
  }
  goto $joinlet$2740;
  $join$863:;
  moonbit_decref(_try_err$864);
  _tmp$2412
  = moonbit_add_string(
    (moonbit_string_t)moonbit_string_literal_23.data, path$865
  );
  _block$2742
  = (void*)moonbit_malloc(sizeof(struct $Result$3c$String$2a$String$3e$$Err));
  Moonbit_object_header(_block$2742)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $Result$3c$String$2a$String$3e$$Err, $0) >> 2, 1, 0
  );
  ((struct $Result$3c$String$2a$String$3e$$Err*)_block$2742)->$0 = _tmp$2412;
  return _block$2742;
  $joinlet$2740:;
  _block$2743
  = (void*)moonbit_malloc(sizeof(struct $Result$3c$String$2a$String$3e$$Ok));
  Moonbit_object_header(_block$2743)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $Result$3c$String$2a$String$3e$$Ok, $0) >> 2, 1, 1
  );
  ((struct $Result$3c$String$2a$String$3e$$Ok*)_block$2743)->$0 = s$862;
  return _block$2743;
}

int32_t $moonbitlang$x$sys$exit(int32_t code$861) {
  $moonbitlang$x$sys$internal$ffi$exit(code$861);
  return 0;
}

struct $$moonbitlang$core$builtin$Array$3c$String$3e$* $moonbitlang$x$sys$get_cli_args(
  
) {
  return $moonbitlang$x$sys$internal$ffi$get_cli_args();
}

int32_t $moonbitlang$x$sys$internal$ffi$exit(int32_t _param$1201) {
  exit(_param$1201);
  return 0;
}

struct $$moonbitlang$core$builtin$Array$3c$String$3e$* $moonbitlang$x$sys$internal$ffi$get_cli_args(
  
) {
  moonbit_bytes_t* _tmp$2409 =
    $moonbitlang$x$sys$internal$ffi$internal_get_cli_args();
  struct $$3c$Bytes$3e$$3d$$3e$String* _tmp$2410 =
    (struct $$3c$Bytes$3e$$3d$$3e$String*)&$$moonbitlang$x$sys$internal$ffi$get_cli_args$fn$4$closure.data;
  moonbit_string_t* _tmp$2408 = $FixedArray$$map$0(_tmp$2409, _tmp$2410);
  return $$moonbitlang$core$builtin$Array$$from_fixed_array$0(_tmp$2408);
}

moonbit_string_t $$moonbitlang$x$sys$internal$ffi$get_cli_args$fn$4(
  struct $$3c$Bytes$3e$$3d$$3e$String* _env$2411,
  moonbit_bytes_t arg$860
) {
  moonbit_decref(_env$2411);
  return $moonbitlang$x$internal$ffi$utf8_bytes_to_mbt_string(arg$860);
}

struct moonbit_result_0 $moonbitlang$x$fs$is_dir(moonbit_string_t path$859) {
  return $moonbitlang$x$fs$is_dir_internal(path$859);
}

struct moonbit_result_0 $moonbitlang$x$fs$create_dir(
  moonbit_string_t path$858
) {
  return $moonbitlang$x$fs$create_dir_internal(path$858);
}

int32_t $moonbitlang$x$fs$path_exists(moonbit_string_t path$857) {
  return $moonbitlang$x$fs$path_exists_internal(path$857);
}

struct moonbit_result_0 $moonbitlang$x$fs$write_string_to_file$inner(
  moonbit_string_t path$854,
  moonbit_string_t content$855,
  moonbit_string_t encoding$856
) {
  return $moonbitlang$x$fs$write_string_to_file_internal$inner(
           path$854, content$855, encoding$856
         );
}

struct moonbit_result_1 $moonbitlang$x$fs$read_file_to_string$inner(
  moonbit_string_t path$852,
  moonbit_string_t encoding$853
) {
  return $moonbitlang$x$fs$read_file_to_string_internal$inner(
           path$852, encoding$853
         );
}

struct moonbit_result_0 $moonbitlang$x$fs$is_dir_internal(
  moonbit_string_t path$851
) {
  moonbit_bytes_t _tmp$2407 =
    $moonbitlang$x$internal$ffi$mbt_string_to_utf8_bytes(path$851, 1);
  int32_t _tmp$2450 = $moonbitlang$x$fs$is_dir_ffi(_tmp$2407);
  int32_t res$850;
  moonbit_decref(_tmp$2407);
  res$850 = _tmp$2450;
  if (res$850 != -1) {
    int32_t _tmp$2404 = res$850 == 1;
    struct moonbit_result_0 _result$2744;
    _result$2744.tag = 1;
    _result$2744.data.ok = _tmp$2404;
    return _result$2744;
  } else {
    moonbit_string_t _tmp$2406 = $moonbitlang$x$fs$get_error_message();
    void* moonbitlang$x$fs$IOError$IOError$2405 =
      (void*)moonbit_malloc(
        sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
      );
    struct moonbit_result_0 _result$2745;
    Moonbit_object_header(moonbitlang$x$fs$IOError$IOError$2405)->meta
    = Moonbit_make_regular_object_header(
      offsetof(struct $Error$moonbitlang$x$fs$IOError$IOError, $0) >> 2, 1, 2
    );
    ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2405)->$0
    = _tmp$2406;
    _result$2745.tag = 0;
    _result$2745.data.err = moonbitlang$x$fs$IOError$IOError$2405;
    return _result$2745;
  }
}

struct moonbit_result_0 $moonbitlang$x$fs$create_dir_internal(
  moonbit_string_t path$849
) {
  moonbit_bytes_t _tmp$2400 =
    $moonbitlang$x$internal$ffi$mbt_string_to_utf8_bytes(path$849, 1);
  int32_t _tmp$2451 = $moonbitlang$x$fs$create_dir_ffi(_tmp$2400);
  int32_t _tmp$2399;
  moonbit_decref(_tmp$2400);
  _tmp$2399 = _tmp$2451;
  if (_tmp$2399 == 0) {
    int32_t _tmp$2401 = 0;
    struct moonbit_result_0 _result$2746;
    _result$2746.tag = 1;
    _result$2746.data.ok = _tmp$2401;
    return _result$2746;
  } else {
    moonbit_string_t _tmp$2403 = $moonbitlang$x$fs$get_error_message();
    void* moonbitlang$x$fs$IOError$IOError$2402 =
      (void*)moonbit_malloc(
        sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
      );
    struct moonbit_result_0 _result$2747;
    Moonbit_object_header(moonbitlang$x$fs$IOError$IOError$2402)->meta
    = Moonbit_make_regular_object_header(
      offsetof(struct $Error$moonbitlang$x$fs$IOError$IOError, $0) >> 2, 1, 2
    );
    ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2402)->$0
    = _tmp$2403;
    _result$2747.tag = 0;
    _result$2747.data.err = moonbitlang$x$fs$IOError$IOError$2402;
    return _result$2747;
  }
}

int32_t $moonbitlang$x$fs$path_exists_internal(moonbit_string_t path$848) {
  moonbit_bytes_t _tmp$2398 =
    $moonbitlang$x$internal$ffi$mbt_string_to_utf8_bytes(path$848, 1);
  int32_t _tmp$2452 = $moonbitlang$x$fs$stat_ffi(_tmp$2398);
  int32_t _tmp$2397;
  moonbit_decref(_tmp$2398);
  _tmp$2397 = _tmp$2452;
  return _tmp$2397 != -1;
}

struct moonbit_result_0 $moonbitlang$x$fs$write_string_to_file_internal$inner(
  moonbit_string_t path$847,
  moonbit_string_t content$846,
  moonbit_string_t encoding$844
) {
  if (
    moonbit_val_array_equal(
      encoding$844, (moonbit_string_t)moonbit_string_literal_21.data
    )
  ) {
    moonbit_bytes_t bytes$845;
    moonbit_decref(encoding$844);
    bytes$845
    = $moonbitlang$x$internal$ffi$mbt_string_to_utf8_bytes(
      content$846, 0
    );
    return $moonbitlang$x$fs$write_bytes_to_file_internal(
             path$847, bytes$845
           );
  } else {
    moonbit_string_t _tmp$2396;
    moonbit_string_t _tmp$2395;
    void* moonbitlang$x$fs$IOError$IOError$2394;
    struct moonbit_result_0 _result$2748;
    moonbit_decref(path$847);
    moonbit_decref(content$846);
    _tmp$2396
    = moonbit_add_string(
      (moonbit_string_t)moonbit_string_literal_24.data, encoding$844
    );
    _tmp$2395
    = moonbit_add_string(
      _tmp$2396, (moonbit_string_t)moonbit_string_literal_25.data
    );
    moonbitlang$x$fs$IOError$IOError$2394
    = (void*)moonbit_malloc(
        sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
      );
    Moonbit_object_header(moonbitlang$x$fs$IOError$IOError$2394)->meta
    = Moonbit_make_regular_object_header(
      offsetof(struct $Error$moonbitlang$x$fs$IOError$IOError, $0) >> 2, 1, 2
    );
    ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2394)->$0
    = _tmp$2395;
    _result$2748.tag = 0;
    _result$2748.data.err = moonbitlang$x$fs$IOError$IOError$2394;
    return _result$2748;
  }
}

struct moonbit_result_0 $moonbitlang$x$fs$write_bytes_to_file_internal(
  moonbit_string_t path$841,
  moonbit_bytes_t content$843
) {
  moonbit_bytes_t _tmp$2393 =
    $moonbitlang$x$internal$ffi$mbt_string_to_utf8_bytes(path$841, 1);
  void* _tmp$2454 =
    $moonbitlang$x$fs$fopen_ffi(
      _tmp$2393, (moonbit_bytes_t)moonbit_bytes_literal_0.data
    );
  void* file$840;
  int32_t _tmp$2379;
  moonbit_decref(_tmp$2393);
  file$840 = _tmp$2454;
  _tmp$2379 = $moonbitlang$x$fs$is_null(file$840);
  if (_tmp$2379 == 0) {
    int32_t _tmp$2390 = Moonbit_array_length(content$843);
    int32_t bytes_written$842 =
      $moonbitlang$x$fs$fwrite_ffi(content$843, 1, _tmp$2390, file$840);
    int32_t _tmp$2453 = Moonbit_array_length(content$843);
    int32_t _tmp$2380;
    moonbit_decref(content$843);
    _tmp$2380 = _tmp$2453;
    if (bytes_written$842 == _tmp$2380) {
      int32_t _tmp$2381 = $moonbitlang$x$fs$fflush_ffi(file$840);
      if (_tmp$2381 == 0) {
        int32_t _tmp$2382 = $moonbitlang$x$fs$fclose_ffi(file$840);
        if (_tmp$2382 == 0) {
          int32_t _tmp$2383 = 0;
          struct moonbit_result_0 _result$2749;
          _result$2749.tag = 1;
          _result$2749.data.ok = _tmp$2383;
          return _result$2749;
        } else {
          moonbit_string_t _tmp$2385 = $moonbitlang$x$fs$get_error_message();
          void* moonbitlang$x$fs$IOError$IOError$2384 =
            (void*)moonbit_malloc(
              sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
            );
          struct moonbit_result_0 _result$2750;
          Moonbit_object_header(moonbitlang$x$fs$IOError$IOError$2384)->meta
          = Moonbit_make_regular_object_header(
            offsetof(struct $Error$moonbitlang$x$fs$IOError$IOError, $0) >> 2,
              1,
              2
          );
          ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2384)->$0
          = _tmp$2385;
          _result$2750.tag = 0;
          _result$2750.data.err = moonbitlang$x$fs$IOError$IOError$2384;
          return _result$2750;
        }
      } else {
        moonbit_string_t _tmp$2387 = $moonbitlang$x$fs$get_error_message();
        void* moonbitlang$x$fs$IOError$IOError$2386 =
          (void*)moonbit_malloc(
            sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
          );
        struct moonbit_result_0 _result$2751;
        Moonbit_object_header(moonbitlang$x$fs$IOError$IOError$2386)->meta
        = Moonbit_make_regular_object_header(
          offsetof(struct $Error$moonbitlang$x$fs$IOError$IOError, $0) >> 2,
            1,
            2
        );
        ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2386)->$0
        = _tmp$2387;
        _result$2751.tag = 0;
        _result$2751.data.err = moonbitlang$x$fs$IOError$IOError$2386;
        return _result$2751;
      }
    } else {
      moonbit_string_t _tmp$2389 = $moonbitlang$x$fs$get_error_message();
      void* moonbitlang$x$fs$IOError$IOError$2388 =
        (void*)moonbit_malloc(
          sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
        );
      struct moonbit_result_0 _result$2752;
      Moonbit_object_header(moonbitlang$x$fs$IOError$IOError$2388)->meta
      = Moonbit_make_regular_object_header(
        offsetof(struct $Error$moonbitlang$x$fs$IOError$IOError, $0) >> 2,
          1,
          2
      );
      ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2388)->$0
      = _tmp$2389;
      _result$2752.tag = 0;
      _result$2752.data.err = moonbitlang$x$fs$IOError$IOError$2388;
      return _result$2752;
    }
  } else {
    moonbit_string_t _tmp$2392;
    void* moonbitlang$x$fs$IOError$IOError$2391;
    struct moonbit_result_0 _result$2753;
    moonbit_decref(content$843);
    _tmp$2392 = $moonbitlang$x$fs$get_error_message();
    moonbitlang$x$fs$IOError$IOError$2391
    = (void*)moonbit_malloc(
        sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
      );
    Moonbit_object_header(moonbitlang$x$fs$IOError$IOError$2391)->meta
    = Moonbit_make_regular_object_header(
      offsetof(struct $Error$moonbitlang$x$fs$IOError$IOError, $0) >> 2, 1, 2
    );
    ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2391)->$0
    = _tmp$2392;
    _result$2753.tag = 0;
    _result$2753.data.err = moonbitlang$x$fs$IOError$IOError$2391;
    return _result$2753;
  }
}

struct moonbit_result_1 $moonbitlang$x$fs$read_file_to_string_internal$inner(
  moonbit_string_t path$839,
  moonbit_string_t encoding$838
) {
  if (
    moonbit_val_array_equal(
      encoding$838, (moonbit_string_t)moonbit_string_literal_21.data
    )
  ) {
    struct moonbit_result_2 _tmp$2754;
    moonbit_bytes_t _tmp$2373;
    moonbit_string_t _tmp$2372;
    struct moonbit_result_1 _result$2756;
    moonbit_decref(encoding$838);
    _tmp$2754 = $moonbitlang$x$fs$read_file_to_bytes_internal(path$839);
    if (_tmp$2754.tag) {
      moonbit_bytes_t const _ok$2374 = _tmp$2754.data.ok;
      _tmp$2373 = _ok$2374;
    } else {
      void* const _err$2375 = _tmp$2754.data.err;
      struct moonbit_result_1 _result$2755;
      _result$2755.tag = 0;
      _result$2755.data.err = _err$2375;
      return _result$2755;
    }
    _tmp$2372
    = $moonbitlang$x$internal$ffi$utf8_bytes_to_mbt_string(
      _tmp$2373
    );
    _result$2756.tag = 1;
    _result$2756.data.ok = _tmp$2372;
    return _result$2756;
  } else {
    moonbit_string_t _tmp$2378;
    moonbit_string_t _tmp$2377;
    void* moonbitlang$x$fs$IOError$IOError$2376;
    struct moonbit_result_1 _result$2757;
    moonbit_decref(path$839);
    _tmp$2378
    = moonbit_add_string(
      (moonbit_string_t)moonbit_string_literal_24.data, encoding$838
    );
    _tmp$2377
    = moonbit_add_string(
      _tmp$2378, (moonbit_string_t)moonbit_string_literal_25.data
    );
    moonbitlang$x$fs$IOError$IOError$2376
    = (void*)moonbit_malloc(
        sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
      );
    Moonbit_object_header(moonbitlang$x$fs$IOError$IOError$2376)->meta
    = Moonbit_make_regular_object_header(
      offsetof(struct $Error$moonbitlang$x$fs$IOError$IOError, $0) >> 2, 1, 2
    );
    ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2376)->$0
    = _tmp$2377;
    _result$2757.tag = 0;
    _result$2757.data.err = moonbitlang$x$fs$IOError$IOError$2376;
    return _result$2757;
  }
}

struct moonbit_result_2 $moonbitlang$x$fs$read_file_to_bytes_internal(
  moonbit_string_t path$834
) {
  moonbit_bytes_t _tmp$2371 =
    $moonbitlang$x$internal$ffi$mbt_string_to_utf8_bytes(path$834, 1);
  void* _tmp$2455 =
    $moonbitlang$x$fs$fopen_ffi(
      _tmp$2371, (moonbit_bytes_t)moonbit_bytes_literal_1.data
    );
  void* file$833;
  int32_t _tmp$2355;
  moonbit_decref(_tmp$2371);
  file$833 = _tmp$2455;
  _tmp$2355 = $moonbitlang$x$fs$is_null(file$833);
  if (_tmp$2355 == 0) {
    int32_t _tmp$2356 = $moonbitlang$x$fs$fseek_ffi(file$833, 0, 2);
    if (_tmp$2356 == 0) {
      int32_t size$835 = $moonbitlang$x$fs$ftell_ffi(file$833);
      if (size$835 != -1) {
        int32_t _tmp$2357 = $moonbitlang$x$fs$fseek_ffi(file$833, 0, 0);
        if (_tmp$2357 == 0) {
          moonbit_bytes_t bytes$836 = $Bytes$$make(size$835, 0);
          int32_t bytes_read$837 =
            $moonbitlang$x$fs$fread_ffi(bytes$836, 1, size$835, file$833);
          if (bytes_read$837 == size$835) {
            int32_t _tmp$2358 = $moonbitlang$x$fs$fclose_ffi(file$833);
            if (_tmp$2358 == 0) {
              struct moonbit_result_2 _result$2758;
              _result$2758.tag = 1;
              _result$2758.data.ok = bytes$836;
              return _result$2758;
            } else {
              moonbit_string_t _tmp$2360;
              void* moonbitlang$x$fs$IOError$IOError$2359;
              struct moonbit_result_2 _result$2759;
              moonbit_decref(bytes$836);
              _tmp$2360 = $moonbitlang$x$fs$get_error_message();
              moonbitlang$x$fs$IOError$IOError$2359
              = (void*)moonbit_malloc(
                  sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
                );
              Moonbit_object_header(
                moonbitlang$x$fs$IOError$IOError$2359
              )->meta
              = Moonbit_make_regular_object_header(
                offsetof(
                  struct $Error$moonbitlang$x$fs$IOError$IOError, $0
                )
                >> 2,
                  1,
                  2
              );
              ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2359)->$0
              = _tmp$2360;
              _result$2759.tag = 0;
              _result$2759.data.err = moonbitlang$x$fs$IOError$IOError$2359;
              return _result$2759;
            }
          } else {
            moonbit_string_t _tmp$2362;
            void* moonbitlang$x$fs$IOError$IOError$2361;
            struct moonbit_result_2 _result$2760;
            moonbit_decref(bytes$836);
            _tmp$2362 = $moonbitlang$x$fs$get_error_message();
            moonbitlang$x$fs$IOError$IOError$2361
            = (void*)moonbit_malloc(
                sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
              );
            Moonbit_object_header(
              moonbitlang$x$fs$IOError$IOError$2361
            )->meta
            = Moonbit_make_regular_object_header(
              offsetof(
                struct $Error$moonbitlang$x$fs$IOError$IOError, $0
              )
              >> 2,
                1,
                2
            );
            ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2361)->$0
            = _tmp$2362;
            _result$2760.tag = 0;
            _result$2760.data.err = moonbitlang$x$fs$IOError$IOError$2361;
            return _result$2760;
          }
        } else {
          moonbit_string_t _tmp$2364 = $moonbitlang$x$fs$get_error_message();
          void* moonbitlang$x$fs$IOError$IOError$2363 =
            (void*)moonbit_malloc(
              sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
            );
          struct moonbit_result_2 _result$2761;
          Moonbit_object_header(moonbitlang$x$fs$IOError$IOError$2363)->meta
          = Moonbit_make_regular_object_header(
            offsetof(struct $Error$moonbitlang$x$fs$IOError$IOError, $0) >> 2,
              1,
              2
          );
          ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2363)->$0
          = _tmp$2364;
          _result$2761.tag = 0;
          _result$2761.data.err = moonbitlang$x$fs$IOError$IOError$2363;
          return _result$2761;
        }
      } else {
        moonbit_string_t _tmp$2366 = $moonbitlang$x$fs$get_error_message();
        void* moonbitlang$x$fs$IOError$IOError$2365 =
          (void*)moonbit_malloc(
            sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
          );
        struct moonbit_result_2 _result$2762;
        Moonbit_object_header(moonbitlang$x$fs$IOError$IOError$2365)->meta
        = Moonbit_make_regular_object_header(
          offsetof(struct $Error$moonbitlang$x$fs$IOError$IOError, $0) >> 2,
            1,
            2
        );
        ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2365)->$0
        = _tmp$2366;
        _result$2762.tag = 0;
        _result$2762.data.err = moonbitlang$x$fs$IOError$IOError$2365;
        return _result$2762;
      }
    } else {
      moonbit_string_t _tmp$2368 = $moonbitlang$x$fs$get_error_message();
      void* moonbitlang$x$fs$IOError$IOError$2367 =
        (void*)moonbit_malloc(
          sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
        );
      struct moonbit_result_2 _result$2763;
      Moonbit_object_header(moonbitlang$x$fs$IOError$IOError$2367)->meta
      = Moonbit_make_regular_object_header(
        offsetof(struct $Error$moonbitlang$x$fs$IOError$IOError, $0) >> 2,
          1,
          2
      );
      ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2367)->$0
      = _tmp$2368;
      _result$2763.tag = 0;
      _result$2763.data.err = moonbitlang$x$fs$IOError$IOError$2367;
      return _result$2763;
    }
  } else {
    moonbit_string_t _tmp$2370 = $moonbitlang$x$fs$get_error_message();
    void* moonbitlang$x$fs$IOError$IOError$2369 =
      (void*)moonbit_malloc(
        sizeof(struct $Error$moonbitlang$x$fs$IOError$IOError)
      );
    struct moonbit_result_2 _result$2764;
    Moonbit_object_header(moonbitlang$x$fs$IOError$IOError$2369)->meta
    = Moonbit_make_regular_object_header(
      offsetof(struct $Error$moonbitlang$x$fs$IOError$IOError, $0) >> 2, 1, 2
    );
    ((struct $Error$moonbitlang$x$fs$IOError$IOError*)moonbitlang$x$fs$IOError$IOError$2369)->$0
    = _tmp$2370;
    _result$2764.tag = 0;
    _result$2764.data.err = moonbitlang$x$fs$IOError$IOError$2369;
    return _result$2764;
  }
}

moonbit_string_t $moonbitlang$x$fs$get_error_message() {
  moonbit_bytes_t _tmp$2354 = $moonbitlang$x$fs$get_error_message_ffi();
  return $moonbitlang$x$internal$ffi$utf8_bytes_to_mbt_string(_tmp$2354);
}

moonbit_string_t $moonbitlang$x$internal$ffi$utf8_bytes_to_mbt_string(
  moonbit_bytes_t bytes$829
) {
  int32_t* _tmp$2353 = (int32_t*)moonbit_empty_int32_array;
  struct $$moonbitlang$core$builtin$Array$3c$Char$3e$* res$827 =
    (struct $$moonbitlang$core$builtin$Array$3c$Char$3e$*)moonbit_malloc(
      sizeof(struct $$moonbitlang$core$builtin$Array$3c$Char$3e$)
    );
  int32_t len$828;
  int32_t i$830;
  int32_t* _field$2457;
  int32_t* buf$2351;
  int32_t _field$2456;
  int32_t _cnt$2656;
  int32_t len$2352;
  struct $$moonbitlang$core$builtin$ArrayView$3c$Char$3e$ _tmp$2350;
  Moonbit_object_header(res$827)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $$moonbitlang$core$builtin$Array$3c$Char$3e$, $0) >> 2,
      1,
      0
  );
  res$827->$0 = _tmp$2353;
  res$827->$1 = 0;
  len$828 = Moonbit_array_length(bytes$829);
  i$830 = 0;
  while (1) {
    int32_t _tmp$2274 = i$830;
    if (_tmp$2274 < len$828) {
      int32_t _tmp$2349 = i$830;
      int32_t _tmp$2348;
      int32_t c$831;
      int32_t _tmp$2275;
      if (_tmp$2349 < 0 || _tmp$2349 >= Moonbit_array_length(bytes$829)) {
        moonbit_panic();
      }
      _tmp$2348 = bytes$829[_tmp$2349];
      c$831 = (int32_t)_tmp$2348;
      _tmp$2275 = c$831;
      if (_tmp$2275 < 128) {
        int32_t _tmp$2277 = c$831;
        int32_t _tmp$2276 = _tmp$2277;
        int32_t _tmp$2278;
        moonbit_incref(res$827);
        $$moonbitlang$core$builtin$Array$$push$1(res$827, _tmp$2276);
        _tmp$2278 = i$830;
        i$830 = _tmp$2278 + 1;
      } else {
        int32_t _tmp$2279 = c$831;
        if (_tmp$2279 < 224) {
          int32_t _tmp$2281 = i$830;
          int32_t _tmp$2280 = _tmp$2281 + 1;
          int32_t _tmp$2289;
          int32_t _tmp$2288;
          int32_t _tmp$2282;
          int32_t _tmp$2287;
          int32_t _tmp$2286;
          int32_t _tmp$2285;
          int32_t _tmp$2284;
          int32_t _tmp$2283;
          int32_t _tmp$2291;
          int32_t _tmp$2290;
          int32_t _tmp$2292;
          if (_tmp$2280 >= len$828) {
            moonbit_decref(bytes$829);
            break;
          }
          _tmp$2289 = c$831;
          _tmp$2288 = _tmp$2289 & 31;
          _tmp$2282 = _tmp$2288 << 6;
          _tmp$2287 = i$830;
          _tmp$2286 = _tmp$2287 + 1;
          if (_tmp$2286 < 0 || _tmp$2286 >= Moonbit_array_length(bytes$829)) {
            moonbit_panic();
          }
          _tmp$2285 = bytes$829[_tmp$2286];
          _tmp$2284 = (int32_t)_tmp$2285;
          _tmp$2283 = _tmp$2284 & 63;
          c$831 = _tmp$2282 | _tmp$2283;
          _tmp$2291 = c$831;
          _tmp$2290 = _tmp$2291;
          moonbit_incref(res$827);
          $$moonbitlang$core$builtin$Array$$push$1(res$827, _tmp$2290);
          _tmp$2292 = i$830;
          i$830 = _tmp$2292 + 2;
        } else {
          int32_t _tmp$2293 = c$831;
          if (_tmp$2293 < 240) {
            int32_t _tmp$2295 = i$830;
            int32_t _tmp$2294 = _tmp$2295 + 2;
            int32_t _tmp$2310;
            int32_t _tmp$2309;
            int32_t _tmp$2302;
            int32_t _tmp$2308;
            int32_t _tmp$2307;
            int32_t _tmp$2306;
            int32_t _tmp$2305;
            int32_t _tmp$2304;
            int32_t _tmp$2303;
            int32_t _tmp$2296;
            int32_t _tmp$2301;
            int32_t _tmp$2300;
            int32_t _tmp$2299;
            int32_t _tmp$2298;
            int32_t _tmp$2297;
            int32_t _tmp$2312;
            int32_t _tmp$2311;
            int32_t _tmp$2313;
            if (_tmp$2294 >= len$828) {
              moonbit_decref(bytes$829);
              break;
            }
            _tmp$2310 = c$831;
            _tmp$2309 = _tmp$2310 & 15;
            _tmp$2302 = _tmp$2309 << 12;
            _tmp$2308 = i$830;
            _tmp$2307 = _tmp$2308 + 1;
            if (
              _tmp$2307 < 0 || _tmp$2307 >= Moonbit_array_length(bytes$829)
            ) {
              moonbit_panic();
            }
            _tmp$2306 = bytes$829[_tmp$2307];
            _tmp$2305 = (int32_t)_tmp$2306;
            _tmp$2304 = _tmp$2305 & 63;
            _tmp$2303 = _tmp$2304 << 6;
            _tmp$2296 = _tmp$2302 | _tmp$2303;
            _tmp$2301 = i$830;
            _tmp$2300 = _tmp$2301 + 2;
            if (
              _tmp$2300 < 0 || _tmp$2300 >= Moonbit_array_length(bytes$829)
            ) {
              moonbit_panic();
            }
            _tmp$2299 = bytes$829[_tmp$2300];
            _tmp$2298 = (int32_t)_tmp$2299;
            _tmp$2297 = _tmp$2298 & 63;
            c$831 = _tmp$2296 | _tmp$2297;
            _tmp$2312 = c$831;
            _tmp$2311 = _tmp$2312;
            moonbit_incref(res$827);
            $$moonbitlang$core$builtin$Array$$push$1(res$827, _tmp$2311);
            _tmp$2313 = i$830;
            i$830 = _tmp$2313 + 3;
          } else {
            int32_t _tmp$2315 = i$830;
            int32_t _tmp$2314 = _tmp$2315 + 3;
            int32_t _tmp$2337;
            int32_t _tmp$2336;
            int32_t _tmp$2329;
            int32_t _tmp$2335;
            int32_t _tmp$2334;
            int32_t _tmp$2333;
            int32_t _tmp$2332;
            int32_t _tmp$2331;
            int32_t _tmp$2330;
            int32_t _tmp$2322;
            int32_t _tmp$2328;
            int32_t _tmp$2327;
            int32_t _tmp$2326;
            int32_t _tmp$2325;
            int32_t _tmp$2324;
            int32_t _tmp$2323;
            int32_t _tmp$2316;
            int32_t _tmp$2321;
            int32_t _tmp$2320;
            int32_t _tmp$2319;
            int32_t _tmp$2318;
            int32_t _tmp$2317;
            int32_t _tmp$2338;
            int32_t _tmp$2342;
            int32_t _tmp$2341;
            int32_t _tmp$2340;
            int32_t _tmp$2339;
            int32_t _tmp$2346;
            int32_t _tmp$2345;
            int32_t _tmp$2344;
            int32_t _tmp$2343;
            int32_t _tmp$2347;
            if (_tmp$2314 >= len$828) {
              moonbit_decref(bytes$829);
              break;
            }
            _tmp$2337 = c$831;
            _tmp$2336 = _tmp$2337 & 7;
            _tmp$2329 = _tmp$2336 << 18;
            _tmp$2335 = i$830;
            _tmp$2334 = _tmp$2335 + 1;
            if (
              _tmp$2334 < 0 || _tmp$2334 >= Moonbit_array_length(bytes$829)
            ) {
              moonbit_panic();
            }
            _tmp$2333 = bytes$829[_tmp$2334];
            _tmp$2332 = (int32_t)_tmp$2333;
            _tmp$2331 = _tmp$2332 & 63;
            _tmp$2330 = _tmp$2331 << 12;
            _tmp$2322 = _tmp$2329 | _tmp$2330;
            _tmp$2328 = i$830;
            _tmp$2327 = _tmp$2328 + 2;
            if (
              _tmp$2327 < 0 || _tmp$2327 >= Moonbit_array_length(bytes$829)
            ) {
              moonbit_panic();
            }
            _tmp$2326 = bytes$829[_tmp$2327];
            _tmp$2325 = (int32_t)_tmp$2326;
            _tmp$2324 = _tmp$2325 & 63;
            _tmp$2323 = _tmp$2324 << 6;
            _tmp$2316 = _tmp$2322 | _tmp$2323;
            _tmp$2321 = i$830;
            _tmp$2320 = _tmp$2321 + 3;
            if (
              _tmp$2320 < 0 || _tmp$2320 >= Moonbit_array_length(bytes$829)
            ) {
              moonbit_panic();
            }
            _tmp$2319 = bytes$829[_tmp$2320];
            _tmp$2318 = (int32_t)_tmp$2319;
            _tmp$2317 = _tmp$2318 & 63;
            c$831 = _tmp$2316 | _tmp$2317;
            _tmp$2338 = c$831;
            c$831 = _tmp$2338 - 65536;
            _tmp$2342 = c$831;
            _tmp$2341 = _tmp$2342 >> 10;
            _tmp$2340 = _tmp$2341 + 55296;
            _tmp$2339 = _tmp$2340;
            moonbit_incref(res$827);
            $$moonbitlang$core$builtin$Array$$push$1(res$827, _tmp$2339);
            _tmp$2346 = c$831;
            _tmp$2345 = _tmp$2346 & 1023;
            _tmp$2344 = _tmp$2345 + 56320;
            _tmp$2343 = _tmp$2344;
            moonbit_incref(res$827);
            $$moonbitlang$core$builtin$Array$$push$1(res$827, _tmp$2343);
            _tmp$2347 = i$830;
            i$830 = _tmp$2347 + 4;
          }
        }
      }
      continue;
    } else {
      moonbit_decref(bytes$829);
    }
    break;
  }
  _field$2457 = res$827->$0;
  buf$2351 = _field$2457;
  _field$2456 = res$827->$1;
  _cnt$2656 = Moonbit_object_header(res$827)->rc;
  if (_cnt$2656 > 1) {
    int32_t _new_cnt$2657 = _cnt$2656 - 1;
    Moonbit_object_header(res$827)->rc = _new_cnt$2657;
    moonbit_incref(buf$2351);
  } else if (_cnt$2656 == 1) {
    moonbit_free(res$827);
  }
  len$2352 = _field$2456;
  _tmp$2350
  = (struct $$moonbitlang$core$builtin$ArrayView$3c$Char$3e$){
    0, len$2352, buf$2351
  };
  return $String$$from_array(_tmp$2350);
}

moonbit_bytes_t $moonbitlang$x$internal$ffi$mbt_string_to_utf8_bytes(
  moonbit_string_t str$821,
  int32_t is_filename$826
) {
  moonbit_bytes_t _tmp$2273 = (moonbit_bytes_t)moonbit_empty_int8_array;
  struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$* res$819 =
    (struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$*)moonbit_malloc(
      sizeof(struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$)
    );
  int32_t len$820;
  int32_t i$822;
  moonbit_bytes_t _field$2459;
  moonbit_bytes_t buf$2271;
  int32_t _field$2458;
  int32_t _cnt$2658;
  int32_t len$2272;
  struct $$moonbitlang$core$builtin$ArrayView$3c$Byte$3e$ _tmp$2270;
  Moonbit_object_header(res$819)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$, $0) >> 2,
      1,
      0
  );
  res$819->$0 = _tmp$2273;
  res$819->$1 = 0;
  len$820 = Moonbit_array_length(str$821);
  i$822 = 0;
  while (1) {
    int32_t _tmp$2211 = i$822;
    if (_tmp$2211 < len$820) {
      int32_t _tmp$2268 = i$822;
      int32_t _tmp$2267;
      int32_t c$823;
      int32_t _tmp$2213;
      int32_t _if_result$2767;
      int32_t _tmp$2222;
      int32_t _tmp$2266;
      if (_tmp$2268 < 0 || _tmp$2268 >= Moonbit_array_length(str$821)) {
        moonbit_panic();
      }
      _tmp$2267 = str$821[_tmp$2268];
      c$823 = (int32_t)_tmp$2267;
      _tmp$2213 = c$823;
      if (55296 <= _tmp$2213) {
        int32_t _tmp$2212 = c$823;
        _if_result$2767 = _tmp$2212 <= 56319;
      } else {
        _if_result$2767 = 0;
      }
      if (_if_result$2767) {
        int32_t _tmp$2214 = c$823;
        int32_t _tmp$2215;
        int32_t _tmp$2221;
        int32_t _tmp$2220;
        int32_t _tmp$2219;
        int32_t l$824;
        int32_t _tmp$2218;
        int32_t _tmp$2217;
        int32_t _tmp$2216;
        c$823 = _tmp$2214 - 55296;
        _tmp$2215 = i$822;
        i$822 = _tmp$2215 + 1;
        _tmp$2221 = i$822;
        if (_tmp$2221 < 0 || _tmp$2221 >= Moonbit_array_length(str$821)) {
          moonbit_panic();
        }
        _tmp$2220 = str$821[_tmp$2221];
        _tmp$2219 = (int32_t)_tmp$2220;
        l$824 = _tmp$2219 - 56320;
        _tmp$2218 = c$823;
        _tmp$2217 = _tmp$2218 << 10;
        _tmp$2216 = _tmp$2217 + l$824;
        c$823 = _tmp$2216 + 65536;
      }
      _tmp$2222 = c$823;
      if (_tmp$2222 < 128) {
        int32_t _tmp$2224 = c$823;
        int32_t _tmp$2223 = _tmp$2224 & 0xff;
        moonbit_incref(res$819);
        $$moonbitlang$core$builtin$Array$$push$4(res$819, _tmp$2223);
      } else {
        int32_t _tmp$2225 = c$823;
        if (_tmp$2225 < 2048) {
          int32_t _tmp$2229 = c$823;
          int32_t _tmp$2228 = _tmp$2229 >> 6;
          int32_t _tmp$2227 = 192 + _tmp$2228;
          int32_t _tmp$2226 = _tmp$2227 & 0xff;
          int32_t _tmp$2233;
          int32_t _tmp$2232;
          int32_t _tmp$2231;
          int32_t _tmp$2230;
          moonbit_incref(res$819);
          $$moonbitlang$core$builtin$Array$$push$4(res$819, _tmp$2226);
          _tmp$2233 = c$823;
          _tmp$2232 = _tmp$2233 & 63;
          _tmp$2231 = 128 + _tmp$2232;
          _tmp$2230 = _tmp$2231 & 0xff;
          moonbit_incref(res$819);
          $$moonbitlang$core$builtin$Array$$push$4(res$819, _tmp$2230);
        } else {
          int32_t _tmp$2234 = c$823;
          if (_tmp$2234 < 65536) {
            int32_t _tmp$2238 = c$823;
            int32_t _tmp$2237 = _tmp$2238 >> 12;
            int32_t _tmp$2236 = 224 + _tmp$2237;
            int32_t _tmp$2235 = _tmp$2236 & 0xff;
            int32_t _tmp$2243;
            int32_t _tmp$2242;
            int32_t _tmp$2241;
            int32_t _tmp$2240;
            int32_t _tmp$2239;
            int32_t _tmp$2247;
            int32_t _tmp$2246;
            int32_t _tmp$2245;
            int32_t _tmp$2244;
            moonbit_incref(res$819);
            $$moonbitlang$core$builtin$Array$$push$4(res$819, _tmp$2235);
            _tmp$2243 = c$823;
            _tmp$2242 = _tmp$2243 >> 6;
            _tmp$2241 = _tmp$2242 & 63;
            _tmp$2240 = 128 + _tmp$2241;
            _tmp$2239 = _tmp$2240 & 0xff;
            moonbit_incref(res$819);
            $$moonbitlang$core$builtin$Array$$push$4(res$819, _tmp$2239);
            _tmp$2247 = c$823;
            _tmp$2246 = _tmp$2247 & 63;
            _tmp$2245 = 128 + _tmp$2246;
            _tmp$2244 = _tmp$2245 & 0xff;
            moonbit_incref(res$819);
            $$moonbitlang$core$builtin$Array$$push$4(res$819, _tmp$2244);
          } else {
            int32_t _tmp$2251 = c$823;
            int32_t _tmp$2250 = _tmp$2251 >> 18;
            int32_t _tmp$2249 = 240 + _tmp$2250;
            int32_t _tmp$2248 = _tmp$2249 & 0xff;
            int32_t _tmp$2256;
            int32_t _tmp$2255;
            int32_t _tmp$2254;
            int32_t _tmp$2253;
            int32_t _tmp$2252;
            int32_t _tmp$2261;
            int32_t _tmp$2260;
            int32_t _tmp$2259;
            int32_t _tmp$2258;
            int32_t _tmp$2257;
            int32_t _tmp$2265;
            int32_t _tmp$2264;
            int32_t _tmp$2263;
            int32_t _tmp$2262;
            moonbit_incref(res$819);
            $$moonbitlang$core$builtin$Array$$push$4(res$819, _tmp$2248);
            _tmp$2256 = c$823;
            _tmp$2255 = _tmp$2256 >> 12;
            _tmp$2254 = _tmp$2255 & 63;
            _tmp$2253 = 128 + _tmp$2254;
            _tmp$2252 = _tmp$2253 & 0xff;
            moonbit_incref(res$819);
            $$moonbitlang$core$builtin$Array$$push$4(res$819, _tmp$2252);
            _tmp$2261 = c$823;
            _tmp$2260 = _tmp$2261 >> 6;
            _tmp$2259 = _tmp$2260 & 63;
            _tmp$2258 = 128 + _tmp$2259;
            _tmp$2257 = _tmp$2258 & 0xff;
            moonbit_incref(res$819);
            $$moonbitlang$core$builtin$Array$$push$4(res$819, _tmp$2257);
            _tmp$2265 = c$823;
            _tmp$2264 = _tmp$2265 & 63;
            _tmp$2263 = 128 + _tmp$2264;
            _tmp$2262 = _tmp$2263 & 0xff;
            moonbit_incref(res$819);
            $$moonbitlang$core$builtin$Array$$push$4(res$819, _tmp$2262);
          }
        }
      }
      _tmp$2266 = i$822;
      i$822 = _tmp$2266 + 1;
      continue;
    } else {
      moonbit_decref(str$821);
    }
    break;
  }
  if (is_filename$826) {
    int32_t _tmp$2269 = 0 & 0xff;
    moonbit_incref(res$819);
    $$moonbitlang$core$builtin$Array$$push$4(res$819, _tmp$2269);
  }
  _field$2459 = res$819->$0;
  buf$2271 = _field$2459;
  _field$2458 = res$819->$1;
  _cnt$2658 = Moonbit_object_header(res$819)->rc;
  if (_cnt$2658 > 1) {
    int32_t _new_cnt$2659 = _cnt$2658 - 1;
    Moonbit_object_header(res$819)->rc = _new_cnt$2659;
    moonbit_incref(buf$2271);
  } else if (_cnt$2658 == 1) {
    moonbit_free(res$819);
  }
  len$2272 = _field$2458;
  _tmp$2270
  = (struct $$moonbitlang$core$builtin$ArrayView$3c$Byte$3e$){
    0, len$2272, buf$2271
  };
  return $Bytes$$from_array(_tmp$2270);
}

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$* $ZSeanYves$Doclint$src$doclint_rules_contract$as_rules_ref(
  struct $$ZSeanYves$Doclint$src$doclint_rules_contract$ContractRuleset* rs$818
) {
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* _field$2461 =
    rs$818->$0;
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* page_limit$2210 =
    _field$2461;
  struct $$ZSeanYves$Doclint$src$core$Rule _tmp$2207;
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* _field$2460;
  int32_t _cnt$2660;
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* require_abs$2209;
  struct $$ZSeanYves$Doclint$src$core$Rule _tmp$2208;
  struct $$ZSeanYves$Doclint$src$core$Rule* _tmp$2206;
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$* _block$2768;
  moonbit_incref(page_limit$2210);
  _tmp$2207
  = (struct $$ZSeanYves$Doclint$src$core$Rule){
    $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$as_$ZSeanYves$Doclint$src$core$Rule$static_method_table_id,
      page_limit$2210
  };
  _field$2460 = rs$818->$1;
  _cnt$2660 = Moonbit_object_header(rs$818)->rc;
  if (_cnt$2660 > 1) {
    int32_t _new_cnt$2662 = _cnt$2660 - 1;
    Moonbit_object_header(rs$818)->rc = _new_cnt$2662;
    moonbit_incref(_field$2460);
  } else if (_cnt$2660 == 1) {
    struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* _field$2661 =
      rs$818->$0;
    moonbit_decref(_field$2661);
    moonbit_free(rs$818);
  }
  require_abs$2209 = _field$2460;
  _tmp$2208
  = (struct $$ZSeanYves$Doclint$src$core$Rule){
    $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$as_$ZSeanYves$Doclint$src$core$Rule$static_method_table_id,
      require_abs$2209
  };
  _tmp$2206
  = (struct $$ZSeanYves$Doclint$src$core$Rule*)moonbit_make_ref_valtype_array_raw(
      2,
        sizeof(struct $$ZSeanYves$Doclint$src$core$Rule),
        Moonbit_make_regular_object_header(
          offsetof(struct $$ZSeanYves$Doclint$src$core$Rule, $0) >> 2, 2, 0
        )
    );
  _tmp$2206[0] = _tmp$2207;
  _tmp$2206[1] = _tmp$2208;
  _block$2768
  = (struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$*)moonbit_malloc(
      sizeof(
        struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$
      )
    );
  Moonbit_object_header(_block$2768)->meta
  = Moonbit_make_regular_object_header(
    offsetof(
      struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$,
        $0
    )
    >> 2,
      1,
      0
  );
  _block$2768->$0 = _tmp$2206;
  _block$2768->$1 = 2;
  return _block$2768;
}

struct $$ZSeanYves$Doclint$src$doclint_rules_contract$ContractRuleset* $ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset(
  
) {
  moonbit_incref(
    $ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$817
  );
  return $ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$817;
}

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$* $ZSeanYves$Doclint$src$doclint_rules_thesis$as_rules_ref(
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$ThesisRuleset* rs$814
) {
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* _field$2463 =
    rs$814->$0;
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* page_limit$2205 =
    _field$2463;
  struct $$ZSeanYves$Doclint$src$core$Rule _tmp$2202;
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* _field$2462;
  int32_t _cnt$2663;
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* require_abs$2204;
  struct $$ZSeanYves$Doclint$src$core$Rule _tmp$2203;
  struct $$ZSeanYves$Doclint$src$core$Rule* _tmp$2201;
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$* _block$2769;
  moonbit_incref(page_limit$2205);
  _tmp$2202
  = (struct $$ZSeanYves$Doclint$src$core$Rule){
    $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$as_$ZSeanYves$Doclint$src$core$Rule$static_method_table_id,
      page_limit$2205
  };
  _field$2462 = rs$814->$1;
  _cnt$2663 = Moonbit_object_header(rs$814)->rc;
  if (_cnt$2663 > 1) {
    int32_t _new_cnt$2665 = _cnt$2663 - 1;
    Moonbit_object_header(rs$814)->rc = _new_cnt$2665;
    moonbit_incref(_field$2462);
  } else if (_cnt$2663 == 1) {
    struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* _field$2664 =
      rs$814->$0;
    moonbit_decref(_field$2664);
    moonbit_free(rs$814);
  }
  require_abs$2204 = _field$2462;
  _tmp$2203
  = (struct $$ZSeanYves$Doclint$src$core$Rule){
    $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$as_$ZSeanYves$Doclint$src$core$Rule$static_method_table_id,
      require_abs$2204
  };
  _tmp$2201
  = (struct $$ZSeanYves$Doclint$src$core$Rule*)moonbit_make_ref_valtype_array_raw(
      2,
        sizeof(struct $$ZSeanYves$Doclint$src$core$Rule),
        Moonbit_make_regular_object_header(
          offsetof(struct $$ZSeanYves$Doclint$src$core$Rule, $0) >> 2, 2, 0
        )
    );
  _tmp$2201[0] = _tmp$2202;
  _tmp$2201[1] = _tmp$2203;
  _block$2769
  = (struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$*)moonbit_malloc(
      sizeof(
        struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$
      )
    );
  Moonbit_object_header(_block$2769)->meta
  = Moonbit_make_regular_object_header(
    offsetof(
      struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$,
        $0
    )
    >> 2,
      1,
      0
  );
  _block$2769->$0 = _tmp$2201;
  _block$2769->$1 = 2;
  return _block$2769;
}

struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$ThesisRuleset* $ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset(
  
) {
  moonbit_incref(
    $ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$813
  );
  return $ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$813;
}

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$$check(
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* self$809,
  int32_t _ctx$810,
  struct $$ZSeanYves$Doclint$src$core$Document* doc$803
) {
  moonbit_string_t all_text$801 =
    (moonbit_string_t)moonbit_string_literal_26.data;
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* _field$2470 =
    doc$803->$1;
  int32_t _cnt$2666 = Moonbit_object_header(doc$803)->rc;
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* _arr$802;
  int32_t _len$804;
  int32_t _i$805;
  moonbit_string_t _tmp$2190;
  moonbit_string_t _field$2466;
  moonbit_string_t _bind$808;
  int32_t _tmp$2192;
  struct $StringView _tmp$2191;
  if (_cnt$2666 > 1) {
    int32_t _new_cnt$2668 = _cnt$2666 - 1;
    Moonbit_object_header(doc$803)->rc = _new_cnt$2668;
    moonbit_incref(_field$2470);
  } else if (_cnt$2666 == 1) {
    struct $$ZSeanYves$Doclint$src$core$DocMeta* _field$2667 = doc$803->$0;
    moonbit_decref(_field$2667);
    moonbit_free(doc$803);
  }
  _arr$802 = _field$2470;
  _len$804 = _arr$802->$1;
  _i$805 = 0;
  while (1) {
    if (_i$805 < _len$804) {
      struct $$ZSeanYves$Doclint$src$core$Page** _field$2469 = _arr$802->$0;
      struct $$ZSeanYves$Doclint$src$core$Page** buf$2188 = _field$2469;
      struct $$ZSeanYves$Doclint$src$core$Page* _tmp$2468 =
        (struct $$ZSeanYves$Doclint$src$core$Page*)buf$2188[_i$805];
      struct $$ZSeanYves$Doclint$src$core$Page* p$806 = _tmp$2468;
      moonbit_string_t _tmp$2187 = all_text$801;
      moonbit_string_t _tmp$2185 =
        moonbit_add_string(
          _tmp$2187, (moonbit_string_t)moonbit_string_literal_10.data
        );
      moonbit_string_t _field$2467 = p$806->$1;
      moonbit_string_t text$2186 = _field$2467;
      int32_t _tmp$2189;
      moonbit_incref(text$2186);
      all_text$801 = moonbit_add_string(_tmp$2185, text$2186);
      _tmp$2189 = _i$805 + 1;
      _i$805 = _tmp$2189;
      continue;
    } else {
      moonbit_decref(_arr$802);
    }
    break;
  }
  _tmp$2190 = all_text$801;
  _field$2466 = self$809->$1;
  _bind$808 = _field$2466;
  _tmp$2192 = Moonbit_array_length(_bind$808);
  moonbit_incref(_bind$808);
  _tmp$2191 = (struct $StringView){0, _tmp$2192, _bind$808};
  if ($String$$contains(_tmp$2190, _tmp$2191)) {
    struct $$ZSeanYves$Doclint$src$core$Issue** _tmp$2193;
    struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* _block$2771;
    moonbit_decref(self$809);
    _tmp$2193
    = (struct $$ZSeanYves$Doclint$src$core$Issue**)moonbit_empty_ref_array;
    _block$2771
    = (struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$*)moonbit_malloc(
        sizeof(
          struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$
        )
      );
    Moonbit_object_header(_block$2771)->meta
    = Moonbit_make_regular_object_header(
      offsetof(
        struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$,
          $0
      )
      >> 2,
        1,
        0
    );
    _block$2771->$0 = _tmp$2193;
    _block$2771->$1 = 0;
    return _block$2771;
  } else {
    moonbit_string_t _field$2465 = self$809->$0;
    moonbit_string_t rid$2196 = _field$2465;
    int32_t sev$2197 = self$809->$2;
    moonbit_string_t _field$2464 = self$809->$1;
    int32_t _cnt$2669 = Moonbit_object_header(self$809)->rc;
    moonbit_string_t keyword$2200;
    moonbit_string_t _tmp$2198;
    struct $$ZSeanYves$Doclint$src$core$Location* _tmp$2199;
    struct $$ZSeanYves$Doclint$src$core$Issue* _tmp$2195;
    struct $$ZSeanYves$Doclint$src$core$Issue** _tmp$2194;
    struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* _block$2772;
    if (_cnt$2669 > 1) {
      int32_t _new_cnt$2670 = _cnt$2669 - 1;
      Moonbit_object_header(self$809)->rc = _new_cnt$2670;
      moonbit_incref(_field$2464);
      moonbit_incref(rid$2196);
    } else if (_cnt$2669 == 1) {
      moonbit_free(self$809);
    }
    keyword$2200 = _field$2464;
    _tmp$2198
    = moonbit_add_string(
      (moonbit_string_t)moonbit_string_literal_27.data, keyword$2200
    );
    _tmp$2199 = 0;
    _tmp$2195
    = $ZSeanYves$Doclint$src$core$mk_issue(
      rid$2196, sev$2197, _tmp$2198, _tmp$2199
    );
    _tmp$2194
    = (struct $$ZSeanYves$Doclint$src$core$Issue**)moonbit_make_ref_array_raw(
        1
      );
    _tmp$2194[0] = _tmp$2195;
    _block$2772
    = (struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$*)moonbit_malloc(
        sizeof(
          struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$
        )
      );
    Moonbit_object_header(_block$2772)->meta
    = Moonbit_make_regular_object_header(
      offsetof(
        struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$,
          $0
      )
      >> 2,
        1,
        0
    );
    _block$2772->$0 = _tmp$2194;
    _block$2772->$1 = 1;
    return _block$2772;
  }
}

moonbit_string_t $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$$id(
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* self$800
) {
  moonbit_string_t _field$2471 = self$800->$0;
  int32_t _cnt$2671 = Moonbit_object_header(self$800)->rc;
  if (_cnt$2671 > 1) {
    int32_t _new_cnt$2673 = _cnt$2671 - 1;
    Moonbit_object_header(self$800)->rc = _new_cnt$2673;
    moonbit_incref(_field$2471);
  } else if (_cnt$2671 == 1) {
    moonbit_string_t _field$2672 = self$800->$1;
    moonbit_decref(_field$2672);
    moonbit_free(self$800);
  }
  return _field$2471;
}

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$$check(
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* self$798,
  int32_t _ctx$799,
  struct $$ZSeanYves$Doclint$src$core$Document* doc$797
) {
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* _field$2474 =
    doc$797->$1;
  int32_t _cnt$2674 = Moonbit_object_header(doc$797)->rc;
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* pages$2184;
  int32_t _field$2473;
  int32_t n$796;
  int32_t min$2179;
  int32_t _if_result$2773;
  if (_cnt$2674 > 1) {
    int32_t _new_cnt$2676 = _cnt$2674 - 1;
    Moonbit_object_header(doc$797)->rc = _new_cnt$2676;
    moonbit_incref(_field$2474);
  } else if (_cnt$2674 == 1) {
    struct $$ZSeanYves$Doclint$src$core$DocMeta* _field$2675 = doc$797->$0;
    moonbit_decref(_field$2675);
    moonbit_free(doc$797);
  }
  pages$2184 = _field$2474;
  _field$2473 = pages$2184->$1;
  moonbit_decref(pages$2184);
  n$796 = _field$2473;
  min$2179 = self$798->$0;
  if (n$796 < min$2179) {
    moonbit_decref(self$798);
    _if_result$2773 = 1;
  } else {
    int32_t _field$2472 = self$798->$1;
    int32_t max$2178;
    moonbit_decref(self$798);
    max$2178 = _field$2472;
    _if_result$2773 = n$796 > max$2178;
  }
  if (_if_result$2773) {
    struct $$ZSeanYves$Doclint$src$core$Location* _tmp$2182 = 0;
    struct $$ZSeanYves$Doclint$src$core$Issue* _tmp$2181 =
      $ZSeanYves$Doclint$src$core$mk_issue(
        (moonbit_string_t)moonbit_string_literal_28.data,
          2,
          (moonbit_string_t)moonbit_string_literal_29.data,
          _tmp$2182
      );
    struct $$ZSeanYves$Doclint$src$core$Issue** _tmp$2180 =
      (struct $$ZSeanYves$Doclint$src$core$Issue**)moonbit_make_ref_array_raw(
        1
      );
    struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* _block$2774;
    _tmp$2180[0] = _tmp$2181;
    _block$2774
    = (struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$*)moonbit_malloc(
        sizeof(
          struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$
        )
      );
    Moonbit_object_header(_block$2774)->meta
    = Moonbit_make_regular_object_header(
      offsetof(
        struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$,
          $0
      )
      >> 2,
        1,
        0
    );
    _block$2774->$0 = _tmp$2180;
    _block$2774->$1 = 1;
    return _block$2774;
  } else {
    struct $$ZSeanYves$Doclint$src$core$Issue** _tmp$2183 =
      (struct $$ZSeanYves$Doclint$src$core$Issue**)moonbit_empty_ref_array;
    struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* _block$2775 =
      (struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$*)moonbit_malloc(
        sizeof(
          struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$
        )
      );
    Moonbit_object_header(_block$2775)->meta
    = Moonbit_make_regular_object_header(
      offsetof(
        struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$,
          $0
      )
      >> 2,
        1,
        0
    );
    _block$2775->$0 = _tmp$2183;
    _block$2775->$1 = 0;
    return _block$2775;
  }
}

moonbit_string_t $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$$id(
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* _self$795
) {
  moonbit_decref(_self$795);
  return (moonbit_string_t)moonbit_string_literal_28.data;
}

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $ZSeanYves$Doclint$src$core$check_doc(
  int32_t ctx$792,
  struct $$ZSeanYves$Doclint$src$core$Document* doc$793,
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$* rules$794
) {
  return $ZSeanYves$Doclint$src$core$run_rules(ctx$792, doc$793, rules$794);
}

struct $$ZSeanYves$Doclint$src$core$Issue* $ZSeanYves$Doclint$src$core$mk_issue(
  moonbit_string_t rule_id$788,
  int32_t severity$789,
  moonbit_string_t message$790,
  struct $$ZSeanYves$Doclint$src$core$Location* location$791
) {
  struct $$ZSeanYves$Doclint$src$core$Issue* _block$2776 =
    (struct $$ZSeanYves$Doclint$src$core$Issue*)moonbit_malloc(
      sizeof(struct $$ZSeanYves$Doclint$src$core$Issue)
    );
  Moonbit_object_header(_block$2776)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $$ZSeanYves$Doclint$src$core$Issue, $0) >> 2, 3, 0
  );
  _block$2776->$0 = rule_id$788;
  _block$2776->$1 = severity$789;
  _block$2776->$2 = message$790;
  _block$2776->$3 = location$791;
  return _block$2776;
}

int32_t $ZSeanYves$Doclint$src$core$page_count(
  struct $$ZSeanYves$Doclint$src$core$Document* doc$787
) {
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* _field$2476 =
    doc$787->$1;
  int32_t _cnt$2677 = Moonbit_object_header(doc$787)->rc;
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* pages$2177;
  int32_t _field$2475;
  if (_cnt$2677 > 1) {
    int32_t _new_cnt$2679 = _cnt$2677 - 1;
    Moonbit_object_header(doc$787)->rc = _new_cnt$2679;
    moonbit_incref(_field$2476);
  } else if (_cnt$2677 == 1) {
    struct $$ZSeanYves$Doclint$src$core$DocMeta* _field$2678 = doc$787->$0;
    moonbit_decref(_field$2678);
    moonbit_free(doc$787);
  }
  pages$2177 = _field$2476;
  _field$2475 = pages$2177->$1;
  moonbit_decref(pages$2177);
  return _field$2475;
}

void* $ZSeanYves$Doclint$src$core$parse_doc_json(moonbit_string_t s$784) {
  struct $$ZSeanYves$Doclint$src$core$Document* _bind$783 =
    $ZSeanYves$Doclint$src$core$parse_meta_and_pages(s$784);
  if (_bind$783 == 0) {
    if (_bind$783) {
      moonbit_decref(_bind$783);
    }
    moonbit_incref($ZSeanYves$Doclint$src$core$parse_doc_json$constr$782);
    return $ZSeanYves$Doclint$src$core$parse_doc_json$constr$782;
  } else {
    struct $$ZSeanYves$Doclint$src$core$Document* _Some$785 = _bind$783;
    struct $$ZSeanYves$Doclint$src$core$Document* _doc$786 = _Some$785;
    void* _block$2777 =
      (void*)moonbit_malloc(
        sizeof(
          struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Ok
        )
      );
    Moonbit_object_header(_block$2777)->meta
    = Moonbit_make_regular_object_header(
      offsetof(
        struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Ok,
          $0
      )
      >> 2,
        1,
        1
    );
    ((struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Ok*)_block$2777)->$0
    = _doc$786;
    return _block$2777;
  }
}

struct $$ZSeanYves$Doclint$src$core$Document* $ZSeanYves$Doclint$src$core$parse_meta_and_pages(
  moonbit_string_t s$770
) {
  moonbit_string_t _bind$769;
  moonbit_string_t title$771;
  moonbit_string_t _bind$773;
  moonbit_string_t author$774;
  moonbit_string_t _bind$776;
  moonbit_string_t created_at$777;
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* _bind$779;
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* pages$780;
  struct $$ZSeanYves$Doclint$src$core$DocMeta* _tmp$2175;
  struct $$ZSeanYves$Doclint$src$core$Document* _tmp$2174;
  moonbit_incref(s$770);
  _bind$769
  = $ZSeanYves$Doclint$src$core$find_string_field(
    s$770, (moonbit_string_t)moonbit_string_literal_30.data
  );
  if (_bind$769 == 0) {
    if (_bind$769) {
      moonbit_decref(_bind$769);
    }
    title$771 = (moonbit_string_t)moonbit_string_literal_26.data;
  } else {
    moonbit_string_t _Some$772 = _bind$769;
    title$771 = _Some$772;
  }
  moonbit_incref(s$770);
  _bind$773
  = $ZSeanYves$Doclint$src$core$find_string_field(
    s$770, (moonbit_string_t)moonbit_string_literal_31.data
  );
  if (_bind$773 == 0) {
    if (_bind$773) {
      moonbit_decref(_bind$773);
    }
    author$774 = (moonbit_string_t)moonbit_string_literal_26.data;
  } else {
    moonbit_string_t _Some$775 = _bind$773;
    author$774 = _Some$775;
  }
  moonbit_incref(s$770);
  _bind$776
  = $ZSeanYves$Doclint$src$core$find_string_field(
    s$770, (moonbit_string_t)moonbit_string_literal_32.data
  );
  if (_bind$776 == 0) {
    if (_bind$776) {
      moonbit_decref(_bind$776);
    }
    created_at$777 = (moonbit_string_t)moonbit_string_literal_26.data;
  } else {
    moonbit_string_t _Some$778 = _bind$776;
    created_at$777 = _Some$778;
  }
  _bind$779 = $ZSeanYves$Doclint$src$core$parse_pages(s$770);
  if (_bind$779 == 0) {
    struct $$ZSeanYves$Doclint$src$core$Page** _tmp$2176;
    if (_bind$779) {
      moonbit_decref(_bind$779);
    }
    _tmp$2176
    = (struct $$ZSeanYves$Doclint$src$core$Page**)moonbit_empty_ref_array;
    pages$780
    = (struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$*)moonbit_malloc(
        sizeof(
          struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$
        )
      );
    Moonbit_object_header(pages$780)->meta
    = Moonbit_make_regular_object_header(
      offsetof(
        struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$,
          $0
      )
      >> 2,
        1,
        0
    );
    pages$780->$0 = _tmp$2176;
    pages$780->$1 = 0;
  } else {
    struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* _Some$781 =
      _bind$779;
    pages$780 = _Some$781;
  }
  _tmp$2175
  = (struct $$ZSeanYves$Doclint$src$core$DocMeta*)moonbit_malloc(
      sizeof(struct $$ZSeanYves$Doclint$src$core$DocMeta)
    );
  Moonbit_object_header(_tmp$2175)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $$ZSeanYves$Doclint$src$core$DocMeta, $0) >> 2, 3, 0
  );
  _tmp$2175->$0 = title$771;
  _tmp$2175->$1 = author$774;
  _tmp$2175->$2 = created_at$777;
  _tmp$2174
  = (struct $$ZSeanYves$Doclint$src$core$Document*)moonbit_malloc(
      sizeof(struct $$ZSeanYves$Doclint$src$core$Document)
    );
  Moonbit_object_header(_tmp$2174)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $$ZSeanYves$Doclint$src$core$Document, $0) >> 2, 2, 0
  );
  _tmp$2174->$0 = _tmp$2175;
  _tmp$2174->$1 = pages$780;
  return _tmp$2174;
}

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* $ZSeanYves$Doclint$src$core$parse_pages(
  moonbit_string_t s$745
) {
  int64_t _bind$744;
  int32_t ppos$746;
  int32_t _tmp$2173;
  int32_t _tmp$2172;
  int64_t _bind$748;
  int32_t colon$749;
  int32_t _tmp$2171;
  int64_t _bind$751;
  int32_t lb$752;
  int32_t i$754;
  struct $$ZSeanYves$Doclint$src$core$Page** _tmp$2170;
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* pages$755;
  moonbit_incref(s$745);
  moonbit_incref($ZSeanYves$Doclint$src$core$parse_pages$pat$7c$82);
  _bind$744
  = $ZSeanYves$Doclint$src$core$find_sub(
    s$745, $ZSeanYves$Doclint$src$core$parse_pages$pat$7c$82, 0
  );
  if (_bind$744 == 4294967296ll) {
    moonbit_decref(s$745);
    return 0;
  } else {
    int64_t _Some$747 = _bind$744;
    ppos$746 = (int32_t)_Some$747;
  }
  _tmp$2173
  = Moonbit_array_length(
    $ZSeanYves$Doclint$src$core$parse_pages$pat$7c$82
  );
  _tmp$2172 = ppos$746 + _tmp$2173;
  moonbit_incref(s$745);
  _bind$748 = $ZSeanYves$Doclint$src$core$find_char(s$745, 58, _tmp$2172);
  if (_bind$748 == 4294967296ll) {
    moonbit_decref(s$745);
    return 0;
  } else {
    int64_t _Some$750 = _bind$748;
    colon$749 = (int32_t)_Some$750;
  }
  _tmp$2171 = colon$749 + 1;
  moonbit_incref(s$745);
  _bind$751 = $ZSeanYves$Doclint$src$core$find_char(s$745, 91, _tmp$2171);
  if (_bind$751 == 4294967296ll) {
    moonbit_decref(s$745);
    return 0;
  } else {
    int64_t _Some$753 = _bind$751;
    lb$752 = (int32_t)_Some$753;
  }
  i$754 = lb$752 + 1;
  _tmp$2170
  = (struct $$ZSeanYves$Doclint$src$core$Page**)moonbit_empty_ref_array;
  pages$755
  = (struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$*)moonbit_malloc(
      sizeof(
        struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$
      )
    );
  Moonbit_object_header(pages$755)->meta
  = Moonbit_make_regular_object_header(
    offsetof(
      struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$,
        $0
    )
    >> 2,
      1,
      0
  );
  pages$755->$0 = _tmp$2170;
  pages$755->$1 = 0;
  while (1) {
    int32_t _tmp$2160 = i$754;
    int32_t _tmp$2161;
    int32_t _tmp$2162;
    int32_t _tmp$2164;
    int32_t _tmp$2166;
    int32_t _tmp$2165;
    moonbit_string_t _tmp$2163;
    int32_t _tmp$2477;
    int32_t _tmp$2169;
    int64_t _bind$757;
    int32_t ob$758;
    int64_t _bind$760;
    int32_t idx$761;
    moonbit_string_t _bind$763;
    moonbit_string_t txt$764;
    struct $$ZSeanYves$Doclint$src$core$Page* _tmp$2167;
    int32_t _tmp$2168;
    int64_t _bind$766;
    int32_t cb$767;
    moonbit_incref(s$745);
    i$754 = $ZSeanYves$Doclint$src$core$skip_ws_comma(s$745, _tmp$2160);
    _tmp$2161 = i$754;
    _tmp$2162 = Moonbit_array_length(s$745);
    if (_tmp$2161 >= _tmp$2162) {
      moonbit_decref(pages$755);
      moonbit_decref(s$745);
      return 0;
    }
    _tmp$2164 = i$754;
    _tmp$2166 = i$754;
    _tmp$2165 = _tmp$2166 + 1;
    moonbit_incref(s$745);
    _tmp$2163
    = $ZSeanYves$Doclint$src$core$slice(
      s$745, _tmp$2164, _tmp$2165
    );
    _tmp$2477
    = moonbit_val_array_equal(
      _tmp$2163, (moonbit_string_t)moonbit_string_literal_33.data
    );
    moonbit_decref(_tmp$2163);
    if (_tmp$2477) {
      moonbit_decref(s$745);
      break;
    }
    _tmp$2169 = i$754;
    moonbit_incref(s$745);
    _bind$757 = $ZSeanYves$Doclint$src$core$find_char(s$745, 123, _tmp$2169);
    if (_bind$757 == 4294967296ll) {
      moonbit_decref(pages$755);
      moonbit_decref(s$745);
      return 0;
    } else {
      int64_t _Some$759 = _bind$757;
      ob$758 = (int32_t)_Some$759;
    }
    moonbit_incref(s$745);
    _bind$760
    = $ZSeanYves$Doclint$src$core$find_int_field_from(
      s$745, (moonbit_string_t)moonbit_string_literal_34.data, ob$758
    );
    if (_bind$760 == 4294967296ll) {
      moonbit_decref(pages$755);
      moonbit_decref(s$745);
      return 0;
    } else {
      int64_t _Some$762 = _bind$760;
      idx$761 = (int32_t)_Some$762;
    }
    moonbit_incref(s$745);
    _bind$763
    = $ZSeanYves$Doclint$src$core$find_string_field_from(
      s$745, (moonbit_string_t)moonbit_string_literal_35.data, ob$758
    );
    if (_bind$763 == 0) {
      if (_bind$763) {
        moonbit_decref(_bind$763);
      }
      moonbit_decref(pages$755);
      moonbit_decref(s$745);
      return 0;
    } else {
      moonbit_string_t _Some$765 = _bind$763;
      txt$764 = _Some$765;
    }
    _tmp$2167
    = (struct $$ZSeanYves$Doclint$src$core$Page*)moonbit_malloc(
        sizeof(struct $$ZSeanYves$Doclint$src$core$Page)
      );
    Moonbit_object_header(_tmp$2167)->meta
    = Moonbit_make_regular_object_header(
      offsetof(struct $$ZSeanYves$Doclint$src$core$Page, $1) >> 2, 1, 0
    );
    _tmp$2167->$0 = idx$761;
    _tmp$2167->$1 = txt$764;
    moonbit_incref(pages$755);
    $$moonbitlang$core$builtin$Array$$push$2(pages$755, _tmp$2167);
    _tmp$2168 = ob$758 + 1;
    moonbit_incref(s$745);
    _bind$766 = $ZSeanYves$Doclint$src$core$find_char(s$745, 125, _tmp$2168);
    if (_bind$766 == 4294967296ll) {
      moonbit_decref(pages$755);
      moonbit_decref(s$745);
      return 0;
    } else {
      int64_t _Some$768 = _bind$766;
      cb$767 = (int32_t)_Some$768;
    }
    i$754 = cb$767 + 1;
    continue;
    break;
  }
  return pages$755;
}

int32_t $ZSeanYves$Doclint$src$core$skip_ws_comma(
  moonbit_string_t s$741,
  int32_t i$740
) {
  int32_t n$739 = i$740;
  while (1) {
    int32_t _tmp$2154 = n$739;
    int32_t _tmp$2155 = Moonbit_array_length(s$741);
    if (_tmp$2154 < _tmp$2155) {
      int32_t _tmp$2157 = n$739;
      int32_t _tmp$2159 = n$739;
      int32_t _tmp$2158 = _tmp$2159 + 1;
      moonbit_string_t c$742;
      int32_t _if_result$2780;
      moonbit_incref(s$741);
      c$742 = $ZSeanYves$Doclint$src$core$slice(s$741, _tmp$2157, _tmp$2158);
      if (
        moonbit_val_array_equal(
          c$742, (moonbit_string_t)moonbit_string_literal_36.data
        )
      ) {
        moonbit_decref(c$742);
        _if_result$2780 = 1;
      } else if (
               moonbit_val_array_equal(
                 c$742, (moonbit_string_t)moonbit_string_literal_10.data
               )
             ) {
        moonbit_decref(c$742);
        _if_result$2780 = 1;
      } else if (
               moonbit_val_array_equal(
                 c$742, (moonbit_string_t)moonbit_string_literal_37.data
               )
             ) {
        moonbit_decref(c$742);
        _if_result$2780 = 1;
      } else if (
               moonbit_val_array_equal(
                 c$742, (moonbit_string_t)moonbit_string_literal_38.data
               )
             ) {
        moonbit_decref(c$742);
        _if_result$2780 = 1;
      } else {
        int32_t _tmp$2478 =
          moonbit_val_array_equal(
            c$742, (moonbit_string_t)moonbit_string_literal_0.data
          );
        moonbit_decref(c$742);
        _if_result$2780 = _tmp$2478;
      }
      if (_if_result$2780) {
        int32_t _tmp$2156 = n$739;
        n$739 = _tmp$2156 + 1;
      } else {
        moonbit_decref(s$741);
        break;
      }
      continue;
    } else {
      moonbit_decref(s$741);
    }
    break;
  }
  return n$739;
}

moonbit_string_t $ZSeanYves$Doclint$src$core$find_string_field_from(
  moonbit_string_t s$725,
  moonbit_string_t key$723,
  int32_t start$726
) {
  moonbit_string_t _tmp$2153 =
    moonbit_add_string(
      (moonbit_string_t)moonbit_string_literal_39.data, key$723
    );
  moonbit_string_t pat$722 =
    moonbit_add_string(
      _tmp$2153, (moonbit_string_t)moonbit_string_literal_39.data
    );
  int64_t _bind$724;
  int32_t kpos$727;
  int32_t _tmp$2480;
  int32_t _tmp$2152;
  int32_t _tmp$2151;
  int64_t _bind$729;
  int32_t colon$730;
  int32_t _tmp$2150;
  int64_t _bind$732;
  int32_t q1$733;
  int32_t _tmp$2149;
  struct $$3c$String$2a$Int$3e$* _bind$735;
  struct $$3c$String$2a$Int$3e$* _bind$736;
  moonbit_string_t _field$2479;
  int32_t _cnt$2680;
  moonbit_string_t _val$738;
  moonbit_incref(s$725);
  moonbit_incref(pat$722);
  _bind$724 = $ZSeanYves$Doclint$src$core$find_sub(s$725, pat$722, start$726);
  if (_bind$724 == 4294967296ll) {
    moonbit_decref(s$725);
    moonbit_decref(pat$722);
    return 0;
  } else {
    int64_t _Some$728 = _bind$724;
    kpos$727 = (int32_t)_Some$728;
  }
  _tmp$2480 = Moonbit_array_length(pat$722);
  moonbit_decref(pat$722);
  _tmp$2152 = _tmp$2480;
  _tmp$2151 = kpos$727 + _tmp$2152;
  moonbit_incref(s$725);
  _bind$729 = $ZSeanYves$Doclint$src$core$find_char(s$725, 58, _tmp$2151);
  if (_bind$729 == 4294967296ll) {
    moonbit_decref(s$725);
    return 0;
  } else {
    int64_t _Some$731 = _bind$729;
    colon$730 = (int32_t)_Some$731;
  }
  _tmp$2150 = colon$730 + 1;
  moonbit_incref(s$725);
  _bind$732 = $ZSeanYves$Doclint$src$core$find_char(s$725, 34, _tmp$2150);
  if (_bind$732 == 4294967296ll) {
    moonbit_decref(s$725);
    return 0;
  } else {
    int64_t _Some$734 = _bind$732;
    q1$733 = (int32_t)_Some$734;
  }
  _tmp$2149 = q1$733 + 1;
  _bind$735 = $ZSeanYves$Doclint$src$core$read_json_string(s$725, _tmp$2149);
  if (_bind$735 == 0) {
    if (_bind$735) {
      moonbit_decref(_bind$735);
    }
    return 0;
  } else {
    struct $$3c$String$2a$Int$3e$* _Some$737 = _bind$735;
    _bind$736 = _Some$737;
  }
  _field$2479 = _bind$736->$0;
  _cnt$2680 = Moonbit_object_header(_bind$736)->rc;
  if (_cnt$2680 > 1) {
    int32_t _new_cnt$2681 = _cnt$2680 - 1;
    Moonbit_object_header(_bind$736)->rc = _new_cnt$2681;
    moonbit_incref(_field$2479);
  } else if (_cnt$2680 == 1) {
    moonbit_free(_bind$736);
  }
  _val$738 = _field$2479;
  return _val$738;
}

int64_t $ZSeanYves$Doclint$src$core$find_int_field_from(
  moonbit_string_t s$705,
  moonbit_string_t key$703,
  int32_t start$706
) {
  moonbit_string_t _tmp$2148 =
    moonbit_add_string(
      (moonbit_string_t)moonbit_string_literal_39.data, key$703
    );
  moonbit_string_t pat$702 =
    moonbit_add_string(
      _tmp$2148, (moonbit_string_t)moonbit_string_literal_39.data
    );
  int64_t _bind$704;
  int32_t kpos$707;
  int32_t _tmp$2482;
  int32_t _tmp$2147;
  int32_t _tmp$2146;
  int64_t _bind$709;
  int32_t colon$710;
  int32_t _tmp$2145;
  int32_t i$712;
  int32_t neg$713;
  int32_t _tmp$2132;
  int32_t _tmp$2133;
  int32_t _if_result$2781;
  int32_t j$714;
  int32_t n$715;
  int32_t _tmp$2140;
  int32_t _tmp$2141;
  int32_t _tmp$2143;
  int32_t _tmp$2144;
  moonbit_string_t digits$718;
  int64_t _bind$719;
  int32_t num$720;
  int32_t _tmp$2142;
  moonbit_incref(s$705);
  moonbit_incref(pat$702);
  _bind$704 = $ZSeanYves$Doclint$src$core$find_sub(s$705, pat$702, start$706);
  if (_bind$704 == 4294967296ll) {
    moonbit_decref(s$705);
    moonbit_decref(pat$702);
    return 4294967296ll;
  } else {
    int64_t _Some$708 = _bind$704;
    kpos$707 = (int32_t)_Some$708;
  }
  _tmp$2482 = Moonbit_array_length(pat$702);
  moonbit_decref(pat$702);
  _tmp$2147 = _tmp$2482;
  _tmp$2146 = kpos$707 + _tmp$2147;
  moonbit_incref(s$705);
  _bind$709 = $ZSeanYves$Doclint$src$core$find_char(s$705, 58, _tmp$2146);
  if (_bind$709 == 4294967296ll) {
    moonbit_decref(s$705);
    return 4294967296ll;
  } else {
    int64_t _Some$711 = _bind$709;
    colon$710 = (int32_t)_Some$711;
  }
  _tmp$2145 = colon$710 + 1;
  moonbit_incref(s$705);
  i$712 = $ZSeanYves$Doclint$src$core$skip_ws(s$705, _tmp$2145);
  neg$713 = 0;
  _tmp$2132 = i$712;
  _tmp$2133 = Moonbit_array_length(s$705);
  if (_tmp$2132 < _tmp$2133) {
    int32_t _tmp$2129 = i$712;
    int32_t _tmp$2131 = i$712;
    int32_t _tmp$2130 = _tmp$2131 + 1;
    moonbit_string_t _tmp$2128;
    int32_t _tmp$2481;
    moonbit_incref(s$705);
    _tmp$2128
    = $ZSeanYves$Doclint$src$core$slice(
      s$705, _tmp$2129, _tmp$2130
    );
    _tmp$2481
    = moonbit_val_array_equal(
      _tmp$2128, (moonbit_string_t)moonbit_string_literal_40.data
    );
    moonbit_decref(_tmp$2128);
    _if_result$2781 = _tmp$2481;
  } else {
    _if_result$2781 = 0;
  }
  if (_if_result$2781) {
    int32_t _tmp$2134;
    neg$713 = 1;
    _tmp$2134 = i$712;
    i$712 = _tmp$2134 + 1;
  }
  j$714 = i$712;
  n$715 = Moonbit_array_length(s$705);
  while (1) {
    int32_t _tmp$2135 = j$714;
    if (_tmp$2135 < n$715) {
      int32_t _tmp$2137 = j$714;
      int32_t _tmp$2139 = j$714;
      int32_t _tmp$2138 = _tmp$2139 + 1;
      moonbit_string_t c$716;
      int32_t _if_result$2783;
      moonbit_incref(s$705);
      c$716 = $ZSeanYves$Doclint$src$core$slice(s$705, _tmp$2137, _tmp$2138);
      moonbit_incref(c$716);
      if (
        $$moonbitlang$core$builtin$Compare$$$default_impl$$op_ge$0(
          c$716, (moonbit_string_t)moonbit_string_literal_41.data
        )
      ) {
        _if_result$2783
        = $$moonbitlang$core$builtin$Compare$$$default_impl$$op_le$0(
          c$716, (moonbit_string_t)moonbit_string_literal_42.data
        );
      } else {
        moonbit_decref(c$716);
        _if_result$2783 = 0;
      }
      if (_if_result$2783) {
        int32_t _tmp$2136 = j$714;
        j$714 = _tmp$2136 + 1;
      } else {
        break;
      }
      continue;
    }
    break;
  }
  _tmp$2140 = j$714;
  _tmp$2141 = i$712;
  if (_tmp$2140 == _tmp$2141) {
    moonbit_decref(s$705);
    return 4294967296ll;
  }
  _tmp$2143 = i$712;
  _tmp$2144 = j$714;
  digits$718 = $ZSeanYves$Doclint$src$core$slice(s$705, _tmp$2143, _tmp$2144);
  _bind$719 = $ZSeanYves$Doclint$src$core$parse_int_dec(digits$718);
  if (_bind$719 == 4294967296ll) {
    return 4294967296ll;
  } else {
    int64_t _Some$721 = _bind$719;
    num$720 = (int32_t)_Some$721;
  }
  if (neg$713) {
    _tmp$2142 = -num$720;
  } else {
    _tmp$2142 = num$720;
  }
  return (int64_t)_tmp$2142;
}

int32_t $ZSeanYves$Doclint$src$core$skip_ws(
  moonbit_string_t s$699,
  int32_t i$698
) {
  int32_t n$697 = i$698;
  while (1) {
    int32_t _tmp$2122 = n$697;
    int32_t _tmp$2123 = Moonbit_array_length(s$699);
    if (_tmp$2122 < _tmp$2123) {
      int32_t _tmp$2125 = n$697;
      int32_t _tmp$2127 = n$697;
      int32_t _tmp$2126 = _tmp$2127 + 1;
      moonbit_string_t c$700;
      int32_t _if_result$2785;
      moonbit_incref(s$699);
      c$700 = $ZSeanYves$Doclint$src$core$slice(s$699, _tmp$2125, _tmp$2126);
      if (
        moonbit_val_array_equal(
          c$700, (moonbit_string_t)moonbit_string_literal_36.data
        )
      ) {
        moonbit_decref(c$700);
        _if_result$2785 = 1;
      } else if (
               moonbit_val_array_equal(
                 c$700, (moonbit_string_t)moonbit_string_literal_10.data
               )
             ) {
        moonbit_decref(c$700);
        _if_result$2785 = 1;
      } else if (
               moonbit_val_array_equal(
                 c$700, (moonbit_string_t)moonbit_string_literal_37.data
               )
             ) {
        moonbit_decref(c$700);
        _if_result$2785 = 1;
      } else {
        int32_t _tmp$2483 =
          moonbit_val_array_equal(
            c$700, (moonbit_string_t)moonbit_string_literal_38.data
          );
        moonbit_decref(c$700);
        _if_result$2785 = _tmp$2483;
      }
      if (_if_result$2785) {
        int32_t _tmp$2124 = n$697;
        n$697 = _tmp$2124 + 1;
      } else {
        moonbit_decref(s$699);
        break;
      }
      continue;
    } else {
      moonbit_decref(s$699);
    }
    break;
  }
  return n$697;
}

int64_t $ZSeanYves$Doclint$src$core$parse_int_dec(moonbit_string_t s$692) {
  int32_t acc$689 = 0;
  int32_t any$690 = 0;
  struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* _it$691 = $String$$iter(s$692);
  while (1) {
    int32_t _bind$693;
    moonbit_incref(_it$691);
    _bind$693 = $$moonbitlang$core$builtin$Iter$$next$1(_it$691);
    if (_bind$693 == -1) {
      moonbit_decref(_it$691);
    } else {
      int32_t _Some$694 = _bind$693;
      int32_t _ch$695 = _Some$694;
      int32_t _tmp$2120;
      int32_t _tmp$2116;
      int32_t _tmp$2118;
      int32_t _tmp$2119;
      int32_t _tmp$2117;
      if (_ch$695 < 48 || _ch$695 > 57) {
        moonbit_decref(_it$691);
        return 4294967296ll;
      }
      _tmp$2120 = acc$689;
      _tmp$2116 = _tmp$2120 * 10;
      _tmp$2118 = _ch$695;
      _tmp$2119 = 48;
      _tmp$2117 = _tmp$2118 - _tmp$2119;
      acc$689 = _tmp$2116 + _tmp$2117;
      any$690 = 1;
      continue;
    }
    break;
  }
  if (any$690) {
    int32_t _tmp$2121 = acc$689;
    return (int64_t)_tmp$2121;
  } else {
    return 4294967296ll;
  }
}

moonbit_string_t $ZSeanYves$Doclint$src$core$find_string_field(
  moonbit_string_t s$676,
  moonbit_string_t key$674
) {
  moonbit_string_t _tmp$2115 =
    moonbit_add_string(
      (moonbit_string_t)moonbit_string_literal_39.data, key$674
    );
  moonbit_string_t pat$673 =
    moonbit_add_string(
      _tmp$2115, (moonbit_string_t)moonbit_string_literal_39.data
    );
  int64_t _bind$675;
  int32_t kpos$677;
  int32_t _tmp$2485;
  int32_t _tmp$2114;
  int32_t _tmp$2113;
  int64_t _bind$679;
  int32_t colon$680;
  int32_t _tmp$2112;
  int64_t _bind$682;
  int32_t q1$683;
  int32_t _tmp$2111;
  struct $$3c$String$2a$Int$3e$* _bind$685;
  struct $$3c$String$2a$Int$3e$* _bind$686;
  moonbit_string_t _field$2484;
  int32_t _cnt$2682;
  moonbit_string_t _val$688;
  moonbit_incref(s$676);
  moonbit_incref(pat$673);
  _bind$675 = $ZSeanYves$Doclint$src$core$find_sub(s$676, pat$673, 0);
  if (_bind$675 == 4294967296ll) {
    moonbit_decref(s$676);
    moonbit_decref(pat$673);
    return 0;
  } else {
    int64_t _Some$678 = _bind$675;
    kpos$677 = (int32_t)_Some$678;
  }
  _tmp$2485 = Moonbit_array_length(pat$673);
  moonbit_decref(pat$673);
  _tmp$2114 = _tmp$2485;
  _tmp$2113 = kpos$677 + _tmp$2114;
  moonbit_incref(s$676);
  _bind$679 = $ZSeanYves$Doclint$src$core$find_char(s$676, 58, _tmp$2113);
  if (_bind$679 == 4294967296ll) {
    moonbit_decref(s$676);
    return 0;
  } else {
    int64_t _Some$681 = _bind$679;
    colon$680 = (int32_t)_Some$681;
  }
  _tmp$2112 = colon$680 + 1;
  moonbit_incref(s$676);
  _bind$682 = $ZSeanYves$Doclint$src$core$find_char(s$676, 34, _tmp$2112);
  if (_bind$682 == 4294967296ll) {
    moonbit_decref(s$676);
    return 0;
  } else {
    int64_t _Some$684 = _bind$682;
    q1$683 = (int32_t)_Some$684;
  }
  _tmp$2111 = q1$683 + 1;
  _bind$685 = $ZSeanYves$Doclint$src$core$read_json_string(s$676, _tmp$2111);
  if (_bind$685 == 0) {
    if (_bind$685) {
      moonbit_decref(_bind$685);
    }
    return 0;
  } else {
    struct $$3c$String$2a$Int$3e$* _Some$687 = _bind$685;
    _bind$686 = _Some$687;
  }
  _field$2484 = _bind$686->$0;
  _cnt$2682 = Moonbit_object_header(_bind$686)->rc;
  if (_cnt$2682 > 1) {
    int32_t _new_cnt$2683 = _cnt$2682 - 1;
    Moonbit_object_header(_bind$686)->rc = _new_cnt$2683;
    moonbit_incref(_field$2484);
  } else if (_cnt$2682 == 1) {
    moonbit_free(_bind$686);
  }
  _val$688 = _field$2484;
  return _val$688;
}

struct $$3c$String$2a$Int$3e$* $ZSeanYves$Doclint$src$core$read_json_string(
  moonbit_string_t s$669,
  int32_t start$668
) {
  moonbit_string_t out$666 = (moonbit_string_t)moonbit_string_literal_26.data;
  int32_t i$667 = start$668;
  while (1) {
    int32_t _tmp$2086 = i$667;
    int32_t _tmp$2087 = Moonbit_array_length(s$669);
    if (_tmp$2086 < _tmp$2087) {
      int32_t _tmp$2108 = i$667;
      int32_t _tmp$2110 = i$667;
      int32_t _tmp$2109 = _tmp$2110 + 1;
      moonbit_string_t ch$670;
      moonbit_incref(s$669);
      ch$670 = $ZSeanYves$Doclint$src$core$slice(s$669, _tmp$2108, _tmp$2109);
      if (
        moonbit_val_array_equal(
          ch$670, (moonbit_string_t)moonbit_string_literal_39.data
        )
      ) {
        moonbit_string_t _tmp$2089;
        int32_t _tmp$2091;
        int32_t _tmp$2090;
        struct $$3c$String$2a$Int$3e$* _tuple$2088;
        moonbit_decref(ch$670);
        moonbit_decref(s$669);
        _tmp$2089 = out$666;
        _tmp$2091 = i$667;
        _tmp$2090 = _tmp$2091 + 1;
        _tuple$2088
        = (struct $$3c$String$2a$Int$3e$*)moonbit_malloc(
            sizeof(struct $$3c$String$2a$Int$3e$)
          );
        Moonbit_object_header(_tuple$2088)->meta
        = Moonbit_make_regular_object_header(
          offsetof(struct $$3c$String$2a$Int$3e$, $0) >> 2, 1, 0
        );
        _tuple$2088->$0 = _tmp$2089;
        _tuple$2088->$1 = _tmp$2090;
        return _tuple$2088;
      } else if (
               moonbit_val_array_equal(
                 ch$670, (moonbit_string_t)moonbit_string_literal_43.data
               )
             ) {
        int32_t _tmp$2094;
        int32_t _tmp$2092;
        int32_t _tmp$2093;
        int32_t _tmp$2105;
        int32_t _tmp$2102;
        int32_t _tmp$2104;
        int32_t _tmp$2103;
        moonbit_string_t esc$671;
        int32_t _tmp$2101;
        moonbit_decref(ch$670);
        _tmp$2094 = i$667;
        _tmp$2092 = _tmp$2094 + 1;
        _tmp$2093 = Moonbit_array_length(s$669);
        if (_tmp$2092 >= _tmp$2093) {
          moonbit_decref(out$666);
          moonbit_decref(s$669);
          return 0;
        }
        _tmp$2105 = i$667;
        _tmp$2102 = _tmp$2105 + 1;
        _tmp$2104 = i$667;
        _tmp$2103 = _tmp$2104 + 2;
        moonbit_incref(s$669);
        esc$671
        = $ZSeanYves$Doclint$src$core$slice(
          s$669, _tmp$2102, _tmp$2103
        );
        if (
          moonbit_val_array_equal(
            esc$671, (moonbit_string_t)moonbit_string_literal_46.data
          )
        ) {
          moonbit_string_t _tmp$2095;
          moonbit_decref(esc$671);
          _tmp$2095 = out$666;
          out$666
          = moonbit_add_string(
            _tmp$2095, (moonbit_string_t)moonbit_string_literal_10.data
          );
        } else if (
                 moonbit_val_array_equal(
                   esc$671, (moonbit_string_t)moonbit_string_literal_45.data
                 )
               ) {
          moonbit_string_t _tmp$2096;
          moonbit_decref(esc$671);
          _tmp$2096 = out$666;
          out$666
          = moonbit_add_string(
            _tmp$2096, (moonbit_string_t)moonbit_string_literal_37.data
          );
        } else if (
                 moonbit_val_array_equal(
                   esc$671, (moonbit_string_t)moonbit_string_literal_44.data
                 )
               ) {
          moonbit_string_t _tmp$2097;
          moonbit_decref(esc$671);
          _tmp$2097 = out$666;
          out$666
          = moonbit_add_string(
            _tmp$2097, (moonbit_string_t)moonbit_string_literal_38.data
          );
        } else if (
                 moonbit_val_array_equal(
                   esc$671, (moonbit_string_t)moonbit_string_literal_39.data
                 )
               ) {
          moonbit_string_t _tmp$2098;
          moonbit_decref(esc$671);
          _tmp$2098 = out$666;
          out$666
          = moonbit_add_string(
            _tmp$2098, (moonbit_string_t)moonbit_string_literal_39.data
          );
        } else if (
                 moonbit_val_array_equal(
                   esc$671, (moonbit_string_t)moonbit_string_literal_43.data
                 )
               ) {
          moonbit_string_t _tmp$2099;
          moonbit_decref(esc$671);
          _tmp$2099 = out$666;
          out$666
          = moonbit_add_string(
            _tmp$2099, (moonbit_string_t)moonbit_string_literal_43.data
          );
        } else {
          moonbit_string_t _tmp$2100 = out$666;
          out$666 = moonbit_add_string(_tmp$2100, esc$671);
        }
        _tmp$2101 = i$667;
        i$667 = _tmp$2101 + 2;
      } else {
        moonbit_string_t _tmp$2106 = out$666;
        int32_t _tmp$2107;
        out$666 = moonbit_add_string(_tmp$2106, ch$670);
        _tmp$2107 = i$667;
        i$667 = _tmp$2107 + 1;
      }
      continue;
    } else {
      moonbit_decref(out$666);
      moonbit_decref(s$669);
    }
    break;
  }
  return 0;
}

int64_t $ZSeanYves$Doclint$src$core$find_sub(
  moonbit_string_t s$660,
  moonbit_string_t pat$662,
  int32_t start$663
) {
  int32_t n$659 = Moonbit_array_length(s$660);
  int32_t m$661 = Moonbit_array_length(pat$662);
  int32_t i$664;
  if (m$661 == 0) {
    moonbit_decref(pat$662);
    moonbit_decref(s$660);
    return (int64_t)start$663;
  }
  i$664 = start$663;
  while (1) {
    int32_t _tmp$2079 = i$664;
    int32_t _tmp$2078 = _tmp$2079 + m$661;
    if (_tmp$2078 <= n$659) {
      int32_t _tmp$2081 = i$664;
      int32_t _tmp$2083 = i$664;
      int32_t _tmp$2082 = _tmp$2083 + m$661;
      moonbit_string_t _tmp$2080;
      int32_t _tmp$2486;
      int32_t _tmp$2085;
      moonbit_incref(s$660);
      _tmp$2080
      = $ZSeanYves$Doclint$src$core$slice(
        s$660, _tmp$2081, _tmp$2082
      );
      _tmp$2486 = moonbit_val_array_equal(_tmp$2080, pat$662);
      moonbit_decref(_tmp$2080);
      if (_tmp$2486) {
        int32_t _tmp$2084;
        moonbit_decref(pat$662);
        moonbit_decref(s$660);
        _tmp$2084 = i$664;
        return (int64_t)_tmp$2084;
      }
      _tmp$2085 = i$664;
      i$664 = _tmp$2085 + 1;
      continue;
    } else {
      moonbit_decref(pat$662);
      moonbit_decref(s$660);
    }
    break;
  }
  return 4294967296ll;
}

int64_t $ZSeanYves$Doclint$src$core$find_char(
  moonbit_string_t s$656,
  int32_t ch$657,
  int32_t start$655
) {
  int32_t i$654 = start$655;
  while (1) {
    int32_t _tmp$2069 = i$654;
    int32_t _tmp$2070 = Moonbit_array_length(s$656);
    if (_tmp$2069 < _tmp$2070) {
      int32_t _tmp$2073 = i$654;
      int32_t _tmp$2075 = i$654;
      int32_t _tmp$2074 = _tmp$2075 + 1;
      moonbit_string_t _tmp$2071;
      moonbit_string_t _tmp$2072;
      int32_t _tmp$2487;
      int32_t _tmp$2077;
      moonbit_incref(s$656);
      _tmp$2071
      = $ZSeanYves$Doclint$src$core$slice(
        s$656, _tmp$2073, _tmp$2074
      );
      _tmp$2072 = $$moonbitlang$core$builtin$Show$$Char$$to_string(ch$657);
      _tmp$2487 = moonbit_val_array_equal(_tmp$2071, _tmp$2072);
      moonbit_decref(_tmp$2071);
      moonbit_decref(_tmp$2072);
      if (_tmp$2487) {
        int32_t _tmp$2076;
        moonbit_decref(s$656);
        _tmp$2076 = i$654;
        return (int64_t)_tmp$2076;
      }
      _tmp$2077 = i$654;
      i$654 = _tmp$2077 + 1;
      continue;
    } else {
      moonbit_decref(s$656);
    }
    break;
  }
  return 4294967296ll;
}

moonbit_string_t $ZSeanYves$Doclint$src$core$slice(
  moonbit_string_t s$647,
  int32_t start$651,
  int32_t end_$652
) {
  moonbit_string_t out$644 = (moonbit_string_t)moonbit_string_literal_26.data;
  int32_t i$645 = 0;
  struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* _it$646 = $String$$iter(s$647);
  while (1) {
    int32_t _bind$648;
    moonbit_incref(_it$646);
    _bind$648 = $$moonbitlang$core$builtin$Iter$$next$1(_it$646);
    if (_bind$648 == -1) {
      moonbit_decref(_it$646);
    } else {
      int32_t _Some$649 = _bind$648;
      int32_t _ch$650 = _Some$649;
      int32_t _tmp$2065 = i$645;
      int32_t _if_result$2791;
      int32_t _tmp$2068;
      if (_tmp$2065 >= start$651) {
        int32_t _tmp$2064 = i$645;
        _if_result$2791 = _tmp$2064 < end_$652;
      } else {
        _if_result$2791 = 0;
      }
      if (_if_result$2791) {
        moonbit_string_t _tmp$2066 = out$644;
        moonbit_string_t _tmp$2067 =
          $$moonbitlang$core$builtin$Show$$Char$$to_string(_ch$650);
        out$644 = moonbit_add_string(_tmp$2066, _tmp$2067);
      }
      _tmp$2068 = i$645;
      i$645 = _tmp$2068 + 1;
      continue;
    }
    break;
  }
  return out$644;
}

moonbit_string_t $ZSeanYves$Doclint$src$core$issues_to_html(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* issues$629
) {
  int32_t err$625 = 0;
  int32_t warn$626 = 0;
  int32_t info$627 = 0;
  int32_t _len$628 = issues$629->$1;
  int32_t _i$630 = 0;
  moonbit_string_t err_rows$634;
  moonbit_string_t warn_rows$635;
  moonbit_string_t info_rows$636;
  int32_t _len$637;
  int32_t _i$638;
  int32_t _field$2488;
  int32_t len$2063;
  moonbit_string_t total$643;
  moonbit_string_t _tmp$2062;
  moonbit_string_t _tmp$2061;
  moonbit_string_t _tmp$2060;
  moonbit_string_t _tmp$2059;
  moonbit_string_t _tmp$2058;
  moonbit_string_t _tmp$2057;
  moonbit_string_t _tmp$2056;
  moonbit_string_t _tmp$2055;
  moonbit_string_t _tmp$2054;
  moonbit_string_t _tmp$2053;
  moonbit_string_t _tmp$2052;
  moonbit_string_t _tmp$2051;
  moonbit_string_t _tmp$2050;
  moonbit_string_t _tmp$2049;
  moonbit_string_t _tmp$2048;
  moonbit_string_t _tmp$2047;
  moonbit_string_t _tmp$2046;
  moonbit_string_t _tmp$2045;
  moonbit_string_t _tmp$2044;
  moonbit_string_t _tmp$2043;
  moonbit_string_t _tmp$2042;
  moonbit_string_t _tmp$2041;
  moonbit_string_t _tmp$2040;
  moonbit_string_t _tmp$2039;
  moonbit_string_t _tmp$2038;
  moonbit_string_t _tmp$2037;
  moonbit_string_t _tmp$2036;
  moonbit_string_t _tmp$2033;
  int32_t _tmp$2035;
  moonbit_string_t _tmp$2034;
  moonbit_string_t _tmp$2032;
  moonbit_string_t _tmp$2031;
  moonbit_string_t _tmp$2028;
  int32_t _tmp$2030;
  moonbit_string_t _tmp$2029;
  moonbit_string_t _tmp$2027;
  moonbit_string_t _tmp$2026;
  moonbit_string_t _tmp$2023;
  int32_t _tmp$2025;
  moonbit_string_t _tmp$2024;
  moonbit_string_t _tmp$2022;
  moonbit_string_t _tmp$2021;
  moonbit_string_t _tmp$2020;
  moonbit_string_t _tmp$2017;
  moonbit_string_t _tmp$2019;
  moonbit_string_t _tmp$2018;
  moonbit_string_t _tmp$2014;
  moonbit_string_t _tmp$2016;
  moonbit_string_t _tmp$2015;
  moonbit_string_t _tmp$2011;
  moonbit_string_t _tmp$2013;
  moonbit_string_t _tmp$2012;
  moonbit_string_t _tmp$2010;
  while (1) {
    if (_i$630 < _len$628) {
      struct $$ZSeanYves$Doclint$src$core$Issue** _field$2494 =
        issues$629->$0;
      struct $$ZSeanYves$Doclint$src$core$Issue** buf$2003 = _field$2494;
      struct $$ZSeanYves$Doclint$src$core$Issue* _tmp$2493 =
        (struct $$ZSeanYves$Doclint$src$core$Issue*)buf$2003[_i$630];
      struct $$ZSeanYves$Doclint$src$core$Issue* it$631 = _tmp$2493;
      int32_t _field$2492 = it$631->$1;
      int32_t _bind$632 = _field$2492;
      int32_t _tmp$2004;
      switch (_bind$632) {
        case 2: {
          int32_t _tmp$2000 = err$625;
          err$625 = _tmp$2000 + 1;
          break;
        }
        
        case 1: {
          int32_t _tmp$2001 = warn$626;
          warn$626 = _tmp$2001 + 1;
          break;
        }
        default: {
          int32_t _tmp$2002 = info$627;
          info$627 = _tmp$2002 + 1;
          break;
        }
      }
      _tmp$2004 = _i$630 + 1;
      _i$630 = _tmp$2004;
      continue;
    }
    break;
  }
  err_rows$634 = (moonbit_string_t)moonbit_string_literal_26.data;
  warn_rows$635 = (moonbit_string_t)moonbit_string_literal_26.data;
  info_rows$636 = (moonbit_string_t)moonbit_string_literal_26.data;
  _len$637 = issues$629->$1;
  _i$638 = 0;
  while (1) {
    if (_i$638 < _len$637) {
      struct $$ZSeanYves$Doclint$src$core$Issue** _field$2491 =
        issues$629->$0;
      struct $$ZSeanYves$Doclint$src$core$Issue** buf$2008 = _field$2491;
      struct $$ZSeanYves$Doclint$src$core$Issue* _tmp$2490 =
        (struct $$ZSeanYves$Doclint$src$core$Issue*)buf$2008[_i$638];
      struct $$ZSeanYves$Doclint$src$core$Issue* it$639 = _tmp$2490;
      moonbit_string_t row$640;
      int32_t _field$2489;
      int32_t _bind$641;
      int32_t _tmp$2009;
      moonbit_incref(it$639);
      moonbit_incref(it$639);
      row$640 = $ZSeanYves$Doclint$src$core$issue_row_html(it$639);
      _field$2489 = it$639->$1;
      moonbit_decref(it$639);
      _bind$641 = _field$2489;
      switch (_bind$641) {
        case 2: {
          moonbit_string_t _tmp$2005 = err_rows$634;
          err_rows$634 = moonbit_add_string(_tmp$2005, row$640);
          break;
        }
        
        case 1: {
          moonbit_string_t _tmp$2006 = warn_rows$635;
          warn_rows$635 = moonbit_add_string(_tmp$2006, row$640);
          break;
        }
        default: {
          moonbit_string_t _tmp$2007 = info_rows$636;
          info_rows$636 = moonbit_add_string(_tmp$2007, row$640);
          break;
        }
      }
      _tmp$2009 = _i$638 + 1;
      _i$638 = _tmp$2009;
      continue;
    }
    break;
  }
  _field$2488 = issues$629->$1;
  moonbit_decref(issues$629);
  len$2063 = _field$2488;
  total$643 = $Int$$to_string$inner(len$2063, 10);
  _tmp$2062
  = moonbit_add_string(
    (moonbit_string_t)moonbit_string_literal_47.data,
      (moonbit_string_t)moonbit_string_literal_48.data
  );
  _tmp$2061
  = moonbit_add_string(
    _tmp$2062, (moonbit_string_t)moonbit_string_literal_49.data
  );
  _tmp$2060
  = moonbit_add_string(
    _tmp$2061, (moonbit_string_t)moonbit_string_literal_50.data
  );
  _tmp$2059
  = moonbit_add_string(
    _tmp$2060, (moonbit_string_t)moonbit_string_literal_51.data
  );
  _tmp$2058
  = moonbit_add_string(
    _tmp$2059, (moonbit_string_t)moonbit_string_literal_52.data
  );
  _tmp$2057
  = moonbit_add_string(
    _tmp$2058, (moonbit_string_t)moonbit_string_literal_53.data
  );
  _tmp$2056
  = moonbit_add_string(
    _tmp$2057, (moonbit_string_t)moonbit_string_literal_54.data
  );
  _tmp$2055
  = moonbit_add_string(
    _tmp$2056, (moonbit_string_t)moonbit_string_literal_55.data
  );
  _tmp$2054
  = moonbit_add_string(
    _tmp$2055, (moonbit_string_t)moonbit_string_literal_56.data
  );
  _tmp$2053
  = moonbit_add_string(
    _tmp$2054, (moonbit_string_t)moonbit_string_literal_57.data
  );
  _tmp$2052
  = moonbit_add_string(
    _tmp$2053, (moonbit_string_t)moonbit_string_literal_58.data
  );
  _tmp$2051
  = moonbit_add_string(
    _tmp$2052, (moonbit_string_t)moonbit_string_literal_59.data
  );
  _tmp$2050
  = moonbit_add_string(
    _tmp$2051, (moonbit_string_t)moonbit_string_literal_60.data
  );
  _tmp$2049
  = moonbit_add_string(
    _tmp$2050, (moonbit_string_t)moonbit_string_literal_61.data
  );
  _tmp$2048
  = moonbit_add_string(
    _tmp$2049, (moonbit_string_t)moonbit_string_literal_62.data
  );
  _tmp$2047
  = moonbit_add_string(
    _tmp$2048, (moonbit_string_t)moonbit_string_literal_63.data
  );
  _tmp$2046
  = moonbit_add_string(
    _tmp$2047, (moonbit_string_t)moonbit_string_literal_64.data
  );
  _tmp$2045
  = moonbit_add_string(
    _tmp$2046, (moonbit_string_t)moonbit_string_literal_65.data
  );
  _tmp$2044
  = moonbit_add_string(
    _tmp$2045, (moonbit_string_t)moonbit_string_literal_66.data
  );
  _tmp$2043
  = moonbit_add_string(
    _tmp$2044, (moonbit_string_t)moonbit_string_literal_67.data
  );
  _tmp$2042
  = moonbit_add_string(
    _tmp$2043, (moonbit_string_t)moonbit_string_literal_68.data
  );
  _tmp$2041
  = moonbit_add_string(
    _tmp$2042, (moonbit_string_t)moonbit_string_literal_69.data
  );
  _tmp$2040
  = moonbit_add_string(
    _tmp$2041, (moonbit_string_t)moonbit_string_literal_70.data
  );
  _tmp$2039
  = moonbit_add_string(
    _tmp$2040, (moonbit_string_t)moonbit_string_literal_71.data
  );
  _tmp$2038
  = moonbit_add_string(
    _tmp$2039, (moonbit_string_t)moonbit_string_literal_72.data
  );
  _tmp$2037 = moonbit_add_string(_tmp$2038, total$643);
  _tmp$2036
  = moonbit_add_string(
    _tmp$2037, (moonbit_string_t)moonbit_string_literal_73.data
  );
  _tmp$2033
  = moonbit_add_string(
    _tmp$2036, (moonbit_string_t)moonbit_string_literal_74.data
  );
  _tmp$2035 = err$625;
  _tmp$2034 = $Int$$to_string$inner(_tmp$2035, 10);
  _tmp$2032 = moonbit_add_string(_tmp$2033, _tmp$2034);
  _tmp$2031
  = moonbit_add_string(
    _tmp$2032, (moonbit_string_t)moonbit_string_literal_73.data
  );
  _tmp$2028
  = moonbit_add_string(
    _tmp$2031, (moonbit_string_t)moonbit_string_literal_75.data
  );
  _tmp$2030 = warn$626;
  _tmp$2029 = $Int$$to_string$inner(_tmp$2030, 10);
  _tmp$2027 = moonbit_add_string(_tmp$2028, _tmp$2029);
  _tmp$2026
  = moonbit_add_string(
    _tmp$2027, (moonbit_string_t)moonbit_string_literal_73.data
  );
  _tmp$2023
  = moonbit_add_string(
    _tmp$2026, (moonbit_string_t)moonbit_string_literal_76.data
  );
  _tmp$2025 = info$627;
  _tmp$2024 = $Int$$to_string$inner(_tmp$2025, 10);
  _tmp$2022 = moonbit_add_string(_tmp$2023, _tmp$2024);
  _tmp$2021
  = moonbit_add_string(
    _tmp$2022, (moonbit_string_t)moonbit_string_literal_73.data
  );
  _tmp$2020
  = moonbit_add_string(
    _tmp$2021, (moonbit_string_t)moonbit_string_literal_77.data
  );
  _tmp$2017
  = moonbit_add_string(
    _tmp$2020, (moonbit_string_t)moonbit_string_literal_77.data
  );
  _tmp$2019 = err_rows$634;
  _tmp$2018
  = $ZSeanYves$Doclint$src$core$section_table(
    (moonbit_string_t)moonbit_string_literal_78.data, _tmp$2019
  );
  _tmp$2014 = moonbit_add_string(_tmp$2017, _tmp$2018);
  _tmp$2016 = warn_rows$635;
  _tmp$2015
  = $ZSeanYves$Doclint$src$core$section_table(
    (moonbit_string_t)moonbit_string_literal_79.data, _tmp$2016
  );
  _tmp$2011 = moonbit_add_string(_tmp$2014, _tmp$2015);
  _tmp$2013 = info_rows$636;
  _tmp$2012
  = $ZSeanYves$Doclint$src$core$section_table(
    (moonbit_string_t)moonbit_string_literal_80.data, _tmp$2013
  );
  _tmp$2010 = moonbit_add_string(_tmp$2011, _tmp$2012);
  return moonbit_add_string(
           _tmp$2010, (moonbit_string_t)moonbit_string_literal_81.data
         );
}

moonbit_string_t $ZSeanYves$Doclint$src$core$section_table(
  moonbit_string_t title$624,
  moonbit_string_t rows$623
) {
  if (
    moonbit_val_array_equal(
      rows$623, (moonbit_string_t)moonbit_string_literal_26.data
    )
  ) {
    moonbit_string_t _tmp$1986;
    moonbit_string_t _tmp$1987;
    moonbit_string_t _tmp$1985;
    moonbit_string_t _tmp$1984;
    moonbit_string_t _tmp$1983;
    moonbit_decref(rows$623);
    _tmp$1986
    = moonbit_add_string(
      (moonbit_string_t)moonbit_string_literal_69.data,
        (moonbit_string_t)moonbit_string_literal_82.data
    );
    _tmp$1987 = $ZSeanYves$Doclint$src$core$escape_html(title$624);
    _tmp$1985 = moonbit_add_string(_tmp$1986, _tmp$1987);
    _tmp$1984
    = moonbit_add_string(
      _tmp$1985, (moonbit_string_t)moonbit_string_literal_83.data
    );
    _tmp$1983
    = moonbit_add_string(
      _tmp$1984, (moonbit_string_t)moonbit_string_literal_84.data
    );
    return moonbit_add_string(
             _tmp$1983, (moonbit_string_t)moonbit_string_literal_77.data
           );
  } else {
    moonbit_string_t _tmp$1998 =
      moonbit_add_string(
        (moonbit_string_t)moonbit_string_literal_69.data,
          (moonbit_string_t)moonbit_string_literal_82.data
      );
    moonbit_string_t _tmp$1999 =
      $ZSeanYves$Doclint$src$core$escape_html(title$624);
    moonbit_string_t _tmp$1997 = moonbit_add_string(_tmp$1998, _tmp$1999);
    moonbit_string_t _tmp$1996 =
      moonbit_add_string(
        _tmp$1997, (moonbit_string_t)moonbit_string_literal_83.data
      );
    moonbit_string_t _tmp$1995 =
      moonbit_add_string(
        _tmp$1996, (moonbit_string_t)moonbit_string_literal_85.data
      );
    moonbit_string_t _tmp$1994 =
      moonbit_add_string(
        _tmp$1995, (moonbit_string_t)moonbit_string_literal_86.data
      );
    moonbit_string_t _tmp$1993 =
      moonbit_add_string(
        _tmp$1994, (moonbit_string_t)moonbit_string_literal_87.data
      );
    moonbit_string_t _tmp$1992 =
      moonbit_add_string(
        _tmp$1993, (moonbit_string_t)moonbit_string_literal_88.data
      );
    moonbit_string_t _tmp$1991 =
      moonbit_add_string(
        _tmp$1992, (moonbit_string_t)moonbit_string_literal_89.data
      );
    moonbit_string_t _tmp$1990 = moonbit_add_string(_tmp$1991, rows$623);
    moonbit_string_t _tmp$1989 =
      moonbit_add_string(
        _tmp$1990, (moonbit_string_t)moonbit_string_literal_90.data
      );
    moonbit_string_t _tmp$1988 =
      moonbit_add_string(
        _tmp$1989, (moonbit_string_t)moonbit_string_literal_91.data
      );
    return moonbit_add_string(
             _tmp$1988, (moonbit_string_t)moonbit_string_literal_77.data
           );
  }
}

moonbit_string_t $ZSeanYves$Doclint$src$core$issue_row_html(
  struct $$ZSeanYves$Doclint$src$core$Issue* it$618
) {
  int32_t severity$1982 = it$618->$1;
  moonbit_string_t badge$617 =
    $ZSeanYves$Doclint$src$core$severity_badge(severity$1982);
  struct $$ZSeanYves$Doclint$src$core$Location* _field$2498 = it$618->$3;
  struct $$ZSeanYves$Doclint$src$core$Location* _bind$619 = _field$2498;
  moonbit_string_t loc$620;
  moonbit_string_t _tmp$1968;
  moonbit_string_t _field$2496;
  moonbit_string_t rule_id$1970;
  moonbit_string_t _tmp$1969;
  moonbit_string_t _tmp$1967;
  moonbit_string_t _tmp$1966;
  moonbit_string_t _tmp$1965;
  moonbit_string_t _tmp$1964;
  moonbit_string_t _tmp$1963;
  moonbit_string_t _tmp$1960;
  moonbit_string_t _field$2495;
  int32_t _cnt$2684;
  moonbit_string_t message$1962;
  moonbit_string_t _tmp$1961;
  moonbit_string_t _tmp$1959;
  moonbit_string_t _tmp$1958;
  moonbit_string_t _tmp$1956;
  moonbit_string_t _tmp$1957;
  moonbit_string_t _tmp$1955;
  moonbit_string_t _tmp$1954;
  if (_bind$619 == 0) {
    loc$620 = (moonbit_string_t)moonbit_string_literal_26.data;
  } else {
    struct $$ZSeanYves$Doclint$src$core$Location* _Some$621 = _bind$619;
    struct $$ZSeanYves$Doclint$src$core$Location* _l$622 = _Some$621;
    int32_t page$1981 = _l$622->$0;
    moonbit_string_t _tmp$1980;
    moonbit_string_t _tmp$1979;
    moonbit_string_t _tmp$1976;
    int32_t span_start$1978;
    moonbit_string_t _tmp$1977;
    moonbit_string_t _tmp$1975;
    moonbit_string_t _tmp$1972;
    int32_t _field$2497;
    int32_t span_end$1974;
    moonbit_string_t _tmp$1973;
    moonbit_string_t _tmp$1971;
    moonbit_incref(_l$622);
    _tmp$1980 = $Int$$to_string$inner(page$1981, 10);
    _tmp$1979
    = moonbit_add_string(
      (moonbit_string_t)moonbit_string_literal_92.data, _tmp$1980
    );
    _tmp$1976
    = moonbit_add_string(
      _tmp$1979, (moonbit_string_t)moonbit_string_literal_93.data
    );
    span_start$1978 = _l$622->$1;
    _tmp$1977 = $Int$$to_string$inner(span_start$1978, 10);
    _tmp$1975 = moonbit_add_string(_tmp$1976, _tmp$1977);
    _tmp$1972
    = moonbit_add_string(
      _tmp$1975, (moonbit_string_t)moonbit_string_literal_0.data
    );
    _field$2497 = _l$622->$2;
    moonbit_decref(_l$622);
    span_end$1974 = _field$2497;
    _tmp$1973 = $Int$$to_string$inner(span_end$1974, 10);
    _tmp$1971 = moonbit_add_string(_tmp$1972, _tmp$1973);
    loc$620
    = moonbit_add_string(
      _tmp$1971, (moonbit_string_t)moonbit_string_literal_94.data
    );
  }
  _tmp$1968
  = moonbit_add_string(
    (moonbit_string_t)moonbit_string_literal_95.data,
      (moonbit_string_t)moonbit_string_literal_96.data
  );
  _field$2496 = it$618->$0;
  rule_id$1970 = _field$2496;
  moonbit_incref(rule_id$1970);
  _tmp$1969 = $ZSeanYves$Doclint$src$core$escape_html(rule_id$1970);
  _tmp$1967 = moonbit_add_string(_tmp$1968, _tmp$1969);
  _tmp$1966
  = moonbit_add_string(
    _tmp$1967, (moonbit_string_t)moonbit_string_literal_97.data
  );
  _tmp$1965
  = moonbit_add_string(
    _tmp$1966, (moonbit_string_t)moonbit_string_literal_98.data
  );
  _tmp$1964 = moonbit_add_string(_tmp$1965, badge$617);
  _tmp$1963
  = moonbit_add_string(
    _tmp$1964, (moonbit_string_t)moonbit_string_literal_99.data
  );
  _tmp$1960
  = moonbit_add_string(
    _tmp$1963, (moonbit_string_t)moonbit_string_literal_98.data
  );
  _field$2495 = it$618->$2;
  _cnt$2684 = Moonbit_object_header(it$618)->rc;
  if (_cnt$2684 > 1) {
    int32_t _new_cnt$2687 = _cnt$2684 - 1;
    Moonbit_object_header(it$618)->rc = _new_cnt$2687;
    moonbit_incref(_field$2495);
  } else if (_cnt$2684 == 1) {
    struct $$ZSeanYves$Doclint$src$core$Location* _field$2686 = it$618->$3;
    moonbit_string_t _field$2685;
    if (_field$2686) {
      moonbit_decref(_field$2686);
    }
    _field$2685 = it$618->$0;
    moonbit_decref(_field$2685);
    moonbit_free(it$618);
  }
  message$1962 = _field$2495;
  _tmp$1961 = $ZSeanYves$Doclint$src$core$escape_html(message$1962);
  _tmp$1959 = moonbit_add_string(_tmp$1960, _tmp$1961);
  _tmp$1958
  = moonbit_add_string(
    _tmp$1959, (moonbit_string_t)moonbit_string_literal_99.data
  );
  _tmp$1956
  = moonbit_add_string(
    _tmp$1958, (moonbit_string_t)moonbit_string_literal_98.data
  );
  _tmp$1957 = $ZSeanYves$Doclint$src$core$escape_html(loc$620);
  _tmp$1955 = moonbit_add_string(_tmp$1956, _tmp$1957);
  _tmp$1954
  = moonbit_add_string(
    _tmp$1955, (moonbit_string_t)moonbit_string_literal_99.data
  );
  return moonbit_add_string(
           _tmp$1954, (moonbit_string_t)moonbit_string_literal_100.data
         );
}

moonbit_string_t $ZSeanYves$Doclint$src$core$severity_badge(int32_t sev$616) {
  switch (sev$616) {
    case 2: {
      return (moonbit_string_t)moonbit_string_literal_101.data;
      break;
    }
    
    case 1: {
      return (moonbit_string_t)moonbit_string_literal_102.data;
      break;
    }
    default: {
      return (moonbit_string_t)moonbit_string_literal_103.data;
      break;
    }
  }
}

moonbit_string_t $ZSeanYves$Doclint$src$core$escape_html(
  moonbit_string_t s$611
) {
  moonbit_string_t out$609 = (moonbit_string_t)moonbit_string_literal_26.data;
  struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* _it$610 = $String$$iter(s$611);
  while (1) {
    int32_t _bind$612;
    moonbit_incref(_it$610);
    _bind$612 = $$moonbitlang$core$builtin$Iter$$next$1(_it$610);
    if (_bind$612 == -1) {
      moonbit_decref(_it$610);
    } else {
      int32_t _Some$613 = _bind$612;
      int32_t _ch$614 = _Some$613;
      if (_ch$614 == 60) {
        moonbit_string_t _tmp$1953 = out$609;
        out$609
        = moonbit_add_string(
          _tmp$1953, (moonbit_string_t)moonbit_string_literal_104.data
        );
      } else if (_ch$614 == 62) {
        moonbit_string_t _tmp$1952 = out$609;
        out$609
        = moonbit_add_string(
          _tmp$1952, (moonbit_string_t)moonbit_string_literal_105.data
        );
      } else if (_ch$614 == 38) {
        moonbit_string_t _tmp$1951 = out$609;
        out$609
        = moonbit_add_string(
          _tmp$1951, (moonbit_string_t)moonbit_string_literal_106.data
        );
      } else if (_ch$614 == 34) {
        moonbit_string_t _tmp$1950 = out$609;
        out$609
        = moonbit_add_string(
          _tmp$1950, (moonbit_string_t)moonbit_string_literal_107.data
        );
      } else if (_ch$614 == 39) {
        moonbit_string_t _tmp$1949 = out$609;
        out$609
        = moonbit_add_string(
          _tmp$1949, (moonbit_string_t)moonbit_string_literal_108.data
        );
      } else {
        moonbit_string_t _tmp$1947 = out$609;
        moonbit_string_t _tmp$1948 =
          $$moonbitlang$core$builtin$Show$$Char$$to_string(_ch$614);
        out$609 = moonbit_add_string(_tmp$1947, _tmp$1948);
      }
      continue;
    }
    break;
  }
  return out$609;
}

moonbit_string_t $ZSeanYves$Doclint$src$core$issues_to_json(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* issues$605
) {
  moonbit_string_t* _tmp$1946 = (moonbit_string_t*)moonbit_empty_ref_array;
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* parts$603 =
    (struct $$moonbitlang$core$builtin$Array$3c$String$3e$*)moonbit_malloc(
      sizeof(struct $$moonbitlang$core$builtin$Array$3c$String$3e$)
    );
  int32_t _len$604;
  int32_t _i$606;
  moonbit_string_t _tmp$1942;
  int32_t _tmp$1945;
  struct $StringView _tmp$1944;
  moonbit_string_t _tmp$1943;
  moonbit_string_t _tmp$1941;
  moonbit_string_t _tmp$1940;
  Moonbit_object_header(parts$603)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $$moonbitlang$core$builtin$Array$3c$String$3e$, $0) >> 2,
      1,
      0
  );
  parts$603->$0 = _tmp$1946;
  parts$603->$1 = 0;
  _len$604 = issues$605->$1;
  _i$606 = 0;
  while (1) {
    if (_i$606 < _len$604) {
      struct $$ZSeanYves$Doclint$src$core$Issue** _field$2500 =
        issues$605->$0;
      struct $$ZSeanYves$Doclint$src$core$Issue** buf$1938 = _field$2500;
      struct $$ZSeanYves$Doclint$src$core$Issue* _tmp$2499 =
        (struct $$ZSeanYves$Doclint$src$core$Issue*)buf$1938[_i$606];
      struct $$ZSeanYves$Doclint$src$core$Issue* it$607 = _tmp$2499;
      moonbit_string_t _tmp$1937;
      int32_t _tmp$1939;
      moonbit_incref(it$607);
      _tmp$1937 = $ZSeanYves$Doclint$src$core$issue_to_json(it$607);
      moonbit_incref(parts$603);
      $$moonbitlang$core$builtin$Array$$push$0(parts$603, _tmp$1937);
      _tmp$1939 = _i$606 + 1;
      _i$606 = _tmp$1939;
      continue;
    } else {
      moonbit_decref(issues$605);
    }
    break;
  }
  _tmp$1942
  = moonbit_add_string(
    (moonbit_string_t)moonbit_string_literal_109.data,
      (moonbit_string_t)moonbit_string_literal_110.data
  );
  _tmp$1945
  = Moonbit_array_length(
    $ZSeanYves$Doclint$src$core$issues_to_json$$2a$bind$7c$202
  );
  moonbit_incref($ZSeanYves$Doclint$src$core$issues_to_json$$2a$bind$7c$202);
  _tmp$1944
  = (struct $StringView){
    0, _tmp$1945, $ZSeanYves$Doclint$src$core$issues_to_json$$2a$bind$7c$202
  };
  _tmp$1943 = $$moonbitlang$core$builtin$Array$$join$0(parts$603, _tmp$1944);
  _tmp$1941 = moonbit_add_string(_tmp$1942, _tmp$1943);
  _tmp$1940
  = moonbit_add_string(
    _tmp$1941, (moonbit_string_t)moonbit_string_literal_33.data
  );
  return moonbit_add_string(
           _tmp$1940, (moonbit_string_t)moonbit_string_literal_111.data
         );
}

moonbit_string_t $ZSeanYves$Doclint$src$core$issue_to_json(
  struct $$ZSeanYves$Doclint$src$core$Issue* it$599
) {
  struct $$ZSeanYves$Doclint$src$core$Location* _field$2503 = it$599->$3;
  struct $$ZSeanYves$Doclint$src$core$Location* _bind$598 = _field$2503;
  moonbit_string_t loc_json$600;
  moonbit_string_t _tmp$1934;
  moonbit_string_t _field$2502;
  moonbit_string_t rule_id$1936;
  moonbit_string_t _tmp$1935;
  moonbit_string_t _tmp$1933;
  moonbit_string_t _tmp$1932;
  moonbit_string_t _tmp$1928;
  int32_t severity$1931;
  moonbit_string_t _tmp$1930;
  moonbit_string_t _tmp$1929;
  moonbit_string_t _tmp$1927;
  moonbit_string_t _tmp$1926;
  moonbit_string_t _tmp$1923;
  moonbit_string_t _field$2501;
  int32_t _cnt$2688;
  moonbit_string_t message$1925;
  moonbit_string_t _tmp$1924;
  moonbit_string_t _tmp$1922;
  moonbit_string_t _tmp$1921;
  moonbit_string_t _tmp$1920;
  moonbit_string_t _tmp$1919;
  if (_bind$598 == 0) {
    loc_json$600 = (moonbit_string_t)moonbit_string_literal_112.data;
  } else {
    struct $$ZSeanYves$Doclint$src$core$Location* _Some$601 = _bind$598;
    struct $$ZSeanYves$Doclint$src$core$Location* _loc$602 = _Some$601;
    moonbit_incref(_loc$602);
    loc_json$600 = $ZSeanYves$Doclint$src$core$location_to_json(_loc$602);
  }
  _tmp$1934
  = moonbit_add_string(
    (moonbit_string_t)moonbit_string_literal_109.data,
      (moonbit_string_t)moonbit_string_literal_113.data
  );
  _field$2502 = it$599->$0;
  rule_id$1936 = _field$2502;
  moonbit_incref(rule_id$1936);
  _tmp$1935 = $ZSeanYves$Doclint$src$core$quote(rule_id$1936);
  _tmp$1933 = moonbit_add_string(_tmp$1934, _tmp$1935);
  _tmp$1932
  = moonbit_add_string(
    _tmp$1933, (moonbit_string_t)moonbit_string_literal_0.data
  );
  _tmp$1928
  = moonbit_add_string(
    _tmp$1932, (moonbit_string_t)moonbit_string_literal_114.data
  );
  severity$1931 = it$599->$1;
  _tmp$1930 = $ZSeanYves$Doclint$src$core$severity_to_string(severity$1931);
  _tmp$1929 = $ZSeanYves$Doclint$src$core$quote(_tmp$1930);
  _tmp$1927 = moonbit_add_string(_tmp$1928, _tmp$1929);
  _tmp$1926
  = moonbit_add_string(
    _tmp$1927, (moonbit_string_t)moonbit_string_literal_0.data
  );
  _tmp$1923
  = moonbit_add_string(
    _tmp$1926, (moonbit_string_t)moonbit_string_literal_115.data
  );
  _field$2501 = it$599->$2;
  _cnt$2688 = Moonbit_object_header(it$599)->rc;
  if (_cnt$2688 > 1) {
    int32_t _new_cnt$2691 = _cnt$2688 - 1;
    Moonbit_object_header(it$599)->rc = _new_cnt$2691;
    moonbit_incref(_field$2501);
  } else if (_cnt$2688 == 1) {
    struct $$ZSeanYves$Doclint$src$core$Location* _field$2690 = it$599->$3;
    moonbit_string_t _field$2689;
    if (_field$2690) {
      moonbit_decref(_field$2690);
    }
    _field$2689 = it$599->$0;
    moonbit_decref(_field$2689);
    moonbit_free(it$599);
  }
  message$1925 = _field$2501;
  _tmp$1924 = $ZSeanYves$Doclint$src$core$quote(message$1925);
  _tmp$1922 = moonbit_add_string(_tmp$1923, _tmp$1924);
  _tmp$1921
  = moonbit_add_string(
    _tmp$1922, (moonbit_string_t)moonbit_string_literal_0.data
  );
  _tmp$1920
  = moonbit_add_string(
    _tmp$1921, (moonbit_string_t)moonbit_string_literal_116.data
  );
  _tmp$1919 = moonbit_add_string(_tmp$1920, loc_json$600);
  return moonbit_add_string(
           _tmp$1919, (moonbit_string_t)moonbit_string_literal_111.data
         );
}

moonbit_string_t $ZSeanYves$Doclint$src$core$location_to_json(
  struct $$ZSeanYves$Doclint$src$core$Location* loc$597
) {
  moonbit_string_t _tmp$1916 =
    moonbit_add_string(
      (moonbit_string_t)moonbit_string_literal_109.data,
        (moonbit_string_t)moonbit_string_literal_117.data
    );
  int32_t page$1918 = loc$597->$0;
  moonbit_string_t _tmp$1917 = $Int$$to_string$inner(page$1918, 10);
  moonbit_string_t _tmp$1915 = moonbit_add_string(_tmp$1916, _tmp$1917);
  moonbit_string_t _tmp$1914 =
    moonbit_add_string(
      _tmp$1915, (moonbit_string_t)moonbit_string_literal_0.data
    );
  moonbit_string_t _tmp$1911 =
    moonbit_add_string(
      _tmp$1914, (moonbit_string_t)moonbit_string_literal_118.data
    );
  int32_t span_start$1913 = loc$597->$1;
  moonbit_string_t _tmp$1912 = $Int$$to_string$inner(span_start$1913, 10);
  moonbit_string_t _tmp$1910 = moonbit_add_string(_tmp$1911, _tmp$1912);
  moonbit_string_t _tmp$1909 =
    moonbit_add_string(
      _tmp$1910, (moonbit_string_t)moonbit_string_literal_0.data
    );
  moonbit_string_t _tmp$1906 =
    moonbit_add_string(
      _tmp$1909, (moonbit_string_t)moonbit_string_literal_119.data
    );
  int32_t _field$2504 = loc$597->$2;
  int32_t span_end$1908;
  moonbit_string_t _tmp$1907;
  moonbit_string_t _tmp$1905;
  moonbit_decref(loc$597);
  span_end$1908 = _field$2504;
  _tmp$1907 = $Int$$to_string$inner(span_end$1908, 10);
  _tmp$1905 = moonbit_add_string(_tmp$1906, _tmp$1907);
  return moonbit_add_string(
           _tmp$1905, (moonbit_string_t)moonbit_string_literal_111.data
         );
}

moonbit_string_t $ZSeanYves$Doclint$src$core$quote(moonbit_string_t s$596) {
  moonbit_string_t _tmp$1904 = $ZSeanYves$Doclint$src$core$escape_str(s$596);
  moonbit_string_t _tmp$1903 =
    moonbit_add_string(
      (moonbit_string_t)moonbit_string_literal_39.data, _tmp$1904
    );
  return moonbit_add_string(
           _tmp$1903, (moonbit_string_t)moonbit_string_literal_39.data
         );
}

moonbit_string_t $ZSeanYves$Doclint$src$core$escape_str(
  moonbit_string_t s$591
) {
  moonbit_string_t out$589 = (moonbit_string_t)moonbit_string_literal_26.data;
  struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* _it$590 = $String$$iter(s$591);
  while (1) {
    int32_t _bind$592;
    moonbit_incref(_it$590);
    _bind$592 = $$moonbitlang$core$builtin$Iter$$next$1(_it$590);
    if (_bind$592 == -1) {
      moonbit_decref(_it$590);
    } else {
      int32_t _Some$593 = _bind$592;
      int32_t _ch$594 = _Some$593;
      if (_ch$594 == 34) {
        moonbit_string_t _tmp$1902 = out$589;
        out$589
        = moonbit_add_string(
          _tmp$1902, (moonbit_string_t)moonbit_string_literal_120.data
        );
      } else if (_ch$594 == 92) {
        moonbit_string_t _tmp$1901 = out$589;
        out$589
        = moonbit_add_string(
          _tmp$1901, (moonbit_string_t)moonbit_string_literal_121.data
        );
      } else if (_ch$594 == 10) {
        moonbit_string_t _tmp$1900 = out$589;
        out$589
        = moonbit_add_string(
          _tmp$1900, (moonbit_string_t)moonbit_string_literal_122.data
        );
      } else if (_ch$594 == 13) {
        moonbit_string_t _tmp$1899 = out$589;
        out$589
        = moonbit_add_string(
          _tmp$1899, (moonbit_string_t)moonbit_string_literal_123.data
        );
      } else if (_ch$594 == 9) {
        moonbit_string_t _tmp$1898 = out$589;
        out$589
        = moonbit_add_string(
          _tmp$1898, (moonbit_string_t)moonbit_string_literal_124.data
        );
      } else {
        moonbit_string_t _tmp$1896 = out$589;
        moonbit_string_t _tmp$1897 =
          $$moonbitlang$core$builtin$Show$$Char$$to_string(_ch$594);
        out$589 = moonbit_add_string(_tmp$1896, _tmp$1897);
      }
      continue;
    }
    break;
  }
  return out$589;
}

moonbit_string_t $ZSeanYves$Doclint$src$core$severity_to_string(
  int32_t s$588
) {
  switch (s$588) {
    case 0: {
      return (moonbit_string_t)moonbit_string_literal_125.data;
      break;
    }
    
    case 1: {
      return (moonbit_string_t)moonbit_string_literal_126.data;
      break;
    }
    default: {
      return (moonbit_string_t)moonbit_string_literal_127.data;
      break;
    }
  }
}

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $ZSeanYves$Doclint$src$core$run_rules(
  int32_t ctx$585,
  struct $$ZSeanYves$Doclint$src$core$Document* doc$586,
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$* rules$581
) {
  struct $$ZSeanYves$Doclint$src$core$Issue** _tmp$1895 =
    (struct $$ZSeanYves$Doclint$src$core$Issue**)moonbit_empty_ref_array;
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* all$579 =
    (struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$*)moonbit_malloc(
      sizeof(
        struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$
      )
    );
  int32_t _len$580;
  int32_t _i$582;
  Moonbit_object_header(all$579)->meta
  = Moonbit_make_regular_object_header(
    offsetof(
      struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$,
        $0
    )
    >> 2,
      1,
      0
  );
  all$579->$0 = _tmp$1895;
  all$579->$1 = 0;
  _len$580 = rules$581->$1;
  _i$582 = 0;
  while (1) {
    if (_i$582 < _len$580) {
      struct $$ZSeanYves$Doclint$src$core$Rule* _field$2506 = rules$581->$0;
      struct $$ZSeanYves$Doclint$src$core$Rule* buf$1893 = _field$2506;
      struct $$ZSeanYves$Doclint$src$core$Rule _tmp$2505 = buf$1893[_i$582];
      struct $$ZSeanYves$Doclint$src$core$Rule r$583 = _tmp$2505;
      struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* issues$584;
      struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* _tmp$1892;
      int32_t _tmp$1894;
      moonbit_incref(doc$586);
      if (r$583.$1) {
        moonbit_incref(r$583.$1);
      }
      issues$584 = r$583.$0->$method_1(r$583.$1, ctx$585, doc$586);
      _tmp$1892 = $$moonbitlang$core$builtin$Array$$iter$0(issues$584);
      moonbit_incref(all$579);
      $$moonbitlang$core$builtin$Array$$push_iter$0(all$579, _tmp$1892);
      _tmp$1894 = _i$582 + 1;
      _i$582 = _tmp$1894;
      continue;
    } else {
      moonbit_decref(doc$586);
      moonbit_decref(rules$581);
    }
    break;
  }
  return all$579;
}

moonbit_string_t $$moonbitlang$core$builtin$Array$$join$0(
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* self$577,
  struct $StringView separator$578
) {
  moonbit_string_t* _field$2508 = self$577->$0;
  moonbit_string_t* buf$1890 = _field$2508;
  int32_t _field$2507 = self$577->$1;
  int32_t _cnt$2692 = Moonbit_object_header(self$577)->rc;
  int32_t len$1891;
  struct $$moonbitlang$core$builtin$ArrayView$3c$String$3e$ _tmp$1889;
  if (_cnt$2692 > 1) {
    int32_t _new_cnt$2693 = _cnt$2692 - 1;
    Moonbit_object_header(self$577)->rc = _new_cnt$2693;
    moonbit_incref(buf$1890);
  } else if (_cnt$2692 == 1) {
    moonbit_free(self$577);
  }
  len$1891 = _field$2507;
  _tmp$1889
  = (struct $$moonbitlang$core$builtin$ArrayView$3c$String$3e$){
    0, len$1891, buf$1890
  };
  return $$moonbitlang$core$builtin$ArrayView$$join$0(
           _tmp$1889, separator$578
         );
}

int32_t $$moonbitlang$core$builtin$Array$$push_iter$0(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$575,
  struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* iter$572
) {
  while (1) {
    struct $$ZSeanYves$Doclint$src$core$Issue* _bind$571;
    moonbit_incref(iter$572);
    _bind$571 = $$moonbitlang$core$builtin$Iter$$next$0(iter$572);
    if (_bind$571 == 0) {
      moonbit_decref(self$575);
      moonbit_decref(iter$572);
      if (_bind$571) {
        moonbit_decref(_bind$571);
      }
    } else {
      struct $$ZSeanYves$Doclint$src$core$Issue* _Some$573 = _bind$571;
      struct $$ZSeanYves$Doclint$src$core$Issue* _x$574 = _Some$573;
      moonbit_incref(self$575);
      $$moonbitlang$core$builtin$Array$$push$3(self$575, _x$574);
      continue;
    }
    break;
  }
  return 0;
}

struct $$moonbitlang$core$builtin$Array$3c$String$3e$* $$moonbitlang$core$builtin$Array$$from_fixed_array$0(
  moonbit_string_t* arr$569
) {
  int32_t len$568 = Moonbit_array_length(arr$569);
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* arr2$570 =
    $$moonbitlang$core$builtin$Array$$make_uninit$0(len$568);
  moonbit_string_t* _field$2509 = arr2$570->$0;
  moonbit_string_t* buf$1888 = _field$2509;
  moonbit_incref(buf$1888);
  $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit_fixed$0(
    buf$1888, 0, arr$569, 0, len$568
  );
  return arr2$570;
}

moonbit_string_t $$moonbitlang$core$builtin$ArrayView$$join$0(
  struct $$moonbitlang$core$builtin$ArrayView$3c$String$3e$ self$544,
  struct $StringView separator$555
) {
  int32_t end$1855 = self$544.$2;
  int32_t start$1856 = self$544.$1;
  int32_t _tmp$1854 = end$1855 - start$1856;
  if (_tmp$1854 == 0) {
    moonbit_decref(separator$555.$0);
    moonbit_decref(self$544.$0);
    return (moonbit_string_t)moonbit_string_literal_26.data;
  } else {
    moonbit_string_t* _field$2520 = self$544.$0;
    moonbit_string_t* buf$1886 = _field$2520;
    int32_t start$1887 = self$544.$1;
    moonbit_string_t _tmp$2519 = (moonbit_string_t)buf$1886[start$1887];
    moonbit_string_t _hd$545 = _tmp$2519;
    moonbit_string_t* _field$2518 = self$544.$0;
    moonbit_string_t* _bind$546 = _field$2518;
    int32_t start$1885 = self$544.$1;
    int32_t _bind$547 = 1 + start$1885;
    int32_t _field$2517 = self$544.$2;
    int32_t _bind$548;
    struct $$moonbitlang$core$builtin$ArrayView$3c$String$3e$ _x$549;
    struct $StringView hd$550;
    int32_t end$1883;
    int32_t start$1884;
    int32_t size_hint$551;
    int32_t end$1867;
    int32_t start$1868;
    int32_t _len$552;
    int32_t _i$553;
    int32_t _tmp$1869;
    int32_t _tmp$1882;
    struct $$moonbitlang$core$builtin$StringBuilder* buf$557;
    moonbit_string_t _field$2514;
    moonbit_string_t str$1870;
    int32_t start$1871;
    int32_t end$1873;
    int64_t _tmp$1872;
    moonbit_incref(_hd$545);
    _bind$548 = _field$2517;
    moonbit_incref(_bind$546);
    _x$549
    = (struct $$moonbitlang$core$builtin$ArrayView$3c$String$3e$){
      _bind$547, _bind$548, _bind$546
    };
    hd$550
    = $$moonbitlang$core$builtin$ToStringView$$String$$to_string_view(
      _hd$545
    );
    end$1883 = hd$550.$2;
    start$1884 = hd$550.$1;
    size_hint$551 = end$1883 - start$1884;
    end$1867 = _x$549.$2;
    start$1868 = _x$549.$1;
    _len$552 = end$1867 - start$1868;
    _i$553 = 0;
    while (1) {
      if (_i$553 < _len$552) {
        int32_t _tmp$1865 = _bind$547 + _i$553;
        moonbit_string_t _tmp$2516 = (moonbit_string_t)_bind$546[_tmp$1865];
        moonbit_string_t s$554 = _tmp$2516;
        int32_t _tmp$1857 = size_hint$551;
        struct $StringView _p$1145;
        int32_t end$1863;
        int32_t _field$2515;
        int32_t start$1864;
        int32_t _tmp$1859;
        int32_t end$1861;
        int32_t start$1862;
        int32_t _tmp$1860;
        int32_t _tmp$1858;
        int32_t _tmp$1866;
        moonbit_incref(s$554);
        _p$1145
        = $$moonbitlang$core$builtin$ToStringView$$String$$to_string_view(
          s$554
        );
        end$1863 = _p$1145.$2;
        _field$2515 = _p$1145.$1;
        moonbit_decref(_p$1145.$0);
        start$1864 = _field$2515;
        _tmp$1859 = end$1863 - start$1864;
        end$1861 = separator$555.$2;
        start$1862 = separator$555.$1;
        _tmp$1860 = end$1861 - start$1862;
        _tmp$1858 = _tmp$1859 + _tmp$1860;
        size_hint$551 = _tmp$1857 + _tmp$1858;
        _tmp$1866 = _i$553 + 1;
        _i$553 = _tmp$1866;
        continue;
      }
      break;
    }
    _tmp$1869 = size_hint$551;
    size_hint$551 = _tmp$1869 << 1;
    _tmp$1882 = size_hint$551;
    buf$557 = $$moonbitlang$core$builtin$StringBuilder$$new$inner(_tmp$1882);
    moonbit_incref(buf$557);
    $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_view(
      buf$557, hd$550
    );
    _field$2514 = separator$555.$0;
    str$1870 = _field$2514;
    start$1871 = separator$555.$1;
    end$1873 = separator$555.$2;
    _tmp$1872 = (int64_t)end$1873;
    moonbit_incref(str$1870);
    if ($String$$char_length_eq$inner(str$1870, 0, start$1871, _tmp$1872)) {
      int32_t end$1876;
      int32_t _field$2511;
      int32_t start$1877;
      int32_t _len$558;
      int32_t _i$559;
      moonbit_decref(separator$555.$0);
      end$1876 = _x$549.$2;
      _field$2511 = _x$549.$1;
      moonbit_decref(_x$549.$0);
      start$1877 = _field$2511;
      _len$558 = end$1876 - start$1877;
      _i$559 = 0;
      while (1) {
        if (_i$559 < _len$558) {
          int32_t _tmp$1874 = _bind$547 + _i$559;
          moonbit_string_t _tmp$2510 = (moonbit_string_t)_bind$546[_tmp$1874];
          moonbit_string_t s$560 = _tmp$2510;
          struct $StringView s$561;
          int32_t _tmp$1875;
          moonbit_incref(s$560);
          s$561
          = $$moonbitlang$core$builtin$ToStringView$$String$$to_string_view(
            s$560
          );
          moonbit_incref(buf$557);
          $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_view(
            buf$557, s$561
          );
          _tmp$1875 = _i$559 + 1;
          _i$559 = _tmp$1875;
          continue;
        } else {
          moonbit_decref(_bind$546);
        }
        break;
      }
    } else {
      int32_t end$1880 = _x$549.$2;
      int32_t _field$2513 = _x$549.$1;
      int32_t start$1881;
      int32_t _len$563;
      int32_t _i$564;
      moonbit_decref(_x$549.$0);
      start$1881 = _field$2513;
      _len$563 = end$1880 - start$1881;
      _i$564 = 0;
      while (1) {
        if (_i$564 < _len$563) {
          int32_t _tmp$1878 = _bind$547 + _i$564;
          moonbit_string_t _tmp$2512 = (moonbit_string_t)_bind$546[_tmp$1878];
          moonbit_string_t s$565 = _tmp$2512;
          struct $StringView s$566;
          int32_t _tmp$1879;
          moonbit_incref(s$565);
          s$566
          = $$moonbitlang$core$builtin$ToStringView$$String$$to_string_view(
            s$565
          );
          moonbit_incref(buf$557);
          moonbit_incref(separator$555.$0);
          $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_view(
            buf$557, separator$555
          );
          moonbit_incref(buf$557);
          $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_view(
            buf$557, s$566
          );
          _tmp$1879 = _i$564 + 1;
          _i$564 = _tmp$1879;
          continue;
        } else {
          moonbit_decref(separator$555.$0);
          moonbit_decref(_bind$546);
        }
        break;
      }
    }
    return $$moonbitlang$core$builtin$StringBuilder$$to_string(buf$557);
  }
}

int32_t $$moonbitlang$core$builtin$Show$$$moonbitlang$core$builtin$SourceLoc$$output(
  moonbit_string_t self$542,
  struct $$moonbitlang$core$builtin$Logger logger$543
) {
  moonbit_string_t _tmp$1853 = self$542;
  struct $$moonbitlang$core$builtin$SourceLocRepr* _tmp$1852 =
    $$moonbitlang$core$builtin$SourceLocRepr$$parse(_tmp$1853);
  $$moonbitlang$core$builtin$Show$$$moonbitlang$core$builtin$SourceLocRepr$$output(
    _tmp$1852, logger$543
  );
  return 0;
}

int32_t $$moonbitlang$core$builtin$Show$$$moonbitlang$core$builtin$SourceLocRepr$$output(
  struct $$moonbitlang$core$builtin$SourceLocRepr* self$505,
  struct $$moonbitlang$core$builtin$Logger logger$541
) {
  struct $StringView _field$2530 =
    (struct $StringView){self$505->$0_1, self$505->$0_2, self$505->$0_0};
  struct $StringView pkg$504 = _field$2530;
  moonbit_string_t _field$2529 = pkg$504.$0;
  moonbit_string_t _data$506 = _field$2529;
  int32_t _start$507 = pkg$504.$1;
  int32_t end$1850 = pkg$504.$2;
  int32_t start$1851 = pkg$504.$1;
  int32_t _tmp$1849 = end$1850 - start$1851;
  int32_t _end$508 = _start$507 + _tmp$1849;
  int32_t _cursor$509 = _start$507;
  int32_t accept_state$510 = -1;
  int32_t match_end$511 = -1;
  int32_t match_tag_saver_0$512 = -1;
  int32_t tag_0$513 = -1;
  struct $$3c$StringView$2a$Option$3c$StringView$3e$$3e$* _bind$514;
  struct $StringView _field$2528;
  struct $StringView _module_name$537;
  void* _field$2527;
  int32_t _cnt$2694;
  void* _package_name$538;
  struct $StringView _field$2525;
  struct $StringView filename$1812;
  struct $StringView _field$2524;
  struct $StringView start_line$1813;
  struct $StringView _field$2523;
  struct $StringView start_column$1814;
  struct $StringView _field$2522;
  struct $StringView end_line$1815;
  struct $StringView _field$2521;
  int32_t _cnt$2698;
  struct $StringView end_column$1816;
  struct $$moonbitlang$core$builtin$Logger _bind$1811;
  moonbit_incref(_data$506);
  moonbit_incref(pkg$504.$0);
  while (1) {
    int32_t _tmp$1831 = _cursor$509;
    if (_tmp$1831 < _end$508) {
      int32_t _p$1114 = _cursor$509;
      int32_t next_char$524 = _data$506[_p$1114];
      int32_t _tmp$1832 = _cursor$509;
      _cursor$509 = _tmp$1832 + 1;
      if (next_char$524 < 55296) {
        if (next_char$524 < 47) {
          goto $join$522;
        } else if (next_char$524 > 47) {
          goto $join$522;
        } else {
          while (1) {
            int32_t _tmp$1833;
            tag_0$513 = _cursor$509;
            _tmp$1833 = _cursor$509;
            if (_tmp$1833 < _end$508) {
              int32_t _p$1117 = _cursor$509;
              int32_t next_char$527 = _data$506[_p$1117];
              int32_t _tmp$1834 = _cursor$509;
              _cursor$509 = _tmp$1834 + 1;
              if (next_char$527 < 55296) {
                if (next_char$527 < 47) {
                  goto $join$525;
                } else if (next_char$527 > 47) {
                  goto $join$525;
                } else {
                  while (1) {
                    int32_t _tmp$1835 = _cursor$509;
                    if (_tmp$1835 < _end$508) {
                      int32_t _p$1120 = _cursor$509;
                      int32_t next_char$530 = _data$506[_p$1120];
                      int32_t _tmp$1836 = _cursor$509;
                      _cursor$509 = _tmp$1836 + 1;
                      if (next_char$530 < 56319) {
                        if (next_char$530 < 55296) {
                          goto $join$528;
                        } else {
                          int32_t _tmp$1837 = _cursor$509;
                          if (_tmp$1837 < _end$508) {
                            int32_t _p$1123 = _cursor$509;
                            int32_t next_char$531 = _data$506[_p$1123];
                            int32_t _tmp$1838 = _cursor$509;
                            _cursor$509 = _tmp$1838 + 1;
                            if (next_char$531 < 56320) {
                              goto $join$515;
                            } else if (next_char$531 > 65535) {
                              goto $join$515;
                            } else {
                              continue;
                            }
                          } else {
                            goto $join$515;
                          }
                        }
                      } else if (next_char$530 > 56319) {
                        if (next_char$530 < 65536) {
                          goto $join$528;
                        } else {
                          goto $join$515;
                        }
                      } else {
                        int32_t _tmp$1839 = _cursor$509;
                        if (_tmp$1839 < _end$508) {
                          int32_t _p$1126 = _cursor$509;
                          int32_t next_char$532 = _data$506[_p$1126];
                          int32_t _tmp$1840 = _cursor$509;
                          _cursor$509 = _tmp$1840 + 1;
                          if (next_char$532 < 56320) {
                            goto $join$515;
                          } else if (next_char$532 > 57343) {
                            goto $join$515;
                          } else {
                            continue;
                          }
                        } else {
                          goto $join$515;
                        }
                      }
                      goto $joinlet$2808;
                      $join$528:;
                      continue;
                      $joinlet$2808:;
                    } else {
                      match_tag_saver_0$512 = tag_0$513;
                      accept_state$510 = 0;
                      match_end$511 = _cursor$509;
                      goto $join$515;
                    }
                    break;
                  }
                }
              } else if (next_char$527 > 56318) {
                if (next_char$527 < 57344) {
                  int32_t _tmp$1841 = _cursor$509;
                  if (_tmp$1841 < _end$508) {
                    int32_t _p$1129 = _cursor$509;
                    int32_t next_char$533 = _data$506[_p$1129];
                    int32_t _tmp$1842 = _cursor$509;
                    _cursor$509 = _tmp$1842 + 1;
                    if (next_char$533 < 56320) {
                      goto $join$515;
                    } else if (next_char$533 > 57343) {
                      goto $join$515;
                    } else {
                      continue;
                    }
                  } else {
                    goto $join$515;
                  }
                } else if (next_char$527 > 65535) {
                  goto $join$515;
                } else {
                  goto $join$525;
                }
              } else {
                int32_t _tmp$1843 = _cursor$509;
                if (_tmp$1843 < _end$508) {
                  int32_t _p$1132 = _cursor$509;
                  int32_t next_char$534 = _data$506[_p$1132];
                  int32_t _tmp$1844 = _cursor$509;
                  _cursor$509 = _tmp$1844 + 1;
                  if (next_char$534 < 56320) {
                    goto $join$515;
                  } else if (next_char$534 > 65535) {
                    goto $join$515;
                  } else {
                    continue;
                  }
                } else {
                  goto $join$515;
                }
              }
              goto $joinlet$2806;
              $join$525:;
              continue;
              $joinlet$2806:;
            } else {
              goto $join$515;
            }
            break;
          }
        }
      } else if (next_char$524 > 56318) {
        if (next_char$524 < 57344) {
          int32_t _tmp$1845 = _cursor$509;
          if (_tmp$1845 < _end$508) {
            int32_t _p$1135 = _cursor$509;
            int32_t next_char$535 = _data$506[_p$1135];
            int32_t _tmp$1846 = _cursor$509;
            _cursor$509 = _tmp$1846 + 1;
            if (next_char$535 < 56320) {
              goto $join$515;
            } else if (next_char$535 > 57343) {
              goto $join$515;
            } else {
              continue;
            }
          } else {
            goto $join$515;
          }
        } else if (next_char$524 > 65535) {
          goto $join$515;
        } else {
          goto $join$522;
        }
      } else {
        int32_t _tmp$1847 = _cursor$509;
        if (_tmp$1847 < _end$508) {
          int32_t _p$1138 = _cursor$509;
          int32_t next_char$536 = _data$506[_p$1138];
          int32_t _tmp$1848 = _cursor$509;
          _cursor$509 = _tmp$1848 + 1;
          if (next_char$536 < 56320) {
            goto $join$515;
          } else if (next_char$536 > 65535) {
            goto $join$515;
          } else {
            continue;
          }
        } else {
          goto $join$515;
        }
      }
      goto $joinlet$2804;
      $join$522:;
      continue;
      $joinlet$2804:;
    } else {
      goto $join$515;
    }
    break;
  }
  goto $joinlet$2802;
  $join$515:;
  switch (accept_state$510) {
    case 0: {
      void* _try_err$518;
      struct $StringView package_name$516;
      int32_t _tmp$1827;
      int32_t _tmp$1826;
      int64_t _tmp$1823;
      int32_t _tmp$1825;
      int64_t _tmp$1824;
      struct moonbit_result_3 _tmp$2810;
      void* _try_err$521;
      struct $StringView module_name$519;
      int64_t _tmp$1818;
      int32_t _tmp$1820;
      int64_t _tmp$1819;
      struct moonbit_result_3 _tmp$2812;
      void* Some$1817;
      moonbit_decref(pkg$504.$0);
      _tmp$1827 = match_tag_saver_0$512;
      _tmp$1826 = _tmp$1827 + 1;
      _tmp$1823 = (int64_t)_tmp$1826;
      _tmp$1825 = match_end$511;
      _tmp$1824 = (int64_t)_tmp$1825;
      moonbit_incref(_data$506);
      _tmp$2810 = $String$$sub(_data$506, _tmp$1823, _tmp$1824);
      if (_tmp$2810.tag) {
        struct $StringView const _ok$1828 = _tmp$2810.data.ok;
        package_name$516 = _ok$1828;
      } else {
        void* const _err$1829 = _tmp$2810.data.err;
        _try_err$518 = _err$1829;
        goto $join$517;
      }
      goto $joinlet$2809;
      $join$517:;
      moonbit_decref(_try_err$518);
      moonbit_panic();
      $joinlet$2809:;
      _tmp$1818 = (int64_t)_start$507;
      _tmp$1820 = match_tag_saver_0$512;
      _tmp$1819 = (int64_t)_tmp$1820;
      _tmp$2812 = $String$$sub(_data$506, _tmp$1818, _tmp$1819);
      if (_tmp$2812.tag) {
        struct $StringView const _ok$1821 = _tmp$2812.data.ok;
        module_name$519 = _ok$1821;
      } else {
        void* const _err$1822 = _tmp$2812.data.err;
        _try_err$521 = _err$1822;
        goto $join$520;
      }
      goto $joinlet$2811;
      $join$520:;
      moonbit_decref(_try_err$521);
      moonbit_panic();
      $joinlet$2811:;
      Some$1817
      = (void*)moonbit_malloc(sizeof(struct $Option$3c$StringView$3e$$Some));
      Moonbit_object_header(Some$1817)->meta
      = Moonbit_make_regular_object_header(
        offsetof(struct $Option$3c$StringView$3e$$Some, $0_0) >> 2, 1, 1
      );
      ((struct $Option$3c$StringView$3e$$Some*)Some$1817)->$0_0
      = package_name$516.$0;
      ((struct $Option$3c$StringView$3e$$Some*)Some$1817)->$0_1
      = package_name$516.$1;
      ((struct $Option$3c$StringView$3e$$Some*)Some$1817)->$0_2
      = package_name$516.$2;
      _bind$514
      = (struct $$3c$StringView$2a$Option$3c$StringView$3e$$3e$*)moonbit_malloc(
          sizeof(struct $$3c$StringView$2a$Option$3c$StringView$3e$$3e$)
        );
      Moonbit_object_header(_bind$514)->meta
      = Moonbit_make_regular_object_header(
        offsetof(
          struct $$3c$StringView$2a$Option$3c$StringView$3e$$3e$, $0_0
        )
        >> 2,
          2,
          0
      );
      _bind$514->$0_0 = module_name$519.$0;
      _bind$514->$0_1 = module_name$519.$1;
      _bind$514->$0_2 = module_name$519.$2;
      _bind$514->$1 = Some$1817;
      break;
    }
    default: {
      void* None$1830;
      moonbit_decref(_data$506);
      None$1830 = (struct moonbit_object*)&moonbit_constant_constructor_0 + 1;
      _bind$514
      = (struct $$3c$StringView$2a$Option$3c$StringView$3e$$3e$*)moonbit_malloc(
          sizeof(struct $$3c$StringView$2a$Option$3c$StringView$3e$$3e$)
        );
      Moonbit_object_header(_bind$514)->meta
      = Moonbit_make_regular_object_header(
        offsetof(
          struct $$3c$StringView$2a$Option$3c$StringView$3e$$3e$, $0_0
        )
        >> 2,
          2,
          0
      );
      _bind$514->$0_0 = pkg$504.$0;
      _bind$514->$0_1 = pkg$504.$1;
      _bind$514->$0_2 = pkg$504.$2;
      _bind$514->$1 = None$1830;
      break;
    }
  }
  $joinlet$2802:;
  _field$2528
  = (struct $StringView){
    _bind$514->$0_1, _bind$514->$0_2, _bind$514->$0_0
  };
  _module_name$537 = _field$2528;
  _field$2527 = _bind$514->$1;
  _cnt$2694 = Moonbit_object_header(_bind$514)->rc;
  if (_cnt$2694 > 1) {
    int32_t _new_cnt$2695 = _cnt$2694 - 1;
    Moonbit_object_header(_bind$514)->rc = _new_cnt$2695;
    moonbit_incref(_field$2527);
    moonbit_incref(_module_name$537.$0);
  } else if (_cnt$2694 == 1) {
    moonbit_free(_bind$514);
  }
  _package_name$538 = _field$2527;
  switch (Moonbit_object_tag(_package_name$538)) {
    case 1: {
      struct $Option$3c$StringView$3e$$Some* _Some$539 =
        (struct $Option$3c$StringView$3e$$Some*)_package_name$538;
      struct $StringView _field$2526 =
        (struct $StringView){
          _Some$539->$0_1, _Some$539->$0_2, _Some$539->$0_0
        };
      int32_t _cnt$2696 = Moonbit_object_header(_Some$539)->rc;
      struct $StringView _pkg_name$540;
      struct $$moonbitlang$core$builtin$Logger _bind$1810;
      if (_cnt$2696 > 1) {
        int32_t _new_cnt$2697 = _cnt$2696 - 1;
        Moonbit_object_header(_Some$539)->rc = _new_cnt$2697;
        moonbit_incref(_field$2526.$0);
      } else if (_cnt$2696 == 1) {
        moonbit_free(_Some$539);
      }
      _pkg_name$540 = _field$2526;
      if (logger$541.$1) {
        moonbit_incref(logger$541.$1);
      }
      logger$541.$0->$method_2(logger$541.$1, _pkg_name$540);
      _bind$1810 = logger$541;
      if (_bind$1810.$1) {
        moonbit_incref(_bind$1810.$1);
      }
      _bind$1810.$0->$method_3(_bind$1810.$1, 47);
      break;
    }
    default: {
      moonbit_decref(_package_name$538);
      break;
    }
  }
  _field$2525
  = (struct $StringView){
    self$505->$1_1, self$505->$1_2, self$505->$1_0
  };
  filename$1812 = _field$2525;
  moonbit_incref(filename$1812.$0);
  if (logger$541.$1) {
    moonbit_incref(logger$541.$1);
  }
  logger$541.$0->$method_2(logger$541.$1, filename$1812);
  if (logger$541.$1) {
    moonbit_incref(logger$541.$1);
  }
  logger$541.$0->$method_3(logger$541.$1, 58);
  _field$2524
  = (struct $StringView){
    self$505->$2_1, self$505->$2_2, self$505->$2_0
  };
  start_line$1813 = _field$2524;
  moonbit_incref(start_line$1813.$0);
  if (logger$541.$1) {
    moonbit_incref(logger$541.$1);
  }
  logger$541.$0->$method_2(logger$541.$1, start_line$1813);
  if (logger$541.$1) {
    moonbit_incref(logger$541.$1);
  }
  logger$541.$0->$method_3(logger$541.$1, 58);
  _field$2523
  = (struct $StringView){
    self$505->$3_1, self$505->$3_2, self$505->$3_0
  };
  start_column$1814 = _field$2523;
  moonbit_incref(start_column$1814.$0);
  if (logger$541.$1) {
    moonbit_incref(logger$541.$1);
  }
  logger$541.$0->$method_2(logger$541.$1, start_column$1814);
  if (logger$541.$1) {
    moonbit_incref(logger$541.$1);
  }
  logger$541.$0->$method_3(logger$541.$1, 45);
  _field$2522
  = (struct $StringView){
    self$505->$4_1, self$505->$4_2, self$505->$4_0
  };
  end_line$1815 = _field$2522;
  moonbit_incref(end_line$1815.$0);
  if (logger$541.$1) {
    moonbit_incref(logger$541.$1);
  }
  logger$541.$0->$method_2(logger$541.$1, end_line$1815);
  if (logger$541.$1) {
    moonbit_incref(logger$541.$1);
  }
  logger$541.$0->$method_3(logger$541.$1, 58);
  _field$2521
  = (struct $StringView){
    self$505->$5_1, self$505->$5_2, self$505->$5_0
  };
  _cnt$2698 = Moonbit_object_header(self$505)->rc;
  if (_cnt$2698 > 1) {
    int32_t _new_cnt$2704 = _cnt$2698 - 1;
    Moonbit_object_header(self$505)->rc = _new_cnt$2704;
    moonbit_incref(_field$2521.$0);
  } else if (_cnt$2698 == 1) {
    struct $StringView _field$2703 =
      (struct $StringView){self$505->$4_1, self$505->$4_2, self$505->$4_0};
    struct $StringView _field$2702;
    struct $StringView _field$2701;
    struct $StringView _field$2700;
    struct $StringView _field$2699;
    moonbit_decref(_field$2703.$0);
    _field$2702
    = (struct $StringView){
      self$505->$3_1, self$505->$3_2, self$505->$3_0
    };
    moonbit_decref(_field$2702.$0);
    _field$2701
    = (struct $StringView){
      self$505->$2_1, self$505->$2_2, self$505->$2_0
    };
    moonbit_decref(_field$2701.$0);
    _field$2700
    = (struct $StringView){
      self$505->$1_1, self$505->$1_2, self$505->$1_0
    };
    moonbit_decref(_field$2700.$0);
    _field$2699
    = (struct $StringView){
      self$505->$0_1, self$505->$0_2, self$505->$0_0
    };
    moonbit_decref(_field$2699.$0);
    moonbit_free(self$505);
  }
  end_column$1816 = _field$2521;
  if (logger$541.$1) {
    moonbit_incref(logger$541.$1);
  }
  logger$541.$0->$method_2(logger$541.$1, end_column$1816);
  if (logger$541.$1) {
    moonbit_incref(logger$541.$1);
  }
  logger$541.$0->$method_3(logger$541.$1, 64);
  _bind$1811 = logger$541;
  _bind$1811.$0->$method_2(_bind$1811.$1, _module_name$537);
  return 0;
}

moonbit_bytes_t $Bytes$$from_array(
  struct $$moonbitlang$core$builtin$ArrayView$3c$Byte$3e$ arr$502
) {
  int32_t end$1808 = arr$502.$2;
  int32_t start$1809 = arr$502.$1;
  int32_t _tmp$1804 = end$1808 - start$1809;
  struct $Bytes$$from_array$fn$3$2d$cap* _closure$2813 =
    (struct $Bytes$$from_array$fn$3$2d$cap*)moonbit_malloc(
      sizeof(struct $Bytes$$from_array$fn$3$2d$cap)
    );
  struct $$3c$Int$3e$$3d$$3e$Byte* _tmp$1805;
  Moonbit_object_header(_closure$2813)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $Bytes$$from_array$fn$3$2d$cap, $0_0) >> 2, 1, 0
  );
  _closure$2813->code = &$Bytes$$from_array$fn$3;
  _closure$2813->$0_0 = arr$502.$0;
  _closure$2813->$0_1 = arr$502.$1;
  _closure$2813->$0_2 = arr$502.$2;
  _tmp$1805 = (struct $$3c$Int$3e$$3d$$3e$Byte*)_closure$2813;
  return $Bytes$$makei$0(_tmp$1804, _tmp$1805);
}

int32_t $Bytes$$from_array$fn$3(
  struct $$3c$Int$3e$$3d$$3e$Byte* _env$1806,
  int32_t i$503
) {
  struct $Bytes$$from_array$fn$3$2d$cap* _casted_env$1807 =
    (struct $Bytes$$from_array$fn$3$2d$cap*)_env$1806;
  struct $$moonbitlang$core$builtin$ArrayView$3c$Byte$3e$ _field$2531 =
    (struct $$moonbitlang$core$builtin$ArrayView$3c$Byte$3e$){
      _casted_env$1807->$0_1, _casted_env$1807->$0_2, _casted_env$1807->$0_0
    };
  int32_t _cnt$2705 = Moonbit_object_header(_casted_env$1807)->rc;
  struct $$moonbitlang$core$builtin$ArrayView$3c$Byte$3e$ arr$502;
  if (_cnt$2705 > 1) {
    int32_t _new_cnt$2706 = _cnt$2705 - 1;
    Moonbit_object_header(_casted_env$1807)->rc = _new_cnt$2706;
    moonbit_incref(_field$2531.$0);
  } else if (_cnt$2705 == 1) {
    moonbit_free(_casted_env$1807);
  }
  arr$502 = _field$2531;
  return $$moonbitlang$core$builtin$ArrayView$$at$0(arr$502, i$503);
}

int32_t $moonbitlang$core$builtin$println$0(moonbit_string_t input$501) {
  moonbit_println(input$501);
  moonbit_decref(input$501);
  return 0;
}

moonbit_bytes_t $Bytes$$makei$0(
  int32_t length$496,
  struct $$3c$Int$3e$$3d$$3e$Byte* value$498
) {
  int32_t _tmp$1803;
  moonbit_bytes_t arr$497;
  int32_t i$499;
  if (length$496 <= 0) {
    moonbit_decref(value$498);
    return (moonbit_bytes_t)moonbit_bytes_literal_2.data;
  }
  moonbit_incref(value$498);
  _tmp$1803 = value$498->code(value$498, 0);
  arr$497 = (moonbit_bytes_t)moonbit_make_bytes(length$496, _tmp$1803);
  i$499 = 1;
  while (1) {
    if (i$499 < length$496) {
      int32_t _tmp$1801;
      int32_t _tmp$1802;
      moonbit_incref(value$498);
      _tmp$1801 = value$498->code(value$498, i$499);
      if (i$499 < 0 || i$499 >= Moonbit_array_length(arr$497)) {
        moonbit_panic();
      }
      arr$497[i$499] = _tmp$1801;
      _tmp$1802 = i$499 + 1;
      i$499 = _tmp$1802;
      continue;
    } else {
      moonbit_decref(value$498);
    }
    break;
  }
  return arr$497;
}

int32_t $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit_fixed$0(
  moonbit_string_t* dst$490,
  int32_t dst_offset$491,
  moonbit_string_t* src$492,
  int32_t src_offset$493,
  int32_t len$495
) {
  int32_t _tmp$1800 = len$495 - 1;
  int32_t i$489 = _tmp$1800;
  while (1) {
    if (i$489 >= 0) {
      int32_t _tmp$1796 = dst_offset$491 + i$489;
      int32_t _tmp$1798 = src_offset$493 + i$489;
      moonbit_string_t _tmp$2533;
      moonbit_string_t _tmp$1797;
      moonbit_string_t _old$2532;
      int32_t _tmp$1799;
      if (_tmp$1798 < 0 || _tmp$1798 >= Moonbit_array_length(src$492)) {
        moonbit_panic();
      }
      _tmp$2533 = (moonbit_string_t)src$492[_tmp$1798];
      _tmp$1797 = _tmp$2533;
      if (_tmp$1796 < 0 || _tmp$1796 >= Moonbit_array_length(dst$490)) {
        moonbit_panic();
      }
      _old$2532 = (moonbit_string_t)dst$490[_tmp$1796];
      moonbit_incref(_tmp$1797);
      moonbit_decref(_old$2532);
      dst$490[_tmp$1796] = _tmp$1797;
      _tmp$1799 = i$489 - 1;
      i$489 = _tmp$1799;
      continue;
    } else {
      moonbit_decref(src$492);
      moonbit_decref(dst$490);
    }
    break;
  }
  return 0;
}

int32_t $String$$unsafe_charcode_at(
  moonbit_string_t self$487,
  int32_t idx$488
) {
  int32_t _tmp$2534 = self$487[idx$488];
  moonbit_decref(self$487);
  return _tmp$2534;
}

moonbit_bytes_t $Bytes$$make(int32_t len$485, int32_t init$486) {
  if (len$485 < 0) {
    return (moonbit_bytes_t)moonbit_bytes_literal_2.data;
  }
  return moonbit_make_bytes(len$485, init$486);
}

struct $$moonbitlang$core$builtin$Array$3c$String$3e$* $$moonbitlang$core$builtin$Array$$make_uninit$0(
  int32_t len$484
) {
  moonbit_string_t* _tmp$1795 =
    (moonbit_string_t*)moonbit_make_ref_array(
      len$484, (moonbit_string_t)moonbit_string_literal_26.data
    );
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* _block$2816 =
    (struct $$moonbitlang$core$builtin$Array$3c$String$3e$*)moonbit_malloc(
      sizeof(struct $$moonbitlang$core$builtin$Array$3c$String$3e$)
    );
  Moonbit_object_header(_block$2816)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $$moonbitlang$core$builtin$Array$3c$String$3e$, $0) >> 2,
      1,
      0
  );
  _block$2816->$0 = _tmp$1795;
  _block$2816->$1 = len$484;
  return _block$2816;
}

int32_t $$moonbitlang$core$builtin$ArrayView$$at$0(
  struct $$moonbitlang$core$builtin$ArrayView$3c$Byte$3e$ self$483,
  int32_t index$482
) {
  int32_t _if_result$2817;
  if (index$482 >= 0) {
    int32_t end$1782 = self$483.$2;
    int32_t start$1783 = self$483.$1;
    int32_t _tmp$1781 = end$1782 - start$1783;
    _if_result$2817 = index$482 < _tmp$1781;
  } else {
    _if_result$2817 = 0;
  }
  if (_if_result$2817) {
    moonbit_bytes_t _field$2537 = self$483.$0;
    moonbit_bytes_t buf$1784 = _field$2537;
    int32_t _field$2536 = self$483.$1;
    int32_t start$1786 = _field$2536;
    int32_t _tmp$1785 = start$1786 + index$482;
    int32_t _tmp$2535;
    if (_tmp$1785 < 0 || _tmp$1785 >= Moonbit_array_length(buf$1784)) {
      moonbit_panic();
    }
    _tmp$2535 = (int32_t)buf$1784[_tmp$1785];
    moonbit_decref(buf$1784);
    return _tmp$2535;
  } else {
    int32_t end$1793 = self$483.$2;
    int32_t _field$2538 = self$483.$1;
    int32_t start$1794;
    int32_t _tmp$1792;
    moonbit_string_t _tmp$1791;
    moonbit_string_t _tmp$1790;
    moonbit_string_t _tmp$1788;
    moonbit_string_t _tmp$1789;
    moonbit_string_t _tmp$1787;
    moonbit_decref(self$483.$0);
    start$1794 = _field$2538;
    _tmp$1792 = end$1793 - start$1794;
    _tmp$1791
    = $$moonbitlang$core$builtin$Show$$$default_impl$$to_string$1(
      _tmp$1792
    );
    _tmp$1790
    = moonbit_add_string(
      (moonbit_string_t)moonbit_string_literal_128.data, _tmp$1791
    );
    _tmp$1788
    = moonbit_add_string(
      _tmp$1790, (moonbit_string_t)moonbit_string_literal_129.data
    );
    _tmp$1789
    = $$moonbitlang$core$builtin$Show$$$default_impl$$to_string$1(
      index$482
    );
    _tmp$1787 = moonbit_add_string(_tmp$1788, _tmp$1789);
    return $moonbitlang$core$builtin$abort$2(
             _tmp$1787, (moonbit_string_t)moonbit_string_literal_130.data
           );
  }
}

moonbit_string_t* $FixedArray$$map$0(
  moonbit_bytes_t* self$476,
  struct $$3c$Bytes$3e$$3d$$3e$String* f$478
) {
  int32_t _tmp$1774 = Moonbit_array_length(self$476);
  int32_t _tmp$1778;
  moonbit_bytes_t _tmp$2541;
  moonbit_bytes_t _tmp$1780;
  moonbit_string_t _tmp$1779;
  moonbit_string_t* res$477;
  int32_t _end2262$479;
  int32_t i$480;
  if (_tmp$1774 == 0) {
    moonbit_decref(f$478);
    moonbit_decref(self$476);
    return (moonbit_string_t*)moonbit_empty_ref_array;
  }
  _tmp$1778 = Moonbit_array_length(self$476);
  if (0 < 0 || 0 >= Moonbit_array_length(self$476)) {
    moonbit_panic();
  }
  _tmp$2541 = (moonbit_bytes_t)self$476[0];
  _tmp$1780 = _tmp$2541;
  moonbit_incref(_tmp$1780);
  moonbit_incref(f$478);
  _tmp$1779 = f$478->code(f$478, _tmp$1780);
  res$477 = (moonbit_string_t*)moonbit_make_ref_array(_tmp$1778, _tmp$1779);
  _end2262$479 = Moonbit_array_length(self$476);
  i$480 = 1;
  while (1) {
    if (i$480 < _end2262$479) {
      moonbit_bytes_t _tmp$2540;
      moonbit_bytes_t _tmp$1776;
      moonbit_string_t _tmp$1775;
      moonbit_string_t _old$2539;
      int32_t _tmp$1777;
      if (i$480 < 0 || i$480 >= Moonbit_array_length(self$476)) {
        moonbit_panic();
      }
      _tmp$2540 = (moonbit_bytes_t)self$476[i$480];
      _tmp$1776 = _tmp$2540;
      moonbit_incref(_tmp$1776);
      moonbit_incref(f$478);
      _tmp$1775 = f$478->code(f$478, _tmp$1776);
      if (i$480 < 0 || i$480 >= Moonbit_array_length(res$477)) {
        moonbit_panic();
      }
      _old$2539 = (moonbit_string_t)res$477[i$480];
      moonbit_decref(_old$2539);
      res$477[i$480] = _tmp$1775;
      _tmp$1777 = i$480 + 1;
      i$480 = _tmp$1777;
      continue;
    } else {
      moonbit_decref(f$478);
      moonbit_decref(self$476);
    }
    break;
  }
  return res$477;
}

struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$moonbitlang$core$builtin$Array$$iter$0(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$475
) {
  struct $$ZSeanYves$Doclint$src$core$Issue** _field$2543 = self$475->$0;
  struct $$ZSeanYves$Doclint$src$core$Issue** buf$1772 = _field$2543;
  int32_t _field$2542 = self$475->$1;
  int32_t _cnt$2707 = Moonbit_object_header(self$475)->rc;
  int32_t len$1773;
  struct $$moonbitlang$core$builtin$ArrayView$3c$$ZSeanYves$Doclint$src$core$Issue$3e$ _tmp$1771;
  if (_cnt$2707 > 1) {
    int32_t _new_cnt$2708 = _cnt$2707 - 1;
    Moonbit_object_header(self$475)->rc = _new_cnt$2708;
    moonbit_incref(buf$1772);
  } else if (_cnt$2707 == 1) {
    moonbit_free(self$475);
  }
  len$1773 = _field$2542;
  _tmp$1771
  = (struct $$moonbitlang$core$builtin$ArrayView$3c$$ZSeanYves$Doclint$src$core$Issue$3e$){
    0, len$1773, buf$1772
  };
  return $$moonbitlang$core$builtin$ArrayView$$iter$0(_tmp$1771);
}

struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$moonbitlang$core$builtin$ArrayView$$iter$0(
  struct $$moonbitlang$core$builtin$ArrayView$3c$$ZSeanYves$Doclint$src$core$Issue$3e$ self$473
) {
  struct $Ref$3c$Int$3e$* i$472 =
    (struct $Ref$3c$Int$3e$*)moonbit_malloc(sizeof(struct $Ref$3c$Int$3e$));
  struct $ArrayView$$iter$7c$$ZSeanYves$Doclint$src$core$Issue$7c$$$2a$p$fn$2$2d$cap* _closure$2819;
  struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* _p$1101;
  Moonbit_object_header(i$472)->meta
  = Moonbit_make_regular_object_header(
    sizeof(struct $Ref$3c$Int$3e$) >> 2, 0, 0
  );
  i$472->$0 = 0;
  _closure$2819
  = (struct $ArrayView$$iter$7c$$ZSeanYves$Doclint$src$core$Issue$7c$$$2a$p$fn$2$2d$cap*)moonbit_malloc(
      sizeof(
        struct $ArrayView$$iter$7c$$ZSeanYves$Doclint$src$core$Issue$7c$$$2a$p$fn$2$2d$cap
      )
    );
  Moonbit_object_header(_closure$2819)->meta
  = Moonbit_make_regular_object_header(
    offsetof(
      struct $ArrayView$$iter$7c$$ZSeanYves$Doclint$src$core$Issue$7c$$$2a$p$fn$2$2d$cap,
        $0_0
    )
    >> 2,
      2,
      0
  );
  _closure$2819->code
  = &$ArrayView$$iter$7c$$ZSeanYves$Doclint$src$core$Issue$7c$$$2a$p$fn$2;
  _closure$2819->$0_0 = self$473.$0;
  _closure$2819->$0_1 = self$473.$1;
  _closure$2819->$0_2 = self$473.$2;
  _closure$2819->$1 = i$472;
  _p$1101
  = (struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$*)_closure$2819;
  return _p$1101;
}

struct $$ZSeanYves$Doclint$src$core$Issue* $ArrayView$$iter$7c$$ZSeanYves$Doclint$src$core$Issue$7c$$$2a$p$fn$2(
  struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* _env$1759
) {
  struct $ArrayView$$iter$7c$$ZSeanYves$Doclint$src$core$Issue$7c$$$2a$p$fn$2$2d$cap* _casted_env$1760 =
    (struct $ArrayView$$iter$7c$$ZSeanYves$Doclint$src$core$Issue$7c$$$2a$p$fn$2$2d$cap*)_env$1759;
  struct $Ref$3c$Int$3e$* _field$2548 = _casted_env$1760->$1;
  struct $Ref$3c$Int$3e$* i$472 = _field$2548;
  struct $$moonbitlang$core$builtin$ArrayView$3c$$ZSeanYves$Doclint$src$core$Issue$3e$ _field$2547 =
    (struct $$moonbitlang$core$builtin$ArrayView$3c$$ZSeanYves$Doclint$src$core$Issue$3e$){
      _casted_env$1760->$0_1, _casted_env$1760->$0_2, _casted_env$1760->$0_0
    };
  int32_t _cnt$2709 = Moonbit_object_header(_casted_env$1760)->rc;
  struct $$moonbitlang$core$builtin$ArrayView$3c$$ZSeanYves$Doclint$src$core$Issue$3e$ self$473;
  int32_t val$1761;
  int32_t end$1763;
  int32_t start$1764;
  int32_t _tmp$1762;
  if (_cnt$2709 > 1) {
    int32_t _new_cnt$2710 = _cnt$2709 - 1;
    Moonbit_object_header(_casted_env$1760)->rc = _new_cnt$2710;
    moonbit_incref(i$472);
    moonbit_incref(_field$2547.$0);
  } else if (_cnt$2709 == 1) {
    moonbit_free(_casted_env$1760);
  }
  self$473 = _field$2547;
  val$1761 = i$472->$0;
  end$1763 = self$473.$2;
  start$1764 = self$473.$1;
  _tmp$1762 = end$1763 - start$1764;
  if (val$1761 < _tmp$1762) {
    struct $$ZSeanYves$Doclint$src$core$Issue** _field$2546 = self$473.$0;
    struct $$ZSeanYves$Doclint$src$core$Issue** buf$1767 = _field$2546;
    int32_t _field$2545 = self$473.$1;
    int32_t start$1769 = _field$2545;
    int32_t val$1770 = i$472->$0;
    int32_t _tmp$1768 = start$1769 + val$1770;
    struct $$ZSeanYves$Doclint$src$core$Issue* _tmp$2544 =
      (struct $$ZSeanYves$Doclint$src$core$Issue*)buf$1767[_tmp$1768];
    struct $$ZSeanYves$Doclint$src$core$Issue* elem$474;
    int32_t val$1766;
    int32_t _tmp$1765;
    if (_tmp$2544) {
      moonbit_incref(_tmp$2544);
    }
    moonbit_decref(buf$1767);
    elem$474 = _tmp$2544;
    val$1766 = i$472->$0;
    _tmp$1765 = val$1766 + 1;
    i$472->$0 = _tmp$1765;
    moonbit_decref(i$472);
    return elem$474;
  } else {
    moonbit_decref(self$473.$0);
    moonbit_decref(i$472);
    return 0;
  }
}

moonbit_string_t $$moonbitlang$core$builtin$Show$$String$$to_string(
  moonbit_string_t self$471
) {
  return self$471;
}

int32_t $$moonbitlang$core$builtin$Show$$Int$$output(
  int32_t self$470,
  struct $$moonbitlang$core$builtin$Logger logger$469
) {
  moonbit_string_t _tmp$1758 = $Int$$to_string$inner(self$470, 10);
  logger$469.$0->$method_0(logger$469.$1, _tmp$1758);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Compare$$String$$compare(
  moonbit_string_t self$463,
  moonbit_string_t other$465
) {
  int32_t len$462 = Moonbit_array_length(self$463);
  int32_t _tmp$1757 = Moonbit_array_length(other$465);
  int32_t _bind$464 = (len$462 >= _tmp$1757) - (len$462 <= _tmp$1757);
  switch (_bind$464) {
    case 0: {
      int32_t i$466 = 0;
      while (1) {
        if (i$466 < len$462) {
          int32_t _p$1095 = self$463[i$466];
          int32_t _p$1096 = other$465[i$466];
          int32_t _tmp$1754 = (int32_t)_p$1095;
          int32_t _tmp$1755 = (int32_t)_p$1096;
          int32_t order$467 =
            (_tmp$1754 >= _tmp$1755) - (_tmp$1754 <= _tmp$1755);
          int32_t _tmp$1756;
          if (order$467 != 0) {
            moonbit_decref(other$465);
            moonbit_decref(self$463);
            return order$467;
          }
          _tmp$1756 = i$466 + 1;
          i$466 = _tmp$1756;
          continue;
        } else {
          moonbit_decref(other$465);
          moonbit_decref(self$463);
        }
        break;
      }
      return 0;
      break;
    }
    default: {
      moonbit_decref(other$465);
      moonbit_decref(self$463);
      return _bind$464;
      break;
    }
  }
}

struct $StringView $$moonbitlang$core$builtin$ToStringView$$String$$to_string_view(
  moonbit_string_t self$461
) {
  int32_t _tmp$1753 = Moonbit_array_length(self$461);
  return (struct $StringView){0, _tmp$1753, self$461};
}

moonbit_string_t $$moonbitlang$core$builtin$Show$$Char$$to_string(
  int32_t self$460
) {
  return $moonbitlang$core$builtin$char_to_string(self$460);
}

moonbit_string_t $moonbitlang$core$builtin$char_to_string(int32_t char$459) {
  struct $$moonbitlang$core$builtin$StringBuilder* _self$458 =
    $$moonbitlang$core$builtin$StringBuilder$$new$inner(0);
  struct $$moonbitlang$core$builtin$StringBuilder* _tmp$1752;
  moonbit_incref(_self$458);
  $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_char(
    _self$458, char$459
  );
  _tmp$1752 = _self$458;
  return $$moonbitlang$core$builtin$StringBuilder$$to_string(_tmp$1752);
}

struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* $String$$iter(
  moonbit_string_t self$453
) {
  int32_t len$452 = Moonbit_array_length(self$453);
  struct $Ref$3c$Int$3e$* index$454 =
    (struct $Ref$3c$Int$3e$*)moonbit_malloc(sizeof(struct $Ref$3c$Int$3e$));
  struct $String$$iter$$2a$p$fn$1$2d$cap* _closure$2821;
  struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* _p$1092;
  Moonbit_object_header(index$454)->meta
  = Moonbit_make_regular_object_header(
    sizeof(struct $Ref$3c$Int$3e$) >> 2, 0, 0
  );
  index$454->$0 = 0;
  _closure$2821
  = (struct $String$$iter$$2a$p$fn$1$2d$cap*)moonbit_malloc(
      sizeof(struct $String$$iter$$2a$p$fn$1$2d$cap)
    );
  Moonbit_object_header(_closure$2821)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $String$$iter$$2a$p$fn$1$2d$cap, $0) >> 2, 2, 0
  );
  _closure$2821->code = &$String$$iter$$2a$p$fn$1;
  _closure$2821->$0 = index$454;
  _closure$2821->$1 = self$453;
  _closure$2821->$2 = len$452;
  _p$1092 = (struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$*)_closure$2821;
  return _p$1092;
}

int32_t $String$$iter$$2a$p$fn$1(
  struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* _env$1736
) {
  struct $String$$iter$$2a$p$fn$1$2d$cap* _casted_env$1737 =
    (struct $String$$iter$$2a$p$fn$1$2d$cap*)_env$1736;
  int32_t len$452 = _casted_env$1737->$2;
  moonbit_string_t _field$2551 = _casted_env$1737->$1;
  moonbit_string_t self$453 = _field$2551;
  struct $Ref$3c$Int$3e$* _field$2550 = _casted_env$1737->$0;
  int32_t _cnt$2711 = Moonbit_object_header(_casted_env$1737)->rc;
  struct $Ref$3c$Int$3e$* index$454;
  int32_t val$1738;
  if (_cnt$2711 > 1) {
    int32_t _new_cnt$2712 = _cnt$2711 - 1;
    Moonbit_object_header(_casted_env$1737)->rc = _new_cnt$2712;
    moonbit_incref(self$453);
    moonbit_incref(_field$2550);
  } else if (_cnt$2711 == 1) {
    moonbit_free(_casted_env$1737);
  }
  index$454 = _field$2550;
  val$1738 = index$454->$0;
  if (val$1738 < len$452) {
    int32_t val$1751 = index$454->$0;
    int32_t c1$455 = self$453[val$1751];
    int32_t _if_result$2822;
    int32_t val$1748;
    int32_t _tmp$1747;
    int32_t _tmp$1750;
    int32_t _tmp$1749;
    if (c1$455 >= 55296 && c1$455 <= 56319) {
      int32_t val$1740 = index$454->$0;
      int32_t _tmp$1739 = val$1740 + 1;
      _if_result$2822 = _tmp$1739 < len$452;
    } else {
      _if_result$2822 = 0;
    }
    if (_if_result$2822) {
      int32_t val$1746 = index$454->$0;
      int32_t _tmp$1745 = val$1746 + 1;
      int32_t _tmp$2549 = self$453[_tmp$1745];
      int32_t c2$456;
      moonbit_decref(self$453);
      c2$456 = _tmp$2549;
      if (c2$456 >= 56320 && c2$456 <= 57343) {
        int32_t _tmp$1743 = (int32_t)c1$455;
        int32_t _tmp$1744 = (int32_t)c2$456;
        int32_t c$457 =
          $moonbitlang$core$builtin$code_point_of_surrogate_pair(
            _tmp$1743, _tmp$1744
          );
        int32_t val$1742 = index$454->$0;
        int32_t _tmp$1741 = val$1742 + 2;
        index$454->$0 = _tmp$1741;
        moonbit_decref(index$454);
        return c$457;
      }
    } else {
      moonbit_decref(self$453);
    }
    val$1748 = index$454->$0;
    _tmp$1747 = val$1748 + 1;
    index$454->$0 = _tmp$1747;
    moonbit_decref(index$454);
    _tmp$1750 = (int32_t)c1$455;
    _tmp$1749 = _tmp$1750;
    return _tmp$1749;
  } else {
    moonbit_decref(index$454);
    moonbit_decref(self$453);
    return -1;
  }
}

int32_t $String$$contains(
  moonbit_string_t self$450,
  struct $StringView str$451
) {
  int32_t _tmp$1735 = Moonbit_array_length(self$450);
  struct $StringView _tmp$1734 = (struct $StringView){0, _tmp$1735, self$450};
  return $StringView$$contains(_tmp$1734, str$451);
}

int32_t $StringView$$contains(
  struct $StringView self$448,
  struct $StringView str$449
) {
  int64_t _bind$447 = $StringView$$find(self$448, str$449);
  int32_t _tmp$1733 = _bind$447 == 4294967296ll;
  return !_tmp$1733;
}

int32_t $$moonbitlang$core$builtin$Iter$$next$1(
  struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* self$446
) {
  struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* _func$445 = self$446;
  return _func$445->code(_func$445);
}

struct $$ZSeanYves$Doclint$src$core$Issue* $$moonbitlang$core$builtin$Iter$$next$0(
  struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$444
) {
  struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* _func$443 =
    self$444;
  return _func$443->code(_func$443);
}

int32_t $$moonbitlang$core$builtin$Array$$push$4(
  struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$* self$440,
  int32_t value$442
) {
  int32_t len$1728 = self$440->$1;
  moonbit_bytes_t _field$2554 = self$440->$0;
  moonbit_bytes_t buf$1730 = _field$2554;
  int32_t _tmp$2553 = Moonbit_array_length(buf$1730);
  int32_t _tmp$1729 = _tmp$2553;
  int32_t length$441;
  moonbit_bytes_t _field$2552;
  moonbit_bytes_t buf$1731;
  int32_t _tmp$1732;
  if (len$1728 == _tmp$1729) {
    moonbit_incref(self$440);
    $$moonbitlang$core$builtin$Array$$realloc$4(self$440);
  }
  length$441 = self$440->$1;
  _field$2552 = self$440->$0;
  buf$1731 = _field$2552;
  buf$1731[length$441] = value$442;
  _tmp$1732 = length$441 + 1;
  self$440->$1 = _tmp$1732;
  moonbit_decref(self$440);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$push$3(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$437,
  struct $$ZSeanYves$Doclint$src$core$Issue* value$439
) {
  int32_t len$1723 = self$437->$1;
  struct $$ZSeanYves$Doclint$src$core$Issue** _field$2558 = self$437->$0;
  struct $$ZSeanYves$Doclint$src$core$Issue** buf$1725 = _field$2558;
  int32_t _tmp$2557 = Moonbit_array_length(buf$1725);
  int32_t _tmp$1724 = _tmp$2557;
  int32_t length$438;
  struct $$ZSeanYves$Doclint$src$core$Issue** _field$2556;
  struct $$ZSeanYves$Doclint$src$core$Issue** buf$1726;
  struct $$ZSeanYves$Doclint$src$core$Issue* _old$2555;
  int32_t _tmp$1727;
  if (len$1723 == _tmp$1724) {
    moonbit_incref(self$437);
    $$moonbitlang$core$builtin$Array$$realloc$3(self$437);
  }
  length$438 = self$437->$1;
  _field$2556 = self$437->$0;
  buf$1726 = _field$2556;
  _old$2555
  = (struct $$ZSeanYves$Doclint$src$core$Issue*)buf$1726[length$438];
  if (_old$2555) {
    moonbit_decref(_old$2555);
  }
  buf$1726[length$438] = value$439;
  _tmp$1727 = length$438 + 1;
  self$437->$1 = _tmp$1727;
  moonbit_decref(self$437);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$push$2(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* self$434,
  struct $$ZSeanYves$Doclint$src$core$Page* value$436
) {
  int32_t len$1718 = self$434->$1;
  struct $$ZSeanYves$Doclint$src$core$Page** _field$2562 = self$434->$0;
  struct $$ZSeanYves$Doclint$src$core$Page** buf$1720 = _field$2562;
  int32_t _tmp$2561 = Moonbit_array_length(buf$1720);
  int32_t _tmp$1719 = _tmp$2561;
  int32_t length$435;
  struct $$ZSeanYves$Doclint$src$core$Page** _field$2560;
  struct $$ZSeanYves$Doclint$src$core$Page** buf$1721;
  struct $$ZSeanYves$Doclint$src$core$Page* _old$2559;
  int32_t _tmp$1722;
  if (len$1718 == _tmp$1719) {
    moonbit_incref(self$434);
    $$moonbitlang$core$builtin$Array$$realloc$2(self$434);
  }
  length$435 = self$434->$1;
  _field$2560 = self$434->$0;
  buf$1721 = _field$2560;
  _old$2559 = (struct $$ZSeanYves$Doclint$src$core$Page*)buf$1721[length$435];
  if (_old$2559) {
    moonbit_decref(_old$2559);
  }
  buf$1721[length$435] = value$436;
  _tmp$1722 = length$435 + 1;
  self$434->$1 = _tmp$1722;
  moonbit_decref(self$434);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$push$1(
  struct $$moonbitlang$core$builtin$Array$3c$Char$3e$* self$431,
  int32_t value$433
) {
  int32_t len$1713 = self$431->$1;
  int32_t* _field$2565 = self$431->$0;
  int32_t* buf$1715 = _field$2565;
  int32_t _tmp$2564 = Moonbit_array_length(buf$1715);
  int32_t _tmp$1714 = _tmp$2564;
  int32_t length$432;
  int32_t* _field$2563;
  int32_t* buf$1716;
  int32_t _tmp$1717;
  if (len$1713 == _tmp$1714) {
    moonbit_incref(self$431);
    $$moonbitlang$core$builtin$Array$$realloc$1(self$431);
  }
  length$432 = self$431->$1;
  _field$2563 = self$431->$0;
  buf$1716 = _field$2563;
  buf$1716[length$432] = value$433;
  _tmp$1717 = length$432 + 1;
  self$431->$1 = _tmp$1717;
  moonbit_decref(self$431);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$push$0(
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* self$428,
  moonbit_string_t value$430
) {
  int32_t len$1708 = self$428->$1;
  moonbit_string_t* _field$2569 = self$428->$0;
  moonbit_string_t* buf$1710 = _field$2569;
  int32_t _tmp$2568 = Moonbit_array_length(buf$1710);
  int32_t _tmp$1709 = _tmp$2568;
  int32_t length$429;
  moonbit_string_t* _field$2567;
  moonbit_string_t* buf$1711;
  moonbit_string_t _old$2566;
  int32_t _tmp$1712;
  if (len$1708 == _tmp$1709) {
    moonbit_incref(self$428);
    $$moonbitlang$core$builtin$Array$$realloc$0(self$428);
  }
  length$429 = self$428->$1;
  _field$2567 = self$428->$0;
  buf$1711 = _field$2567;
  _old$2566 = (moonbit_string_t)buf$1711[length$429];
  moonbit_decref(_old$2566);
  buf$1711[length$429] = value$430;
  _tmp$1712 = length$429 + 1;
  self$428->$1 = _tmp$1712;
  moonbit_decref(self$428);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$realloc$4(
  struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$* self$426
) {
  int32_t old_cap$425 = self$426->$1;
  int32_t new_cap$427;
  if (old_cap$425 == 0) {
    new_cap$427 = 8;
  } else {
    new_cap$427 = old_cap$425 * 2;
  }
  $$moonbitlang$core$builtin$Array$$resize_buffer$4(self$426, new_cap$427);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$realloc$3(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$423
) {
  int32_t old_cap$422 = self$423->$1;
  int32_t new_cap$424;
  if (old_cap$422 == 0) {
    new_cap$424 = 8;
  } else {
    new_cap$424 = old_cap$422 * 2;
  }
  $$moonbitlang$core$builtin$Array$$resize_buffer$3(self$423, new_cap$424);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$realloc$2(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* self$420
) {
  int32_t old_cap$419 = self$420->$1;
  int32_t new_cap$421;
  if (old_cap$419 == 0) {
    new_cap$421 = 8;
  } else {
    new_cap$421 = old_cap$419 * 2;
  }
  $$moonbitlang$core$builtin$Array$$resize_buffer$2(self$420, new_cap$421);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$realloc$1(
  struct $$moonbitlang$core$builtin$Array$3c$Char$3e$* self$417
) {
  int32_t old_cap$416 = self$417->$1;
  int32_t new_cap$418;
  if (old_cap$416 == 0) {
    new_cap$418 = 8;
  } else {
    new_cap$418 = old_cap$416 * 2;
  }
  $$moonbitlang$core$builtin$Array$$resize_buffer$1(self$417, new_cap$418);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$realloc$0(
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* self$414
) {
  int32_t old_cap$413 = self$414->$1;
  int32_t new_cap$415;
  if (old_cap$413 == 0) {
    new_cap$415 = 8;
  } else {
    new_cap$415 = old_cap$413 * 2;
  }
  $$moonbitlang$core$builtin$Array$$resize_buffer$0(self$414, new_cap$415);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$resize_buffer$4(
  struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$* self$410,
  int32_t new_capacity$408
) {
  moonbit_bytes_t new_buf$407 =
    (moonbit_bytes_t)moonbit_make_bytes_raw(new_capacity$408);
  moonbit_bytes_t _field$2571 = self$410->$0;
  moonbit_bytes_t old_buf$409 = _field$2571;
  int32_t old_cap$411 = Moonbit_array_length(old_buf$409);
  int32_t copy_len$412;
  moonbit_bytes_t _old$2570;
  if (old_cap$411 < new_capacity$408) {
    copy_len$412 = old_cap$411;
  } else {
    copy_len$412 = new_capacity$408;
  }
  moonbit_incref(old_buf$409);
  moonbit_incref(new_buf$407);
  $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$4(
    new_buf$407, 0, old_buf$409, 0, copy_len$412
  );
  _old$2570 = self$410->$0;
  moonbit_decref(_old$2570);
  self$410->$0 = new_buf$407;
  moonbit_decref(self$410);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$resize_buffer$3(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$404,
  int32_t new_capacity$402
) {
  struct $$ZSeanYves$Doclint$src$core$Issue** new_buf$401 =
    (struct $$ZSeanYves$Doclint$src$core$Issue**)moonbit_make_ref_array(
      new_capacity$402, 0
    );
  struct $$ZSeanYves$Doclint$src$core$Issue** _field$2573 = self$404->$0;
  struct $$ZSeanYves$Doclint$src$core$Issue** old_buf$403 = _field$2573;
  int32_t old_cap$405 = Moonbit_array_length(old_buf$403);
  int32_t copy_len$406;
  struct $$ZSeanYves$Doclint$src$core$Issue** _old$2572;
  if (old_cap$405 < new_capacity$402) {
    copy_len$406 = old_cap$405;
  } else {
    copy_len$406 = new_capacity$402;
  }
  moonbit_incref(old_buf$403);
  moonbit_incref(new_buf$401);
  $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$3(
    new_buf$401, 0, old_buf$403, 0, copy_len$406
  );
  _old$2572 = self$404->$0;
  moonbit_decref(_old$2572);
  self$404->$0 = new_buf$401;
  moonbit_decref(self$404);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$resize_buffer$2(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* self$398,
  int32_t new_capacity$396
) {
  struct $$ZSeanYves$Doclint$src$core$Page** new_buf$395 =
    (struct $$ZSeanYves$Doclint$src$core$Page**)moonbit_make_ref_array(
      new_capacity$396, 0
    );
  struct $$ZSeanYves$Doclint$src$core$Page** _field$2575 = self$398->$0;
  struct $$ZSeanYves$Doclint$src$core$Page** old_buf$397 = _field$2575;
  int32_t old_cap$399 = Moonbit_array_length(old_buf$397);
  int32_t copy_len$400;
  struct $$ZSeanYves$Doclint$src$core$Page** _old$2574;
  if (old_cap$399 < new_capacity$396) {
    copy_len$400 = old_cap$399;
  } else {
    copy_len$400 = new_capacity$396;
  }
  moonbit_incref(old_buf$397);
  moonbit_incref(new_buf$395);
  $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$2(
    new_buf$395, 0, old_buf$397, 0, copy_len$400
  );
  _old$2574 = self$398->$0;
  moonbit_decref(_old$2574);
  self$398->$0 = new_buf$395;
  moonbit_decref(self$398);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$resize_buffer$1(
  struct $$moonbitlang$core$builtin$Array$3c$Char$3e$* self$392,
  int32_t new_capacity$390
) {
  int32_t* new_buf$389 =
    (int32_t*)moonbit_make_int32_array_raw(new_capacity$390);
  int32_t* _field$2577 = self$392->$0;
  int32_t* old_buf$391 = _field$2577;
  int32_t old_cap$393 = Moonbit_array_length(old_buf$391);
  int32_t copy_len$394;
  int32_t* _old$2576;
  if (old_cap$393 < new_capacity$390) {
    copy_len$394 = old_cap$393;
  } else {
    copy_len$394 = new_capacity$390;
  }
  moonbit_incref(old_buf$391);
  moonbit_incref(new_buf$389);
  $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$1(
    new_buf$389, 0, old_buf$391, 0, copy_len$394
  );
  _old$2576 = self$392->$0;
  moonbit_decref(_old$2576);
  self$392->$0 = new_buf$389;
  moonbit_decref(self$392);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Array$$resize_buffer$0(
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* self$386,
  int32_t new_capacity$384
) {
  moonbit_string_t* new_buf$383 =
    (moonbit_string_t*)moonbit_make_ref_array(
      new_capacity$384, (moonbit_string_t)moonbit_string_literal_26.data
    );
  moonbit_string_t* _field$2579 = self$386->$0;
  moonbit_string_t* old_buf$385 = _field$2579;
  int32_t old_cap$387 = Moonbit_array_length(old_buf$385);
  int32_t copy_len$388;
  moonbit_string_t* _old$2578;
  if (old_cap$387 < new_capacity$384) {
    copy_len$388 = old_cap$387;
  } else {
    copy_len$388 = new_capacity$384;
  }
  moonbit_incref(old_buf$385);
  moonbit_incref(new_buf$383);
  $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$0(
    new_buf$383, 0, old_buf$385, 0, copy_len$388
  );
  _old$2578 = self$386->$0;
  moonbit_decref(_old$2578);
  self$386->$0 = new_buf$383;
  moonbit_decref(self$386);
  return 0;
}

int64_t $StringView$$find(
  struct $StringView self$382,
  struct $StringView str$381
) {
  int32_t end$1706 = str$381.$2;
  int32_t start$1707 = str$381.$1;
  int32_t _tmp$1705 = end$1706 - start$1707;
  if (_tmp$1705 <= 4) {
    return $moonbitlang$core$builtin$brute_force_find(self$382, str$381);
  } else {
    return $moonbitlang$core$builtin$boyer_moore_horspool_find(
             self$382, str$381
           );
  }
}

int64_t $moonbitlang$core$builtin$brute_force_find(
  struct $StringView haystack$371,
  struct $StringView needle$373
) {
  int32_t end$1703 = haystack$371.$2;
  int32_t start$1704 = haystack$371.$1;
  int32_t haystack_len$370 = end$1703 - start$1704;
  int32_t end$1701 = needle$373.$2;
  int32_t start$1702 = needle$373.$1;
  int32_t needle_len$372 = end$1701 - start$1702;
  if (needle_len$372 > 0) {
    if (haystack_len$370 >= needle_len$372) {
      int32_t _p$1063 = 0;
      moonbit_string_t _field$2587 = needle$373.$0;
      moonbit_string_t str$1698 = _field$2587;
      int32_t start$1700 = needle$373.$1;
      int32_t _tmp$1699 = start$1700 + _p$1063;
      int32_t _tmp$2586 = str$1698[_tmp$1699];
      int32_t needle_first$374 = _tmp$2586;
      int32_t forward_len$375 = haystack_len$370 - needle_len$372;
      int32_t i$376 = 0;
      while (1) {
        int32_t _tmp$1678 = i$376;
        if (_tmp$1678 <= forward_len$375) {
          int32_t _tmp$1685;
          while (1) {
            int32_t _tmp$1683 = i$376;
            int32_t _if_result$2825;
            if (_tmp$1683 <= forward_len$375) {
              int32_t _p$1066 = i$376;
              moonbit_string_t _field$2585 = haystack$371.$0;
              moonbit_string_t str$1680 = _field$2585;
              int32_t start$1682 = haystack$371.$1;
              int32_t _tmp$1681 = start$1682 + _p$1066;
              int32_t _tmp$2584 = str$1680[_tmp$1681];
              int32_t _tmp$1679 = _tmp$2584;
              _if_result$2825 = _tmp$1679 != needle_first$374;
            } else {
              _if_result$2825 = 0;
            }
            if (_if_result$2825) {
              int32_t _tmp$1684 = i$376;
              i$376 = _tmp$1684 + 1;
              continue;
            }
            break;
          }
          _tmp$1685 = i$376;
          if (_tmp$1685 <= forward_len$375) {
            int32_t j$378 = 1;
            int32_t _tmp$1697;
            while (1) {
              if (j$378 < needle_len$372) {
                int32_t _tmp$1694 = i$376;
                int32_t _p$1069 = _tmp$1694 + j$378;
                moonbit_string_t _field$2583 = haystack$371.$0;
                moonbit_string_t str$1691 = _field$2583;
                int32_t start$1693 = haystack$371.$1;
                int32_t _tmp$1692 = start$1693 + _p$1069;
                int32_t _tmp$2582 = str$1691[_tmp$1692];
                int32_t _tmp$1686 = _tmp$2582;
                moonbit_string_t _field$2581 = needle$373.$0;
                moonbit_string_t str$1688 = _field$2581;
                int32_t start$1690 = needle$373.$1;
                int32_t _tmp$1689 = start$1690 + j$378;
                int32_t _tmp$2580 = str$1688[_tmp$1689];
                int32_t _tmp$1687 = _tmp$2580;
                int32_t _tmp$1695;
                if (_tmp$1686 != _tmp$1687) {
                  break;
                }
                _tmp$1695 = j$378 + 1;
                j$378 = _tmp$1695;
                continue;
              } else {
                int32_t _tmp$1696;
                moonbit_decref(needle$373.$0);
                moonbit_decref(haystack$371.$0);
                _tmp$1696 = i$376;
                return (int64_t)_tmp$1696;
              }
              break;
            }
            _tmp$1697 = i$376;
            i$376 = _tmp$1697 + 1;
          }
          continue;
        } else {
          moonbit_decref(needle$373.$0);
          moonbit_decref(haystack$371.$0);
        }
        break;
      }
      return 4294967296ll;
    } else {
      moonbit_decref(needle$373.$0);
      moonbit_decref(haystack$371.$0);
      return 4294967296ll;
    }
  } else {
    moonbit_decref(needle$373.$0);
    moonbit_decref(haystack$371.$0);
    return $moonbitlang$core$builtin$brute_force_find$constr$369;
  }
}

int64_t $moonbitlang$core$builtin$boyer_moore_horspool_find(
  struct $StringView haystack$357,
  struct $StringView needle$359
) {
  int32_t end$1676 = haystack$357.$2;
  int32_t start$1677 = haystack$357.$1;
  int32_t haystack_len$356 = end$1676 - start$1677;
  int32_t end$1674 = needle$359.$2;
  int32_t start$1675 = needle$359.$1;
  int32_t needle_len$358 = end$1674 - start$1675;
  if (needle_len$358 > 0) {
    if (haystack_len$356 >= needle_len$358) {
      int32_t* skip_table$360 =
        (int32_t*)moonbit_make_int32_array(256, needle_len$358);
      int32_t _end4087$361 = needle_len$358 - 1;
      int32_t i$362 = 0;
      int32_t i$364;
      while (1) {
        if (i$362 < _end4087$361) {
          moonbit_string_t _field$2595 = needle$359.$0;
          moonbit_string_t str$1651 = _field$2595;
          int32_t start$1653 = needle$359.$1;
          int32_t _tmp$1652 = start$1653 + i$362;
          int32_t _tmp$2594 = str$1651[_tmp$1652];
          int32_t _tmp$1650 = _tmp$2594;
          int32_t _tmp$1649 = (int32_t)_tmp$1650;
          int32_t _tmp$1646 = _tmp$1649 & 255;
          int32_t _tmp$1648 = needle_len$358 - 1;
          int32_t _tmp$1647 = _tmp$1648 - i$362;
          int32_t _tmp$1654;
          if (
            _tmp$1646 < 0
            || _tmp$1646 >= Moonbit_array_length(skip_table$360)
          ) {
            moonbit_panic();
          }
          skip_table$360[_tmp$1646] = _tmp$1647;
          _tmp$1654 = i$362 + 1;
          i$362 = _tmp$1654;
          continue;
        }
        break;
      }
      i$364 = 0;
      while (1) {
        int32_t _tmp$1655 = haystack_len$356 - needle_len$358;
        if (i$364 <= _tmp$1655) {
          int32_t _end4093$365 = needle_len$358 - 1;
          int32_t j$366 = 0;
          int32_t _tmp$1673;
          int32_t _p$1056;
          moonbit_string_t _field$2589;
          moonbit_string_t str$1670;
          int32_t start$1672;
          int32_t _tmp$1671;
          int32_t _tmp$2588;
          int32_t _tmp$1669;
          int32_t _tmp$1668;
          int32_t _tmp$1667;
          int32_t _tmp$1666;
          int32_t _tmp$1665;
          while (1) {
            if (j$366 <= _end4093$365) {
              int32_t _p$1051 = i$364 + j$366;
              moonbit_string_t _field$2593 = haystack$357.$0;
              moonbit_string_t str$1661 = _field$2593;
              int32_t start$1663 = haystack$357.$1;
              int32_t _tmp$1662 = start$1663 + _p$1051;
              int32_t _tmp$2592 = str$1661[_tmp$1662];
              int32_t _tmp$1656 = _tmp$2592;
              moonbit_string_t _field$2591 = needle$359.$0;
              moonbit_string_t str$1658 = _field$2591;
              int32_t start$1660 = needle$359.$1;
              int32_t _tmp$1659 = start$1660 + j$366;
              int32_t _tmp$2590 = str$1658[_tmp$1659];
              int32_t _tmp$1657 = _tmp$2590;
              int32_t _tmp$1664;
              if (_tmp$1656 != _tmp$1657) {
                break;
              }
              _tmp$1664 = j$366 + 1;
              j$366 = _tmp$1664;
              continue;
            } else {
              moonbit_decref(skip_table$360);
              moonbit_decref(needle$359.$0);
              moonbit_decref(haystack$357.$0);
              return (int64_t)i$364;
            }
            break;
          }
          _tmp$1673 = i$364 + needle_len$358;
          _p$1056 = _tmp$1673 - 1;
          _field$2589 = haystack$357.$0;
          str$1670 = _field$2589;
          start$1672 = haystack$357.$1;
          _tmp$1671 = start$1672 + _p$1056;
          _tmp$2588 = str$1670[_tmp$1671];
          _tmp$1669 = _tmp$2588;
          _tmp$1668 = (int32_t)_tmp$1669;
          _tmp$1667 = _tmp$1668 & 255;
          if (
            _tmp$1667 < 0
            || _tmp$1667 >= Moonbit_array_length(skip_table$360)
          ) {
            moonbit_panic();
          }
          _tmp$1666 = (int32_t)skip_table$360[_tmp$1667];
          _tmp$1665 = i$364 + _tmp$1666;
          i$364 = _tmp$1665;
          continue;
        } else {
          moonbit_decref(skip_table$360);
          moonbit_decref(needle$359.$0);
          moonbit_decref(haystack$357.$0);
        }
        break;
      }
      return 4294967296ll;
    } else {
      moonbit_decref(needle$359.$0);
      moonbit_decref(haystack$357.$0);
      return 4294967296ll;
    }
  } else {
    moonbit_decref(needle$359.$0);
    moonbit_decref(haystack$357.$0);
    return $moonbitlang$core$builtin$boyer_moore_horspool_find$constr$355;
  }
}

int32_t $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_view(
  struct $$moonbitlang$core$builtin$StringBuilder* self$353,
  struct $StringView str$354
) {
  int32_t len$1628 = self$353->$1;
  int32_t end$1631 = str$354.$2;
  int32_t start$1632 = str$354.$1;
  int32_t _tmp$1630 = end$1631 - start$1632;
  int32_t _tmp$1629 = _tmp$1630 * 2;
  int32_t _tmp$1627 = len$1628 + _tmp$1629;
  moonbit_bytes_t _field$2598;
  moonbit_bytes_t data$1633;
  int32_t len$1634;
  moonbit_string_t _field$2597;
  moonbit_string_t str$1635;
  int32_t start$1636;
  int32_t end$1638;
  int32_t start$1639;
  int32_t _tmp$1637;
  int32_t len$1641;
  int32_t end$1644;
  int32_t _field$2596;
  int32_t start$1645;
  int32_t _tmp$1643;
  int32_t _tmp$1642;
  int32_t _tmp$1640;
  moonbit_incref(self$353);
  $$moonbitlang$core$builtin$StringBuilder$$grow_if_necessary(
    self$353, _tmp$1627
  );
  _field$2598 = self$353->$0;
  data$1633 = _field$2598;
  len$1634 = self$353->$1;
  _field$2597 = str$354.$0;
  str$1635 = _field$2597;
  start$1636 = str$354.$1;
  end$1638 = str$354.$2;
  start$1639 = str$354.$1;
  _tmp$1637 = end$1638 - start$1639;
  moonbit_incref(str$1635);
  moonbit_incref(data$1633);
  $FixedArray$$blit_from_string(
    data$1633, len$1634, str$1635, start$1636, _tmp$1637
  );
  len$1641 = self$353->$1;
  end$1644 = str$354.$2;
  _field$2596 = str$354.$1;
  moonbit_decref(str$354.$0);
  start$1645 = _field$2596;
  _tmp$1643 = end$1644 - start$1645;
  _tmp$1642 = _tmp$1643 * 2;
  _tmp$1640 = len$1641 + _tmp$1642;
  self$353->$1 = _tmp$1640;
  moonbit_decref(self$353);
  return 0;
}

int32_t $String$$char_length_eq$inner(
  moonbit_string_t self$345,
  int32_t len$348,
  int32_t start_offset$352,
  int64_t end_offset$343
) {
  int32_t end_offset$342;
  int32_t index$346;
  int32_t count$347;
  if (end_offset$343 == 4294967296ll) {
    end_offset$342 = Moonbit_array_length(self$345);
  } else {
    int64_t _Some$344 = end_offset$343;
    end_offset$342 = (int32_t)_Some$344;
  }
  index$346 = start_offset$352;
  count$347 = 0;
  while (1) {
    if (index$346 < end_offset$342 && count$347 < len$348) {
      int32_t c1$349 = self$345[index$346];
      int32_t _if_result$2831;
      int32_t _tmp$1625;
      int32_t _tmp$1626;
      if (c1$349 >= 55296 && c1$349 <= 56319) {
        int32_t _tmp$1621 = index$346 + 1;
        _if_result$2831 = _tmp$1621 < end_offset$342;
      } else {
        _if_result$2831 = 0;
      }
      if (_if_result$2831) {
        int32_t _tmp$1624 = index$346 + 1;
        int32_t c2$350 = self$345[_tmp$1624];
        if (c2$350 >= 56320 && c2$350 <= 57343) {
          int32_t _tmp$1622 = index$346 + 2;
          int32_t _tmp$1623 = count$347 + 1;
          index$346 = _tmp$1622;
          count$347 = _tmp$1623;
          continue;
        } else {
          $moonbitlang$core$builtin$abort$0(
            (moonbit_string_t)moonbit_string_literal_131.data,
              (moonbit_string_t)moonbit_string_literal_132.data
          );
        }
      }
      _tmp$1625 = index$346 + 1;
      _tmp$1626 = count$347 + 1;
      index$346 = _tmp$1625;
      count$347 = _tmp$1626;
      continue;
    } else {
      moonbit_decref(self$345);
      return count$347 == len$348 && index$346 == end_offset$342;
    }
    break;
  }
}

moonbit_string_t $String$$from_array(
  struct $$moonbitlang$core$builtin$ArrayView$3c$Char$3e$ chars$337
) {
  int32_t end$1619 = chars$337.$2;
  int32_t start$1620 = chars$337.$1;
  int32_t _tmp$1618 = end$1619 - start$1620;
  int32_t _tmp$1617 = _tmp$1618 * 4;
  struct $$moonbitlang$core$builtin$StringBuilder* buf$336 =
    $$moonbitlang$core$builtin$StringBuilder$$new$inner(_tmp$1617);
  int32_t end$1615 = chars$337.$2;
  int32_t start$1616 = chars$337.$1;
  int32_t _len$338 = end$1615 - start$1616;
  int32_t _i$339 = 0;
  while (1) {
    if (_i$339 < _len$338) {
      int32_t* _field$2600 = chars$337.$0;
      int32_t* buf$1611 = _field$2600;
      int32_t start$1613 = chars$337.$1;
      int32_t _tmp$1612 = start$1613 + _i$339;
      int32_t _tmp$2599 = (int32_t)buf$1611[_tmp$1612];
      int32_t c$340 = _tmp$2599;
      int32_t _tmp$1614;
      moonbit_incref(buf$336);
      $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_char(
        buf$336, c$340
      );
      _tmp$1614 = _i$339 + 1;
      _i$339 = _tmp$1614;
      continue;
    } else {
      moonbit_decref(chars$337.$0);
    }
    break;
  }
  return $$moonbitlang$core$builtin$StringBuilder$$to_string(buf$336);
}

int32_t $$moonbitlang$core$builtin$ArrayView$$length$3(
  struct $$moonbitlang$core$builtin$ArrayView$3c$Byte$3e$ self$335
) {
  int32_t end$1609 = self$335.$2;
  int32_t _field$2601 = self$335.$1;
  int32_t start$1610;
  moonbit_decref(self$335.$0);
  start$1610 = _field$2601;
  return end$1609 - start$1610;
}

int32_t $$moonbitlang$core$builtin$ArrayView$$length$2(
  struct $$moonbitlang$core$builtin$ArrayView$3c$$ZSeanYves$Doclint$src$core$Issue$3e$ self$334
) {
  int32_t end$1607 = self$334.$2;
  int32_t _field$2602 = self$334.$1;
  int32_t start$1608;
  moonbit_decref(self$334.$0);
  start$1608 = _field$2602;
  return end$1607 - start$1608;
}

int32_t $$moonbitlang$core$builtin$ArrayView$$length$1(
  struct $$moonbitlang$core$builtin$ArrayView$3c$String$3e$ self$333
) {
  int32_t end$1605 = self$333.$2;
  int32_t _field$2603 = self$333.$1;
  int32_t start$1606;
  moonbit_decref(self$333.$0);
  start$1606 = _field$2603;
  return end$1605 - start$1606;
}

int32_t $$moonbitlang$core$builtin$ArrayView$$length$0(
  struct $$moonbitlang$core$builtin$ArrayView$3c$Char$3e$ self$332
) {
  int32_t end$1603 = self$332.$2;
  int32_t _field$2604 = self$332.$1;
  int32_t start$1604;
  moonbit_decref(self$332.$0);
  start$1604 = _field$2604;
  return end$1603 - start$1604;
}

struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* $$moonbitlang$core$builtin$Iter$$new$1(
  struct $$3c$$3e$$3d$$3e$Option$3c$Char$3e$* f$331
) {
  return f$331;
}

struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$moonbitlang$core$builtin$Iter$$new$0(
  struct $$3c$$3e$$3d$$3e$Option$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* f$330
) {
  return f$330;
}

int32_t $StringView$$unsafe_get(
  struct $StringView self$328,
  int32_t index$329
) {
  moonbit_string_t _field$2607 = self$328.$0;
  moonbit_string_t str$1600 = _field$2607;
  int32_t _field$2606 = self$328.$1;
  int32_t start$1602 = _field$2606;
  int32_t _tmp$1601 = start$1602 + index$329;
  int32_t _tmp$2605 = str$1600[_tmp$1601];
  moonbit_decref(str$1600);
  return _tmp$2605;
}

moonbit_string_t $Int$$to_string$inner(int32_t self$312, int32_t radix$311) {
  int32_t is_negative$313;
  uint32_t num$314;
  uint16_t* buffer$315;
  if (radix$311 < 2 || radix$311 > 36) {
    $moonbitlang$core$builtin$abort$0(
      (moonbit_string_t)moonbit_string_literal_133.data,
        (moonbit_string_t)moonbit_string_literal_134.data
    );
  }
  if (self$312 == 0) {
    return (moonbit_string_t)moonbit_string_literal_41.data;
  }
  is_negative$313 = self$312 < 0;
  if (is_negative$313) {
    int32_t _tmp$1599 = -self$312;
    num$314 = *(uint32_t*)&_tmp$1599;
  } else {
    num$314 = *(uint32_t*)&self$312;
  }
  switch (radix$311) {
    case 10: {
      int32_t digit_len$316 = $moonbitlang$core$builtin$dec_count32(num$314);
      int32_t _tmp$1596;
      int32_t total_len$317;
      uint16_t* buffer$318;
      int32_t digit_start$319;
      if (is_negative$313) {
        _tmp$1596 = 1;
      } else {
        _tmp$1596 = 0;
      }
      total_len$317 = digit_len$316 + _tmp$1596;
      buffer$318 = (uint16_t*)moonbit_make_string(total_len$317, 0);
      if (is_negative$313) {
        digit_start$319 = 1;
      } else {
        digit_start$319 = 0;
      }
      moonbit_incref(buffer$318);
      $moonbitlang$core$builtin$int_to_string_dec(
        buffer$318, num$314, digit_start$319, total_len$317
      );
      buffer$315 = buffer$318;
      break;
    }
    
    case 16: {
      int32_t digit_len$320 = $moonbitlang$core$builtin$hex_count32(num$314);
      int32_t _tmp$1597;
      int32_t total_len$321;
      uint16_t* buffer$322;
      int32_t digit_start$323;
      if (is_negative$313) {
        _tmp$1597 = 1;
      } else {
        _tmp$1597 = 0;
      }
      total_len$321 = digit_len$320 + _tmp$1597;
      buffer$322 = (uint16_t*)moonbit_make_string(total_len$321, 0);
      if (is_negative$313) {
        digit_start$323 = 1;
      } else {
        digit_start$323 = 0;
      }
      moonbit_incref(buffer$322);
      $moonbitlang$core$builtin$int_to_string_hex(
        buffer$322, num$314, digit_start$323, total_len$321
      );
      buffer$315 = buffer$322;
      break;
    }
    default: {
      int32_t digit_len$324 =
        $moonbitlang$core$builtin$radix_count32(num$314, radix$311);
      int32_t _tmp$1598;
      int32_t total_len$325;
      uint16_t* buffer$326;
      int32_t digit_start$327;
      if (is_negative$313) {
        _tmp$1598 = 1;
      } else {
        _tmp$1598 = 0;
      }
      total_len$325 = digit_len$324 + _tmp$1598;
      buffer$326 = (uint16_t*)moonbit_make_string(total_len$325, 0);
      if (is_negative$313) {
        digit_start$327 = 1;
      } else {
        digit_start$327 = 0;
      }
      moonbit_incref(buffer$326);
      $moonbitlang$core$builtin$int_to_string_generic(
        buffer$326, num$314, digit_start$327, total_len$325, radix$311
      );
      buffer$315 = buffer$326;
      break;
    }
  }
  if (is_negative$313) {
    buffer$315[0] = 45;
  }
  return buffer$315;
}

int32_t $moonbitlang$core$builtin$radix_count32(
  uint32_t value$305,
  int32_t radix$308
) {
  uint32_t num$306;
  uint32_t base$307;
  int32_t count$309;
  if (value$305 == 0u) {
    return 1;
  }
  num$306 = value$305;
  base$307 = *(uint32_t*)&radix$308;
  count$309 = 0;
  while (1) {
    uint32_t _tmp$1593 = num$306;
    if (_tmp$1593 > 0u) {
      int32_t _tmp$1594 = count$309;
      uint32_t _tmp$1595;
      count$309 = _tmp$1594 + 1;
      _tmp$1595 = num$306;
      num$306 = _tmp$1595 / base$307;
      continue;
    }
    break;
  }
  return count$309;
}

int32_t $moonbitlang$core$builtin$hex_count32(uint32_t value$303) {
  if (value$303 == 0u) {
    return 1;
  } else {
    int32_t leading_zeros$304 = moonbit_clz32(value$303);
    int32_t _tmp$1592 = 31 - leading_zeros$304;
    int32_t _tmp$1591 = _tmp$1592 / 4;
    return _tmp$1591 + 1;
  }
}

int32_t $moonbitlang$core$builtin$dec_count32(uint32_t value$302) {
  if (value$302 >= 100000u) {
    if (value$302 >= 10000000u) {
      if (value$302 >= 1000000000u) {
        return 10;
      } else if (value$302 >= 100000000u) {
        return 9;
      } else {
        return 8;
      }
    } else if (value$302 >= 1000000u) {
      return 7;
    } else {
      return 6;
    }
  } else if (value$302 >= 1000u) {
    if (value$302 >= 10000u) {
      return 5;
    } else {
      return 4;
    }
  } else if (value$302 >= 100u) {
    return 3;
  } else if (value$302 >= 10u) {
    return 2;
  } else {
    return 1;
  }
}

int32_t $moonbitlang$core$builtin$int_to_string_dec(
  uint16_t* buffer$292,
  uint32_t num$280,
  int32_t digit_start$283,
  int32_t total_len$282
) {
  uint32_t num$279 = num$280;
  int32_t offset$281 = total_len$282 - digit_start$283;
  uint32_t _tmp$1590;
  int32_t remaining$294;
  int32_t _tmp$1571;
  while (1) {
    uint32_t _tmp$1534 = num$279;
    if (_tmp$1534 >= 10000u) {
      uint32_t _tmp$1557 = num$279;
      uint32_t t$284 = _tmp$1557 / 10000u;
      uint32_t _tmp$1556 = num$279;
      uint32_t _tmp$1555 = _tmp$1556 % 10000u;
      int32_t r$285 = *(int32_t*)&_tmp$1555;
      int32_t d1$286;
      int32_t d2$287;
      int32_t _tmp$1535;
      int32_t _tmp$1554;
      int32_t _tmp$1553;
      int32_t d1_hi$288;
      int32_t _tmp$1552;
      int32_t _tmp$1551;
      int32_t d1_lo$289;
      int32_t _tmp$1550;
      int32_t _tmp$1549;
      int32_t d2_hi$290;
      int32_t _tmp$1548;
      int32_t _tmp$1547;
      int32_t d2_lo$291;
      int32_t _tmp$1537;
      int32_t _tmp$1536;
      int32_t _tmp$1540;
      int32_t _tmp$1539;
      int32_t _tmp$1538;
      int32_t _tmp$1543;
      int32_t _tmp$1542;
      int32_t _tmp$1541;
      int32_t _tmp$1546;
      int32_t _tmp$1545;
      int32_t _tmp$1544;
      num$279 = t$284;
      d1$286 = r$285 / 100;
      d2$287 = r$285 % 100;
      _tmp$1535 = offset$281;
      offset$281 = _tmp$1535 - 4;
      _tmp$1554 = d1$286 / 10;
      _tmp$1553 = 48 + _tmp$1554;
      d1_hi$288 = (uint16_t)_tmp$1553;
      _tmp$1552 = d1$286 % 10;
      _tmp$1551 = 48 + _tmp$1552;
      d1_lo$289 = (uint16_t)_tmp$1551;
      _tmp$1550 = d2$287 / 10;
      _tmp$1549 = 48 + _tmp$1550;
      d2_hi$290 = (uint16_t)_tmp$1549;
      _tmp$1548 = d2$287 % 10;
      _tmp$1547 = 48 + _tmp$1548;
      d2_lo$291 = (uint16_t)_tmp$1547;
      _tmp$1537 = offset$281;
      _tmp$1536 = digit_start$283 + _tmp$1537;
      buffer$292[_tmp$1536] = d1_hi$288;
      _tmp$1540 = offset$281;
      _tmp$1539 = digit_start$283 + _tmp$1540;
      _tmp$1538 = _tmp$1539 + 1;
      buffer$292[_tmp$1538] = d1_lo$289;
      _tmp$1543 = offset$281;
      _tmp$1542 = digit_start$283 + _tmp$1543;
      _tmp$1541 = _tmp$1542 + 2;
      buffer$292[_tmp$1541] = d2_hi$290;
      _tmp$1546 = offset$281;
      _tmp$1545 = digit_start$283 + _tmp$1546;
      _tmp$1544 = _tmp$1545 + 3;
      buffer$292[_tmp$1544] = d2_lo$291;
      continue;
    }
    break;
  }
  _tmp$1590 = num$279;
  remaining$294 = *(int32_t*)&_tmp$1590;
  while (1) {
    int32_t _tmp$1558 = remaining$294;
    if (_tmp$1558 >= 100) {
      int32_t _tmp$1570 = remaining$294;
      int32_t t$295 = _tmp$1570 / 100;
      int32_t _tmp$1569 = remaining$294;
      int32_t d$296 = _tmp$1569 % 100;
      int32_t _tmp$1559;
      int32_t _tmp$1568;
      int32_t _tmp$1567;
      int32_t d_hi$297;
      int32_t _tmp$1566;
      int32_t _tmp$1565;
      int32_t d_lo$298;
      int32_t _tmp$1561;
      int32_t _tmp$1560;
      int32_t _tmp$1564;
      int32_t _tmp$1563;
      int32_t _tmp$1562;
      remaining$294 = t$295;
      _tmp$1559 = offset$281;
      offset$281 = _tmp$1559 - 2;
      _tmp$1568 = d$296 / 10;
      _tmp$1567 = 48 + _tmp$1568;
      d_hi$297 = (uint16_t)_tmp$1567;
      _tmp$1566 = d$296 % 10;
      _tmp$1565 = 48 + _tmp$1566;
      d_lo$298 = (uint16_t)_tmp$1565;
      _tmp$1561 = offset$281;
      _tmp$1560 = digit_start$283 + _tmp$1561;
      buffer$292[_tmp$1560] = d_hi$297;
      _tmp$1564 = offset$281;
      _tmp$1563 = digit_start$283 + _tmp$1564;
      _tmp$1562 = _tmp$1563 + 1;
      buffer$292[_tmp$1562] = d_lo$298;
      continue;
    }
    break;
  }
  _tmp$1571 = remaining$294;
  if (_tmp$1571 >= 10) {
    int32_t _tmp$1572 = offset$281;
    int32_t _tmp$1583;
    int32_t _tmp$1582;
    int32_t _tmp$1581;
    int32_t d_hi$300;
    int32_t _tmp$1580;
    int32_t _tmp$1579;
    int32_t _tmp$1578;
    int32_t d_lo$301;
    int32_t _tmp$1574;
    int32_t _tmp$1573;
    int32_t _tmp$1577;
    int32_t _tmp$1576;
    int32_t _tmp$1575;
    offset$281 = _tmp$1572 - 2;
    _tmp$1583 = remaining$294;
    _tmp$1582 = _tmp$1583 / 10;
    _tmp$1581 = 48 + _tmp$1582;
    d_hi$300 = (uint16_t)_tmp$1581;
    _tmp$1580 = remaining$294;
    _tmp$1579 = _tmp$1580 % 10;
    _tmp$1578 = 48 + _tmp$1579;
    d_lo$301 = (uint16_t)_tmp$1578;
    _tmp$1574 = offset$281;
    _tmp$1573 = digit_start$283 + _tmp$1574;
    buffer$292[_tmp$1573] = d_hi$300;
    _tmp$1577 = offset$281;
    _tmp$1576 = digit_start$283 + _tmp$1577;
    _tmp$1575 = _tmp$1576 + 1;
    buffer$292[_tmp$1575] = d_lo$301;
    moonbit_decref(buffer$292);
  } else {
    int32_t _tmp$1584 = offset$281;
    int32_t _tmp$1589;
    int32_t _tmp$1585;
    int32_t _tmp$1588;
    int32_t _tmp$1587;
    int32_t _tmp$1586;
    offset$281 = _tmp$1584 - 1;
    _tmp$1589 = offset$281;
    _tmp$1585 = digit_start$283 + _tmp$1589;
    _tmp$1588 = remaining$294;
    _tmp$1587 = 48 + _tmp$1588;
    _tmp$1586 = (uint16_t)_tmp$1587;
    buffer$292[_tmp$1585] = _tmp$1586;
    moonbit_decref(buffer$292);
  }
  return 0;
}

int32_t $moonbitlang$core$builtin$int_to_string_generic(
  uint16_t* buffer$274,
  uint32_t num$268,
  int32_t digit_start$266,
  int32_t total_len$265,
  int32_t radix$270
) {
  int32_t offset$264 = total_len$265 - digit_start$266;
  uint32_t n$267 = num$268;
  uint32_t base$269 = *(uint32_t*)&radix$270;
  int32_t _tmp$1516 = radix$270 - 1;
  int32_t _tmp$1515 = radix$270 & _tmp$1516;
  if (_tmp$1515 == 0) {
    int32_t shift$271 = moonbit_ctz32(radix$270);
    uint32_t mask$272 = base$269 - 1u;
    while (1) {
      uint32_t _tmp$1517 = n$267;
      if (_tmp$1517 > 0u) {
        int32_t _tmp$1518 = offset$264;
        uint32_t _tmp$1524;
        uint32_t _tmp$1523;
        int32_t digit$273;
        int32_t _tmp$1521;
        int32_t _tmp$1519;
        int32_t _tmp$1520;
        uint32_t _tmp$1522;
        offset$264 = _tmp$1518 - 1;
        _tmp$1524 = n$267;
        _tmp$1523 = _tmp$1524 & mask$272;
        digit$273 = *(int32_t*)&_tmp$1523;
        _tmp$1521 = offset$264;
        _tmp$1519 = digit_start$266 + _tmp$1521;
        _tmp$1520
        = ((moonbit_string_t)moonbit_string_literal_135.data)[
          digit$273
        ];
        buffer$274[_tmp$1519] = _tmp$1520;
        _tmp$1522 = n$267;
        n$267 = _tmp$1522 >> (shift$271 & 31);
        continue;
      } else {
        moonbit_decref(buffer$274);
      }
      break;
    }
  } else {
    while (1) {
      uint32_t _tmp$1525 = n$267;
      if (_tmp$1525 > 0u) {
        int32_t _tmp$1526 = offset$264;
        uint32_t _tmp$1533;
        uint32_t q$276;
        uint32_t _tmp$1531;
        uint32_t _tmp$1532;
        uint32_t _tmp$1530;
        int32_t digit$277;
        int32_t _tmp$1529;
        int32_t _tmp$1527;
        int32_t _tmp$1528;
        offset$264 = _tmp$1526 - 1;
        _tmp$1533 = n$267;
        q$276 = _tmp$1533 / base$269;
        _tmp$1531 = n$267;
        _tmp$1532 = q$276 * base$269;
        _tmp$1530 = _tmp$1531 - _tmp$1532;
        digit$277 = *(int32_t*)&_tmp$1530;
        _tmp$1529 = offset$264;
        _tmp$1527 = digit_start$266 + _tmp$1529;
        _tmp$1528
        = ((moonbit_string_t)moonbit_string_literal_135.data)[
          digit$277
        ];
        buffer$274[_tmp$1527] = _tmp$1528;
        n$267 = q$276;
        continue;
      } else {
        moonbit_decref(buffer$274);
      }
      break;
    }
  }
  return 0;
}

int32_t $moonbitlang$core$builtin$int_to_string_hex(
  uint16_t* buffer$261,
  uint32_t num$257,
  int32_t digit_start$255,
  int32_t total_len$254
) {
  int32_t offset$253 = total_len$254 - digit_start$255;
  uint32_t n$256 = num$257;
  int32_t _tmp$1511;
  while (1) {
    int32_t _tmp$1499 = offset$253;
    if (_tmp$1499 >= 2) {
      int32_t _tmp$1500 = offset$253;
      uint32_t _tmp$1510;
      uint32_t _tmp$1509;
      int32_t byte_val$258;
      int32_t hi$259;
      int32_t lo$260;
      int32_t _tmp$1503;
      int32_t _tmp$1501;
      int32_t _tmp$1502;
      int32_t _tmp$1507;
      int32_t _tmp$1506;
      int32_t _tmp$1504;
      int32_t _tmp$1505;
      uint32_t _tmp$1508;
      offset$253 = _tmp$1500 - 2;
      _tmp$1510 = n$256;
      _tmp$1509 = _tmp$1510 & 255u;
      byte_val$258 = *(int32_t*)&_tmp$1509;
      hi$259 = byte_val$258 / 16;
      lo$260 = byte_val$258 % 16;
      _tmp$1503 = offset$253;
      _tmp$1501 = digit_start$255 + _tmp$1503;
      _tmp$1502 = ((moonbit_string_t)moonbit_string_literal_135.data)[hi$259];
      buffer$261[_tmp$1501] = _tmp$1502;
      _tmp$1507 = offset$253;
      _tmp$1506 = digit_start$255 + _tmp$1507;
      _tmp$1504 = _tmp$1506 + 1;
      _tmp$1505 = ((moonbit_string_t)moonbit_string_literal_135.data)[lo$260];
      buffer$261[_tmp$1504] = _tmp$1505;
      _tmp$1508 = n$256;
      n$256 = _tmp$1508 >> 8;
      continue;
    }
    break;
  }
  _tmp$1511 = offset$253;
  if (_tmp$1511 == 1) {
    uint32_t _tmp$1514 = n$256;
    uint32_t _tmp$1513 = _tmp$1514 & 15u;
    int32_t nibble$263 = *(int32_t*)&_tmp$1513;
    int32_t _tmp$1512 =
      ((moonbit_string_t)moonbit_string_literal_135.data)[nibble$263];
    buffer$261[digit_start$255] = _tmp$1512;
    moonbit_decref(buffer$261);
  } else {
    moonbit_decref(buffer$261);
  }
  return 0;
}

moonbit_string_t $$moonbitlang$core$builtin$Show$$$default_impl$$to_string$1(
  int32_t self$252
) {
  struct $$moonbitlang$core$builtin$StringBuilder* logger$251 =
    $$moonbitlang$core$builtin$StringBuilder$$new$inner(0);
  struct $$moonbitlang$core$builtin$Logger _tmp$1498;
  moonbit_incref(logger$251);
  _tmp$1498
  = (struct $$moonbitlang$core$builtin$Logger){
    $$moonbitlang$core$builtin$StringBuilder$as_$moonbitlang$core$builtin$Logger$static_method_table_id,
      logger$251
  };
  $$moonbitlang$core$builtin$Show$$Int$$output(self$252, _tmp$1498);
  return $$moonbitlang$core$builtin$StringBuilder$$to_string(logger$251);
}

moonbit_string_t $$moonbitlang$core$builtin$Show$$$default_impl$$to_string$0(
  moonbit_string_t self$250
) {
  struct $$moonbitlang$core$builtin$StringBuilder* logger$249 =
    $$moonbitlang$core$builtin$StringBuilder$$new$inner(0);
  struct $$moonbitlang$core$builtin$Logger _tmp$1497;
  moonbit_incref(logger$249);
  _tmp$1497
  = (struct $$moonbitlang$core$builtin$Logger){
    $$moonbitlang$core$builtin$StringBuilder$as_$moonbitlang$core$builtin$Logger$static_method_table_id,
      logger$249
  };
  $$moonbitlang$core$builtin$Show$$$moonbitlang$core$builtin$SourceLoc$$output(
    self$250, _tmp$1497
  );
  return $$moonbitlang$core$builtin$StringBuilder$$to_string(logger$249);
}

int32_t $StringView$$start_offset(struct $StringView self$248) {
  int32_t _field$2608 = self$248.$1;
  moonbit_decref(self$248.$0);
  return _field$2608;
}

int32_t $StringView$$length(struct $StringView self$247) {
  int32_t end$1495 = self$247.$2;
  int32_t _field$2609 = self$247.$1;
  int32_t start$1496;
  moonbit_decref(self$247.$0);
  start$1496 = _field$2609;
  return end$1495 - start$1496;
}

moonbit_string_t $StringView$$data(struct $StringView self$246) {
  moonbit_string_t _field$2610 = self$246.$0;
  return _field$2610;
}

int32_t $$moonbitlang$core$builtin$Logger$$$default_impl$$write_substring$0(
  struct $$moonbitlang$core$builtin$StringBuilder* self$240,
  moonbit_string_t value$243,
  int32_t start$244,
  int32_t len$245
) {
  void* _try_err$242;
  struct $StringView _tmp$1490;
  int32_t _tmp$1492 = start$244 + len$245;
  int64_t _tmp$1491 = (int64_t)_tmp$1492;
  struct moonbit_result_3 _tmp$2840 =
    $String$$sub$inner(value$243, start$244, _tmp$1491);
  if (_tmp$2840.tag) {
    struct $StringView const _ok$1493 = _tmp$2840.data.ok;
    _tmp$1490 = _ok$1493;
  } else {
    void* const _err$1494 = _tmp$2840.data.err;
    _try_err$242 = _err$1494;
    goto $join$241;
  }
  goto $joinlet$2839;
  $join$241:;
  moonbit_decref(_try_err$242);
  moonbit_panic();
  $joinlet$2839:;
  $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_view(
    self$240, _tmp$1490
  );
  return 0;
}

struct moonbit_result_3 $String$$sub(
  moonbit_string_t self$238,
  int64_t start$opt$236,
  int64_t end$239
) {
  int32_t start$235;
  if (start$opt$236 == 4294967296ll) {
    start$235 = 0;
  } else {
    int64_t _Some$237 = start$opt$236;
    start$235 = (int32_t)_Some$237;
  }
  return $String$$sub$inner(self$238, start$235, end$239);
}

struct moonbit_result_3 $String$$sub$inner(
  moonbit_string_t self$228,
  int32_t start$234,
  int64_t end$230
) {
  int32_t len$227 = Moonbit_array_length(self$228);
  int32_t end$229;
  int32_t start$233;
  if (end$230 == 4294967296ll) {
    end$229 = len$227;
  } else {
    int64_t _Some$231 = end$230;
    int32_t _end$232 = (int32_t)_Some$231;
    if (_end$232 < 0) {
      end$229 = len$227 + _end$232;
    } else {
      end$229 = _end$232;
    }
  }
  if (start$234 < 0) {
    start$233 = len$227 + start$234;
  } else {
    start$233 = start$234;
  }
  if (start$233 >= 0 && start$233 <= end$229 && end$229 <= len$227) {
    int32_t _if_result$2841;
    int32_t _if_result$2843;
    struct $StringView _tmp$1488;
    struct moonbit_result_3 _result$2845;
    if (start$233 < len$227) {
      int32_t _p$1021 = self$228[start$233];
      _if_result$2841 = _p$1021 >= 56320 && _p$1021 <= 57343;
    } else {
      _if_result$2841 = 0;
    }
    if (_if_result$2841) {
      void* moonbitlang$core$builtin$CreatingViewError$InvalidIndex$1486;
      struct moonbit_result_3 _result$2842;
      moonbit_decref(self$228);
      moonbitlang$core$builtin$CreatingViewError$InvalidIndex$1486
      = (struct moonbit_object*)&moonbit_constant_constructor_0 + 1;
      _result$2842.tag = 0;
      _result$2842.data.err
      = moonbitlang$core$builtin$CreatingViewError$InvalidIndex$1486;
      return _result$2842;
    }
    if (end$229 < len$227) {
      int32_t _p$1024 = self$228[end$229];
      _if_result$2843 = _p$1024 >= 56320 && _p$1024 <= 57343;
    } else {
      _if_result$2843 = 0;
    }
    if (_if_result$2843) {
      void* moonbitlang$core$builtin$CreatingViewError$InvalidIndex$1487;
      struct moonbit_result_3 _result$2844;
      moonbit_decref(self$228);
      moonbitlang$core$builtin$CreatingViewError$InvalidIndex$1487
      = (struct moonbit_object*)&moonbit_constant_constructor_0 + 1;
      _result$2844.tag = 0;
      _result$2844.data.err
      = moonbitlang$core$builtin$CreatingViewError$InvalidIndex$1487;
      return _result$2844;
    }
    _tmp$1488 = (struct $StringView){start$233, end$229, self$228};
    _result$2845.tag = 1;
    _result$2845.data.ok = _tmp$1488;
    return _result$2845;
  } else {
    void* moonbitlang$core$builtin$CreatingViewError$IndexOutOfBounds$1489;
    struct moonbit_result_3 _result$2846;
    moonbit_decref(self$228);
    moonbitlang$core$builtin$CreatingViewError$IndexOutOfBounds$1489
    = (struct moonbit_object*)&moonbit_constant_constructor_1 + 1;
    _result$2846.tag = 0;
    _result$2846.data.err
    = moonbitlang$core$builtin$CreatingViewError$IndexOutOfBounds$1489;
    return _result$2846;
  }
}

int32_t $$moonbitlang$core$builtin$Compare$$$default_impl$$op_ge$0(
  moonbit_string_t x$225,
  moonbit_string_t y$226
) {
  int32_t _tmp$1485 =
    $$moonbitlang$core$builtin$Compare$$String$$compare(x$225, y$226);
  return _tmp$1485 >= 0;
}

int32_t $$moonbitlang$core$builtin$Compare$$$default_impl$$op_le$0(
  moonbit_string_t x$223,
  moonbit_string_t y$224
) {
  int32_t _tmp$1484 =
    $$moonbitlang$core$builtin$Compare$$String$$compare(x$223, y$224);
  return _tmp$1484 <= 0;
}

int32_t $$moonbitlang$core$builtin$Compare$$UInt16$$compare(
  int32_t self$221,
  int32_t that$222
) {
  int32_t _tmp$1482 = (int32_t)self$221;
  int32_t _tmp$1483 = (int32_t)that$222;
  return (_tmp$1482 >= _tmp$1483) - (_tmp$1482 <= _tmp$1483);
}

int32_t $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_string(
  struct $$moonbitlang$core$builtin$StringBuilder* self$219,
  moonbit_string_t str$220
) {
  int32_t len$1472 = self$219->$1;
  int32_t _tmp$1474 = Moonbit_array_length(str$220);
  int32_t _tmp$1473 = _tmp$1474 * 2;
  int32_t _tmp$1471 = len$1472 + _tmp$1473;
  moonbit_bytes_t _field$2612;
  moonbit_bytes_t data$1475;
  int32_t len$1476;
  int32_t _tmp$1477;
  int32_t len$1479;
  int32_t _tmp$2611;
  int32_t _tmp$1481;
  int32_t _tmp$1480;
  int32_t _tmp$1478;
  moonbit_incref(self$219);
  $$moonbitlang$core$builtin$StringBuilder$$grow_if_necessary(
    self$219, _tmp$1471
  );
  _field$2612 = self$219->$0;
  data$1475 = _field$2612;
  len$1476 = self$219->$1;
  _tmp$1477 = Moonbit_array_length(str$220);
  moonbit_incref(data$1475);
  moonbit_incref(str$220);
  $FixedArray$$blit_from_string(data$1475, len$1476, str$220, 0, _tmp$1477);
  len$1479 = self$219->$1;
  _tmp$2611 = Moonbit_array_length(str$220);
  moonbit_decref(str$220);
  _tmp$1481 = _tmp$2611;
  _tmp$1480 = _tmp$1481 * 2;
  _tmp$1478 = len$1479 + _tmp$1480;
  self$219->$1 = _tmp$1478;
  moonbit_decref(self$219);
  return 0;
}

int32_t $FixedArray$$blit_from_string(
  moonbit_bytes_t self$211,
  int32_t bytes_offset$206,
  moonbit_string_t str$213,
  int32_t str_offset$209,
  int32_t length$207
) {
  int32_t _tmp$1470 = length$207 * 2;
  int32_t _tmp$1469 = bytes_offset$206 + _tmp$1470;
  int32_t e1$205 = _tmp$1469 - 1;
  int32_t _tmp$1468 = str_offset$209 + length$207;
  int32_t e2$208 = _tmp$1468 - 1;
  int32_t len1$210 = Moonbit_array_length(self$211);
  int32_t len2$212 = Moonbit_array_length(str$213);
  if (
    length$207 >= 0
    && bytes_offset$206 >= 0
    && e1$205 < len1$210
    && str_offset$209 >= 0
    && e2$208 < len2$212
  ) {
    int32_t end_str_offset$214 = str_offset$209 + length$207;
    int32_t i$215 = str_offset$209;
    int32_t j$216 = bytes_offset$206;
    while (1) {
      if (i$215 < end_str_offset$214) {
        int32_t _tmp$1465 = str$213[i$215];
        int32_t _tmp$1464 = (int32_t)_tmp$1465;
        uint32_t c$217 = *(uint32_t*)&_tmp$1464;
        uint32_t _p$1015 = c$217 & 255u;
        int32_t _tmp$1460 = *(int32_t*)&_p$1015;
        int32_t _tmp$1459 = _tmp$1460 & 0xff;
        int32_t _tmp$1461;
        uint32_t _p$1018;
        int32_t _tmp$1463;
        int32_t _tmp$1462;
        int32_t _tmp$1466;
        int32_t _tmp$1467;
        if (j$216 < 0 || j$216 >= Moonbit_array_length(self$211)) {
          moonbit_panic();
        }
        self$211[j$216] = _tmp$1459;
        _tmp$1461 = j$216 + 1;
        _p$1018 = c$217 >> 8;
        _tmp$1463 = *(int32_t*)&_p$1018;
        _tmp$1462 = _tmp$1463 & 0xff;
        if (_tmp$1461 < 0 || _tmp$1461 >= Moonbit_array_length(self$211)) {
          moonbit_panic();
        }
        self$211[_tmp$1461] = _tmp$1462;
        _tmp$1466 = i$215 + 1;
        _tmp$1467 = j$216 + 2;
        i$215 = _tmp$1466;
        j$216 = _tmp$1467;
        continue;
      } else {
        moonbit_decref(str$213);
        moonbit_decref(self$211);
      }
      break;
    }
  } else {
    moonbit_decref(str$213);
    moonbit_decref(self$211);
    moonbit_panic();
  }
  return 0;
}

struct $$moonbitlang$core$builtin$SourceLocRepr* $$moonbitlang$core$builtin$SourceLocRepr$$parse(
  moonbit_string_t repr$128
) {
  int32_t _tmp$1458 = Moonbit_array_length(repr$128);
  struct $StringView _bind$127 = (struct $StringView){0, _tmp$1458, repr$128};
  moonbit_string_t _field$2614 = _bind$127.$0;
  moonbit_string_t _data$129 = _field$2614;
  int32_t _start$130 = _bind$127.$1;
  int32_t end$1456 = _bind$127.$2;
  int32_t _field$2613 = _bind$127.$1;
  int32_t start$1457 = _field$2613;
  int32_t _tmp$1455 = end$1456 - start$1457;
  int32_t _end$131 = _start$130 + _tmp$1455;
  int32_t _cursor$132 = _start$130;
  int32_t accept_state$133 = -1;
  int32_t match_end$134 = -1;
  int32_t match_tag_saver_0$135 = -1;
  int32_t match_tag_saver_1$136 = -1;
  int32_t match_tag_saver_2$137 = -1;
  int32_t match_tag_saver_3$138 = -1;
  int32_t match_tag_saver_4$139 = -1;
  int32_t tag_0$140 = -1;
  int32_t tag_1$141 = -1;
  int32_t tag_1_1$142 = -1;
  int32_t tag_1_2$143 = -1;
  int32_t tag_3$144 = -1;
  int32_t tag_2$145 = -1;
  int32_t tag_2_1$146 = -1;
  int32_t tag_4$147 = -1;
  int32_t join_dispatch_19$168;
  int32_t _tmp$1445 = _cursor$132;
  int32_t dispatch_19$169;
  if (_tmp$1445 < _end$131) {
    int32_t _p$961 = _cursor$132;
    int32_t next_char$197 = _data$129[_p$961];
    int32_t _tmp$1446 = _cursor$132;
    _cursor$132 = _tmp$1446 + 1;
    if (next_char$197 < 65) {
      if (next_char$197 < 64) {
        goto $join$148;
      } else {
        while (1) {
          int32_t _tmp$1447;
          tag_0$140 = _cursor$132;
          _tmp$1447 = _cursor$132;
          if (_tmp$1447 < _end$131) {
            int32_t _p$964 = _cursor$132;
            int32_t next_char$200 = _data$129[_p$964];
            int32_t _tmp$1448 = _cursor$132;
            _cursor$132 = _tmp$1448 + 1;
            if (next_char$200 < 55296) {
              if (next_char$200 < 58) {
                goto $join$198;
              } else if (next_char$200 > 58) {
                goto $join$198;
              } else {
                int32_t _tmp$1449 = _cursor$132;
                if (_tmp$1449 < _end$131) {
                  int32_t _p$967 = _cursor$132;
                  int32_t next_char$202 = _data$129[_p$967];
                  int32_t _tmp$1450 = _cursor$132;
                  _cursor$132 = _tmp$1450 + 1;
                  if (next_char$202 < 56319) {
                    if (next_char$202 < 55296) {
                      goto $join$201;
                    } else {
                      join_dispatch_19$168 = 7;
                      goto $join$167;
                    }
                  } else if (next_char$202 > 56319) {
                    if (next_char$202 < 65536) {
                      goto $join$201;
                    } else {
                      goto $join$148;
                    }
                  } else {
                    join_dispatch_19$168 = 8;
                    goto $join$167;
                  }
                  $join$201:;
                  join_dispatch_19$168 = 0;
                  goto $join$167;
                } else {
                  goto $join$148;
                }
              }
            } else if (next_char$200 > 56318) {
              if (next_char$200 < 57344) {
                int32_t _tmp$1451 = _cursor$132;
                if (_tmp$1451 < _end$131) {
                  int32_t _p$970 = _cursor$132;
                  int32_t next_char$203 = _data$129[_p$970];
                  int32_t _tmp$1452 = _cursor$132;
                  _cursor$132 = _tmp$1452 + 1;
                  if (next_char$203 < 56320) {
                    goto $join$148;
                  } else if (next_char$203 > 57343) {
                    goto $join$148;
                  } else {
                    continue;
                  }
                } else {
                  goto $join$148;
                }
              } else if (next_char$200 > 65535) {
                goto $join$148;
              } else {
                goto $join$198;
              }
            } else {
              int32_t _tmp$1453 = _cursor$132;
              if (_tmp$1453 < _end$131) {
                int32_t _p$973 = _cursor$132;
                int32_t next_char$204 = _data$129[_p$973];
                int32_t _tmp$1454 = _cursor$132;
                _cursor$132 = _tmp$1454 + 1;
                if (next_char$204 < 56320) {
                  goto $join$148;
                } else if (next_char$204 > 65535) {
                  goto $join$148;
                } else {
                  continue;
                }
              } else {
                goto $join$148;
              }
            }
            $join$198:;
            continue;
          } else {
            goto $join$148;
          }
          break;
        }
      }
    } else {
      goto $join$148;
    }
  } else {
    goto $join$148;
  }
  $join$167:;
  dispatch_19$169 = join_dispatch_19$168;
  $loop_label_19$172:;
  while (1) {
    int32_t _tmp$1419;
    switch (dispatch_19$169) {
      case 3: {
        int32_t _tmp$1421;
        tag_1_2$143 = tag_1_1$142;
        tag_1_1$142 = tag_1$141;
        tag_1$141 = _cursor$132;
        _tmp$1421 = _cursor$132;
        if (_tmp$1421 < _end$131) {
          int32_t _p$976 = _cursor$132;
          int32_t next_char$176 = _data$129[_p$976];
          int32_t _tmp$1422 = _cursor$132;
          _cursor$132 = _tmp$1422 + 1;
          if (next_char$176 < 55296) {
            if (next_char$176 < 58) {
              if (next_char$176 < 48) {
                goto $join$175;
              } else {
                int32_t _tmp$1423;
                tag_1$141 = _cursor$132;
                tag_2_1$146 = tag_2$145;
                tag_2$145 = _cursor$132;
                tag_3$144 = _cursor$132;
                _tmp$1423 = _cursor$132;
                if (_tmp$1423 < _end$131) {
                  int32_t _p$979 = _cursor$132;
                  int32_t next_char$178 = _data$129[_p$979];
                  int32_t _tmp$1424 = _cursor$132;
                  _cursor$132 = _tmp$1424 + 1;
                  if (next_char$178 < 59) {
                    if (next_char$178 < 46) {
                      if (next_char$178 < 45) {
                        goto $join$177;
                      } else {
                        goto $join$170;
                      }
                    } else if (next_char$178 > 47) {
                      if (next_char$178 < 58) {
                        dispatch_19$169 = 6;
                        goto $loop_label_19$172;
                      } else {
                        dispatch_19$169 = 3;
                        goto $loop_label_19$172;
                      }
                    } else {
                      goto $join$177;
                    }
                  } else if (next_char$178 > 55295) {
                    if (next_char$178 < 57344) {
                      if (next_char$178 < 56319) {
                        dispatch_19$169 = 7;
                        goto $loop_label_19$172;
                      } else {
                        dispatch_19$169 = 8;
                        goto $loop_label_19$172;
                      }
                    } else if (next_char$178 > 65535) {
                      goto $join$148;
                    } else {
                      goto $join$177;
                    }
                  } else {
                    goto $join$177;
                  }
                  $join$177:;
                  dispatch_19$169 = 0;
                  goto $loop_label_19$172;
                } else {
                  goto $join$148;
                }
              }
            } else if (next_char$176 > 58) {
              goto $join$175;
            } else {
              dispatch_19$169 = 1;
              goto $loop_label_19$172;
            }
          } else if (next_char$176 > 56318) {
            if (next_char$176 < 57344) {
              dispatch_19$169 = 8;
              goto $loop_label_19$172;
            } else if (next_char$176 > 65535) {
              goto $join$148;
            } else {
              goto $join$175;
            }
          } else {
            dispatch_19$169 = 7;
            goto $loop_label_19$172;
          }
          $join$175:;
          dispatch_19$169 = 0;
          goto $loop_label_19$172;
        } else {
          goto $join$148;
        }
        break;
      }
      
      case 2: {
        int32_t _tmp$1425;
        tag_1$141 = _cursor$132;
        tag_2$145 = _cursor$132;
        _tmp$1425 = _cursor$132;
        if (_tmp$1425 < _end$131) {
          int32_t _p$982 = _cursor$132;
          int32_t next_char$180 = _data$129[_p$982];
          int32_t _tmp$1426 = _cursor$132;
          _cursor$132 = _tmp$1426 + 1;
          if (next_char$180 < 55296) {
            if (next_char$180 < 58) {
              if (next_char$180 < 48) {
                goto $join$179;
              } else {
                dispatch_19$169 = 2;
                goto $loop_label_19$172;
              }
            } else if (next_char$180 > 58) {
              goto $join$179;
            } else {
              dispatch_19$169 = 3;
              goto $loop_label_19$172;
            }
          } else if (next_char$180 > 56318) {
            if (next_char$180 < 57344) {
              dispatch_19$169 = 8;
              goto $loop_label_19$172;
            } else if (next_char$180 > 65535) {
              goto $join$148;
            } else {
              goto $join$179;
            }
          } else {
            dispatch_19$169 = 7;
            goto $loop_label_19$172;
          }
          $join$179:;
          dispatch_19$169 = 0;
          goto $loop_label_19$172;
        } else {
          goto $join$148;
        }
        break;
      }
      
      case 0: {
        int32_t _tmp$1427;
        tag_1$141 = _cursor$132;
        _tmp$1427 = _cursor$132;
        if (_tmp$1427 < _end$131) {
          int32_t _p$985 = _cursor$132;
          int32_t next_char$182 = _data$129[_p$985];
          int32_t _tmp$1428 = _cursor$132;
          _cursor$132 = _tmp$1428 + 1;
          if (next_char$182 < 55296) {
            if (next_char$182 < 58) {
              goto $join$181;
            } else if (next_char$182 > 58) {
              goto $join$181;
            } else {
              dispatch_19$169 = 1;
              goto $loop_label_19$172;
            }
          } else if (next_char$182 > 56318) {
            if (next_char$182 < 57344) {
              dispatch_19$169 = 8;
              goto $loop_label_19$172;
            } else if (next_char$182 > 65535) {
              goto $join$148;
            } else {
              goto $join$181;
            }
          } else {
            dispatch_19$169 = 7;
            goto $loop_label_19$172;
          }
          $join$181:;
          dispatch_19$169 = 0;
          goto $loop_label_19$172;
        } else {
          goto $join$148;
        }
        break;
      }
      
      case 8: {
        int32_t _tmp$1429 = _cursor$132;
        if (_tmp$1429 < _end$131) {
          int32_t _p$988 = _cursor$132;
          int32_t next_char$183 = _data$129[_p$988];
          int32_t _tmp$1430 = _cursor$132;
          _cursor$132 = _tmp$1430 + 1;
          if (next_char$183 < 56320) {
            goto $join$148;
          } else if (next_char$183 > 57343) {
            goto $join$148;
          } else {
            dispatch_19$169 = 0;
            goto $loop_label_19$172;
          }
        } else {
          goto $join$148;
        }
        break;
      }
      
      case 4: {
        int32_t _tmp$1431;
        tag_1$141 = _cursor$132;
        tag_4$147 = _cursor$132;
        _tmp$1431 = _cursor$132;
        if (_tmp$1431 < _end$131) {
          int32_t _p$991 = _cursor$132;
          int32_t next_char$185 = _data$129[_p$991];
          int32_t _tmp$1432 = _cursor$132;
          _cursor$132 = _tmp$1432 + 1;
          if (next_char$185 < 55296) {
            if (next_char$185 < 58) {
              if (next_char$185 < 48) {
                goto $join$184;
              } else {
                dispatch_19$169 = 4;
                goto $loop_label_19$172;
              }
            } else if (next_char$185 > 58) {
              goto $join$184;
            } else {
              int32_t _tmp$1433;
              tag_1_2$143 = tag_1_1$142;
              tag_1_1$142 = tag_1$141;
              tag_1$141 = _cursor$132;
              _tmp$1433 = _cursor$132;
              if (_tmp$1433 < _end$131) {
                int32_t _p$994 = _cursor$132;
                int32_t next_char$187 = _data$129[_p$994];
                int32_t _tmp$1434 = _cursor$132;
                _cursor$132 = _tmp$1434 + 1;
                if (next_char$187 < 55296) {
                  if (next_char$187 < 58) {
                    if (next_char$187 < 48) {
                      goto $join$186;
                    } else {
                      int32_t _tmp$1435;
                      tag_1$141 = _cursor$132;
                      tag_2_1$146 = tag_2$145;
                      tag_2$145 = _cursor$132;
                      _tmp$1435 = _cursor$132;
                      if (_tmp$1435 < _end$131) {
                        int32_t _p$997 = _cursor$132;
                        int32_t next_char$189 = _data$129[_p$997];
                        int32_t _tmp$1436 = _cursor$132;
                        _cursor$132 = _tmp$1436 + 1;
                        if (next_char$189 < 55296) {
                          if (next_char$189 < 58) {
                            if (next_char$189 < 48) {
                              goto $join$188;
                            } else {
                              dispatch_19$169 = 5;
                              goto $loop_label_19$172;
                            }
                          } else if (next_char$189 > 58) {
                            goto $join$188;
                          } else {
                            dispatch_19$169 = 3;
                            goto $loop_label_19$172;
                          }
                        } else if (next_char$189 > 56318) {
                          if (next_char$189 < 57344) {
                            dispatch_19$169 = 8;
                            goto $loop_label_19$172;
                          } else if (next_char$189 > 65535) {
                            goto $join$148;
                          } else {
                            goto $join$188;
                          }
                        } else {
                          dispatch_19$169 = 7;
                          goto $loop_label_19$172;
                        }
                        $join$188:;
                        dispatch_19$169 = 0;
                        goto $loop_label_19$172;
                      } else {
                        goto $join$174;
                      }
                    }
                  } else if (next_char$187 > 58) {
                    goto $join$186;
                  } else {
                    dispatch_19$169 = 1;
                    goto $loop_label_19$172;
                  }
                } else if (next_char$187 > 56318) {
                  if (next_char$187 < 57344) {
                    dispatch_19$169 = 8;
                    goto $loop_label_19$172;
                  } else if (next_char$187 > 65535) {
                    goto $join$148;
                  } else {
                    goto $join$186;
                  }
                } else {
                  dispatch_19$169 = 7;
                  goto $loop_label_19$172;
                }
                $join$186:;
                dispatch_19$169 = 0;
                goto $loop_label_19$172;
              } else {
                goto $join$148;
              }
            }
          } else if (next_char$185 > 56318) {
            if (next_char$185 < 57344) {
              dispatch_19$169 = 8;
              goto $loop_label_19$172;
            } else if (next_char$185 > 65535) {
              goto $join$148;
            } else {
              goto $join$184;
            }
          } else {
            dispatch_19$169 = 7;
            goto $loop_label_19$172;
          }
          $join$184:;
          dispatch_19$169 = 0;
          goto $loop_label_19$172;
        } else {
          goto $join$148;
        }
        break;
      }
      
      case 5: {
        int32_t _tmp$1437;
        tag_1$141 = _cursor$132;
        tag_2$145 = _cursor$132;
        _tmp$1437 = _cursor$132;
        if (_tmp$1437 < _end$131) {
          int32_t _p$1000 = _cursor$132;
          int32_t next_char$191 = _data$129[_p$1000];
          int32_t _tmp$1438 = _cursor$132;
          _cursor$132 = _tmp$1438 + 1;
          if (next_char$191 < 55296) {
            if (next_char$191 < 58) {
              if (next_char$191 < 48) {
                goto $join$190;
              } else {
                dispatch_19$169 = 5;
                goto $loop_label_19$172;
              }
            } else if (next_char$191 > 58) {
              goto $join$190;
            } else {
              dispatch_19$169 = 3;
              goto $loop_label_19$172;
            }
          } else if (next_char$191 > 56318) {
            if (next_char$191 < 57344) {
              dispatch_19$169 = 8;
              goto $loop_label_19$172;
            } else if (next_char$191 > 65535) {
              goto $join$148;
            } else {
              goto $join$190;
            }
          } else {
            dispatch_19$169 = 7;
            goto $loop_label_19$172;
          }
          $join$190:;
          dispatch_19$169 = 0;
          goto $loop_label_19$172;
        } else {
          goto $join$174;
        }
        break;
      }
      
      case 6: {
        int32_t _tmp$1439;
        tag_1$141 = _cursor$132;
        tag_2$145 = _cursor$132;
        tag_3$144 = _cursor$132;
        _tmp$1439 = _cursor$132;
        if (_tmp$1439 < _end$131) {
          int32_t _p$1003 = _cursor$132;
          int32_t next_char$193 = _data$129[_p$1003];
          int32_t _tmp$1440 = _cursor$132;
          _cursor$132 = _tmp$1440 + 1;
          if (next_char$193 < 59) {
            if (next_char$193 < 46) {
              if (next_char$193 < 45) {
                goto $join$192;
              } else {
                goto $join$170;
              }
            } else if (next_char$193 > 47) {
              if (next_char$193 < 58) {
                dispatch_19$169 = 6;
                goto $loop_label_19$172;
              } else {
                dispatch_19$169 = 3;
                goto $loop_label_19$172;
              }
            } else {
              goto $join$192;
            }
          } else if (next_char$193 > 55295) {
            if (next_char$193 < 57344) {
              if (next_char$193 < 56319) {
                dispatch_19$169 = 7;
                goto $loop_label_19$172;
              } else {
                dispatch_19$169 = 8;
                goto $loop_label_19$172;
              }
            } else if (next_char$193 > 65535) {
              goto $join$148;
            } else {
              goto $join$192;
            }
          } else {
            goto $join$192;
          }
          $join$192:;
          dispatch_19$169 = 0;
          goto $loop_label_19$172;
        } else {
          goto $join$148;
        }
        break;
      }
      
      case 7: {
        int32_t _tmp$1441 = _cursor$132;
        if (_tmp$1441 < _end$131) {
          int32_t _p$1006 = _cursor$132;
          int32_t next_char$194 = _data$129[_p$1006];
          int32_t _tmp$1442 = _cursor$132;
          _cursor$132 = _tmp$1442 + 1;
          if (next_char$194 < 56320) {
            goto $join$148;
          } else if (next_char$194 > 65535) {
            goto $join$148;
          } else {
            dispatch_19$169 = 0;
            goto $loop_label_19$172;
          }
        } else {
          goto $join$148;
        }
        break;
      }
      
      case 1: {
        int32_t _tmp$1443;
        tag_1_1$142 = tag_1$141;
        tag_1$141 = _cursor$132;
        _tmp$1443 = _cursor$132;
        if (_tmp$1443 < _end$131) {
          int32_t _p$1009 = _cursor$132;
          int32_t next_char$196 = _data$129[_p$1009];
          int32_t _tmp$1444 = _cursor$132;
          _cursor$132 = _tmp$1444 + 1;
          if (next_char$196 < 55296) {
            if (next_char$196 < 58) {
              if (next_char$196 < 48) {
                goto $join$195;
              } else {
                dispatch_19$169 = 2;
                goto $loop_label_19$172;
              }
            } else if (next_char$196 > 58) {
              goto $join$195;
            } else {
              dispatch_19$169 = 1;
              goto $loop_label_19$172;
            }
          } else if (next_char$196 > 56318) {
            if (next_char$196 < 57344) {
              dispatch_19$169 = 8;
              goto $loop_label_19$172;
            } else if (next_char$196 > 65535) {
              goto $join$148;
            } else {
              goto $join$195;
            }
          } else {
            dispatch_19$169 = 7;
            goto $loop_label_19$172;
          }
          $join$195:;
          dispatch_19$169 = 0;
          goto $loop_label_19$172;
        } else {
          goto $join$148;
        }
        break;
      }
      default: {
        goto $join$148;
        break;
      }
    }
    $join$174:;
    tag_1$141 = tag_1_2$143;
    tag_2$145 = tag_2_1$146;
    match_tag_saver_0$135 = tag_0$140;
    match_tag_saver_1$136 = tag_1$141;
    match_tag_saver_2$137 = tag_2$145;
    match_tag_saver_3$138 = tag_3$144;
    match_tag_saver_4$139 = tag_4$147;
    accept_state$133 = 0;
    match_end$134 = _cursor$132;
    goto $join$148;
    $join$170:;
    tag_1_1$142 = tag_1_2$143;
    tag_1$141 = _cursor$132;
    tag_2$145 = tag_2_1$146;
    _tmp$1419 = _cursor$132;
    if (_tmp$1419 < _end$131) {
      int32_t _p$1012 = _cursor$132;
      int32_t next_char$173 = _data$129[_p$1012];
      int32_t _tmp$1420 = _cursor$132;
      _cursor$132 = _tmp$1420 + 1;
      if (next_char$173 < 55296) {
        if (next_char$173 < 58) {
          if (next_char$173 < 48) {
            goto $join$171;
          } else {
            dispatch_19$169 = 4;
            continue;
          }
        } else if (next_char$173 > 58) {
          goto $join$171;
        } else {
          dispatch_19$169 = 1;
          continue;
        }
      } else if (next_char$173 > 56318) {
        if (next_char$173 < 57344) {
          dispatch_19$169 = 8;
          continue;
        } else if (next_char$173 > 65535) {
          goto $join$148;
        } else {
          goto $join$171;
        }
      } else {
        dispatch_19$169 = 7;
        continue;
      }
      $join$171:;
      dispatch_19$169 = 0;
      continue;
    } else {
      goto $join$148;
    }
    break;
  }
  $join$148:;
  switch (accept_state$133) {
    case 0: {
      void* _try_err$151;
      struct $StringView start_line$149;
      int32_t _tmp$1416 = match_tag_saver_1$136;
      int32_t _tmp$1415 = _tmp$1416 + 1;
      int64_t _tmp$1412 = (int64_t)_tmp$1415;
      int32_t _tmp$1414 = match_tag_saver_2$137;
      int64_t _tmp$1413 = (int64_t)_tmp$1414;
      struct moonbit_result_3 _tmp$2868;
      void* _try_err$154;
      struct $StringView start_column$152;
      int32_t _tmp$1409;
      int32_t _tmp$1408;
      int64_t _tmp$1405;
      int32_t _tmp$1407;
      int64_t _tmp$1406;
      struct moonbit_result_3 _tmp$2870;
      void* _try_err$157;
      struct $StringView pkg$155;
      int32_t _tmp$1402;
      int64_t _tmp$1399;
      int32_t _tmp$1401;
      int64_t _tmp$1400;
      struct moonbit_result_3 _tmp$2872;
      void* _try_err$160;
      struct $StringView filename$158;
      int32_t _tmp$1396;
      int32_t _tmp$1395;
      int64_t _tmp$1392;
      int32_t _tmp$1394;
      int64_t _tmp$1393;
      struct moonbit_result_3 _tmp$2874;
      void* _try_err$163;
      struct $StringView end_line$161;
      int32_t _tmp$1389;
      int32_t _tmp$1388;
      int64_t _tmp$1385;
      int32_t _tmp$1387;
      int64_t _tmp$1386;
      struct moonbit_result_3 _tmp$2876;
      void* _try_err$166;
      struct $StringView end_column$164;
      int32_t _tmp$1382;
      int32_t _tmp$1381;
      int64_t _tmp$1378;
      int32_t _tmp$1380;
      int64_t _tmp$1379;
      struct moonbit_result_3 _tmp$2878;
      struct $$moonbitlang$core$builtin$SourceLocRepr* _block$2879;
      moonbit_incref(_data$129);
      _tmp$2868 = $String$$sub(_data$129, _tmp$1412, _tmp$1413);
      if (_tmp$2868.tag) {
        struct $StringView const _ok$1417 = _tmp$2868.data.ok;
        start_line$149 = _ok$1417;
      } else {
        void* const _err$1418 = _tmp$2868.data.err;
        _try_err$151 = _err$1418;
        goto $join$150;
      }
      goto $joinlet$2867;
      $join$150:;
      moonbit_decref(_try_err$151);
      moonbit_panic();
      $joinlet$2867:;
      _tmp$1409 = match_tag_saver_2$137;
      _tmp$1408 = _tmp$1409 + 1;
      _tmp$1405 = (int64_t)_tmp$1408;
      _tmp$1407 = match_tag_saver_3$138;
      _tmp$1406 = (int64_t)_tmp$1407;
      moonbit_incref(_data$129);
      _tmp$2870 = $String$$sub(_data$129, _tmp$1405, _tmp$1406);
      if (_tmp$2870.tag) {
        struct $StringView const _ok$1410 = _tmp$2870.data.ok;
        start_column$152 = _ok$1410;
      } else {
        void* const _err$1411 = _tmp$2870.data.err;
        _try_err$154 = _err$1411;
        goto $join$153;
      }
      goto $joinlet$2869;
      $join$153:;
      moonbit_decref(_try_err$154);
      moonbit_panic();
      $joinlet$2869:;
      _tmp$1402 = _start$130 + 1;
      _tmp$1399 = (int64_t)_tmp$1402;
      _tmp$1401 = match_tag_saver_0$135;
      _tmp$1400 = (int64_t)_tmp$1401;
      moonbit_incref(_data$129);
      _tmp$2872 = $String$$sub(_data$129, _tmp$1399, _tmp$1400);
      if (_tmp$2872.tag) {
        struct $StringView const _ok$1403 = _tmp$2872.data.ok;
        pkg$155 = _ok$1403;
      } else {
        void* const _err$1404 = _tmp$2872.data.err;
        _try_err$157 = _err$1404;
        goto $join$156;
      }
      goto $joinlet$2871;
      $join$156:;
      moonbit_decref(_try_err$157);
      moonbit_panic();
      $joinlet$2871:;
      _tmp$1396 = match_tag_saver_0$135;
      _tmp$1395 = _tmp$1396 + 1;
      _tmp$1392 = (int64_t)_tmp$1395;
      _tmp$1394 = match_tag_saver_1$136;
      _tmp$1393 = (int64_t)_tmp$1394;
      moonbit_incref(_data$129);
      _tmp$2874 = $String$$sub(_data$129, _tmp$1392, _tmp$1393);
      if (_tmp$2874.tag) {
        struct $StringView const _ok$1397 = _tmp$2874.data.ok;
        filename$158 = _ok$1397;
      } else {
        void* const _err$1398 = _tmp$2874.data.err;
        _try_err$160 = _err$1398;
        goto $join$159;
      }
      goto $joinlet$2873;
      $join$159:;
      moonbit_decref(_try_err$160);
      moonbit_panic();
      $joinlet$2873:;
      _tmp$1389 = match_tag_saver_3$138;
      _tmp$1388 = _tmp$1389 + 1;
      _tmp$1385 = (int64_t)_tmp$1388;
      _tmp$1387 = match_tag_saver_4$139;
      _tmp$1386 = (int64_t)_tmp$1387;
      moonbit_incref(_data$129);
      _tmp$2876 = $String$$sub(_data$129, _tmp$1385, _tmp$1386);
      if (_tmp$2876.tag) {
        struct $StringView const _ok$1390 = _tmp$2876.data.ok;
        end_line$161 = _ok$1390;
      } else {
        void* const _err$1391 = _tmp$2876.data.err;
        _try_err$163 = _err$1391;
        goto $join$162;
      }
      goto $joinlet$2875;
      $join$162:;
      moonbit_decref(_try_err$163);
      moonbit_panic();
      $joinlet$2875:;
      _tmp$1382 = match_tag_saver_4$139;
      _tmp$1381 = _tmp$1382 + 1;
      _tmp$1378 = (int64_t)_tmp$1381;
      _tmp$1380 = match_end$134;
      _tmp$1379 = (int64_t)_tmp$1380;
      _tmp$2878 = $String$$sub(_data$129, _tmp$1378, _tmp$1379);
      if (_tmp$2878.tag) {
        struct $StringView const _ok$1383 = _tmp$2878.data.ok;
        end_column$164 = _ok$1383;
      } else {
        void* const _err$1384 = _tmp$2878.data.err;
        _try_err$166 = _err$1384;
        goto $join$165;
      }
      goto $joinlet$2877;
      $join$165:;
      moonbit_decref(_try_err$166);
      moonbit_panic();
      $joinlet$2877:;
      _block$2879
      = (struct $$moonbitlang$core$builtin$SourceLocRepr*)moonbit_malloc(
          sizeof(struct $$moonbitlang$core$builtin$SourceLocRepr)
        );
      Moonbit_object_header(_block$2879)->meta
      = Moonbit_make_regular_object_header(
        offsetof(struct $$moonbitlang$core$builtin$SourceLocRepr, $0_0) >> 2,
          6,
          0
      );
      _block$2879->$0_0 = pkg$155.$0;
      _block$2879->$0_1 = pkg$155.$1;
      _block$2879->$0_2 = pkg$155.$2;
      _block$2879->$1_0 = filename$158.$0;
      _block$2879->$1_1 = filename$158.$1;
      _block$2879->$1_2 = filename$158.$2;
      _block$2879->$2_0 = start_line$149.$0;
      _block$2879->$2_1 = start_line$149.$1;
      _block$2879->$2_2 = start_line$149.$2;
      _block$2879->$3_0 = start_column$152.$0;
      _block$2879->$3_1 = start_column$152.$1;
      _block$2879->$3_2 = start_column$152.$2;
      _block$2879->$4_0 = end_line$161.$0;
      _block$2879->$4_1 = end_line$161.$1;
      _block$2879->$4_2 = end_line$161.$2;
      _block$2879->$5_0 = end_column$164.$0;
      _block$2879->$5_1 = end_column$164.$1;
      _block$2879->$5_2 = end_column$164.$2;
      return _block$2879;
      break;
    }
    default: {
      moonbit_decref(_data$129);
      moonbit_panic();
      break;
    }
  }
}

moonbit_bytes_t $$moonbitlang$core$builtin$Array$$buffer$4(
  struct $$moonbitlang$core$builtin$Array$3c$Byte$3e$* self$126
) {
  moonbit_bytes_t _field$2615 = self$126->$0;
  int32_t _cnt$2713 = Moonbit_object_header(self$126)->rc;
  if (_cnt$2713 > 1) {
    int32_t _new_cnt$2714 = _cnt$2713 - 1;
    Moonbit_object_header(self$126)->rc = _new_cnt$2714;
    moonbit_incref(_field$2615);
  } else if (_cnt$2713 == 1) {
    moonbit_free(self$126);
  }
  return _field$2615;
}

struct $$ZSeanYves$Doclint$src$core$Issue** $$moonbitlang$core$builtin$Array$$buffer$3(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* self$125
) {
  struct $$ZSeanYves$Doclint$src$core$Issue** _field$2616 = self$125->$0;
  int32_t _cnt$2715 = Moonbit_object_header(self$125)->rc;
  if (_cnt$2715 > 1) {
    int32_t _new_cnt$2716 = _cnt$2715 - 1;
    Moonbit_object_header(self$125)->rc = _new_cnt$2716;
    moonbit_incref(_field$2616);
  } else if (_cnt$2715 == 1) {
    moonbit_free(self$125);
  }
  return _field$2616;
}

struct $$ZSeanYves$Doclint$src$core$Page** $$moonbitlang$core$builtin$Array$$buffer$2(
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Page$3e$* self$124
) {
  struct $$ZSeanYves$Doclint$src$core$Page** _field$2617 = self$124->$0;
  int32_t _cnt$2717 = Moonbit_object_header(self$124)->rc;
  if (_cnt$2717 > 1) {
    int32_t _new_cnt$2718 = _cnt$2717 - 1;
    Moonbit_object_header(self$124)->rc = _new_cnt$2718;
    moonbit_incref(_field$2617);
  } else if (_cnt$2717 == 1) {
    moonbit_free(self$124);
  }
  return _field$2617;
}

int32_t* $$moonbitlang$core$builtin$Array$$buffer$1(
  struct $$moonbitlang$core$builtin$Array$3c$Char$3e$* self$123
) {
  int32_t* _field$2618 = self$123->$0;
  int32_t _cnt$2719 = Moonbit_object_header(self$123)->rc;
  if (_cnt$2719 > 1) {
    int32_t _new_cnt$2720 = _cnt$2719 - 1;
    Moonbit_object_header(self$123)->rc = _new_cnt$2720;
    moonbit_incref(_field$2618);
  } else if (_cnt$2719 == 1) {
    moonbit_free(self$123);
  }
  return _field$2618;
}

moonbit_string_t* $$moonbitlang$core$builtin$Array$$buffer$0(
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* self$122
) {
  moonbit_string_t* _field$2619 = self$122->$0;
  int32_t _cnt$2721 = Moonbit_object_header(self$122)->rc;
  if (_cnt$2721 > 1) {
    int32_t _new_cnt$2722 = _cnt$2721 - 1;
    Moonbit_object_header(self$122)->rc = _new_cnt$2722;
    moonbit_incref(_field$2619);
  } else if (_cnt$2721 == 1) {
    moonbit_free(self$122);
  }
  return _field$2619;
}

int32_t $UInt16$$unsafe_to_char(int32_t self$121) {
  int32_t _tmp$1377 = (int32_t)self$121;
  return _tmp$1377;
}

int32_t $moonbitlang$core$builtin$code_point_of_surrogate_pair(
  int32_t leading$119,
  int32_t trailing$120
) {
  int32_t _tmp$1376 = leading$119 - 55296;
  int32_t _tmp$1375 = _tmp$1376 * 1024;
  int32_t _tmp$1374 = _tmp$1375 + trailing$120;
  int32_t _tmp$1373 = _tmp$1374 - 56320;
  int32_t _tmp$1372 = _tmp$1373 + 65536;
  return _tmp$1372;
}

int32_t $UInt16$$is_trailing_surrogate(int32_t self$118) {
  return self$118 >= 56320 && self$118 <= 57343;
}

int32_t $UInt16$$is_leading_surrogate(int32_t self$117) {
  return self$117 >= 55296 && self$117 <= 56319;
}

int32_t $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_char(
  struct $$moonbitlang$core$builtin$StringBuilder* self$114,
  int32_t ch$116
) {
  int32_t len$1367 = self$114->$1;
  int32_t _tmp$1366 = len$1367 + 4;
  moonbit_bytes_t _field$2620;
  moonbit_bytes_t data$1370;
  int32_t len$1371;
  int32_t inc$115;
  int32_t len$1369;
  int32_t _tmp$1368;
  moonbit_incref(self$114);
  $$moonbitlang$core$builtin$StringBuilder$$grow_if_necessary(
    self$114, _tmp$1366
  );
  _field$2620 = self$114->$0;
  data$1370 = _field$2620;
  len$1371 = self$114->$1;
  moonbit_incref(data$1370);
  inc$115 = $FixedArray$$set_utf16le_char(data$1370, len$1371, ch$116);
  len$1369 = self$114->$1;
  _tmp$1368 = len$1369 + inc$115;
  self$114->$1 = _tmp$1368;
  moonbit_decref(self$114);
  return 0;
}

int32_t $$moonbitlang$core$builtin$StringBuilder$$grow_if_necessary(
  struct $$moonbitlang$core$builtin$StringBuilder* self$109,
  int32_t required$110
) {
  moonbit_bytes_t _field$2624 = self$109->$0;
  moonbit_bytes_t data$1365 = _field$2624;
  int32_t _tmp$2623 = Moonbit_array_length(data$1365);
  int32_t current_len$108 = _tmp$2623;
  int32_t enough_space$111;
  int32_t _tmp$1364;
  moonbit_bytes_t new_data$113;
  moonbit_bytes_t _field$2622;
  moonbit_bytes_t data$1362;
  int32_t len$1363;
  moonbit_bytes_t _old$2621;
  if (required$110 <= current_len$108) {
    moonbit_decref(self$109);
    return 0;
  }
  enough_space$111 = current_len$108;
  while (1) {
    int32_t _tmp$1360 = enough_space$111;
    if (_tmp$1360 < required$110) {
      int32_t _tmp$1361 = enough_space$111;
      enough_space$111 = _tmp$1361 * 2;
      continue;
    }
    break;
  }
  _tmp$1364 = enough_space$111;
  new_data$113 = (moonbit_bytes_t)moonbit_make_bytes(_tmp$1364, 0);
  _field$2622 = self$109->$0;
  data$1362 = _field$2622;
  len$1363 = self$109->$1;
  moonbit_incref(data$1362);
  moonbit_incref(new_data$113);
  $FixedArray$$unsafe_blit$1(new_data$113, 0, data$1362, 0, len$1363);
  _old$2621 = self$109->$0;
  moonbit_decref(_old$2621);
  self$109->$0 = new_data$113;
  moonbit_decref(self$109);
  return 0;
}

int32_t $$moonbitlang$core$builtin$Default$$Byte$$default() {
  return 0;
}

int32_t $FixedArray$$set_utf16le_char(
  moonbit_bytes_t self$103,
  int32_t offset$104,
  int32_t value$102
) {
  int32_t _tmp$1359 = value$102;
  uint32_t code$101 = *(uint32_t*)&_tmp$1359;
  if (code$101 < 65536u) {
    uint32_t _p$935 = code$101 & 255u;
    int32_t _tmp$1342 = *(int32_t*)&_p$935;
    int32_t _tmp$1341 = _tmp$1342 & 0xff;
    int32_t _tmp$1343;
    uint32_t _p$938;
    int32_t _tmp$1345;
    int32_t _tmp$1344;
    if (offset$104 < 0 || offset$104 >= Moonbit_array_length(self$103)) {
      moonbit_panic();
    }
    self$103[offset$104] = _tmp$1341;
    _tmp$1343 = offset$104 + 1;
    _p$938 = code$101 >> 8;
    _tmp$1345 = *(int32_t*)&_p$938;
    _tmp$1344 = _tmp$1345 & 0xff;
    if (_tmp$1343 < 0 || _tmp$1343 >= Moonbit_array_length(self$103)) {
      moonbit_panic();
    }
    self$103[_tmp$1343] = _tmp$1344;
    moonbit_decref(self$103);
    return 2;
  } else if (code$101 < 1114112u) {
    uint32_t hi$105 = code$101 - 65536u;
    uint32_t _tmp$1358 = hi$105 >> 10;
    uint32_t lo$106 = _tmp$1358 | 55296u;
    uint32_t _tmp$1357 = hi$105 & 1023u;
    uint32_t hi$107 = _tmp$1357 | 56320u;
    uint32_t _p$941 = lo$106 & 255u;
    int32_t _tmp$1347 = *(int32_t*)&_p$941;
    int32_t _tmp$1346 = _tmp$1347 & 0xff;
    int32_t _tmp$1348;
    uint32_t _p$944;
    int32_t _tmp$1350;
    int32_t _tmp$1349;
    int32_t _tmp$1351;
    uint32_t _p$947;
    int32_t _tmp$1353;
    int32_t _tmp$1352;
    int32_t _tmp$1354;
    uint32_t _p$950;
    int32_t _tmp$1356;
    int32_t _tmp$1355;
    if (offset$104 < 0 || offset$104 >= Moonbit_array_length(self$103)) {
      moonbit_panic();
    }
    self$103[offset$104] = _tmp$1346;
    _tmp$1348 = offset$104 + 1;
    _p$944 = lo$106 >> 8;
    _tmp$1350 = *(int32_t*)&_p$944;
    _tmp$1349 = _tmp$1350 & 0xff;
    if (_tmp$1348 < 0 || _tmp$1348 >= Moonbit_array_length(self$103)) {
      moonbit_panic();
    }
    self$103[_tmp$1348] = _tmp$1349;
    _tmp$1351 = offset$104 + 2;
    _p$947 = hi$107 & 255u;
    _tmp$1353 = *(int32_t*)&_p$947;
    _tmp$1352 = _tmp$1353 & 0xff;
    if (_tmp$1351 < 0 || _tmp$1351 >= Moonbit_array_length(self$103)) {
      moonbit_panic();
    }
    self$103[_tmp$1351] = _tmp$1352;
    _tmp$1354 = offset$104 + 3;
    _p$950 = hi$107 >> 8;
    _tmp$1356 = *(int32_t*)&_p$950;
    _tmp$1355 = _tmp$1356 & 0xff;
    if (_tmp$1354 < 0 || _tmp$1354 >= Moonbit_array_length(self$103)) {
      moonbit_panic();
    }
    self$103[_tmp$1354] = _tmp$1355;
    moonbit_decref(self$103);
    return 4;
  } else {
    moonbit_decref(self$103);
    return $moonbitlang$core$builtin$abort$1(
             (moonbit_string_t)moonbit_string_literal_136.data,
               (moonbit_string_t)moonbit_string_literal_137.data
           );
  }
}

int32_t $UInt$$to_byte(uint32_t self$100) {
  int32_t _tmp$1340 = *(int32_t*)&self$100;
  return _tmp$1340 & 0xff;
}

uint32_t $Char$$to_uint(int32_t self$99) {
  int32_t _tmp$1339 = self$99;
  return *(uint32_t*)&_tmp$1339;
}

moonbit_string_t $$moonbitlang$core$builtin$StringBuilder$$to_string(
  struct $$moonbitlang$core$builtin$StringBuilder* self$98
) {
  moonbit_bytes_t _field$2626 = self$98->$0;
  moonbit_bytes_t data$1338 = _field$2626;
  moonbit_bytes_t _tmp$1335;
  int32_t _field$2625;
  int32_t len$1337;
  int64_t _tmp$1336;
  moonbit_incref(data$1338);
  _tmp$1335 = data$1338;
  _field$2625 = self$98->$1;
  moonbit_decref(self$98);
  len$1337 = _field$2625;
  _tmp$1336 = (int64_t)len$1337;
  return $Bytes$$to_unchecked_string$inner(_tmp$1335, 0, _tmp$1336);
}

moonbit_string_t $Bytes$$to_unchecked_string$inner(
  moonbit_bytes_t self$93,
  int32_t offset$97,
  int64_t length$95
) {
  int32_t len$92 = Moonbit_array_length(self$93);
  int32_t length$94;
  int32_t _if_result$2881;
  if (length$95 == 4294967296ll) {
    length$94 = len$92 - offset$97;
  } else {
    int64_t _Some$96 = length$95;
    length$94 = (int32_t)_Some$96;
  }
  if (offset$97 >= 0) {
    if (length$94 >= 0) {
      int32_t _tmp$1334 = offset$97 + length$94;
      _if_result$2881 = _tmp$1334 <= len$92;
    } else {
      _if_result$2881 = 0;
    }
  } else {
    _if_result$2881 = 0;
  }
  if (_if_result$2881) {
    return $moonbitlang$core$builtin$unsafe_sub_string(
             self$93, offset$97, length$94
           );
  } else {
    moonbit_decref(self$93);
    moonbit_panic();
  }
}

struct $$moonbitlang$core$builtin$StringBuilder* $$moonbitlang$core$builtin$StringBuilder$$new$inner(
  int32_t size_hint$90
) {
  int32_t initial$89;
  moonbit_bytes_t data$91;
  struct $$moonbitlang$core$builtin$StringBuilder* _block$2882;
  if (size_hint$90 < 1) {
    initial$89 = 1;
  } else {
    initial$89 = size_hint$90;
  }
  data$91 = (moonbit_bytes_t)moonbit_make_bytes(initial$89, 0);
  _block$2882
  = (struct $$moonbitlang$core$builtin$StringBuilder*)moonbit_malloc(
      sizeof(struct $$moonbitlang$core$builtin$StringBuilder)
    );
  Moonbit_object_header(_block$2882)->meta
  = Moonbit_make_regular_object_header(
    offsetof(struct $$moonbitlang$core$builtin$StringBuilder, $0) >> 2, 1, 0
  );
  _block$2882->$0 = data$91;
  _block$2882->$1 = 0;
  return _block$2882;
}

int32_t $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$4(
  moonbit_bytes_t dst$84,
  int32_t dst_offset$85,
  moonbit_bytes_t src$86,
  int32_t src_offset$87,
  int32_t len$88
) {
  $FixedArray$$unsafe_blit$5(
    dst$84, dst_offset$85, src$86, src_offset$87, len$88
  );
  return 0;
}

int32_t $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$3(
  struct $$ZSeanYves$Doclint$src$core$Issue** dst$79,
  int32_t dst_offset$80,
  struct $$ZSeanYves$Doclint$src$core$Issue** src$81,
  int32_t src_offset$82,
  int32_t len$83
) {
  $FixedArray$$unsafe_blit$4(
    dst$79, dst_offset$80, src$81, src_offset$82, len$83
  );
  return 0;
}

int32_t $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$2(
  struct $$ZSeanYves$Doclint$src$core$Page** dst$74,
  int32_t dst_offset$75,
  struct $$ZSeanYves$Doclint$src$core$Page** src$76,
  int32_t src_offset$77,
  int32_t len$78
) {
  $FixedArray$$unsafe_blit$3(
    dst$74, dst_offset$75, src$76, src_offset$77, len$78
  );
  return 0;
}

int32_t $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$1(
  int32_t* dst$69,
  int32_t dst_offset$70,
  int32_t* src$71,
  int32_t src_offset$72,
  int32_t len$73
) {
  $FixedArray$$unsafe_blit$2(
    dst$69, dst_offset$70, src$71, src_offset$72, len$73
  );
  return 0;
}

int32_t $$moonbitlang$core$builtin$UninitializedArray$$unsafe_blit$0(
  moonbit_string_t* dst$64,
  int32_t dst_offset$65,
  moonbit_string_t* src$66,
  int32_t src_offset$67,
  int32_t len$68
) {
  $FixedArray$$unsafe_blit$0(
    dst$64, dst_offset$65, src$66, src_offset$67, len$68
  );
  return 0;
}

int32_t $FixedArray$$unsafe_blit$5(
  moonbit_bytes_t dst$55,
  int32_t dst_offset$57,
  moonbit_bytes_t src$56,
  int32_t src_offset$58,
  int32_t len$60
) {
  if (dst$55 == src$56 && dst_offset$57 < src_offset$58) {
    int32_t i$59 = 0;
    while (1) {
      if (i$59 < len$60) {
        int32_t _tmp$1325 = dst_offset$57 + i$59;
        int32_t _tmp$1327 = src_offset$58 + i$59;
        int32_t _tmp$1326;
        int32_t _tmp$1328;
        if (_tmp$1327 < 0 || _tmp$1327 >= Moonbit_array_length(src$56)) {
          moonbit_panic();
        }
        _tmp$1326 = (int32_t)src$56[_tmp$1327];
        if (_tmp$1325 < 0 || _tmp$1325 >= Moonbit_array_length(dst$55)) {
          moonbit_panic();
        }
        dst$55[_tmp$1325] = _tmp$1326;
        _tmp$1328 = i$59 + 1;
        i$59 = _tmp$1328;
        continue;
      } else {
        moonbit_decref(src$56);
        moonbit_decref(dst$55);
      }
      break;
    }
  } else {
    int32_t _tmp$1333 = len$60 - 1;
    int32_t i$62 = _tmp$1333;
    while (1) {
      if (i$62 >= 0) {
        int32_t _tmp$1329 = dst_offset$57 + i$62;
        int32_t _tmp$1331 = src_offset$58 + i$62;
        int32_t _tmp$1330;
        int32_t _tmp$1332;
        if (_tmp$1331 < 0 || _tmp$1331 >= Moonbit_array_length(src$56)) {
          moonbit_panic();
        }
        _tmp$1330 = (int32_t)src$56[_tmp$1331];
        if (_tmp$1329 < 0 || _tmp$1329 >= Moonbit_array_length(dst$55)) {
          moonbit_panic();
        }
        dst$55[_tmp$1329] = _tmp$1330;
        _tmp$1332 = i$62 - 1;
        i$62 = _tmp$1332;
        continue;
      } else {
        moonbit_decref(src$56);
        moonbit_decref(dst$55);
      }
      break;
    }
  }
  return 0;
}

int32_t $FixedArray$$unsafe_blit$4(
  struct $$ZSeanYves$Doclint$src$core$Issue** dst$46,
  int32_t dst_offset$48,
  struct $$ZSeanYves$Doclint$src$core$Issue** src$47,
  int32_t src_offset$49,
  int32_t len$51
) {
  if (dst$46 == src$47 && dst_offset$48 < src_offset$49) {
    int32_t i$50 = 0;
    while (1) {
      if (i$50 < len$51) {
        int32_t _tmp$1316 = dst_offset$48 + i$50;
        int32_t _tmp$1318 = src_offset$49 + i$50;
        struct $$ZSeanYves$Doclint$src$core$Issue* _tmp$2628;
        struct $$ZSeanYves$Doclint$src$core$Issue* _tmp$1317;
        struct $$ZSeanYves$Doclint$src$core$Issue* _old$2627;
        int32_t _tmp$1319;
        if (_tmp$1318 < 0 || _tmp$1318 >= Moonbit_array_length(src$47)) {
          moonbit_panic();
        }
        _tmp$2628
        = (struct $$ZSeanYves$Doclint$src$core$Issue*)src$47[_tmp$1318];
        _tmp$1317 = _tmp$2628;
        if (_tmp$1316 < 0 || _tmp$1316 >= Moonbit_array_length(dst$46)) {
          moonbit_panic();
        }
        _old$2627
        = (struct $$ZSeanYves$Doclint$src$core$Issue*)dst$46[_tmp$1316];
        if (_tmp$1317) {
          moonbit_incref(_tmp$1317);
        }
        if (_old$2627) {
          moonbit_decref(_old$2627);
        }
        dst$46[_tmp$1316] = _tmp$1317;
        _tmp$1319 = i$50 + 1;
        i$50 = _tmp$1319;
        continue;
      } else {
        moonbit_decref(src$47);
        moonbit_decref(dst$46);
      }
      break;
    }
  } else {
    int32_t _tmp$1324 = len$51 - 1;
    int32_t i$53 = _tmp$1324;
    while (1) {
      if (i$53 >= 0) {
        int32_t _tmp$1320 = dst_offset$48 + i$53;
        int32_t _tmp$1322 = src_offset$49 + i$53;
        struct $$ZSeanYves$Doclint$src$core$Issue* _tmp$2630;
        struct $$ZSeanYves$Doclint$src$core$Issue* _tmp$1321;
        struct $$ZSeanYves$Doclint$src$core$Issue* _old$2629;
        int32_t _tmp$1323;
        if (_tmp$1322 < 0 || _tmp$1322 >= Moonbit_array_length(src$47)) {
          moonbit_panic();
        }
        _tmp$2630
        = (struct $$ZSeanYves$Doclint$src$core$Issue*)src$47[_tmp$1322];
        _tmp$1321 = _tmp$2630;
        if (_tmp$1320 < 0 || _tmp$1320 >= Moonbit_array_length(dst$46)) {
          moonbit_panic();
        }
        _old$2629
        = (struct $$ZSeanYves$Doclint$src$core$Issue*)dst$46[_tmp$1320];
        if (_tmp$1321) {
          moonbit_incref(_tmp$1321);
        }
        if (_old$2629) {
          moonbit_decref(_old$2629);
        }
        dst$46[_tmp$1320] = _tmp$1321;
        _tmp$1323 = i$53 - 1;
        i$53 = _tmp$1323;
        continue;
      } else {
        moonbit_decref(src$47);
        moonbit_decref(dst$46);
      }
      break;
    }
  }
  return 0;
}

int32_t $FixedArray$$unsafe_blit$3(
  struct $$ZSeanYves$Doclint$src$core$Page** dst$37,
  int32_t dst_offset$39,
  struct $$ZSeanYves$Doclint$src$core$Page** src$38,
  int32_t src_offset$40,
  int32_t len$42
) {
  if (dst$37 == src$38 && dst_offset$39 < src_offset$40) {
    int32_t i$41 = 0;
    while (1) {
      if (i$41 < len$42) {
        int32_t _tmp$1307 = dst_offset$39 + i$41;
        int32_t _tmp$1309 = src_offset$40 + i$41;
        struct $$ZSeanYves$Doclint$src$core$Page* _tmp$2632;
        struct $$ZSeanYves$Doclint$src$core$Page* _tmp$1308;
        struct $$ZSeanYves$Doclint$src$core$Page* _old$2631;
        int32_t _tmp$1310;
        if (_tmp$1309 < 0 || _tmp$1309 >= Moonbit_array_length(src$38)) {
          moonbit_panic();
        }
        _tmp$2632
        = (struct $$ZSeanYves$Doclint$src$core$Page*)src$38[_tmp$1309];
        _tmp$1308 = _tmp$2632;
        if (_tmp$1307 < 0 || _tmp$1307 >= Moonbit_array_length(dst$37)) {
          moonbit_panic();
        }
        _old$2631
        = (struct $$ZSeanYves$Doclint$src$core$Page*)dst$37[_tmp$1307];
        if (_tmp$1308) {
          moonbit_incref(_tmp$1308);
        }
        if (_old$2631) {
          moonbit_decref(_old$2631);
        }
        dst$37[_tmp$1307] = _tmp$1308;
        _tmp$1310 = i$41 + 1;
        i$41 = _tmp$1310;
        continue;
      } else {
        moonbit_decref(src$38);
        moonbit_decref(dst$37);
      }
      break;
    }
  } else {
    int32_t _tmp$1315 = len$42 - 1;
    int32_t i$44 = _tmp$1315;
    while (1) {
      if (i$44 >= 0) {
        int32_t _tmp$1311 = dst_offset$39 + i$44;
        int32_t _tmp$1313 = src_offset$40 + i$44;
        struct $$ZSeanYves$Doclint$src$core$Page* _tmp$2634;
        struct $$ZSeanYves$Doclint$src$core$Page* _tmp$1312;
        struct $$ZSeanYves$Doclint$src$core$Page* _old$2633;
        int32_t _tmp$1314;
        if (_tmp$1313 < 0 || _tmp$1313 >= Moonbit_array_length(src$38)) {
          moonbit_panic();
        }
        _tmp$2634
        = (struct $$ZSeanYves$Doclint$src$core$Page*)src$38[_tmp$1313];
        _tmp$1312 = _tmp$2634;
        if (_tmp$1311 < 0 || _tmp$1311 >= Moonbit_array_length(dst$37)) {
          moonbit_panic();
        }
        _old$2633
        = (struct $$ZSeanYves$Doclint$src$core$Page*)dst$37[_tmp$1311];
        if (_tmp$1312) {
          moonbit_incref(_tmp$1312);
        }
        if (_old$2633) {
          moonbit_decref(_old$2633);
        }
        dst$37[_tmp$1311] = _tmp$1312;
        _tmp$1314 = i$44 - 1;
        i$44 = _tmp$1314;
        continue;
      } else {
        moonbit_decref(src$38);
        moonbit_decref(dst$37);
      }
      break;
    }
  }
  return 0;
}

int32_t $FixedArray$$unsafe_blit$2(
  int32_t* dst$28,
  int32_t dst_offset$30,
  int32_t* src$29,
  int32_t src_offset$31,
  int32_t len$33
) {
  if (dst$28 == src$29 && dst_offset$30 < src_offset$31) {
    int32_t i$32 = 0;
    while (1) {
      if (i$32 < len$33) {
        int32_t _tmp$1298 = dst_offset$30 + i$32;
        int32_t _tmp$1300 = src_offset$31 + i$32;
        int32_t _tmp$1299;
        int32_t _tmp$1301;
        if (_tmp$1300 < 0 || _tmp$1300 >= Moonbit_array_length(src$29)) {
          moonbit_panic();
        }
        _tmp$1299 = (int32_t)src$29[_tmp$1300];
        if (_tmp$1298 < 0 || _tmp$1298 >= Moonbit_array_length(dst$28)) {
          moonbit_panic();
        }
        dst$28[_tmp$1298] = _tmp$1299;
        _tmp$1301 = i$32 + 1;
        i$32 = _tmp$1301;
        continue;
      } else {
        moonbit_decref(src$29);
        moonbit_decref(dst$28);
      }
      break;
    }
  } else {
    int32_t _tmp$1306 = len$33 - 1;
    int32_t i$35 = _tmp$1306;
    while (1) {
      if (i$35 >= 0) {
        int32_t _tmp$1302 = dst_offset$30 + i$35;
        int32_t _tmp$1304 = src_offset$31 + i$35;
        int32_t _tmp$1303;
        int32_t _tmp$1305;
        if (_tmp$1304 < 0 || _tmp$1304 >= Moonbit_array_length(src$29)) {
          moonbit_panic();
        }
        _tmp$1303 = (int32_t)src$29[_tmp$1304];
        if (_tmp$1302 < 0 || _tmp$1302 >= Moonbit_array_length(dst$28)) {
          moonbit_panic();
        }
        dst$28[_tmp$1302] = _tmp$1303;
        _tmp$1305 = i$35 - 1;
        i$35 = _tmp$1305;
        continue;
      } else {
        moonbit_decref(src$29);
        moonbit_decref(dst$28);
      }
      break;
    }
  }
  return 0;
}

int32_t $FixedArray$$unsafe_blit$1(
  moonbit_bytes_t dst$19,
  int32_t dst_offset$21,
  moonbit_bytes_t src$20,
  int32_t src_offset$22,
  int32_t len$24
) {
  if (dst$19 == src$20 && dst_offset$21 < src_offset$22) {
    int32_t i$23 = 0;
    while (1) {
      if (i$23 < len$24) {
        int32_t _tmp$1289 = dst_offset$21 + i$23;
        int32_t _tmp$1291 = src_offset$22 + i$23;
        int32_t _tmp$1290;
        int32_t _tmp$1292;
        if (_tmp$1291 < 0 || _tmp$1291 >= Moonbit_array_length(src$20)) {
          moonbit_panic();
        }
        _tmp$1290 = (int32_t)src$20[_tmp$1291];
        if (_tmp$1289 < 0 || _tmp$1289 >= Moonbit_array_length(dst$19)) {
          moonbit_panic();
        }
        dst$19[_tmp$1289] = _tmp$1290;
        _tmp$1292 = i$23 + 1;
        i$23 = _tmp$1292;
        continue;
      } else {
        moonbit_decref(src$20);
        moonbit_decref(dst$19);
      }
      break;
    }
  } else {
    int32_t _tmp$1297 = len$24 - 1;
    int32_t i$26 = _tmp$1297;
    while (1) {
      if (i$26 >= 0) {
        int32_t _tmp$1293 = dst_offset$21 + i$26;
        int32_t _tmp$1295 = src_offset$22 + i$26;
        int32_t _tmp$1294;
        int32_t _tmp$1296;
        if (_tmp$1295 < 0 || _tmp$1295 >= Moonbit_array_length(src$20)) {
          moonbit_panic();
        }
        _tmp$1294 = (int32_t)src$20[_tmp$1295];
        if (_tmp$1293 < 0 || _tmp$1293 >= Moonbit_array_length(dst$19)) {
          moonbit_panic();
        }
        dst$19[_tmp$1293] = _tmp$1294;
        _tmp$1296 = i$26 - 1;
        i$26 = _tmp$1296;
        continue;
      } else {
        moonbit_decref(src$20);
        moonbit_decref(dst$19);
      }
      break;
    }
  }
  return 0;
}

int32_t $FixedArray$$unsafe_blit$0(
  moonbit_string_t* dst$10,
  int32_t dst_offset$12,
  moonbit_string_t* src$11,
  int32_t src_offset$13,
  int32_t len$15
) {
  if (dst$10 == src$11 && dst_offset$12 < src_offset$13) {
    int32_t i$14 = 0;
    while (1) {
      if (i$14 < len$15) {
        int32_t _tmp$1280 = dst_offset$12 + i$14;
        int32_t _tmp$1282 = src_offset$13 + i$14;
        moonbit_string_t _tmp$2636;
        moonbit_string_t _tmp$1281;
        moonbit_string_t _old$2635;
        int32_t _tmp$1283;
        if (_tmp$1282 < 0 || _tmp$1282 >= Moonbit_array_length(src$11)) {
          moonbit_panic();
        }
        _tmp$2636 = (moonbit_string_t)src$11[_tmp$1282];
        _tmp$1281 = _tmp$2636;
        if (_tmp$1280 < 0 || _tmp$1280 >= Moonbit_array_length(dst$10)) {
          moonbit_panic();
        }
        _old$2635 = (moonbit_string_t)dst$10[_tmp$1280];
        moonbit_incref(_tmp$1281);
        moonbit_decref(_old$2635);
        dst$10[_tmp$1280] = _tmp$1281;
        _tmp$1283 = i$14 + 1;
        i$14 = _tmp$1283;
        continue;
      } else {
        moonbit_decref(src$11);
        moonbit_decref(dst$10);
      }
      break;
    }
  } else {
    int32_t _tmp$1288 = len$15 - 1;
    int32_t i$17 = _tmp$1288;
    while (1) {
      if (i$17 >= 0) {
        int32_t _tmp$1284 = dst_offset$12 + i$17;
        int32_t _tmp$1286 = src_offset$13 + i$17;
        moonbit_string_t _tmp$2638;
        moonbit_string_t _tmp$1285;
        moonbit_string_t _old$2637;
        int32_t _tmp$1287;
        if (_tmp$1286 < 0 || _tmp$1286 >= Moonbit_array_length(src$11)) {
          moonbit_panic();
        }
        _tmp$2638 = (moonbit_string_t)src$11[_tmp$1286];
        _tmp$1285 = _tmp$2638;
        if (_tmp$1284 < 0 || _tmp$1284 >= Moonbit_array_length(dst$10)) {
          moonbit_panic();
        }
        _old$2637 = (moonbit_string_t)dst$10[_tmp$1284];
        moonbit_incref(_tmp$1285);
        moonbit_decref(_old$2637);
        dst$10[_tmp$1284] = _tmp$1285;
        _tmp$1287 = i$17 - 1;
        i$17 = _tmp$1287;
        continue;
      } else {
        moonbit_decref(src$11);
        moonbit_decref(dst$10);
      }
      break;
    }
  }
  return 0;
}

int32_t $moonbitlang$core$builtin$abort$2(
  moonbit_string_t string$8,
  moonbit_string_t loc$9
) {
  moonbit_string_t _tmp$1278 =
    moonbit_add_string(
      string$8, (moonbit_string_t)moonbit_string_literal_138.data
    );
  moonbit_string_t _tmp$1279 =
    $$moonbitlang$core$builtin$Show$$$default_impl$$to_string$0(loc$9);
  moonbit_string_t _tmp$1277 = moonbit_add_string(_tmp$1278, _tmp$1279);
  moonbit_string_t _tmp$1276 =
    moonbit_add_string(
      _tmp$1277, (moonbit_string_t)moonbit_string_literal_10.data
    );
  return $moonbitlang$core$abort$abort$2(_tmp$1276);
}

int32_t $moonbitlang$core$builtin$abort$1(
  moonbit_string_t string$6,
  moonbit_string_t loc$7
) {
  moonbit_string_t _tmp$1274 =
    moonbit_add_string(
      string$6, (moonbit_string_t)moonbit_string_literal_138.data
    );
  moonbit_string_t _tmp$1275 =
    $$moonbitlang$core$builtin$Show$$$default_impl$$to_string$0(loc$7);
  moonbit_string_t _tmp$1273 = moonbit_add_string(_tmp$1274, _tmp$1275);
  moonbit_string_t _tmp$1272 =
    moonbit_add_string(
      _tmp$1273, (moonbit_string_t)moonbit_string_literal_10.data
    );
  return $moonbitlang$core$abort$abort$1(_tmp$1272);
}

int32_t $moonbitlang$core$builtin$abort$0(
  moonbit_string_t string$4,
  moonbit_string_t loc$5
) {
  moonbit_string_t _tmp$1270 =
    moonbit_add_string(
      string$4, (moonbit_string_t)moonbit_string_literal_138.data
    );
  moonbit_string_t _tmp$1271 =
    $$moonbitlang$core$builtin$Show$$$default_impl$$to_string$0(loc$5);
  moonbit_string_t _tmp$1269 = moonbit_add_string(_tmp$1270, _tmp$1271);
  moonbit_string_t _tmp$1268 =
    moonbit_add_string(
      _tmp$1269, (moonbit_string_t)moonbit_string_literal_10.data
    );
  $moonbitlang$core$abort$abort$0(_tmp$1268);
  return 0;
}

int32_t $moonbitlang$core$abort$abort$2(moonbit_string_t msg$3) {
  moonbit_println(msg$3);
  moonbit_decref(msg$3);
  moonbit_panic();
}

int32_t $moonbitlang$core$abort$abort$1(moonbit_string_t msg$2) {
  moonbit_println(msg$2);
  moonbit_decref(msg$2);
  moonbit_panic();
}

int32_t $moonbitlang$core$abort$abort$0(moonbit_string_t msg$1) {
  moonbit_println(msg$1);
  moonbit_decref(msg$1);
  moonbit_panic();
  return 0;
}

int32_t $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_char$dyncall_as_$moonbitlang$core$builtin$Logger(
  void* _obj_ptr$1227,
  int32_t _param$1226
) {
  struct $$moonbitlang$core$builtin$StringBuilder* _self$1225 =
    (struct $$moonbitlang$core$builtin$StringBuilder*)_obj_ptr$1227;
  $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_char(
    _self$1225, _param$1226
  );
  return 0;
}

int32_t $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_view$dyncall_as_$moonbitlang$core$builtin$Logger(
  void* _obj_ptr$1224,
  struct $StringView _param$1223
) {
  struct $$moonbitlang$core$builtin$StringBuilder* _self$1222 =
    (struct $$moonbitlang$core$builtin$StringBuilder*)_obj_ptr$1224;
  $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_view(
    _self$1222, _param$1223
  );
  return 0;
}

int32_t $$moonbitlang$core$builtin$Logger$$$default_impl$$write_substring$dyncall_as_$moonbitlang$core$builtin$Logger$0(
  void* _obj_ptr$1221,
  moonbit_string_t _param$1218,
  int32_t _param$1219,
  int32_t _param$1220
) {
  struct $$moonbitlang$core$builtin$StringBuilder* _self$1217 =
    (struct $$moonbitlang$core$builtin$StringBuilder*)_obj_ptr$1221;
  $$moonbitlang$core$builtin$Logger$$$default_impl$$write_substring$0(
    _self$1217, _param$1218, _param$1219, _param$1220
  );
  return 0;
}

int32_t $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_string$dyncall_as_$moonbitlang$core$builtin$Logger(
  void* _obj_ptr$1216,
  moonbit_string_t _param$1215
) {
  struct $$moonbitlang$core$builtin$StringBuilder* _self$1214 =
    (struct $$moonbitlang$core$builtin$StringBuilder*)_obj_ptr$1216;
  $$moonbitlang$core$builtin$Logger$$$moonbitlang$core$builtin$StringBuilder$$write_string(
    _self$1214, _param$1215
  );
  return 0;
}

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$$check$dyncall_as_$ZSeanYves$Doclint$src$core$Rule(
  void* _obj_ptr$1213,
  int32_t _param$1211,
  struct $$ZSeanYves$Doclint$src$core$Document* _param$1212
) {
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* _self$1210 =
    (struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule*)_obj_ptr$1213;
  return $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$$check(
           _self$1210, _param$1211, _param$1212
         );
}

moonbit_string_t $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$$id$dyncall_as_$ZSeanYves$Doclint$src$core$Rule(
  void* _obj_ptr$1209
) {
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule* _self$1208 =
    (struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule*)_obj_ptr$1209;
  return $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$RequireKeywordRule$$id(
           _self$1208
         );
}

struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$$check$dyncall_as_$ZSeanYves$Doclint$src$core$Rule(
  void* _obj_ptr$1207,
  int32_t _param$1205,
  struct $$ZSeanYves$Doclint$src$core$Document* _param$1206
) {
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* _self$1204 =
    (struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule*)_obj_ptr$1207;
  return $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$$check(
           _self$1204, _param$1205, _param$1206
         );
}

moonbit_string_t $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$$id$dyncall_as_$ZSeanYves$Doclint$src$core$Rule(
  void* _obj_ptr$1203
) {
  struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule* _self$1202 =
    (struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule*)_obj_ptr$1203;
  return $$ZSeanYves$Doclint$src$core$Rule$$$ZSeanYves$Doclint$src$doclint_rules_thesis$PageLimitRule$$id(
           _self$1202
         );
}

void moonbit_init() {
  $moonbitlang$core$builtin$brute_force_find$constr$369 = (int64_t)0;
  $moonbitlang$core$builtin$boyer_moore_horspool_find$constr$355 = (int64_t)0;
}

int main(int argc, char** argv) {
  int32_t _return_value$895;
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* _tmp$1267;
  struct $$moonbitlang$core$builtin$Array$3c$String$3e$* args$896;
  moonbit_string_t rules_name$897;
  moonbit_string_t in_path$898;
  moonbit_string_t out_dir$899;
  int32_t emit_html$900;
  int32_t i$901;
  moonbit_string_t _tmp$1255;
  moonbit_string_t _p$1166;
  moonbit_string_t _p$1167;
  moonbit_string_t _tmp$1266;
  moonbit_string_t out_json_path$908;
  moonbit_string_t _p$1170;
  moonbit_string_t _p$1171;
  moonbit_string_t _tmp$1265;
  moonbit_string_t out_html_path$909;
  moonbit_string_t _tmp$1264;
  void* doc_json_res$910;
  moonbit_string_t doc_json$911;
  void* doc_res$915;
  struct $$ZSeanYves$Doclint$src$core$Document* doc$916;
  int32_t ctx$921;
  moonbit_string_t _tmp$1257;
  int32_t _tmp$2639;
  struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* issues$922;
  int32_t _tmp$1256;
  moonbit_runtime_init(argc, argv);
  moonbit_init();
  _tmp$1267 = $moonbitlang$x$sys$get_cli_args();
  args$896 = $ZSeanYves$Doclint$src$drop1(_tmp$1267);
  rules_name$897 = (moonbit_string_t)moonbit_string_literal_139.data;
  in_path$898 = (moonbit_string_t)moonbit_string_literal_140.data;
  out_dir$899 = (moonbit_string_t)moonbit_string_literal_141.data;
  emit_html$900 = 1;
  i$901 = 0;
  while (1) {
    int32_t _tmp$1228 = i$901;
    int32_t len$1229 = args$896->$1;
    if (_tmp$1228 < len$1229) {
      int32_t _bind$902 = i$901;
      int32_t _if_result$2897;
      moonbit_string_t* _field$2651;
      moonbit_string_t* buf$1254;
      moonbit_string_t _tmp$2650;
      moonbit_string_t a$903;
      if (_bind$902 < 0) {
        _if_result$2897 = 1;
      } else {
        int32_t len$1253 = args$896->$1;
        _if_result$2897 = _bind$902 >= len$1253;
      }
      if (_if_result$2897) {
        moonbit_panic();
      }
      _field$2651 = args$896->$0;
      buf$1254 = _field$2651;
      _tmp$2650 = (moonbit_string_t)buf$1254[_bind$902];
      a$903 = _tmp$2650;
      if (
        moonbit_val_array_equal(
          a$903, (moonbit_string_t)moonbit_string_literal_142.data
        )
        || moonbit_val_array_equal(
          a$903, (moonbit_string_t)moonbit_string_literal_143.data
        )
      ) {
        $ZSeanYves$Doclint$src$print_help();
        $moonbitlang$x$sys$exit(0);
      } else if (
               moonbit_val_array_equal(
                 a$903, (moonbit_string_t)moonbit_string_literal_144.data
               )
             ) {
        int32_t _tmp$1232;
        int32_t _tmp$1230;
        int32_t len$1231;
        int32_t _tmp$1235;
        int32_t _bind$904;
        int32_t _if_result$2898;
        moonbit_string_t* _field$2645;
        moonbit_string_t* buf$1234;
        moonbit_string_t _tmp$2644;
        int32_t _tmp$1236;
        moonbit_incref(a$903);
        moonbit_decref(rules_name$897);
        moonbit_decref(a$903);
        _tmp$1232 = i$901;
        _tmp$1230 = _tmp$1232 + 1;
        len$1231 = args$896->$1;
        if (_tmp$1230 >= len$1231) {
          $moonbitlang$core$builtin$println$0(
            (moonbit_string_t)moonbit_string_literal_145.data
          );
          $ZSeanYves$Doclint$src$print_help();
          $moonbitlang$x$sys$exit(1);
        }
        _tmp$1235 = i$901;
        _bind$904 = _tmp$1235 + 1;
        if (_bind$904 < 0) {
          _if_result$2898 = 1;
        } else {
          int32_t len$1233 = args$896->$1;
          _if_result$2898 = _bind$904 >= len$1233;
        }
        if (_if_result$2898) {
          moonbit_panic();
        }
        _field$2645 = args$896->$0;
        buf$1234 = _field$2645;
        _tmp$2644 = (moonbit_string_t)buf$1234[_bind$904];
        moonbit_incref(_tmp$2644);
        rules_name$897 = _tmp$2644;
        _tmp$1236 = i$901;
        i$901 = _tmp$1236 + 2;
        continue;
      } else if (
               moonbit_val_array_equal(
                 a$903, (moonbit_string_t)moonbit_string_literal_146.data
               )
             ) {
        int32_t _tmp$1239;
        int32_t _tmp$1237;
        int32_t len$1238;
        int32_t _tmp$1242;
        int32_t _bind$906;
        int32_t _if_result$2899;
        moonbit_string_t* _field$2647;
        moonbit_string_t* buf$1241;
        moonbit_string_t _tmp$2646;
        int32_t _tmp$1243;
        moonbit_incref(a$903);
        moonbit_decref(in_path$898);
        moonbit_decref(a$903);
        _tmp$1239 = i$901;
        _tmp$1237 = _tmp$1239 + 1;
        len$1238 = args$896->$1;
        if (_tmp$1237 >= len$1238) {
          $moonbitlang$core$builtin$println$0(
            (moonbit_string_t)moonbit_string_literal_147.data
          );
          $ZSeanYves$Doclint$src$print_help();
          $moonbitlang$x$sys$exit(1);
        }
        _tmp$1242 = i$901;
        _bind$906 = _tmp$1242 + 1;
        if (_bind$906 < 0) {
          _if_result$2899 = 1;
        } else {
          int32_t len$1240 = args$896->$1;
          _if_result$2899 = _bind$906 >= len$1240;
        }
        if (_if_result$2899) {
          moonbit_panic();
        }
        _field$2647 = args$896->$0;
        buf$1241 = _field$2647;
        _tmp$2646 = (moonbit_string_t)buf$1241[_bind$906];
        moonbit_incref(_tmp$2646);
        in_path$898 = _tmp$2646;
        _tmp$1243 = i$901;
        i$901 = _tmp$1243 + 2;
        continue;
      } else if (
               moonbit_val_array_equal(
                 a$903, (moonbit_string_t)moonbit_string_literal_148.data
               )
             ) {
        int32_t _tmp$1246;
        int32_t _tmp$1244;
        int32_t len$1245;
        int32_t _tmp$1249;
        int32_t _bind$907;
        int32_t _if_result$2900;
        moonbit_string_t* _field$2649;
        moonbit_string_t* buf$1248;
        moonbit_string_t _tmp$2648;
        int32_t _tmp$1250;
        moonbit_incref(a$903);
        moonbit_decref(out_dir$899);
        moonbit_decref(a$903);
        _tmp$1246 = i$901;
        _tmp$1244 = _tmp$1246 + 1;
        len$1245 = args$896->$1;
        if (_tmp$1244 >= len$1245) {
          $moonbitlang$core$builtin$println$0(
            (moonbit_string_t)moonbit_string_literal_149.data
          );
          $ZSeanYves$Doclint$src$print_help();
          $moonbitlang$x$sys$exit(1);
        }
        _tmp$1249 = i$901;
        _bind$907 = _tmp$1249 + 1;
        if (_bind$907 < 0) {
          _if_result$2900 = 1;
        } else {
          int32_t len$1247 = args$896->$1;
          _if_result$2900 = _bind$907 >= len$1247;
        }
        if (_if_result$2900) {
          moonbit_panic();
        }
        _field$2649 = args$896->$0;
        buf$1248 = _field$2649;
        _tmp$2648 = (moonbit_string_t)buf$1248[_bind$907];
        moonbit_incref(_tmp$2648);
        out_dir$899 = _tmp$2648;
        _tmp$1250 = i$901;
        i$901 = _tmp$1250 + 2;
        continue;
      } else if (
               moonbit_val_array_equal(
                 a$903, (moonbit_string_t)moonbit_string_literal_150.data
               )
             ) {
        int32_t _tmp$1251;
        emit_html$900 = 0;
        _tmp$1251 = i$901;
        i$901 = _tmp$1251 + 1;
        continue;
      } else {
        moonbit_string_t _tmp$1252;
        moonbit_incref(a$903);
        _tmp$1252
        = moonbit_add_string(
          (moonbit_string_t)moonbit_string_literal_151.data, a$903
        );
        $moonbitlang$core$builtin$println$0(_tmp$1252);
        $ZSeanYves$Doclint$src$print_help();
        $moonbitlang$x$sys$exit(1);
      }
      continue;
    } else {
      moonbit_decref(args$896);
    }
    break;
  }
  _tmp$1255 = out_dir$899;
  moonbit_incref(_tmp$1255);
  $ZSeanYves$Doclint$src$ensure_dir(_tmp$1255);
  _p$1166 = out_dir$899;
  _p$1167 = (moonbit_string_t)moonbit_string_literal_152.data;
  moonbit_incref(_p$1166);
  _tmp$1266
  = moonbit_add_string(
    _p$1166, (moonbit_string_t)moonbit_string_literal_18.data
  );
  out_json_path$908 = moonbit_add_string(_tmp$1266, _p$1167);
  _p$1170 = out_dir$899;
  _p$1171 = (moonbit_string_t)moonbit_string_literal_153.data;
  _tmp$1265
  = moonbit_add_string(
    _p$1170, (moonbit_string_t)moonbit_string_literal_18.data
  );
  out_html_path$909 = moonbit_add_string(_tmp$1265, _p$1171);
  _tmp$1264 = in_path$898;
  doc_json_res$910 = $ZSeanYves$Doclint$src$read_utf8(_tmp$1264);
  switch (Moonbit_object_tag(doc_json_res$910)) {
    case 0: {
      struct $Result$3c$String$2a$String$3e$$Err* _Err$912 =
        (struct $Result$3c$String$2a$String$3e$$Err*)doc_json_res$910;
      moonbit_string_t _field$2642 = _Err$912->$0;
      int32_t _cnt$2723 = Moonbit_object_header(_Err$912)->rc;
      moonbit_string_t _msg$913;
      if (_cnt$2723 > 1) {
        int32_t _new_cnt$2724 = _cnt$2723 - 1;
        Moonbit_object_header(_Err$912)->rc = _new_cnt$2724;
        moonbit_incref(_field$2642);
      } else if (_cnt$2723 == 1) {
        moonbit_free(_Err$912);
      }
      _msg$913 = _field$2642;
      $moonbitlang$core$builtin$println$0(_msg$913);
      $moonbitlang$x$sys$exit(1);
      doc_json$911 = (moonbit_string_t)moonbit_string_literal_26.data;
      break;
    }
    default: {
      struct $Result$3c$String$2a$String$3e$$Ok* _Ok$914 =
        (struct $Result$3c$String$2a$String$3e$$Ok*)doc_json_res$910;
      moonbit_string_t _field$2643 = _Ok$914->$0;
      int32_t _cnt$2725 = Moonbit_object_header(_Ok$914)->rc;
      if (_cnt$2725 > 1) {
        int32_t _new_cnt$2726 = _cnt$2725 - 1;
        Moonbit_object_header(_Ok$914)->rc = _new_cnt$2726;
        moonbit_incref(_field$2643);
      } else if (_cnt$2725 == 1) {
        moonbit_free(_Ok$914);
      }
      doc_json$911 = _field$2643;
      break;
    }
  }
  doc_res$915 = $ZSeanYves$Doclint$src$core$parse_doc_json(doc_json$911);
  switch (Moonbit_object_tag(doc_res$915)) {
    case 0: {
      struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Err* _Err$917;
      moonbit_string_t _field$2640;
      int32_t _cnt$2727;
      moonbit_string_t _e$918;
      moonbit_string_t _tmp$1262;
      struct $$ZSeanYves$Doclint$src$core$Location* _tmp$1263;
      struct $$ZSeanYves$Doclint$src$core$Issue* _tmp$1261;
      struct $$ZSeanYves$Doclint$src$core$Issue** _tmp$1260;
      struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$* issues$919;
      int32_t _tmp$1258;
      int32_t _tmp$1259;
      moonbit_decref(rules_name$897);
      _Err$917
      = (struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Err*)doc_res$915;
      _field$2640 = _Err$917->$0;
      _cnt$2727 = Moonbit_object_header(_Err$917)->rc;
      if (_cnt$2727 > 1) {
        int32_t _new_cnt$2728 = _cnt$2727 - 1;
        Moonbit_object_header(_Err$917)->rc = _new_cnt$2728;
        moonbit_incref(_field$2640);
      } else if (_cnt$2727 == 1) {
        moonbit_free(_Err$917);
      }
      _e$918 = _field$2640;
      _tmp$1262
      = moonbit_add_string(
        (moonbit_string_t)moonbit_string_literal_154.data, _e$918
      );
      _tmp$1263 = 0;
      _tmp$1261
      = $ZSeanYves$Doclint$src$core$mk_issue(
        (moonbit_string_t)moonbit_string_literal_155.data,
          2,
          _tmp$1262,
          _tmp$1263
      );
      _tmp$1260
      = (struct $$ZSeanYves$Doclint$src$core$Issue**)moonbit_make_ref_array_raw(
          1
        );
      _tmp$1260[0] = _tmp$1261;
      issues$919
      = (struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$*)moonbit_malloc(
          sizeof(
            struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$
          )
        );
      Moonbit_object_header(issues$919)->meta
      = Moonbit_make_regular_object_header(
        offsetof(
          struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Issue$3e$,
            $0
        )
        >> 2,
          1,
          0
      );
      issues$919->$0 = _tmp$1260;
      issues$919->$1 = 1;
      _tmp$1258 = emit_html$900;
      $ZSeanYves$Doclint$src$write_out(
        out_json_path$908, out_html_path$909, issues$919, _tmp$1258
      );
      $moonbitlang$x$sys$exit(1);
      _tmp$1259 = 0;
      _return_value$895 = _tmp$1259;
      goto $join$894;
      break;
    }
    default: {
      struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Ok* _Ok$920 =
        (struct $Result$3c$$ZSeanYves$Doclint$src$core$Document$2a$String$3e$$Ok*)doc_res$915;
      struct $$ZSeanYves$Doclint$src$core$Document* _field$2641 = _Ok$920->$0;
      int32_t _cnt$2729 = Moonbit_object_header(_Ok$920)->rc;
      if (_cnt$2729 > 1) {
        int32_t _new_cnt$2730 = _cnt$2729 - 1;
        Moonbit_object_header(_Ok$920)->rc = _new_cnt$2730;
        moonbit_incref(_field$2641);
      } else if (_cnt$2729 == 1) {
        moonbit_free(_Ok$920);
      }
      doc$916 = _field$2641;
      break;
    }
  }
  ctx$921 = 0;
  _tmp$1257 = rules_name$897;
  _tmp$2639
  = moonbit_val_array_equal(
    _tmp$1257, (moonbit_string_t)moonbit_string_literal_156.data
  );
  moonbit_decref(_tmp$1257);
  if (_tmp$2639) {
    struct $$ZSeanYves$Doclint$src$doclint_rules_contract$ContractRuleset* rs$923 =
      $ZSeanYves$Doclint$src$doclint_rules_contract$build_ruleset$record$817;
    struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$* rules$924;
    moonbit_incref(rs$923);
    rules$924
    = $ZSeanYves$Doclint$src$doclint_rules_contract$as_rules_ref(
      rs$923
    );
    issues$922
    = $ZSeanYves$Doclint$src$core$check_doc(
      ctx$921, doc$916, rules$924
    );
  } else {
    struct $$ZSeanYves$Doclint$src$doclint_rules_thesis$ThesisRuleset* rs$925 =
      $ZSeanYves$Doclint$src$doclint_rules_thesis$build_ruleset$record$813;
    struct $$moonbitlang$core$builtin$Array$3c$$ZSeanYves$Doclint$src$core$Rule$3e$* rules$926;
    moonbit_incref(rs$925);
    rules$926
    = $ZSeanYves$Doclint$src$doclint_rules_thesis$as_rules_ref(
      rs$925
    );
    issues$922
    = $ZSeanYves$Doclint$src$core$check_doc(
      ctx$921, doc$916, rules$926
    );
  }
  _tmp$1256 = emit_html$900;
  $ZSeanYves$Doclint$src$write_out(
    out_json_path$908, out_html_path$909, issues$922, _tmp$1256
  );
  $moonbitlang$x$sys$exit(0);
  goto $joinlet$2895;
  $join$894:;
  $joinlet$2895:;
  return 0;
}