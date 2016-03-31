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

#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_display.h"
#include "yagl_tizen_egl.h"
#include "yagl_tizen_native_image.h"
#include "yagl_state.h"
#include "yagl_egl_state.h"
#include "yagl_client_image.h"
#include "yagl_host_egl_calls.h"
#include <tbm_bufmgr_backend.h>
#include "vigs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void yagl_onscreen_image_tizen_update(struct yagl_image *image)
{
}

static void yagl_onscreen_image_tizen_destroy(struct yagl_ref *ref)
{
	struct yagl_onscreen_image_tizen *image = (struct yagl_onscreen_image_tizen*)ref;

	if (image->buffer)
		tbm_surface_internal_unref(image->buffer);

	yagl_image_cleanup(&image->base);

	yagl_free(image);
}


struct yagl_onscreen_image_tizen
    *yagl_onscreen_image_tizen_create(struct yagl_display *dpy,
                                          EGLClientBuffer pixmap,
                                          struct yagl_client_interface *iface)
{
	EGLint error = 0;
	tpl_display_t *tpl_display;
	tbm_surface_h tbm_surface;
	tbm_bo bo;
	yagl_object_name tex_global_name = yagl_get_global_name();
	struct yagl_client_image *client_image;
	struct yagl_onscreen_image_tizen *image;
	struct vigs_drm_surface *drm_sfc;

	image = yagl_malloc0(sizeof(*image));

	tpl_display = tpl_display_get((tpl_handle_t)dpy->display_id);
	if (tpl_display == NULL)
	{
		yagl_set_error(EGL_BAD_DISPLAY);
		YAGL_LOG_ERROR("%s: tpl_display is NULL\n", __func__);
		goto fail;
	}

	/* Get native buffer from pixmap */
	tbm_surface = tpl_display_get_buffer_from_native_pixmap(tpl_display, pixmap);

	if (tbm_surface == NULL)
	{
		yagl_set_error(EGL_BAD_NATIVE_PIXMAP);
		YAGL_LOG_ERROR("%s: tbm_surface is NULL\n", __func__);
		goto fail;
	}
	tbm_surface_internal_ref(tbm_surface);

	bo = tbm_surface_internal_get_bo(tbm_surface, 0);
	drm_sfc = (struct vigs_drm_surface *)tbm_backend_get_bo_priv(bo);

	if (!yagl_host_eglCreateImageYAGL(tex_global_name,
									dpy->host_dpy,
									drm_sfc->id,
									&error))
	{
		yagl_set_error(error);
		goto fail;
	}

	client_image = iface->create_image(iface, tex_global_name);

	yagl_image_init(&image->base,
					&yagl_onscreen_image_tizen_destroy,
					dpy,
					(EGLImageKHR)INT2VOIDP(drm_sfc->gem.name),
					client_image);

	yagl_client_image_release(client_image);

	image->base.update = &yagl_onscreen_image_tizen_update;

	image->buffer = tbm_surface;

	return image;

fail:
	yagl_free(image);

	return NULL;
}
