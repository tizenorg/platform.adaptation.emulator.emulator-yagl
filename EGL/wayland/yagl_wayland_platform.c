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

#include "yagl_wayland_platform.h"
#include "yagl_wayland_display.h"
#include "yagl_native_platform.h"
#include "yagl_log.h"
#include "EGL/egl.h"
#include <wayland-client.h>

static int yagl_wayland_platform_probe(yagl_os_display os_dpy)
{
    void *first_pointer;

    if (os_dpy == (yagl_os_display)EGL_DEFAULT_DISPLAY) {
        return 0;
    }

    first_pointer = *(void**)os_dpy;

    return (first_pointer == &wl_display_interface);
}

static struct yagl_native_display
    *yagl_wayland_wrap_display(yagl_os_display os_dpy,
                               int enable_drm)
{
    struct yagl_native_display *dpy = NULL;

    YAGL_LOG_FUNC_SET(eglGetDisplay);

    if (os_dpy == (yagl_os_display)EGL_DEFAULT_DISPLAY) {
        struct wl_display *wl_dpy;

        wl_dpy = wl_display_connect(NULL);

        if (!wl_dpy) {
            YAGL_LOG_ERROR("Unable to connect to default display");
            return NULL;
        }

        dpy = yagl_wayland_display_create(&yagl_wayland_platform,
                                          (yagl_os_display)wl_dpy,
                                          1);

        if (!dpy) {
            wl_display_disconnect(wl_dpy);
        }
    } else {
        dpy = yagl_wayland_display_create(&yagl_wayland_platform, os_dpy, 0);
    }

    return dpy;
}

struct yagl_native_platform yagl_wayland_platform =
{
    .pixmaps_supported = 0,
    .buffer_age_supported = 1,
    .probe = yagl_wayland_platform_probe,
    .wrap_display = yagl_wayland_wrap_display
};
