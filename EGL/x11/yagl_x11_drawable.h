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

#ifndef _YAGL_X11_DRAWABLE_H_
#define _YAGL_X11_DRAWABLE_H_

#include "yagl_export.h"
#include "yagl_native_drawable.h"
#include <X11/Xlib.h>
#include <pthread.h>

#define YAGL_X11_DRAWABLE(os_drawable) ((Drawable)(os_drawable))

struct yagl_x11_drawable
{
    struct yagl_native_drawable base;

    int own_drawable;

    int is_pixmap;

    pthread_mutex_t mtx;

    /*
     * Allocated on first request.
     */
    GC x_gc;

    /*
     * For pixmaps only, filled on first request.
     * @{
     */
    int is_geom_acquired;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    /*
     * @}
     */
};

struct yagl_native_drawable
    *yagl_x11_drawable_create(struct yagl_native_display *dpy,
                              yagl_os_drawable os_drawable,
                              int own_drawable,
                              int is_pixmap);

GC yagl_x11_drawable_get_gc(struct yagl_x11_drawable *drawable);

#endif
