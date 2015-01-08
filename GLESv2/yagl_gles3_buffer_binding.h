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

#ifndef _YAGL_GLES3_BUFFER_BINDING_H_
#define _YAGL_GLES3_BUFFER_BINDING_H_

#include "yagl_gles_types.h"
#include "yagl_list.h"

struct yagl_gles_buffer;

struct yagl_gles3_buffer_binding
{
    struct yagl_list list;
    GLenum target;
    GLuint index;

    struct yagl_gles_buffer *buffer;

    int entire;

    GLintptr offset;
    GLsizeiptr size;
};

void yagl_gles3_buffer_binding_init(struct yagl_gles3_buffer_binding *buffer_binding,
                                    GLenum target,
                                    GLuint index);

void yagl_gles3_buffer_binding_reset(struct yagl_gles3_buffer_binding *buffer_binding);

void yagl_gles3_buffer_binding_set_base(struct yagl_gles3_buffer_binding *buffer_binding,
                                        struct yagl_gles_buffer *buffer);

void yagl_gles3_buffer_binding_set_range(struct yagl_gles3_buffer_binding *buffer_binding,
                                         struct yagl_gles_buffer *buffer,
                                         GLintptr offset,
                                         GLsizeiptr size);

void yagl_gles3_buffer_binding_transfer_begin(struct yagl_gles3_buffer_binding *buffer_binding);

void yagl_gles3_buffer_binding_transfer_end(struct yagl_gles3_buffer_binding *buffer_binding);

void yagl_gles3_buffer_binding_set_gpu_dirty(struct yagl_gles3_buffer_binding *buffer_binding);

#endif
