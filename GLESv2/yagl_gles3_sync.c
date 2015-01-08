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

#include "yagl_gles3_sync.h"
#include "yagl_egl_fence.h"
#include "yagl_malloc.h"

static void yagl_gles3_sync_destroy(struct yagl_ref *ref)
{
    struct yagl_gles3_sync *sync = (struct yagl_gles3_sync*)ref;

    yagl_egl_fence_release(sync->egl_fence);

    yagl_object_cleanup(&sync->base);

    yagl_free(sync);
}

struct yagl_gles3_sync *yagl_gles3_sync_create(void)
{
    struct yagl_egl_fence *egl_fence;
    struct yagl_gles3_sync *sync;

    egl_fence = yagl_create_egl_fence();

    if (!egl_fence) {
        return NULL;
    }

    sync = yagl_malloc0(sizeof(*sync));

    yagl_object_init(&sync->base, &yagl_gles3_sync_destroy);

    sync->egl_fence = egl_fence;

    return sync;
}

void yagl_gles3_sync_acquire(struct yagl_gles3_sync *sync)
{
    if (sync) {
        yagl_object_acquire(&sync->base);
    }
}

void yagl_gles3_sync_release(struct yagl_gles3_sync *sync)
{
    if (sync) {
        yagl_object_release(&sync->base);
    }
}
