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

#include "yagl_x11_drawable.h"
#include "yagl_x11_display.h"
#include "yagl_x11_image.h"
#include "yagl_malloc.h"
#include "yagl_dri2.h"
#include "yagl_utils.h"
#include "yagl_log.h"
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

static int yagl_x11_drawable_get_buffer(struct yagl_native_drawable *drawable,
                                        yagl_native_attachment attachment,
                                        uint32_t *buffer_name,
                                        struct vigs_drm_surface **buffer_sfc)
{
    Display *x_dpy = YAGL_X11_DPY(drawable->dpy->os_dpy);
    Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->os_drawable);
    unsigned int attachments[1];
    yagl_DRI2Buffer *buffer;
    int tmp_width, tmp_height, num_buffers;

    YAGL_LOG_FUNC_SET(yagl_x11_drawable_get_buffer);

    if (!drawable->dpy->drm_dev) {
        fprintf(stderr, "Asking for DRI2 buffer when DRI2 not enabled!\n");
        return 0;
    }

    switch (attachment) {
    case yagl_native_attachment_front:
        attachments[0] = DRI2BufferFrontLeft;
        break;
    case yagl_native_attachment_back:
        attachments[0] = DRI2BufferBackLeft;
        break;
    default:
        YAGL_LOG_ERROR("Bad attachment %u", attachment);
        return 0;
    }

    buffer = yagl_DRI2GetBuffers(x_dpy, x_drawable,
                                 &tmp_width, &tmp_height,
                                 &attachments[0],
                                 sizeof(attachments)/sizeof(attachments[0]),
                                 &num_buffers);

    if (!buffer) {
        YAGL_LOG_ERROR("Cannot get buffer for drawable %p",
                       drawable->os_drawable);
        return 0;
    }

    *buffer_name = buffer->name;

    Xfree(buffer);

    return 1;
}

static int yagl_x11_drawable_get_buffer_age(struct yagl_native_drawable *drawable)
{
    return 0;
}

static void yagl_x11_drawable_swap_buffers(struct yagl_native_drawable *drawable)
{
    Display *x_dpy = YAGL_X11_DPY(drawable->dpy->os_dpy);
    Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->os_drawable);
    CARD64 count = 0;

    if (!drawable->dpy->drm_dev) {
        return;
    }

    yagl_DRI2SwapBuffers(x_dpy,
                         x_drawable, 0, 0, 0,
                         &count);
}

static void yagl_x11_drawable_wait(struct yagl_native_drawable *drawable,
                                   uint32_t width,
                                   uint32_t height)
{
    struct yagl_x11_drawable *x11_drawable = (struct yagl_x11_drawable*)drawable;
    Display *x_dpy = YAGL_X11_DPY(drawable->dpy->os_dpy);
    Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->os_drawable);
    XRectangle xrect;
    XserverRegion region;

    if (!x11_drawable->is_pixmap || !drawable->dpy->drm_dev) {
        return;
    }

    xrect.x = 0;
    xrect.y = 0;
    xrect.width = width;
    xrect.height = height;

    region = XFixesCreateRegion(x_dpy, &xrect, 1);
    yagl_DRI2CopyRegion(x_dpy,
                        x_drawable,
                        region,
                        DRI2BufferFakeFrontLeft,
                        DRI2BufferFrontLeft);
    XFixesDestroyRegion(x_dpy, region);
}

static void yagl_x11_drawable_copy_to_pixmap(struct yagl_native_drawable *drawable,
                                             yagl_os_pixmap os_pixmap,
                                             uint32_t from_x,
                                             uint32_t from_y,
                                             uint32_t to_x,
                                             uint32_t to_y,
                                             uint32_t width,
                                             uint32_t height)
{
    Display *x_dpy = YAGL_X11_DPY(drawable->dpy->os_dpy);
    Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->os_drawable);
    Pixmap x_pixmap = YAGL_X11_DRAWABLE(os_pixmap);
    GC x_gc;

    x_gc = XCreateGC(x_dpy, x_pixmap, 0, NULL);

    if (x_gc) {
        XCopyArea(x_dpy,
                  x_drawable,
                  x_pixmap,
                  x_gc,
                  from_x, from_y,
                  width,
                  height,
                  to_x, to_y);

        XFreeGC(x_dpy, x_gc);

        XSync(x_dpy, 0);
    }
}

static void yagl_x11_drawable_set_swap_interval(struct yagl_native_drawable *drawable,
                                                int interval)
{
    struct yagl_x11_drawable *x11_drawable = (struct yagl_x11_drawable*)drawable;
    Display *x_dpy = YAGL_X11_DPY(drawable->dpy->os_dpy);
    Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->os_drawable);

    if (x11_drawable->is_pixmap || !drawable->dpy->drm_dev) {
        return;
    }

    yagl_DRI2SwapInterval(x_dpy, x_drawable, interval);
}

