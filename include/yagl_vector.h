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

#ifndef _YAGL_VECTOR_H_
#define _YAGL_VECTOR_H_

#include "yagl_export.h"
#include "yagl_types.h"

struct yagl_vector
{
    void *data;
    int elem_size;
    int size;
    int capacity;
};

YAGL_API void yagl_vector_init(struct yagl_vector *v,
                               int elem_size,
                               int initial_capacity);

/*
 * Cleans up the vector, you must call 'yagl_vector_init' again to be able
 * to use it.
 */
YAGL_API void yagl_vector_cleanup(struct yagl_vector *v);

/*
 * Detaches the buffer from vector. vector will automatically clean up,
 * you must call 'yagl_vector_init' again to be able to use it.
 */
YAGL_API void *yagl_vector_detach(struct yagl_vector *v);

YAGL_API int yagl_vector_size(struct yagl_vector *v);

YAGL_API int yagl_vector_capacity(struct yagl_vector *v);

YAGL_API void yagl_vector_push_back(struct yagl_vector *v, const void *elem);

YAGL_API void yagl_vector_resize(struct yagl_vector *v, int new_size);

YAGL_API void *yagl_vector_data(struct yagl_vector *v);

#endif
