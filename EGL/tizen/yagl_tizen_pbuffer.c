/*
 * YaGL
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Vasiliy Ulyanov <v.ulyanov@samsung.com>
 * Jinhyung Jo <jinhyung.jo@samsung.com>
 * Sangho Park <sangho1206.park@samsung.com>
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

#include <tpl.h>
#include <tbm_surface.h>
#include <tbm_bufmgr_backend.h>
#include <tbm_surface_internal.h>
#include <tbm_surface.h>

#include "yagl_native_display.h"
#include "yagl_native_drawable.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_gbm.h"
#include "vigs.h"
#include <assert.h>
#include <string.h>



#define TBM_SURF 0xffff
 static int yagl_tizen_pbuffer_get_buffer(struct yagl_native_drawable *drawable,
					    yagl_native_attachment attachment,
					    uint32_t *buffer_name,
					    struct vigs_drm_surface **buffer_sfc)
{
	tpl_surface_t *tpl_surf = (tpl_surface_t*)(drawable->os_drawable);
	tbm_bo bo = NULL;
	tbm_surface_h tbm_surface = NULL;

	YAGL_LOG_FUNC_SET(yagl_wayland_pbuffer_get_buffer);

	switch (attachment)
	{
		case yagl_native_attachment_front:
			break;
		case yagl_native_attachment_back:
		default:
			YAGL_LOG_ERROR("Bad attachment %u", attachment);
		return 0;
	}

	tbm_surface = tpl_surface_dequeue_buffer(tpl_surf);
	if (tbm_surface == NULL)
	{
		YAGL_LOG_ERROR("tpl_surface_dequeue_buffer failed\n");
	}

	tpl_object_set_user_data((tpl_object_t *)tpl_surf,(void*)TBM_SURF,tbm_surface,NULL);

	bo = tbm_surface_internal_get_bo(tbm_surface, 0);
	if(bo == NULL)
	{
		YAGL_LOG_ERROR("tbm_surface_internal_get_bo failed\n");
		return 0;
	}

	/*vigs_drm_gem_ref(&drm_sfc->gem);*//*TODO:needed? */

	*buffer_sfc = tbm_backend_get_bo_priv(bo);

	return 1;
 }

static int yagl_tizen_pbuffer_get_buffer_age(struct yagl_native_drawable *drawable)
{
	return 0;
}

static void yagl_tizen_pbuffer_swap_buffers(struct yagl_native_drawable *drawable)
{
}

static void yagl_tizen_pbuffer_wait(struct yagl_native_drawable *drawable,
                                      uint32_t width,
                                      uint32_t height)
{
}

static void yagl_tizen_pbuffer_copy_to_pixmap(struct yagl_native_drawable *drawable,
                                                yagl_os_pixmap os_pixmap,
                                                uint32_t from_x,
                                                uint32_t from_y,
                                                uint32_t to_x,
                                                uint32_t to_y,
                                                uint32_t width,
                                                uint32_t height)
{
}

static void yagl_tizen_pbuffer_set_swap_interval(struct yagl_native_drawable *drawable,
                                                   int interval)
{
}

static void yagl_tizen_pbuffer_get_geometry(struct yagl_native_drawable *drawable,
                                              uint32_t *width,
                                              uint32_t *height,
                                              uint32_t *depth)
{
	tbm_surface_h tbm_surface = NULL;
	tbm_surface_info_s *info = NULL;
	tpl_surface_t *tpl_surf = (tpl_surface_t*)(drawable->os_drawable);
	if(NULL == tpl_surf)
	{
		YAGL_LOG_ERROR("tpl_surf == NULL\n");
	}

	tbm_surface = tpl_object_get_user_data((tpl_object_t *)tpl_surf,(void*)TBM_SURF);
	if(NULL == tbm_surface)
	{
		YAGL_LOG_ERROR("tbm_surface == NULL\n");
	}

	tbm_surface_get_info(tbm_surface, info);

	*width = info->width;
	*height = info->height;
	*depth = info->bpp;
}

static struct yagl_native_image
    *yagl_tizen_pbuffer_get_image(struct yagl_native_drawable *drawable,
                                    uint32_t width,
                                    uint32_t height)
{
	return NULL;
}

static void yagl_tizen_pbuffer_destroy(struct yagl_native_drawable *drawable)
{
	tbm_surface_h tbm_surface = NULL;
	tpl_surface_t *tpl_surf = (tpl_surface_t*)(drawable->os_drawable);
	if(NULL == tpl_surf)
	{
		YAGL_LOG_ERROR("tpl_surf == NULL\n");
	}

	tbm_surface = tpl_object_get_user_data((tpl_object_t *)tpl_surf,(void*)TBM_SURF);
	if(NULL == tbm_surface)
	{
		YAGL_LOG_ERROR("tbm_surface == NULL\n");
	}

	tbm_surface_destroy(tbm_surface);

	/*vigs_drm_gem_unref(&drm_sfc->gem);*//*TODO*/
	if(tpl_surf != NULL)
		tpl_object_unreference((tpl_object_t *)tpl_surf);

	yagl_native_drawable_cleanup(drawable);

	yagl_free(drawable);
}

struct yagl_native_drawable
     *yagl_tizen_pbuffer_create(struct yagl_native_display *dpy,
				  uint32_t width,
				  uint32_t height,
				  uint32_t depth)
{
	struct yagl_native_drawable *pbuffer;
	tpl_display_t *tpl_display = NULL;
	tpl_surface_t *tpl_surf = NULL;

	YAGL_LOG_FUNC_SET(yagl_wayland_pbuffer_create);

	tpl_display = tpl_display_get( (tpl_handle_t)dpy->os_dpy);
	if (tpl_display == NULL)
	{
		YAGL_LOG_ERROR("tpl_display_get failed\n");
	}

	tpl_surf = tpl_surface_create(tpl_display, (tpl_handle_t)dpy->os_dpy,TPL_SURFACE_TYPE_PIXMAP, TBM_FORMAT_ARGB8888);
	if (tpl_display == NULL)
	{
		YAGL_LOG_ERROR("tpl_surface_create failed\n");
	}

	pbuffer = yagl_malloc0(sizeof(*pbuffer));

	yagl_native_drawable_init(pbuffer, dpy, (yagl_os_drawable)tpl_surf);

	pbuffer->get_buffer = &yagl_tizen_pbuffer_get_buffer;
	pbuffer->get_buffer_age = &yagl_tizen_pbuffer_get_buffer_age;
	pbuffer->swap_buffers = &yagl_tizen_pbuffer_swap_buffers;
	pbuffer->wait = &yagl_tizen_pbuffer_wait;
	pbuffer->copy_to_pixmap = &yagl_tizen_pbuffer_copy_to_pixmap;
	pbuffer->set_swap_interval = &yagl_tizen_pbuffer_set_swap_interval;
	pbuffer->get_geometry = &yagl_tizen_pbuffer_get_geometry;
	pbuffer->get_image = &yagl_tizen_pbuffer_get_image;
	pbuffer->destroy = &yagl_tizen_pbuffer_destroy;

	return pbuffer;
}
