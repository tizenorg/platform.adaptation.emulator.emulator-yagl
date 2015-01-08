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

#ifndef _YAGL_TRANSPORT_EGL_H_
#define _YAGL_TRANSPORT_EGL_H_

#include "yagl_transport.h"
#include "EGL/egl.h"

static __inline int32_t yagl_transport_attrib_list_count(const EGLint *value)
{
    int32_t count = 0;

    if (value) {
        EGLint tmp = value[count];
        while (tmp != EGL_NONE) {
            count += 2;
            tmp = value[count];
        }
        ++count;
    }

    return count;
}

static __inline void yagl_transport_put_out_EGLint(struct yagl_transport *t,
                                                   EGLint value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_EGLenum(struct yagl_transport *t,
                                                    EGLenum value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_in_EGLint(struct yagl_transport *t,
                                                  EGLint *value)
{
    yagl_transport_put_in_uint32_t(t, (uint32_t*)value);
}

static __inline void yagl_transport_put_in_EGLBoolean(struct yagl_transport *t,
                                                      EGLBoolean *value)
{
    yagl_transport_put_in_uint32_t(t, value);
}

#endif
