#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

#include "defines.h"

typedef struct Vector2 {
    f32 x;
    f32 y;
} Vector2;

f32 vector2_magnitude(Vector2 v) { return sqrt(v.x * v.x + v.y * v.y); }
f32 vector2_mag_sqared(Vector2 v) { return v.x * v.x + v.y * v.y; }

// zero vector stays zero
Vector2 vector2_normalize(Vector2 v) {
    if (vector2_mag_sqared(v) < VERY_SMALL) {
        return v;
    }
    Vector2 result = {
        v.x / vector2_magnitude(v),
        v.y / vector2_magnitude(v),
    };
    return result;
}

Vector2 vector2_add(Vector2 a, Vector2 b) {
    Vector2 result = {a.x + b.x, a.y + b.y};
    return result;
}

Vector2 vector2_sub(Vector2 a, Vector2 b) {
    Vector2 result = {a.x - b.x, a.y - b.y};
    return result;
}

Vector2 vector2_scale(Vector2 v, f32 s) {
    Vector2 result = {v.x * s, v.y * s};
    return result;
}

#endif // VECTOR_H
