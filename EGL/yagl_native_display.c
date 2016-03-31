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

#include "yagl_native_display.h"
#include "yagl_log.h"
#include "yagl_backend.h"
#ifdef YAGL_PLATFORM_WAYLAND
#include "wayland-drm.h"
#endif
#ifdef YAGL_PLATFORM_TIZEN
#include "yagl_tizen_egl.h"
#include <tpl.h>
#endif
#include "vigs.h"
#include "EGL/eglext.h"
#include "EGL/eglmesaext.h"
#include <string.h>
#include <stdlib.h>

void yagl_native_display_init(struct yagl_native_display *dpy,
                              struct yagl_native_platform *platform,
                              yagl_os_display os_dpy,
                              struct vigs_drm_device *drm_dev,
                              const char *drm_dev_name)
{
    dpy->platform = platform;
    dpy->os_dpy = os_dpy;
    dpy->drm_dev = drm_dev;
    if (drm_dev && drm_dev_name) {
        dpy->drm_dev_name = strdup(drm_dev_name);
    } else {
        dpy->drm_dev_name = NULL;
    }
#ifdef YAGL_PLATFORM_WAYLAND
    dpy->WL_bind_wayland_display_supported = (drm_dev ? 1 : 0);
#elif YAGL_PLATFORM_TIZEN
    /* TODO: do we need to handle X case? */
    dpy->WL_bind_wayland_display_supported = (drm_dev ? 1 : 0);
#else
    dpy->WL_bind_wayland_display_supported = 0;
#endif
}

void yagl_native_display_cleanup(struct yagl_native_display *dpy)
{
    dpy->drm_dev = NULL;
    if (dpy->drm_dev_name)
        free(dpy->drm_dev_name);
    dpy->drm_dev_name = NULL;
}

#ifdef YAGL_PLATFORM_WAYLAND
static int yagl_native_display_wl_authenticate(void *user_data,
                                               uint32_t id)
{
    struct yagl_native_display *dpy = user_data;

    return dpy->authenticate(dpy, id);
}

static struct vigs_drm_surface
    *yagl_native_display_wl_acquire_buffer(void *user_data, uint32_t name)
{
    struct yagl_native_display *dpy = user_data;
    struct vigs_drm_surface *drm_sfc;
    int ret;

    YAGL_LOG_FUNC_SET(yagl_native_display_wl_acquire_buffer);

    ret = vigs_drm_surface_open(dpy->drm_dev, name, &drm_sfc);

    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_surface_open failed for name %u: %s",
                       name,
                       strerror(-ret));
        return NULL;
    }

    return drm_sfc;
}

static struct wayland_drm_callbacks wl_drm_callbacks =
{
    .authenticate = yagl_native_display_wl_authenticate,
    .acquire_buffer = yagl_native_display_wl_acquire_buffer,
};

int yagl_native_display_bind_wl_display(struct yagl_native_display *dpy,
                                        struct wl_display *wl_dpy)
{
    if (dpy->wl_server_drm) {
        return 0;
    }

    dpy->wl_server_drm = wayland_drm_create(wl_dpy,
                                            dpy->drm_dev_name,
                                            &wl_drm_callbacks,
                                            dpy);

    if (dpy->wl_server_drm) {
        dpy->update_wl_server(dpy);
    }

    return dpy->wl_server_drm ? 1 : 0;
}

int yagl_native_display_unbind_wl_display(struct yagl_native_display *dpy)
{
    if (!dpy->wl_server_drm) {
        return 0;
    }

    wayland_drm_destroy(dpy->wl_server_drm);
    dpy->wl_server_drm = NULL;

    dpy->update_wl_server(dpy);
    return 1;
}

int yagl_native_display_query_wl_buffer(struct yagl_native_display *dpy,
                                        struct wl_resource *buffer,
                                        EGLint attribute,
                                        EGLint *value)
{
    struct wl_drm_buffer *drm_buffer = wayland_drm_get_buffer(dpy->wl_server_drm, buffer);
    struct vigs_drm_surface *drm_sfc;

    if (!drm_buffer) {
        return 0;
    }

    drm_sfc = wayland_drm_buffer_get_sfc(drm_buffer);

	switch (attribute) {
		case EGL_TEXTURE_FORMAT:
			switch (drm_sfc->format) {
				case vigs_drm_surface_bgrx8888:
					*value = EGL_TEXTURE_RGB;
					break;
				case vigs_drm_surface_bgra8888:
					*value = EGL_TEXTURE_RGBA;
					break;
				default:
					return 0;
			}
			break;
		case EGL_WIDTH:
			*value = drm_sfc->width;
			break;
		case EGL_HEIGHT:
			*value = drm_sfc->height;
			break;
		case EGL_WAYLAND_Y_INVERTED_WL:
			*value = yagl_get_backend()->y_inverted;
			break;
		default:
		return 0;
	}

	return 1;
}
#endif

#ifdef YAGL_PLATFORM_TIZEN
int yagl_native_display_bind_wl_display(yagl_os_display dpy)
{
	tpl_display_t *tpl_display = tpl_display_get((tpl_handle_t)dpy);

	if (tpl_display)
		return EGL_TRUE;
	else
		return EGL_FALSE;
}

int yagl_native_display_unbind_wl_display(yagl_os_display dpy)
{
	tpl_display_t *tpl_display = tpl_display_get((tpl_handle_t)dpy);

	if (tpl_display)
		return EGL_TRUE;
	else
		return EGL_TRUE;
}

int yagl_native_display_query_wl_buffer(yagl_os_display dpy,
                                        struct wl_resource *buffer,
                                        EGLint attribute,
                                        EGLint *value)
{
	tpl_display_t *tpl_display = tpl_display_get((tpl_handle_t)dpy);
	tbm_format format = 0;
	tpl_result_t ret;
	int width = 0, height = 0;

	if ( (ret=tpl_display_get_native_pixmap_info(tpl_display,
					(tpl_handle_t)buffer, &width, &height, &format)) != TPL_ERROR_NONE )
	{
		YAGL_LOG_ERROR("%s: get pixmap info failed\n", __func__);
		return 0;
	}

	switch (attribute) {
		case EGL_TEXTURE_FORMAT:
			switch (format) {
				case TBM_FORMAT_ARGB8888:
				case TBM_FORMAT_BGRA8888:
					*value = EGL_TEXTURE_RGBA;
					break;
				case TBM_FORMAT_XRGB8888:
				case TBM_FORMAT_BGRX8888:
				case TBM_FORMAT_RGB565:
					*value = EGL_TEXTURE_RGB;
					break;

				default:
					return 0;
			}
			break;
		case EGL_WIDTH:
			*value = width;
			break;
		case EGL_HEIGHT:
			*value = height;
			break;
		case EGL_WAYLAND_Y_INVERTED_WL:
			*value = 1;
			break;
		default:
			return 0;
    }

	return 1;
}
#endif
