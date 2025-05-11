#ifndef u_COMMON_TYPES_H
#define u_COMMON_TYPES_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

//#############################
//      BASE TYPES
//#############################

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef float       f32;
typedef double      f64;

typedef size_t      usize;
typedef ptrdiff_t   isize;

typedef u8*         byte;
typedef void*       memptr;

//#############################
//      Memory Size
//#############################

#define KiB(x) ((x) * 1024ULL)
#define MiB(x) ((x) * 1024ULL * 1024ULL)
#define GiB(x) ((x) * 1024ULL * 1024ULL * 1024ULL)

#define MIN(a, b) (a < b) ? a : b;
#define MAX(b, a) (a > b) ? a : b;

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifndef ASSERT
#define ASSERT(expr) assert(expr)
#endif

//#############################
//      LOGGING
//#############################

#define LOG_INFO(fmt, ...) fprintf(stderr, "[INFO]:  (%s:%d) : " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) fprintf(stderr, "[WARN]:  (%s:%d) : " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) fprintf(stderr, "[ERROR]:  (%s:%d) :  " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#endif
