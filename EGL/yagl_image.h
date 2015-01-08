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

#ifndef _YAGL_IMAGE_H_
#define _YAGL_IMAGE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_resource.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"

struct yagl_display;
struct yagl_client_image;

struct yagl_image
{
    struct yagl_resource res;

    struct yagl_display *dpy;

    EGLImageKHR client_handle;

    struct yagl_client_image *client_image;

    void (*update)(struct yagl_image */*image*/);
};

void yagl_image_init(struct yagl_image *image,
                     yagl_ref_destroy_func destroy_func,
                     struct yagl_display *dpy,
                     EGLImageKHR client_handle,
                     struct yagl_client_image *client_image);

void yagl_image_cleanup(struct yagl_image *image);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_image_acquire(struct yagl_image *image);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_image_release(struct yagl_image *image);

#endif
