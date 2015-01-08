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

#ifndef _YAGL_EGL_FENCE_H_
#define _YAGL_EGL_FENCE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_resource.h"

struct yagl_egl_fence
{
    struct yagl_resource res;

    int (*wait)(struct yagl_egl_fence */*egl_fence*/);

    int (*signaled)(struct yagl_egl_fence */*egl_fence*/);
};

YAGL_API int yagl_egl_fence_supported(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_egl_fence_acquire(struct yagl_egl_fence *egl_fence);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_egl_fence_release(struct yagl_egl_fence *egl_fence);

YAGL_API struct yagl_egl_fence *yagl_create_egl_fence(void);

#endif
