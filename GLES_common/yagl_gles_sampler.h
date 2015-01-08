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

#ifndef _YAGL_GLES_SAMPLER_H_
#define _YAGL_GLES_SAMPLER_H_

#include "yagl_types.h"
#include "yagl_object.h"

#define YAGL_NS_SAMPLER 4

struct yagl_gles_sampler
{
    struct yagl_object base;

    yagl_object_name global_name;

    GLenum min_filter;
    GLenum mag_filter;
    GLfloat min_lod;
    GLfloat max_lod;
    GLenum wrap_s;
    GLenum wrap_t;
    GLenum wrap_r;
    GLenum compare_mode;
    GLenum compare_func;
};

struct yagl_gles_sampler *yagl_gles_sampler_create(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_sampler_acquire(struct yagl_gles_sampler *sampler);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_sampler_release(struct yagl_gles_sampler *sampler);

void yagl_gles_sampler_bind(struct yagl_gles_sampler *sampler,
                            GLuint unit);

int yagl_gles_sampler_set_parameteriv(struct yagl_gles_sampler *sampler,
                                      GLenum pname, const GLint *param);

int yagl_gles_sampler_set_parameterfv(struct yagl_gles_sampler *sampler,
                                      GLenum pname, const GLfloat *param);

int yagl_gles_sampler_get_parameteriv(struct yagl_gles_sampler *sampler,
                                      GLenum pname, GLint *param);

int yagl_gles_sampler_get_parameterfv(struct yagl_gles_sampler *sampler,
                                      GLenum pname, GLfloat *param);

#endif
