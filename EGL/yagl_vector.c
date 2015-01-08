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

#include "yagl_vector.h"
#include "yagl_malloc.h"
#include <string.h>
#include <assert.h>

#define YAGL_VECTOR_DEFAULT_CAPACITY 10

void yagl_vector_init(struct yagl_vector *v,
                      int elem_size,
                      int initial_capacity)
{
    assert(elem_size > 0);

    v->elem_size = elem_size;
    v->capacity = (initial_capacity < YAGL_VECTOR_DEFAULT_CAPACITY) ?
                      YAGL_VECTOR_DEFAULT_CAPACITY
                    : initial_capacity;
    v->data = yagl_malloc0(v->capacity * v->elem_size);
    v->size = 0;
}

void yagl_vector_cleanup(struct yagl_vector *v)
{
    yagl_free(v->data);
    memset(v, 0, sizeof(*v));
}

void *yagl_vector_detach(struct yagl_vector *v)
{
    void *tmp = v->data;
    memset(v, 0, sizeof(*v));
    return tmp;
}

int yagl_vector_size(struct yagl_vector *v)
{
    return v->size;
}

int yagl_vector_capacity(struct yagl_vector *v)
{
    return v->capacity;
}

void yagl_vector_push_back(struct yagl_vector *v, const void *elem)
{
    if (v->size >= v->capacity) {
        void *tmp;

        v->capacity = (v->size * 3) / 2;

        tmp = yagl_malloc(v->capacity * v->elem_size);
        memcpy(tmp, v->data, (v->size * v->elem_size));

        yagl_free(v->data);

        v->data = tmp;
    }

    memcpy((char*)v->data + (v->size * v->elem_size), elem, v->elem_size);

    ++v->size;
}

void yagl_vector_resize(struct yagl_vector *v, int new_size)
{
    if (new_size <= v->capacity) {
        v->size = new_size;
    } else {
        void *tmp;

        v->capacity = (new_size * 3) / 2;

        tmp = yagl_malloc(v->capacity * v->elem_size);
        memcpy(tmp, v->data, (v->size * v->elem_size));

        yagl_free(v->data);

        v->data = tmp;
        v->size = new_size;
    }
}

void *yagl_vector_data(struct yagl_vector *v)
{
    return v->data;
}
