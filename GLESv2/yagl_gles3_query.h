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

#ifndef _YAGL_GLES3_QUERY_H_
#define _YAGL_GLES3_QUERY_H_

#include "yagl_types.h"
#include "yagl_object.h"

struct yagl_gles3_query
{
    struct yagl_object base;

    yagl_object_name global_name;

    int active;

    int result_available;
    GLuint result;

    int was_active;
};

struct yagl_gles3_query *yagl_gles3_query_create(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles3_query_acquire(struct yagl_gles3_query *query);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles3_query_release(struct yagl_gles3_query *query);

void yagl_gles3_query_begin(struct yagl_gles3_query *query,
                            GLenum target);

void yagl_gles3_query_end(struct yagl_gles3_query *query,
                          GLenum target);

int yagl_gles3_query_is_result_available(struct yagl_gles3_query *query);

GLuint yagl_gles3_query_get_result(struct yagl_gles3_query *query);

int yagl_gles3_query_was_active(struct yagl_gles3_query *query);

#endif
