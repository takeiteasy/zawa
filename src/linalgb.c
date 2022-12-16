//
//  linalgb.c
//  ceelo
//
//  Created by George Watson on 16/12/2022.
//

#include "linalgb.h"

#define X(N)                                               \
    vec##N Vec##N##Zero(void)                              \
    {                                                      \
        vec##N v;                                          \
        for (int i = 0; i < N; i++)                        \
            v[i] = 0;                                      \
        return v;                                          \
    }                                                      \
    vec##N Vec##N(float x, ...)                            \
    {                                                      \
        va_list args;                                      \
        va_start(args, N - 1);                             \
        vec##N v;                                          \
        v[0] = x;                                          \
        for (int i = 0; i < N - 1; i++)                    \
            v[i + 1] = (float)va_arg(args, double);        \
        va_end(args);                                      \
        return v;                                          \
    }                                                      \
    float Vec##N##Sum(vec##N v)                            \
    {                                                      \
        float r = 0.f;                                     \
        for (int i = 0; i < N; i++)                        \
            r += v[i];                                     \
        return r;                                          \
    }                                                      \
    float Vec##N##LengthSqr(vec##N v)                      \
    {                                                      \
        return Vec##N##Sum(v * v);                         \
    }                                                      \
    float Vec##N##Length(vec##N v)                         \
    {                                                      \
        return sqrtf(Vec##N##LengthSqr(v));                \
    }                                                      \
    float Vec##N##Dot(vec##N v1, vec##N v2)                \
    {                                                      \
        return Vec##N##Sum(v1 * v2);                       \
    }                                                      \
    vec##N Vec##N##Normalize(vec##N v)                     \
    {                                                      \
        vec##N r = {0};                                    \
        float l = Vec##N##Length(v);                       \
        for (int i = 0; i < N; i++)                        \
            r[i] = v[i] * (1.f / l);                       \
        return r;                                          \
    }                                                      \
    float Vec##N##DistSqr(vec##N v1, vec##N v2)            \
    {                                                      \
        vec##N v = v2 - v1;                                \
        return Vec##N##Sum(v * v);                         \
    }                                                      \
    float Vec##N##Dist(vec##N v1, vec##N v2)               \
    {                                                      \
        return sqrtf(Vec##N##DistSqr(v1, v2));             \
    }
SIZES
#undef X

vec3 Vec3Cross(vec3 v1, vec3 v2) {
    return (vec3) { v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x };
}

#define X(N)                                     \
    mat##N Mat##N(float v)                       \
    {                                            \
        mat##N result;                           \
        for (int x = 0; x < N; x++)              \
            for (int y = 0; y < N; y++)          \
                result[x][y] = x == y ? v : 0.f; \
        return result;                           \
    }
SIZES
#undef X

mat4 Frustum(double left, double right, double bottom, double top, double near, double far) {
    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(far - near);

    mat4 result = Mat4(0.f);
    result[0][0] = ((float)near*2.0f)/rl;
    result[1][1] = ((float)near*2.0f)/tb;
    result[0][2] = ((float)right + (float)left)/rl;
    result[1][2] = ((float)top + (float)bottom)/tb;
    result[2][2] = -((float)far + (float)near)/fn;
    result[3][2] = -1.0f;
    result[2][3] = -((float)far*(float)near*2.0f)/fn;
    return result;
}

mat4 Perspective(float fov, float aspectRatio, float near, float far) {
    double top = near*tan(fov*0.5);
    double right = top*aspectRatio;
    return Frustum(-right, right, -top, top, near, far);
}

mat4 LookAt(vec3 eye, vec3 target, vec3 up) {
    vec3 vz = Vec3Normalize(eye - target);
    vec3 vx = Vec3Normalize(Vec3Cross(up, vz));
    vec3 vy = Vec3Cross(vz, vx);
    
    mat4 result = Mat4(0.f);
    result[0][0] = vx.x;
    result[1][0] = vy.x;
    result[2][0] = vz.x;
    result[3][0] = 0.0f;
    result[0][1] = vx.y;
    result[1][1] = vy.y;
    result[2][1] = vz.y;
    result[3][1] = 0.0f;
    result[0][2] = vx.z;
    result[1][2] = vy.z;
    result[2][2] = vz.z;
    result[3][2] = 0.0f;
    result[0][3] = -Vec3Dot(vx, eye);
    result[1][3] = -Vec3Dot(vy, eye);
    result[2][3] = -Vec3Dot(vz, eye);
    result[3][3] = 1.0f;
    return result;
}
