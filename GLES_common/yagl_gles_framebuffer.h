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

#ifndef _YAGL_GLES_FRAMEBUFFER_H_
#define _YAGL_GLES_FRAMEBUFFER_H_

#include "yagl_gles_types.h"
#include "yagl_object.h"

struct yagl_gles_texture;
struct yagl_gles_renderbuffer;

struct yagl_gles_framebuffer_attachment_state
{
    GLenum type;

    yagl_object_name local_name;

    union
    {
        struct yagl_object *obj;
        struct yagl_gles_texture *texture;
        struct yagl_gles_renderbuffer *rb;
    };

    GLenum textarget;

    GLint layer;
};

struct yagl_gles_framebuffer
{
    struct yagl_object base;

    yagl_object_name global_name;

    struct yagl_gles_framebuffer_attachment_state
        attachment_states[yagl_gles_framebuffer_attachment_color0 +
                          YAGL_MAX_GLES_FRAMEBUFFER_COLOR_ATTACHMENTS];

    GLenum draw_buffers[YAGL_MAX_GLES_FRAMEBUFFER_COLOR_ATTACHMENTS];
    GLenum read_buffer;

    int was_bound;
};

int yagl_gles_framebuffer_attachment_internalformat(struct yagl_gles_framebuffer_attachment_state *attachment_state,
                                                    GLenum *internalformat);

struct yagl_gles_framebuffer *yagl_gles_framebuffer_create(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_framebuffer_acquire(struct yagl_gles_framebuffer *fb);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_framebuffer_release(struct yagl_gles_framebuffer *fb);

void yagl_gles_framebuffer_renderbuffer(struct yagl_gles_framebuffer *fb,
                                        GLenum target,
                                        GLenum attachment,
                                        yagl_gles_framebuffer_attachment framebuffer_attachment,
                                        GLenum renderbuffer_target,
                                        struct yagl_gles_renderbuffer *rb);

void yagl_gles_framebuffer_texture2d(struct yagl_gles_framebuffer *fb,
                                     GLenum target,
                                     GLenum attachment,
                                     yagl_gles_framebuffer_attachment framebuffer_attachment,
                                     GLenum textarget,
                                     GLint level,
                                     struct yagl_gles_texture *texture);

void yagl_gles_framebuffer_texture_layer(struct yagl_gles_framebuffer *fb,
                                         GLenum target,
                                         GLenum attachment,
                                         yagl_gles_framebuffer_attachment framebuffer_attachment,
                                         struct yagl_gles_texture *texture,
                                         GLint level,
                                         GLint layer);

/*
 * Assumes that 'target' is valid.
 */
void yagl_gles_framebuffer_bind(struct yagl_gles_framebuffer *fb,
                                GLenum target);

int yagl_gles_framebuffer_was_bound(struct yagl_gles_framebuffer *fb);

void yagl_gles_framebuffer_unbind_texture(struct yagl_gles_framebuffer *fb,
                                          yagl_object_name texture_local_name);

void yagl_gles_framebuffer_unbind_renderbuffer(struct yagl_gles_framebuffer *fb,
                                               yagl_object_name rb_local_name);

#endif