static void yagl_x11_drawable_get_geometry(struct yagl_native_drawable *drawable,
                                           uint32_t *width,
                                           uint32_t *height,
                                           uint32_t *depth)
{
    struct yagl_x11_drawable *x11_drawable = (struct yagl_x11_drawable*)drawable;
    Display *x_dpy = YAGL_X11_DPY(drawable->dpy->os_dpy);
    Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->os_drawable);
    union { Window w; int i; unsigned int ui; } tmp_geom;

    if (x11_drawable->is_pixmap) {
        pthread_mutex_lock(&x11_drawable->mtx);

        if (!x11_drawable->is_geom_acquired) {
            XGetGeometry(x_dpy,
                         x_drawable,
                         &tmp_geom.w,
                         &tmp_geom.i,
                         &tmp_geom.i,
                         &x11_drawable->width,
                         &x11_drawable->height,
                         &tmp_geom.ui,
                         &x11_drawable->depth);

            x11_drawable->is_geom_acquired = 1;
        }

        pthread_mutex_unlock(&x11_drawable->mtx);

        *width = x11_drawable->width;
        *height = x11_drawable->height;
        *depth = x11_drawable->depth;
    } else {
        XGetGeometry(x_dpy,
                     x_drawable,
                     &tmp_geom.w,
                     &tmp_geom.i,
                     &tmp_geom.i,
                     width,
                     height,
                     &tmp_geom.ui,
                     depth);
    }
}

static struct yagl_native_image
    *yagl_x11_drawable_get_image(struct yagl_native_drawable *drawable,
                                 uint32_t width,
                                 uint32_t height)
{
    Display *x_dpy = YAGL_X11_DPY(drawable->dpy->os_dpy);
    Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->os_drawable);
    XImage *x_image;
    struct yagl_native_image *image;

    YAGL_LOG_FUNC_SET(yagl_x11_drawable_get_image);

    x_image = XGetImage(x_dpy,
                        x_drawable,
                        0,
                        0,
                        width,
                        height,
                        AllPlanes,
                        ZPixmap);

    if (!x_image) {
        YAGL_LOG_ERROR("XGetImage failed for drawable %p",
                       drawable->os_drawable);
        return NULL;
    }

    image = yagl_x11_image_wrap(drawable->dpy, x_image);

    if (!image) {
        YAGL_LOG_ERROR("yagl_x11_image_wrap failed for drawable %p",
                       drawable->os_drawable);
        XDestroyImage(x_image);
        return NULL;
    }

    return image;
}

static void yagl_x11_drawable_destroy(struct yagl_native_drawable *drawable)
{
    struct yagl_x11_drawable *x11_drawable = (struct yagl_x11_drawable*)drawable;
    Display *x_dpy = YAGL_X11_DPY(drawable->dpy->os_dpy);
    Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->os_drawable);

    if (x11_drawable->x_gc) {
        XFreeGC(x_dpy, x11_drawable->x_gc);
    }

    if (drawable->dpy->drm_dev) {
        yagl_DRI2DestroyDrawable(x_dpy, x_drawable);
    }

    pthread_mutex_destroy(&x11_drawable->mtx);

    if (x11_drawable->own_drawable) {
        if (x11_drawable->is_pixmap) {
            XFreePixmap(x_dpy, x_drawable);
        } else {
            XDestroyWindow(x_dpy, x_drawable);
        }
    }

    yagl_native_drawable_cleanup(drawable);

    yagl_free(x11_drawable);
}

struct yagl_native_drawable *yagl_x11_drawable_create(struct yagl_native_display *dpy,
                                                      yagl_os_drawable os_drawable,
                                                      int own_drawable,
                                                      int is_pixmap)
{
    Display *x_dpy = YAGL_X11_DPY(dpy->os_dpy);
    Drawable x_drawable = YAGL_X11_DRAWABLE(os_drawable);
    struct yagl_x11_drawable *drawable;

    drawable = yagl_malloc0(sizeof(*drawable));

    yagl_native_drawable_init(&drawable->base,
                              dpy,
                              os_drawable);

    drawable->base.get_buffer = &yagl_x11_drawable_get_buffer;
    drawable->base.get_buffer_age = &yagl_x11_drawable_get_buffer_age;
    drawable->base.swap_buffers = &yagl_x11_drawable_swap_buffers;
    drawable->base.wait = &yagl_x11_drawable_wait;
    drawable->base.copy_to_pixmap = &yagl_x11_drawable_copy_to_pixmap;
    drawable->base.set_swap_interval = &yagl_x11_drawable_set_swap_interval;
    drawable->base.get_geometry = &yagl_x11_drawable_get_geometry;
    drawable->base.get_image = &yagl_x11_drawable_get_image;
    drawable->base.destroy = &yagl_x11_drawable_destroy;

    drawable->own_drawable = own_drawable;
    drawable->is_pixmap = is_pixmap;
    yagl_mutex_init(&drawable->mtx);

    if (dpy->drm_dev) {
        yagl_DRI2CreateDrawable(x_dpy, x_drawable);
    }

    return &drawable->base;
}

GC yagl_x11_drawable_get_gc(struct yagl_x11_drawable *drawable)
{
    pthread_mutex_lock(&drawable->mtx);

    if (!drawable->x_gc) {
        Display *x_dpy = YAGL_X11_DPY(drawable->base.dpy->os_dpy);
        Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->base.os_drawable);

        drawable->x_gc = XCreateGC(x_dpy, x_drawable, 0, NULL);
    }

    pthread_mutex_unlock(&drawable->mtx);

    return drawable->x_gc;
}
