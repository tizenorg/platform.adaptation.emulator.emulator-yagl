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

#ifndef _YAGL_WAYLAND_WINDOW_H_
#define _YAGL_WAYLAND_WINDOW_H_

#include "yagl_export.h"
#include "yagl_native_drawable.h"
#include <wayland-client.h>

#define YAGL_WAYLAND_WINDOW(os_window) ((struct wl_egl_window*)(os_window))

struct vigs_drm_surface;

struct yagl_wayland_window
{
    struct yagl_native_drawable base;

    struct
    {
        struct vigs_drm_surface *drm_sfc;
        struct wl_buffer *wl_buffer;
        int locked;
        int age;
    } color_buffers[3], *back, *front;

    int width;
    int height;

    int dx;
    int dy;

    struct wl_callback *frame_callback;
};

struct yagl_native_drawable
    *yagl_wayland_window_create(struct yagl_native_display *dpy,
                                yagl_os_drawable os_drawable);

#endif
