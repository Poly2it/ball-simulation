#ifndef PRIMITIVES_H
#define PRIMITIVES_H


#include <uchar.h>
#include <stddef.h>


typedef __UINT8_TYPE__ u8;
typedef __UINT16_TYPE__ u16;
typedef __UINT32_TYPE__ u32;
typedef __UINT64_TYPE__ u64;
typedef __INT8_TYPE__ s8;
typedef __INT16_TYPE__ s16;
typedef __INT32_TYPE__ s32;
typedef __INT64_TYPE__ s64;

typedef float f32;
typedef double f64;

typedef char c8;
typedef char16_t c16;
typedef char32_t c32;

typedef ptrdiff_t saddr;
typedef size_t uaddr;


#define countof(a)    (uaddr) (sizeof(a) / sizeof(*(a)))
#define lengthof(s)   (countof(s) - 1)


#endif

