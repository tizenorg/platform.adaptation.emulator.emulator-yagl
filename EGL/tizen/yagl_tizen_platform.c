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

#include "yagl_tizen_platform.h"
#include "yagl_tizen_display.h"
#include "yagl_native_platform.h"
#include "yagl_log.h"
#include "EGL/egl.h"
#include <wayland-client.h>

static int yagl_tizen_platform_probe(yagl_os_display os_dpy)
{
	/* Don't need to do anything */
	return 1;
}

static struct yagl_native_display
    *yagl_tizen_wrap_display(yagl_os_display os_dpy,
                               int enable_drm)
{
	struct yagl_native_display *dpy = NULL;

	YAGL_LOG_FUNC_SET(eglGetDisplay);

	dpy = yagl_tizen_display_create(&yagl_tizen_platform,os_dpy);

	return dpy;
}

struct yagl_native_platform yagl_tizen_platform =
{
	.pixmaps_supported = 0,
	.buffer_age_supported = 1,
	.probe = yagl_tizen_platform_probe,
	.wrap_display = yagl_tizen_wrap_display
};
