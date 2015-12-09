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

#include "yagl_egl_fence.h"
#include "yagl_egl_state.h"
#include "yagl_fence.h"
#include "yagl_context.h"
#include "yagl_backend.h"

int yagl_egl_fence_supported(void)
{
    return yagl_get_backend()->fence_supported;
}

void yagl_egl_fence_acquire(struct yagl_egl_fence *egl_fence)
{
    if (egl_fence) {
        yagl_resource_acquire(&egl_fence->res);
    }
}

void yagl_egl_fence_release(struct yagl_egl_fence *egl_fence)
{
    if (egl_fence) {
        yagl_resource_release(&egl_fence->res);
    }
}

struct yagl_egl_fence *yagl_create_egl_fence(void)
{
    struct yagl_context *ctx = yagl_get_context();
    struct yagl_fence *fence;

    if (!ctx) {
        return NULL;
    }

    fence = yagl_get_backend()->create_fence(ctx->dpy);

    if (!fence) {
        return NULL;
    }

    return &fence->base;
}