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

#ifndef _YAGL_BACKEND_H_
#define _YAGL_BACKEND_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_native_types.h"
#include "EGL/egl.h"

struct yagl_display;
struct yagl_surface;
struct yagl_image;
struct yagl_fence;
struct yagl_native_platform;
struct yagl_native_drawable;
struct yagl_client_interface;
struct wl_resource;
struct yagl_context;

struct yagl_backend
{
    struct yagl_display *(*create_display)(struct yagl_native_platform */*platform*/,
                                           yagl_os_display /*os_dpy*/);

    /*
     * Takes ownership of 'native_window'.
     */
    struct yagl_surface *(*create_window_surface)(struct yagl_display */*dpy*/,
                                                  yagl_host_handle /*host_config*/,
                                                  struct yagl_native_drawable */*native_window*/,
                                                  const EGLint */*attrib_list*/);

    /*
     * Takes ownership of 'native_pixmap'.
     */
    struct yagl_surface *(*create_pixmap_surface)(struct yagl_display */*dpy*/,
                                                  yagl_host_handle /*host_config*/,
                                                  struct yagl_native_drawable */*native_pixmap*/,
                                                  const EGLint */*attrib_list*/);

    struct yagl_surface *(*create_pbuffer_surface)(struct yagl_display */*dpy*/,
                                                   yagl_host_handle /*host_config*/,
                                                   const EGLint */*attrib_list*/);

    /*
     * Takes ownership of 'native_pixmap'.
     */
    struct yagl_image *(*create_image_pixmap)(struct yagl_display */*dpy*/,
                                              struct yagl_native_drawable */*native_pixmap*/,
                                              struct yagl_client_interface */*iface*/);

    struct yagl_image *(*create_image_wl_buffer)(struct yagl_display *dpy,
#ifdef YAGL_PLATFORM_WAYLAND
						struct wl_resource *buffer,
#elif YAGL_PLATFORM_TIZEN
						EGLClientBuffer pixmap,
#endif
						struct yagl_client_interface */*iface*/);

    struct yagl_image *(*create_image_gl_texture_2d)(struct yagl_display */*dpy*/,
                                                     struct yagl_context */*ctx*/,
                                                     yagl_object_name /*texture*/,
                                                     struct yagl_client_interface */*iface*/);

    struct yagl_image *(*create_image_tizen_sfc)(struct yagl_display */*dpy*/,
                                                 EGLClientBuffer /*buffer*/,
                                                 struct yagl_client_interface */*iface*/);

    struct yagl_fence *(*create_fence)(struct yagl_display */*dpy*/);

    void (*destroy)(struct yagl_backend */*backend*/);

    EGLint y_inverted;

    int fence_supported;
};

struct yagl_backend *yagl_get_backend();

#endif
