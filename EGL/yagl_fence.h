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

#ifndef _YAGL_FENCE_H_
#define _YAGL_FENCE_H_

#include "yagl_export.h"
#include "yagl_egl_fence.h"

struct yagl_display;

struct yagl_fence
{
    struct yagl_egl_fence base;

    struct yagl_display *dpy;

    uint32_t seq;
};

void yagl_fence_init(struct yagl_fence *fence,
                     yagl_ref_destroy_func destroy_func,
                     struct yagl_display *dpy,
                     uint32_t seq);

void yagl_fence_cleanup(struct yagl_fence *fence);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_fence_acquire(struct yagl_fence *fence);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_fence_release(struct yagl_fence *fence);

#endif
