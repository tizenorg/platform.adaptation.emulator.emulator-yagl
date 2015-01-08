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

#ifndef _YAGL_REF_H_
#define _YAGL_REF_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include <pthread.h>

struct yagl_ref;

typedef void (*yagl_ref_destroy_func)(struct yagl_ref */*ref*/);

struct yagl_ref
{
    yagl_ref_destroy_func destroy;

    pthread_mutex_t mutex;
    volatile uint32_t count;
};

/*
 * Initializes ref count to 1.
 */
YAGL_API void yagl_ref_init(struct yagl_ref *ref, yagl_ref_destroy_func destroy);

YAGL_API void yagl_ref_cleanup(struct yagl_ref *ref);

/*
 * Increments ref count.
 */
YAGL_API void yagl_ref_acquire(struct yagl_ref *ref);

/*
 * Decrements ref count and releases when 0.
 */
YAGL_API void yagl_ref_release(struct yagl_ref *ref);

#endif
