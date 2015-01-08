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

#ifndef _YAGL_GLES_RENDERBUFFER_H_
#define _YAGL_GLES_RENDERBUFFER_H_

#include "yagl_types.h"
#include "yagl_object.h"

#define YAGL_NS_RENDERBUFFER 2

struct yagl_gles_renderbuffer
{
    struct yagl_object base;

    yagl_object_name global_name;

    GLenum internalformat;

    int was_bound;
};

struct yagl_gles_renderbuffer *yagl_gles_renderbuffer_create(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_renderbuffer_acquire(struct yagl_gles_renderbuffer *rb);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_renderbuffer_release(struct yagl_gles_renderbuffer *rb);

/*
 * Assumes that 'target' is valid.
 */
void yagl_gles_renderbuffer_bind(struct yagl_gles_renderbuffer *rb,
                                 GLenum target);

void yagl_gles_renderbuffer_set_internalformat(struct yagl_gles_renderbuffer *rb,
                                               GLenum internalformat);

int yagl_gles_renderbuffer_was_bound(struct yagl_gles_renderbuffer *rb);

#endif
