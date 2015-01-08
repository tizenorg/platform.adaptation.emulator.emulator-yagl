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

#include "GLES3/gl3.h"
#include "yagl_gles3_query.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"

static void yagl_gles3_query_update_result(struct yagl_gles3_query *query)
{
    if (!query->result_available) {
        query->result_available =
            yagl_host_glGetQueryObjectuiv(query->global_name, &query->result);
    }
}

static void yagl_gles3_query_destroy(struct yagl_ref *ref)
{
    struct yagl_gles3_query *query = (struct yagl_gles3_query*)ref;

    yagl_host_glDeleteObjects(&query->global_name, 1);

    yagl_object_cleanup(&query->base);

    yagl_free(query);
}

struct yagl_gles3_query *yagl_gles3_query_create(void)
{
    struct yagl_gles3_query *query;

    query = yagl_malloc0(sizeof(*query));

    yagl_object_init(&query->base, &yagl_gles3_query_destroy);

    query->global_name = yagl_get_global_name();

    yagl_host_glGenQueries(&query->global_name, 1);

    return query;
}

void yagl_gles3_query_acquire(struct yagl_gles3_query *query)
{
    if (query) {
        yagl_object_acquire(&query->base);
    }
}

void yagl_gles3_query_release(struct yagl_gles3_query *query)
{
    if (query) {
        yagl_object_release(&query->base);
    }
}

void yagl_gles3_query_begin(struct yagl_gles3_query *query,
                            GLenum target)
{
    yagl_host_glBeginQuery(target, query->global_name);

    query->active = 1;
    query->result_available = 0;
    query->result = 0;
    query->was_active = 1;
}

void yagl_gles3_query_end(struct yagl_gles3_query *query,
                          GLenum target)
{
    yagl_host_glEndQuery(target);

    query->active = 0;
}

int yagl_gles3_query_is_result_available(struct yagl_gles3_query *query)
{
    yagl_gles3_query_update_result(query);

    return query->result_available;
}

GLuint yagl_gles3_query_get_result(struct yagl_gles3_query *query)
{
    yagl_gles3_query_update_result(query);

    return query->result;
}

int yagl_gles3_query_was_active(struct yagl_gles3_query *query)
{
    return query->was_active;
}
