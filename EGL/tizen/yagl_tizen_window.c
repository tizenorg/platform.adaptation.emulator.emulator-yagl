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

#include "yagl_tizen_window.h"
#include "yagl_tizen_display.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include <tbm_bufmgr_backend.h>
#include "vigs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


static int yagl_tizen_window_get_buffer(struct yagl_native_drawable *drawable,
                                          yagl_native_attachment attachment,
                                          uint32_t *buffer_name,
                                          struct vigs_drm_surface **buffer_sfc)
{
    struct yagl_tizen_window *window = (struct yagl_tizen_window*)drawable;
    int width, height;
    tbm_bo bo;
    tbm_surface_h tbm_surface;
    int i;

    YAGL_LOG_FUNC_SET(yagl_tizen_window_get_buffer);

    switch (attachment) {
        case yagl_native_attachment_back:
            break;
        case yagl_native_attachment_front:
        default:
            YAGL_LOG_ERROR("Bad attachment %u", attachment);
        return 0;
    }

    tbm_surface = tpl_surface_dequeue_buffer(window->surface);
    if (tbm_surface == NULL) {
        YAGL_LOG_ERROR("can't get buffer for %p", window->surface);
        return 0;
    }
    YAGL_LOG_INFO("get buffer %p for %p", tbm_surface, window->surface);
    tbm_surface_internal_ref(tbm_surface);
    width = tbm_surface_get_width(tbm_surface);
    height = tbm_surface_get_height(tbm_surface);

    if ((window->width != width) ||
    (window->height != height)) {
        for (i = 0; i < YAGL_TIZEN_MAX_COLOR_BUF; i++) {
            if (window->color_buffers[i].locked && window->back != &window->color_buffers[i]) {
                /*
                 * Buffer is locked and it's not a back buffer.
                 */
                continue;
            }
            if(window->color_buffers[i].data) {
				tbm_surface_internal_unref(window->color_buffers[i].data);
				window->color_buffers[i].data =  NULL;
            }
			if (window->back == &window->color_buffers[i]) {
                /*
                 * If it's a back buffer and the window was resized
                 * then we MUST destroy it and create a new one
                 * later.
                 *
                 * Otherwise, we'll get very obscure resizing bugs.
                 */
                window->color_buffers[i].locked = 0;
                window->back = NULL;
            }
        }

        window->width = width;
        window->height = height;
    }

    if (!window->back) {
        int cur;
        for (i = 1; i <= YAGL_TIZEN_MAX_COLOR_BUF;i++) {
            cur = (i + window->current_buf >= YAGL_TIZEN_MAX_COLOR_BUF) ?
                (i + window->current_buf)%YAGL_TIZEN_MAX_COLOR_BUF : i + window->current_buf;

            if (window->color_buffers[cur].locked) {
                continue;
            }
            if (!window->back || !window->back->data) {
                window->back = &window->color_buffers[cur];
                window->current_buf = cur;
            }
        }
    }

    if (!window->back) {
        return 0;
    }

    bo = tbm_surface_internal_get_bo(tbm_surface, 0);

    if (buffer_sfc != NULL) {
        *buffer_sfc = (struct vigs_drm_surface *)tbm_backend_get_bo_priv(bo);

        if (vigs_drm_gem_get_name(&(*buffer_sfc)->gem))
                    YAGL_LOG_ERROR("%s: get gem name failed\n", __func__);

        vigs_drm_gem_ref(&(*buffer_sfc)->gem);

        if (buffer_name)
            *buffer_name = (*buffer_sfc)->gem.name;
    }

    window->back->age = 0;
    window->back->locked = 1;
    window->back->data = (void *)tbm_surface;

    return 1;
}

static int yagl_tizen_window_get_buffer_age(struct yagl_native_drawable *drawable)
{
    struct yagl_tizen_window *window = (struct yagl_tizen_window*)drawable;

    return window->back ? window->back->age : 0;
}

