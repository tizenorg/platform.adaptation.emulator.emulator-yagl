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

#ifndef _YAGL_GLES_VERTEX_ARRAY_H_
#define _YAGL_GLES_VERTEX_ARRAY_H_

#include "yagl_gles_types.h"
#include "yagl_object.h"

struct yagl_gles_array;
struct yagl_gles_buffer;

struct yagl_gles_vertex_array
{
    struct yagl_object base;

    yagl_object_name global_name;

    /*
     * GLES arrays, the number of arrays is different depending on
     * GLES version, 'num_arrays' holds that number.
     */
    struct yagl_gles_array *arrays;
    int num_arrays;

    struct yagl_gles_buffer *ebo;

    int was_bound;
};

struct yagl_gles_vertex_array
    *yagl_gles_vertex_array_create(int fake,
                                   struct yagl_gles_array *arrays,
                                   int num_arrays);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_vertex_array_acquire(struct yagl_gles_vertex_array *va);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_vertex_array_release(struct yagl_gles_vertex_array *va);

void yagl_gles_vertex_array_bind(struct yagl_gles_vertex_array *va);

int yagl_gles_vertex_array_was_bound(struct yagl_gles_vertex_array *va);

#endif
