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

#ifndef _YAGL_TRANSPORT_GL_H_
#define _YAGL_TRANSPORT_GL_H_

#include "yagl_transport.h"

static __inline void yagl_transport_put_out_GLenum(struct yagl_transport *t,
                                                   GLenum value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_GLuint(struct yagl_transport *t,
                                                   GLuint value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_GLint(struct yagl_transport *t,
                                                   GLint value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_GLintptr(struct yagl_transport *t,
                                                     GLintptr value)
{
    yagl_transport_put_out_uintptr_t(t, value);
}

static __inline void yagl_transport_put_out_GLbitfield(struct yagl_transport *t,
                                                       GLbitfield value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_GLboolean(struct yagl_transport *t,
                                                      GLboolean value)
{
    yagl_transport_put_out_uint8_t(t, value);
}

static __inline void yagl_transport_put_out_GLubyte(struct yagl_transport *t,
                                                    GLubyte value)
{
    yagl_transport_put_out_uint8_t(t, value);
}

static __inline void yagl_transport_put_out_GLsizei(struct yagl_transport *t,
                                                    GLsizei value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_GLclampf(struct yagl_transport *t,
                                                     GLclampf value)
{
    yagl_transport_put_out_float(t, value);
}

static __inline void yagl_transport_put_out_GLfloat(struct yagl_transport *t,
                                                    GLfloat value)
{
    yagl_transport_put_out_float(t, value);
}

static __inline void yagl_transport_put_in_GLenum(struct yagl_transport *t,
                                                  GLenum *value)
{
    yagl_transport_put_in_uint32_t(t, (uint32_t*)value);
}

static __inline void yagl_transport_put_in_GLuint(struct yagl_transport *t,
                                                  GLuint *value)
{
    yagl_transport_put_in_uint32_t(t, value);
}

static __inline void yagl_transport_put_in_GLint(struct yagl_transport *t,
                                                 GLint *value)
{
    yagl_transport_put_in_uint32_t(t, (uint32_t*)value);
}

static __inline void yagl_transport_put_in_GLsizei(struct yagl_transport *t,
                                                   GLsizei *value)
{
    yagl_transport_put_in_uint32_t(t, (uint32_t*)value);
}

static __inline void yagl_transport_put_in_GLboolean(struct yagl_transport *t,
                                                     GLboolean *value)
{
    yagl_transport_put_in_uint8_t(t, value);
}

static __inline void yagl_transport_put_in_GLfloat(struct yagl_transport *t,
                                                   GLfloat *value)
{
    yagl_transport_put_in_float(t, value);
}

#endif
