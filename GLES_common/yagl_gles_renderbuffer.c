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
#include "yagl_gles_renderbuffer.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"

static void yagl_gles_renderbuffer_destroy(struct yagl_ref *ref)
{
    struct yagl_gles_renderbuffer *rb = (struct yagl_gles_renderbuffer*)ref;

    yagl_host_glDeleteObjects(&rb->global_name, 1);

    yagl_object_cleanup(&rb->base);

    yagl_free(rb);
}

struct yagl_gles_renderbuffer *yagl_gles_renderbuffer_create(void)
{
    struct yagl_gles_renderbuffer *rb;

    rb = yagl_malloc0(sizeof(*rb));

    yagl_object_init(&rb->base, &yagl_gles_renderbuffer_destroy);

    rb->global_name = yagl_get_global_name();

    yagl_host_glGenRenderbuffers(&rb->global_name, 1);

    return rb;
}

void yagl_gles_renderbuffer_acquire(struct yagl_gles_renderbuffer *rb)
{
    if (rb) {
        yagl_object_acquire(&rb->base);
    }
}

void yagl_gles_renderbuffer_release(struct yagl_gles_renderbuffer *rb)
{
    if (rb) {
        yagl_object_release(&rb->base);
    }
}

void yagl_gles_renderbuffer_bind(struct yagl_gles_renderbuffer *rb,
                                 GLenum target)
{
    if (!rb) {
        yagl_host_glBindRenderbuffer(target, 0);
        return;
    }

    yagl_host_glBindRenderbuffer(target, rb->global_name);

    rb->was_bound = 1;
}

void yagl_gles_renderbuffer_set_internalformat(struct yagl_gles_renderbuffer *rb,
                                               GLenum internalformat)
{
    rb->internalformat = internalformat;
}

int yagl_gles_renderbuffer_was_bound(struct yagl_gles_renderbuffer *rb)
{
    return rb->was_bound;
}
