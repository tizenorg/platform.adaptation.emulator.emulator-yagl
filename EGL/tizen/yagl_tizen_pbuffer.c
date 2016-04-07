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

#include "yagl_tizen_pbuffer.h"
#include "yagl_native_display.h"
#include "yagl_native_drawable.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "vigs.h"
#include <assert.h>
#include <string.h>

/* This file is almost the same as wayland backend, not necessary to call tpl */

#define YAGL_TIZEN_PBUFFER(os_pbuffer) ((struct vigs_drm_surface *)(os_pbuffer))

static int yagl_tizen_pbuffer_get_buffer(struct yagl_native_drawable *drawable,
                                           yagl_native_attachment attachment,
                                           uint32_t *buffer_name,
                                           struct vigs_drm_surface **buffer_sfc)
{
    struct vigs_drm_surface *drm_sfc = YAGL_TIZEN_PBUFFER(drawable->os_drawable);

    YAGL_LOG_FUNC_SET(yagl_tizen_pbuffer_get_buffer);

    switch (attachment) {
    case yagl_native_attachment_front:
        break;
    case yagl_native_attachment_back:
    default:
        YAGL_LOG_ERROR("Bad attachment %u", attachment);
        return 0;
    }

    vigs_drm_gem_ref(&drm_sfc->gem);

    *buffer_sfc = drm_sfc;

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
    struct vigs_drm_surface *drm_sfc = YAGL_TIZEN_PBUFFER(drawable->os_drawable);

    *width = drm_sfc->width;
    *height = drm_sfc->height;

    switch (drm_sfc->format) {
    case vigs_drm_surface_bgrx8888:
        *depth = 24;
        break;
    case vigs_drm_surface_bgra8888:
        *depth = 32;
        break;
    default: /* should never happen */
        assert(NULL);
        break;
    }
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
    struct vigs_drm_surface *drm_sfc = YAGL_TIZEN_PBUFFER(drawable->os_drawable);

    vigs_drm_gem_unref(&drm_sfc->gem);

    yagl_native_drawable_cleanup(drawable);

    yagl_free(drawable);
}

struct yagl_native_drawable
    *yagl_tizen_pbuffer_create(struct yagl_native_display *dpy,
                                 uint32_t width,
                                 uint32_t height,
                                 uint32_t depth)
{
    vigs_drm_surface_format format;
    struct vigs_drm_surface *drm_sfc;
    struct yagl_native_drawable *pbuffer;
    int ret;

    YAGL_LOG_FUNC_SET(yagl_tizen_pbuffer_create);

    switch (depth) {
    case 24:
        format = vigs_drm_surface_bgrx8888;
        break;
    case 32:
        format = vigs_drm_surface_bgra8888;
        break;
    default:
        YAGL_LOG_ERROR("Bad depth value: %u", depth);
        return NULL;
    }

    ret = vigs_drm_surface_create(dpy->drm_dev,
                                  width,
                                  height,
                                  width * 4, /* stride */
                                  format,
                                  0, /* scanout */
                                  &drm_sfc);

    if (ret) {
        YAGL_LOG_ERROR("vigs_drm_surface_create failed: %s", strerror(-ret));
        return NULL;
    }

    pbuffer = yagl_malloc0(sizeof(*pbuffer));

    yagl_native_drawable_init(pbuffer, dpy, (yagl_os_drawable)drm_sfc);

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