static void yagl_tizen_window_swap_buffers(struct yagl_native_drawable *drawable)
{
    struct yagl_tizen_window *window = (struct yagl_tizen_window*)drawable;
    tpl_result_t ret = TPL_ERROR_INVALID_OPERATION;
    int i;

    YAGL_LOG_FUNC_SET(yagl_tizen_window_swap_buffers);

    for (i = 0; i < YAGL_TIZEN_MAX_COLOR_BUF; ++i) {
        if (window->color_buffers[i].age > 0) {
            ++window->color_buffers[i].age;
        }
    }

    /*
     * Make sure we have a back buffer in case we're swapping without ever
     * rendering.
     */
    if (!window->back || !window->back->data) {
        yagl_tizen_window_get_buffer(drawable, yagl_native_attachment_back, NULL, NULL);
        if (!window->back || !window->back->data) {
            YAGL_LOG_ERROR("Cannot lock back for egl_window %p\n", window);
            return;
        }
    }

    ret = tpl_surface_enqueue_buffer(window->surface, window->back->data);
    if (ret != TPL_ERROR_NONE)
        YAGL_LOG_ERROR("post failed\n");

    window->back->age = 1;
    window->back->locked = 0;
    tbm_surface_internal_unref(window->back->data);
    window->back->data = NULL;
    window->back = NULL;

    ++drawable->stamp;
}

static void yagl_tizen_window_wait(struct yagl_native_drawable *drawable,
                                     uint32_t width,
                                     uint32_t height)
{
}

static void yagl_tizen_window_copy_to_pixmap(struct yagl_native_drawable *drawable,
                                               yagl_os_pixmap os_pixmap,
                                               uint32_t from_x,
                                               uint32_t from_y,
                                               uint32_t to_x,
                                               uint32_t to_y,
                                               uint32_t width,
                                               uint32_t height)
{
}

static void yagl_tizen_window_set_swap_interval(struct yagl_native_drawable *drawable,
                                                  int interval)
{
}

static void yagl_tizen_window_get_geometry(struct yagl_native_drawable *drawable,
                                             uint32_t *width,
                                             uint32_t *height,
                                             uint32_t *depth)
{
}

static struct yagl_native_image
    *yagl_tizen_window_get_image(struct yagl_native_drawable *drawable,
                                   uint32_t width,
                                   uint32_t height)
{
    return NULL;
}

static void yagl_tizen_window_destroy(struct yagl_native_drawable *drawable)
{
    struct yagl_tizen_window *window = (struct yagl_tizen_window *)drawable;

    yagl_native_drawable_cleanup(drawable);

    window->user_data = NULL;

    /* Destroy TPL surface. */
    if (window->surface)
        tpl_object_unreference((tpl_object_t *)window->surface);

    yagl_free(drawable);
}

struct yagl_native_drawable
    *yagl_tizen_window_create(struct yagl_native_display *dpy,
                                yagl_os_window os_window)
{
    struct yagl_tizen_window *window;
    tpl_display_t *tpl_display = NULL;
    tpl_surface_t *tpl_surface = NULL;

    window = yagl_malloc0(sizeof(*window));

    yagl_native_drawable_init(&window->base, dpy, os_window);

    tpl_display = tpl_display_get((tpl_handle_t)dpy->os_dpy);
    tpl_surface = tpl_surface_create(tpl_display,
                                     (tpl_handle_t)os_window,
                                     TPL_SURFACE_TYPE_WINDOW,
                                     TBM_FORMAT_ARGB8888);
    window->base.get_buffer = &yagl_tizen_window_get_buffer;
    window->base.get_buffer_age = &yagl_tizen_window_get_buffer_age;
    window->base.swap_buffers = &yagl_tizen_window_swap_buffers;
    window->base.wait = &yagl_tizen_window_wait;
    window->base.copy_to_pixmap = &yagl_tizen_window_copy_to_pixmap;
    window->base.set_swap_interval = &yagl_tizen_window_set_swap_interval;
    window->base.get_geometry = &yagl_tizen_window_get_geometry;
    window->base.get_image = &yagl_tizen_window_get_image;
    window->base.destroy = &yagl_tizen_window_destroy;

    /*
    * First comparison against real window
    * dimensions must always fail.
    */
    window->width = -1;
    window->height = -1;
    window->current_buf = 0;
    window->surface = tpl_surface;

    return &window->base;
}
