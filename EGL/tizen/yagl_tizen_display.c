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

#include "yagl_tizen_display.h"
#include "yagl_tizen_window.h"
#include "yagl_tizen_pbuffer.h"
#include <tbm_bufmgr_backend.h>
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "vigs.h"
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>



static int yagl_tizen_display_authenticate(struct yagl_native_display *dpy,
                                             uint32_t id)
{
	return 1;
}

static struct yagl_native_drawable
    *yagl_tizen_display_wrap_window(struct yagl_native_display *dpy,
                                      yagl_os_window os_window)
{
	return yagl_tizen_window_create(dpy, os_window);
}

static struct yagl_native_drawable
    *yagl_tizen_display_wrap_pixmap(struct yagl_native_display *dpy,
                                      yagl_os_pixmap os_pixmap)
{
    return NULL;
}

static struct yagl_native_drawable
    *yagl_tizen_display_create_pixmap(struct yagl_native_display *dpy,
                                        uint32_t width,
                                        uint32_t height,
                                        uint32_t depth)
{
    /*
     * Wayland actaully has no pixmap type. This is just for creating
     * a pbuffer and to make Tizen conformance tests happy.
     */
	return  yagl_tizen_pbuffer_create(dpy, width, height, depth);
}

static struct yagl_native_image
    *yagl_tizen_display_create_image(struct yagl_native_display *dpy,
                                       uint32_t width,
                                       uint32_t height,
                                       uint32_t depth)
{
	return NULL;
}

static int yagl_tizen_display_get_visual(struct yagl_native_display *dpy,
                                           int *visual_id,
                                           int *visual_type)
{
	tpl_display_t	*tpl_display = NULL;
	tpl_result_t	ret;

	YAGL_LOG_FUNC_SET(yagl_tizen_display_get_visual);

	tpl_display = tpl_display_get((tpl_handle_t)dpy->os_dpy);


	ret = tpl_display_query_config(tpl_display, TPL_SURFACE_TYPE_WINDOW,
						      8, 8, 8, 8, 32,
						      visual_id, NULL);
	if (ret != TPL_ERROR_NONE) {
		YAGL_LOG_ERROR("query error");
	}

	*visual_type = 0;

	return 1;
}

static void yagl_tizen_display_update_wl_server(struct yagl_native_display *dpy)
{
	return ;
}

static void yagl_tizen_display_destroy(struct yagl_native_display *dpy)
{
	struct yagl_tizen_display *tizen_dpy = (struct yagl_tizen_display *)dpy;
	if (tizen_dpy->tpl_display != NULL) {
		tpl_object_unreference((tpl_object_t *)tizen_dpy->tpl_display);
	}

	yagl_native_display_cleanup(dpy);

	yagl_free(dpy);
}

struct yagl_native_display
    *yagl_tizen_display_create(struct yagl_native_platform *platform,
                                 yagl_os_display os_dpy)
{
	struct yagl_tizen_display *dpy;
	tbm_bufmgr bufmgr;
	struct vigs_drm_device *drm_dev = NULL;

	YAGL_LOG_FUNC_ENTER(yagl_tizen_display_create, "os_dpy = %p", os_dpy);

	dpy = yagl_malloc0(sizeof(*dpy));

	dpy->drm_fd = -1;

	dpy->tpl_display = tpl_display_create(TPL_BACKEND_UNKNOWN, (tpl_handle_t)os_dpy);

	if (!dpy->tpl_display) {
		YAGL_LOG_ERROR("Failed to tpl_display");
		goto fail;
	}

	bufmgr = tbm_bufmgr_init(-1);
	drm_dev = (struct vigs_drm_device *)tbm_backend_get_priv_from_bufmgr(bufmgr);

	yagl_native_display_init(&dpy->base,
				platform,
				os_dpy,
				drm_dev,
				NULL);
	dpy->base.authenticate = &yagl_tizen_display_authenticate;
	dpy->base.wrap_window = &yagl_tizen_display_wrap_window;
	dpy->base.wrap_pixmap = &yagl_tizen_display_wrap_pixmap;
	dpy->base.create_pixmap = &yagl_tizen_display_create_pixmap;
	dpy->base.create_image = &yagl_tizen_display_create_image;
	dpy->base.get_visual = &yagl_tizen_display_get_visual;
	dpy->base.update_wl_server = &yagl_tizen_display_update_wl_server;
	dpy->base.destroy = &yagl_tizen_display_destroy;

	YAGL_LOG_FUNC_EXIT("display %p created", dpy);

	return &dpy->base;

fail:
	yagl_free(dpy);

	YAGL_LOG_FUNC_EXIT(NULL);

	return NULL;
}
