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

#ifndef _YAGL_NATIVE_IMAGE_H_
#define _YAGL_NATIVE_IMAGE_H_

#include "yagl_export.h"
#include "yagl_native_types.h"

struct yagl_native_drawable;

struct yagl_native_image
{
    struct yagl_native_display *dpy;

    uint32_t width;
    uint32_t height;
    uint32_t depth; /* bit-depth. e.g.: 24 */

    uint32_t bpp; /* bytes-per-pixel. e.g.: 3 */
    void *pixels; /* pixel data */

    void (*draw)(struct yagl_native_image */*image*/,
                 struct yagl_native_drawable */*drawable*/);

    void (*draw_to_pixmap)(struct yagl_native_image */*image*/,
                           yagl_os_pixmap /*os_pixmap*/);

    void (*destroy)(struct yagl_native_image */*image*/);
};

void yagl_native_image_init(struct yagl_native_image *image,
                            struct yagl_native_display *dpy,
                            uint32_t width,
                            uint32_t height,
                            uint32_t depth,
                            uint32_t bpp,
                            void *pixels);

void yagl_native_image_cleanup(struct yagl_native_image *image);

#endif
