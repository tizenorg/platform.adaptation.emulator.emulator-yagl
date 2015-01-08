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

#ifndef _YAGL_CONTEXT_H_
#define _YAGL_CONTEXT_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_resource.h"
#include "EGL/egl.h"

struct yagl_display;
struct yagl_fence;
struct yagl_client_context;

struct yagl_context
{
    struct yagl_resource res;

    struct yagl_display *dpy;

    struct yagl_client_context *client_ctx;

    int need_throttle;
    struct yagl_fence *throttle_fence;

    int client_ctx_prepared;

    pthread_mutex_t mtx;

    int current;
};

struct yagl_context
    *yagl_context_create(yagl_host_handle handle,
                         struct yagl_display *dpy,
                         struct yagl_client_context *client_ctx);

void yagl_context_set_need_throttle(struct yagl_context *ctx,
                                    struct yagl_fence *throttle_fence);

void yagl_context_throttle(struct yagl_context *ctx);

int yagl_context_mark_current(struct yagl_context *ctx, int current);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_context_acquire(struct yagl_context *ctx);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_context_release(struct yagl_context *ctx);

#endif
