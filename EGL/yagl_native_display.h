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

#ifndef _YAGL_NATIVE_DISPLAY_H_
#define _YAGL_NATIVE_DISPLAY_H_

#include "yagl_export.h"
#include "yagl_native_types.h"
#include "EGL/egl.h"

struct yagl_native_platform;
struct yagl_native_drawable;
struct yagl_native_image;
struct vigs_drm_device;

#ifdef YAGL_PLATFORM_WAYLAND
struct wl_display;
struct wl_resource;
struct wl_drm;
#endif

#ifdef YAGL_PLATFORM_TIZEN
struct wl_resource;
#endif

struct yagl_native_display
{
    struct yagl_native_platform *platform;

    yagl_os_display os_dpy;

    struct vigs_drm_device *drm_dev;
    char *drm_dev_name;

    int WL_bind_wayland_display_supported;

#ifdef YAGL_PLATFORM_WAYLAND
    struct wl_drm *wl_server_drm;
#endif

    int (*authenticate)(struct yagl_native_display */*dpy*/, uint32_t /*id*/);

    struct yagl_native_drawable *(*wrap_window)(struct yagl_native_display */*dpy*/,
                                                yagl_os_window /*os_window*/);

    struct yagl_native_drawable *(*wrap_pixmap)(struct yagl_native_display */*dpy*/,
                                                yagl_os_pixmap /*os_pixmap*/);

    struct yagl_native_drawable *(*create_pixmap)(struct yagl_native_display */*dpy*/,
                                                  uint32_t /*width*/,
                                                  uint32_t /*height*/,
                                                  uint32_t /*depth*/);

    struct yagl_native_image *(*create_image)(struct yagl_native_display */*dpy*/,
                                              uint32_t /*width*/,
                                              uint32_t /*height*/,
                                              uint32_t /*depth*/);

    int (*get_visual)(struct yagl_native_display */*dpy*/,
                      int */*visual_id*/,
                      int */*visual_type*/);

    void (*update_wl_server)(struct yagl_native_display */*dpy*/);

    void (*destroy)(struct yagl_native_display */*dpy*/);
};

void yagl_native_display_init(struct yagl_native_display *dpy,
                              struct yagl_native_platform *platform,
                              yagl_os_display os_dpy,
                              struct vigs_drm_device *drm_dev,
                              const char *drm_dev_name);

void yagl_native_display_cleanup(struct yagl_native_display *dpy);

#ifdef YAGL_PLATFORM_WAYLAND
int yagl_native_display_bind_wl_display(struct yagl_native_display *dpy,
                                        struct wl_display *wl_dpy);
int yagl_native_display_unbind_wl_display(struct yagl_native_display *dpy);

int yagl_native_display_query_wl_buffer(struct yagl_native_display *dpy,
                                        struct wl_resource *buffer,
                                        EGLint attribute,
                                        EGLint *value);
#elif YAGL_PLATFORM_TIZEN
int yagl_native_display_bind_wl_display(yagl_os_display dpy);
int yagl_native_display_unbind_wl_display(yagl_os_display dpy);
int yagl_native_display_query_wl_buffer(yagl_os_display dpy,
                                        struct wl_resource *buffer,
                                        EGLint attribute,
                                        EGLint *value);

#endif

#endif
