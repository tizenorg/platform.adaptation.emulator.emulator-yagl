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

#include "yagl_ref.h"
#include "yagl_utils.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

void yagl_ref_init(struct yagl_ref *ref, yagl_ref_destroy_func destroy)
{
    assert(ref);
    assert(destroy);

    memset(ref, 0, sizeof(*ref));

    ref->destroy = destroy;
    yagl_mutex_init(&ref->mutex);
    ref->count = 1;
}

void yagl_ref_cleanup(struct yagl_ref *ref)
{
    assert(ref);
    assert(!ref->count);

    ref->destroy = NULL;
    pthread_mutex_destroy(&ref->mutex);
}

void yagl_ref_acquire(struct yagl_ref *ref)
{
    assert(ref);
    assert(ref->count > 0);

    pthread_mutex_lock(&ref->mutex);
    ++ref->count;
    pthread_mutex_unlock(&ref->mutex);
}

void yagl_ref_release(struct yagl_ref *ref)
{
    int call_destroy = 0;

    assert(ref);
    assert(ref->count > 0);

    pthread_mutex_lock(&ref->mutex);
    call_destroy = (--ref->count == 0);
    pthread_mutex_unlock(&ref->mutex);

    if (call_destroy)
    {
        assert(ref->destroy);
        ref->destroy(ref);
    }
}
