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

#include "yagl_export.h"
#include "yagl_wayland_egl.h"
#include <stdlib.h>
#include <string.h>

YAGL_API struct wl_egl_window *wl_egl_window_create(struct wl_surface *surface,
                                                    int width, int height)
{
    struct wl_egl_window *egl_window;

    egl_window = malloc(sizeof(*egl_window));

    if (!egl_window) {
        return NULL;
    }

    memset(egl_window, 0, sizeof(*egl_window));

    egl_window->surface = surface;

    wl_egl_window_resize(egl_window, width, height, 0, 0);

    return egl_window;
}

YAGL_API void wl_egl_window_destroy(struct wl_egl_window *egl_window)
{
    free(egl_window);
}

YAGL_API void wl_egl_window_resize(struct wl_egl_window *egl_window,
                                   int width, int height,
                                   int dx, int dy)
{
    egl_window->width  = width;
    egl_window->height = height;
    egl_window->dx     = dx;
    egl_window->dy     = dy;

    if (egl_window->resize_callback) {
        egl_window->resize_callback(egl_window, egl_window->user_data);
    }
}

YAGL_API void wl_egl_window_get_attached_size(struct wl_egl_window *egl_window,
                                              int *width, int *height)
{
    if (width) {
        *width = egl_window->attached_width;
    }
    if (height) {
        *height = egl_window->attached_height;
    }
}
