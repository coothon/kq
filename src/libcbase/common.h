#ifndef LIBCBASE_COMMON_H_
#define LIBCBASE_COMMON_H_

#include <stdint.h>

#define CB_UNUSED(var) (void)(var)

typedef int8_t  s8;
typedef uint8_t u8;

typedef int16_t  s16;
typedef uint16_t u16;

typedef int32_t  s32;
typedef uint32_t u32;

typedef int64_t  s64;
typedef uint64_t u64;

typedef unsigned char      uchar;
typedef unsigned short     ushort;
typedef unsigned int       uint;
typedef unsigned long      ulong;
typedef unsigned long long ullong;

typedef long long llong;

// Constants for byte sizes.
#define KB ((size_t)1000)
#define MB ((size_t)1000 * KB)
#define GB ((size_t)1000 * MB)
#define TB ((size_t)1000 * GB)

// Prefer *iB, as they're powers of two.
#define KiB ((size_t)1024)
#define MiB ((size_t)1024 * KiB)
#define GiB ((size_t)1024 * MiB)
#define TiB ((size_t)1024 * GiB)

// Simplifies operator precedence over using the above.
#define KB_v(v) ((size_t)(v) * KB)
#define MB_v(v) ((size_t)(v) * MB)
#define GB_v(v) ((size_t)(v) * GB)
#define TB_v(v) ((size_t)(v) * TB)

#define KiB_v(v) ((size_t)(v) * KiB)
#define MiB_v(v) ((size_t)(v) * MiB)
#define GiB_v(v) ((size_t)(v) * GiB)
#define TiB_v(v) ((size_t)(v) * TiB)

#endif /* LIBCBASE_COMMON_H_ */
