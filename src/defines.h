#ifndef DEFINES_H
#define DEFINES_H

#include <stdint.h>
#include <stddef.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t

#define b8 int8_t
#define b32 int32_t

#define f32 float
#define f64 double

#define TRUE 1
#define FALSE 0

#define VERY_SMALL 0.0001

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define CLAMP(val, min, max) (MAX(min, MIN(val, max)))

#define MAPVAL(val, min_original, max_original, min_new, max_new)              \
    (((val) - (min_original)) / ((max_original) - (min_original))) *           \
            ((max_new) - min_new) +                                            \
        (min_new)

f32 lerpf(f32 a, f32 b, f32 t) { return a * (1 - t) + b * t; }

#endif // DEFINES_H
