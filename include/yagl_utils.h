#ifndef _YAGL_UTILS_H_
#define _YAGL_UTILS_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include <pthread.h>

YAGL_API void yagl_mutex_init(pthread_mutex_t* mutex);

YAGL_API void yagl_recursive_mutex_init(pthread_mutex_t* mutex);

static __inline float yagl_fixed_to_float(int32_t x)
{
    return (float)x * (1.0f / 65536.0f);
}

static __inline int32_t yagl_fixed_to_int(int32_t x)
{
    return (x + 0x0800) >> 16;
}

static __inline int32_t yagl_float_to_fixed(float f)
{
    return (int32_t)(f * 65536.0f + 0.5f);
}

static __inline int32_t yagl_double_to_fixed(double d)
{
    return (int32_t)(d * 65536.0f + 0.5f);
}

static __inline int32_t yagl_int_to_fixed(int32_t i)
{
    return i << 16;
}

static __inline float yagl_clampf(float f)
{
    if (f < 0.0f) {
        return 0.0f;
    } else if (f > 1.0f) {
        return 1.0f;
    } else {
        return f;
    }
}

#endif
