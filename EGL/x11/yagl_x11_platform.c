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

#include "yagl_x11_platform.h"
#include "yagl_x11_display.h"
#include "yagl_native_platform.h"
#include "yagl_log.h"
#include "EGL/egl.h"
#include <X11/Xlib.h>

static int yagl_x11_platform_probe(yagl_os_display os_dpy)
{
    /*
     * Can't probe for X11, so assume probe passed.
     */

    return 1;
}

static struct yagl_native_display
    *yagl_x11_wrap_display(yagl_os_display os_dpy,
                           int enable_drm)
{
    struct yagl_native_display *dpy;

    YAGL_LOG_FUNC_SET(eglGetDisplay);

    if (os_dpy == (yagl_os_display)EGL_DEFAULT_DISPLAY) {
        Display *x_dpy = XOpenDisplay(0);

        if (!x_dpy) {
            YAGL_LOG_ERROR("unable to open display 0");
            return NULL;
        }

        dpy = yagl_x11_display_create(&yagl_x11_platform,
                                      (yagl_os_display)x_dpy,
                                      1,
                                      enable_drm);

        if (!dpy) {
            XCloseDisplay(x_dpy);
        }
    } else {
        dpy = yagl_x11_display_create(&yagl_x11_platform,
                                      os_dpy,
                                      0,
                                      enable_drm);
    }

    return dpy;
}

struct yagl_native_platform yagl_x11_platform =
{
    .pixmaps_supported = 1,
    .buffer_age_supported = 0,
    .probe = yagl_x11_platform_probe,
    .wrap_display = yagl_x11_wrap_display
};
