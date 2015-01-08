/*
 * YaGL
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Stanislav Vorobiov <s.vorobiov@samsung.com>
 * Jinhyung Jo <jinhyung.jo@samsung.com>
 * YeongKyoon Lee <yeongkyoon.lee@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * - S-Core Co., Ltd
 *
 */

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
