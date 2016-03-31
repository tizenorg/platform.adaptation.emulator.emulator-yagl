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

#ifndef _YAGL_WAYLAND_DISPLAY_H_
#define _YAGL_WAYLAND_DISPLAY_H_

#include "yagl_export.h"
#include "yagl_native_display.h"
#include <wayland-client.h>
#include "yagl_tizen_egl.h"

#define YAGL_WAYLAND_DPY(os_dpy) ((struct wl_display*)(os_dpy))

struct wl_drm;

struct yagl_tizen_display
{
	struct yagl_native_display base;

	int own_dpy;

	tpl_display_t *tpl_display;

	struct wl_drm *wl_drm;

	char *drm_dev_name;

	int drm_fd;

	int authenticated;
};

struct yagl_native_display
    *yagl_tizen_display_create(struct yagl_native_platform *platform,
                                 yagl_os_display os_dpy);


#endif
