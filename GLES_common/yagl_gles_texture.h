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

#ifndef _YAGL_GLES_TEXTURE_H_
#define _YAGL_GLES_TEXTURE_H_

#include "yagl_types.h"
#include "yagl_object.h"

#define YAGL_NS_TEXTURE 1

struct yagl_gles_image;
struct yagl_tex_image_binding;

struct yagl_gles_texture
{
    struct yagl_object base;

    yagl_object_name global_name;

    GLenum target;

    GLenum internalformat;
    int is_float;
    int is_swizzled;

    GLboolean immutable;

    GLenum min_filter;
    GLenum mag_filter;

    GLint num_image_units;

    /*
     * Non-NULL if it's an EGLImage/eglBindTexImage target.
     */
    struct yagl_gles_image *image;

    /*
     * Non-NULL if it's an eglBindTexImage target.
     */
    struct yagl_tex_image_binding *binding;
};

struct yagl_gles_texture *yagl_gles_texture_create(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_texture_acquire(struct yagl_gles_texture *texture);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_texture_release(struct yagl_gles_texture *texture);

/*
 * Assumes that 'target' is valid.
 */
int yagl_gles_texture_bind(struct yagl_gles_texture *texture,
                           GLenum target);

void yagl_gles_texture_set_internalformat(struct yagl_gles_texture *texture,
                                          GLenum internalformat,
                                          GLenum type,
                                          int swizzle);

void yagl_gles_texture_set_immutable(struct yagl_gles_texture *texture,
                                     GLenum internalformat,
                                     GLenum type,
                                     int swizzle);

int yagl_gles_texture_color_renderable(struct yagl_gles_texture *texture);

void yagl_gles_texture_set_image(struct yagl_gles_texture *texture,
                                 struct yagl_gles_image *image);

void yagl_gles_texture_unset_image(struct yagl_gles_texture *texture);

void yagl_gles_texture_bind_tex_image(struct yagl_gles_texture *texture,
                                      struct yagl_gles_image *image,
                                      struct yagl_tex_image_binding *binding);

/*
 * Can be called with an arbitrary context being set, careful.
 */
void yagl_gles_texture_release_tex_image(struct yagl_gles_texture *texture);

#endif
