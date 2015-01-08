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

#include "GL/gl.h"
#include "yagl_gles_vertex_array.h"
#include "yagl_gles_array.h"
#include "yagl_gles_buffer.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"

static void yagl_gles_vertex_array_destroy(struct yagl_ref *ref)
{
    struct yagl_gles_vertex_array *va = (struct yagl_gles_vertex_array*)ref;
    int i;

    yagl_gles_buffer_release(va->ebo);

    for (i = 0; i < va->num_arrays; ++i) {
        yagl_gles_array_cleanup(&va->arrays[i]);
    }
    yagl_free(va->arrays);

    if (va->global_name) {
        yagl_host_glDeleteObjects(&va->global_name, 1);
    }

    yagl_object_cleanup(&va->base);

    yagl_free(va);
}

struct yagl_gles_vertex_array
    *yagl_gles_vertex_array_create(int fake,
                                   struct yagl_gles_array *arrays,
                                   int num_arrays)
{
    struct yagl_gles_vertex_array *va;

    va = yagl_malloc0(sizeof(*va));

    yagl_object_init(&va->base, &yagl_gles_vertex_array_destroy);

    va->arrays = arrays;
    va->num_arrays = num_arrays;

    if (!fake) {
        va->global_name = yagl_get_global_name();

        yagl_host_glGenVertexArrays(&va->global_name, 1);
    }

    return va;
}

void yagl_gles_vertex_array_acquire(struct yagl_gles_vertex_array *va)
{
    if (va) {
        yagl_object_acquire(&va->base);
    }
}

void yagl_gles_vertex_array_release(struct yagl_gles_vertex_array *va)
{
    if (va) {
        yagl_object_release(&va->base);
    }
}

void yagl_gles_vertex_array_bind(struct yagl_gles_vertex_array *va)
{
    yagl_host_glBindVertexArray(va->global_name);

    va->was_bound = 1;
}

int yagl_gles_vertex_array_was_bound(struct yagl_gles_vertex_array *va)
{
    return va->was_bound;
}
