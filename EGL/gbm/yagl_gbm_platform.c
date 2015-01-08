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

#include "yagl_gbm_platform.h"
#include "yagl_gbm_display.h"
#include "yagl_gbm.h"
#include "yagl_native_platform.h"
#include "yagl_log.h"
#include "EGL/egl.h"

static int yagl_gbm_platform_probe(yagl_os_display os_dpy)
{
    void *first_pointer;

    if (os_dpy == (yagl_os_display)EGL_DEFAULT_DISPLAY) {
        return 0;
    }

    first_pointer = *(void**)os_dpy;

    return (first_pointer == &gbm_create_device);
}

static struct yagl_native_display
    *yagl_gbm_wrap_display(yagl_os_display os_dpy,
                           int enable_drm)
{
    YAGL_LOG_FUNC_SET(eglGetDisplay);

    if (os_dpy != (yagl_os_display)EGL_DEFAULT_DISPLAY) {
        return yagl_gbm_display_create(&yagl_gbm_platform, os_dpy);
    } else {
        YAGL_LOG_ERROR("Default display not supported on GBM");
        return NULL;
    }
}

struct yagl_native_platform yagl_gbm_platform =
{
    .pixmaps_supported = 1,
    .buffer_age_supported = 1,
    .probe = yagl_gbm_platform_probe,
    .wrap_display = yagl_gbm_wrap_display
};
