//
//  linalgb.h
//  ceelo
//
//  Created by George Watson on 16/12/2022.
//

#ifndef linalgb_h
#define linalgb_h
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdbool.h>

#define SIZES \
    X(2)      \
    X(3)      \
    X(4)

#define X(N)                                                          \
    typedef int pos##N __attribute__((ext_vector_type(N))); \
    typedef float vec##N __attribute__((ext_vector_type(N)));         \
    vec##N Vec##N##Zero(void);                                        \
    float Vec##N##Sum(vec##N v);                                      \
    float Vec##N##LengthSqr(vec##N v);                                \
    float Vec##N##Length(vec##N v);                                   \
    float Vec##N##Dot(vec##N v1, vec##N v2);                          \
    vec##N Vec##N##Normalize(vec##N v);                               \
    float Vec##N##DistSqr(vec##N v1, vec##N v2);                      \
    float Vec##N##Dist(vec##N v1, vec##N v2);                         \
    bool Vec##N##Cmp(vec##N v1, vec##N v2);
SIZES
#undef X
typedef vec4 quat;
#define Quat(...) Vec4(__VA_ARGS__)
#define Vec2(X, Y) (vec2){(X), (Y)}
#define Vec3(X, Y, Z) (vec3){(X), (Y), (Z)}
#define Vec4(X, Y, Z, W) (vec4){(X), (Y), (Z), (W)}

vec3 Vec3Cross(vec3 v1, vec3 v2);

#define X(N)                                                 \
    typedef float mat##N __attribute__((matrix_type(N, N))); \
    mat##N Mat##N(float v);                                  \
    mat##N Mat##N##Identity(void);
SIZES
#undef X

mat4 Perspective(float fov, float aspectRatio, float near, float far);
mat4 LookAt(vec3 eye, vec3 target, vec3 up);

#endif /* linalgb_h */
