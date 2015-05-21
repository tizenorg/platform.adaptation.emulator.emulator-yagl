/*
 * YaGL
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlib-xcb.h>
#include <X11/xshmfence.h>
#include <xcb/xcb.h>
#include <xcb/dri3.h>
#include <xcb/present.h>
#include <xf86drm.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "yagl_dri3.h"
#include "yagl_x11_drawable.h"
#include "yagl_x11_display.h"
#include "yagl_x11_image.h"
#include "yagl_malloc.h"
#include "yagl_log.h"
#include "vigs.h"

static inline void yagl_dri3_fence_reset(xcb_connection_t *c,
                                         struct yagl_dri3_buffer *buffer)
{
    xshmfence_reset(buffer->shm_fence);
}

static inline void yagl_dri3_fence_set(struct yagl_dri3_buffer *buffer)
{
    xshmfence_trigger(buffer->shm_fence);
}

static inline void yagl_dri3_fence_trigger(xcb_connection_t *c,
                                           struct yagl_dri3_buffer *buffer)
{
    xcb_sync_trigger_fence(c, buffer->sync_fence);
}

static inline void yagl_dri3_fence_await(xcb_connection_t *c,
                                         struct yagl_dri3_buffer *buffer)
{
    xcb_flush(c);
    xshmfence_await(buffer->shm_fence);
}

static inline Bool yagl_dri3_fence_triggered(struct yagl_dri3_buffer *buffer)
{
    return xshmfence_query(buffer->shm_fence);
}

/** yagl_dri3_alloc_render_buffer
 *
 * Allocate a render buffer and create an X pixmap from that
 *
 * Allocate an xshmfence for synchronization
 */
static struct yagl_dri3_buffer *yagl_dri3_alloc_render_buffer(struct yagl_dri3_drawable *drawable,
                                                              unsigned int format,
                                                              int width,
                                                              int height,
                                                              int depth)
{
    Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->base.os_drawable);
    Display *x_dpy = YAGL_X11_DPY(drawable->base.dpy->os_dpy);
    xcb_connection_t *c = XGetXCBConnection(x_dpy);
    struct yagl_dri3_buffer *buffer;
    struct vigs_drm_surface *sfc;
    xcb_pixmap_t pixmap;
    xcb_sync_fence_t sync_fence;
    xcb_void_cookie_t cookie;
    xcb_generic_error_t *error;
    struct xshmfence *shm_fence;
    int buffer_fd, fence_fd;
    int stride, cpp, ret;

    YAGL_LOG_FUNC_SET(yagl_dri3_alloc_render_buffer);

    fence_fd = xshmfence_alloc_shm();

    if (fence_fd < 0) {
        YAGL_LOG_ERROR("DRI3 Fence object allocation failure %s",
                       strerror(errno));
        return NULL;
    }

    shm_fence = xshmfence_map_shm(fence_fd);

    if (shm_fence == NULL) {
        YAGL_LOG_ERROR("DRI3 Fence object map failure %s",
                       strerror(errno));
        goto err_shm_fence;
    }

    buffer = yagl_malloc0(sizeof(*buffer));

    if (!buffer) {
        YAGL_LOG_ERROR("DRI3 Buffer object allocation failure %s",
                       strerror(errno));
        goto err_buffer;
    }

    switch (format) {
    case vigs_drm_surface_bgrx8888:
    case vigs_drm_surface_bgra8888:
        cpp = 4;
        stride = width * cpp;
        break;
    default:
        cpp = 0;
        break;
    }

    if (!cpp) {
        YAGL_LOG_ERROR("DRI3 buffer format %d invalid", format);
        goto err_format;
    }

    ret = vigs_drm_surface_create(drawable->base.dpy->drm_dev,
                                  width,
                                  height,
                                  stride,
                                  format,
                                  0, /* scanout */
                                  &sfc);

    if (ret) {
        YAGL_LOG_ERROR("DRI3 surface creation failure %s", strerror(errno));
        goto err_sfc;
    }

    /* Export allocated buffer */
    ret = vigs_drm_prime_export_fd(drawable->base.dpy->drm_dev,
                                   sfc,
                                   &buffer_fd);

    if (ret) {
        YAGL_LOG_ERROR("DRI3 fd export failure %s", strerror(errno));
        goto err_export;
    }

    /* Import fd at X side */
    cookie = xcb_dri3_pixmap_from_buffer_checked(c,
                                                 (pixmap = xcb_generate_id(c)),
                                                 x_drawable,
                                                 sfc->gem.size,
                                                 width,
                                                 height,
                                                 stride,
                                                 depth,
                                                 cpp * 8, /* bpp */
                                                 buffer_fd);
    error = xcb_request_check(c, cookie);

    if (error) {
        YAGL_LOG_ERROR("DRI3 pixmap_from_buffer failed (pixmap = %p, errcode = %d)",
                       (void *)pixmap, (int)error->error_code);
        free(error);
        goto err_export;
    }

    cookie = xcb_dri3_fence_from_fd_checked(c,
                                            pixmap,
                                            (sync_fence = xcb_generate_id(c)),
                                            false,
                                            fence_fd);
    error = xcb_request_check(c, cookie);

    if (error) {
        YAGL_LOG_ERROR("DRI3 fence_from_fd failed (pixmap = %p, errcode = %d)",
                       (void *)pixmap, (int)error->error_code);
        free(error);
        goto err_export;
    }

    buffer->pixmap = pixmap;
    buffer->own_pixmap = true;
    buffer->sync_fence = sync_fence;
    buffer->shm_fence = shm_fence;
    buffer->width = width;
    buffer->height = height;
    buffer->pitch = stride;
    buffer->sfc = sfc;

    /* Mark the buffer as idle
     */
    yagl_dri3_fence_set(buffer);

    return buffer;

err_export:
    vigs_drm_gem_unref(&sfc->gem);

err_sfc:
    /* nothing to do here */

err_format:
    yagl_free(buffer);

err_buffer:
    xshmfence_unmap_shm(shm_fence);

err_shm_fence:
    close(fence_fd);

    YAGL_LOG_ERROR("DRI3 alloc_render_buffer failed\n");

    return NULL;
}

/** yagl_dri3_free_render_buffer
 *
 * Free everything associated with one render buffer including pixmap, fence
 * stuff and the driver surface
 */
static void yagl_dri3_free_render_buffer(struct yagl_dri3_drawable *drawable,
                                         struct yagl_dri3_buffer *buffer)
{
    Display *x_dpy = YAGL_X11_DPY(drawable->base.dpy->os_dpy);
    xcb_connection_t *c = XGetXCBConnection(x_dpy);

    if (buffer->own_pixmap) {
        xcb_free_pixmap(c, buffer->pixmap);
    }

    if (buffer->sfc) {
        vigs_drm_gem_unref(&buffer->sfc->gem);
    }

    xcb_sync_destroy_fence(c, buffer->sync_fence);
    xshmfence_unmap_shm(buffer->shm_fence);

    yagl_free(buffer);
}

static void yagl_dri3_update_num_back(struct yagl_dri3_drawable *drawable)
{
    drawable->num_back = 1;

    if (drawable->flipping) {
        if (!drawable->is_pixmap && !(drawable->present_capabilities & XCB_PRESENT_CAPABILITY_ASYNC)) {
           drawable->num_back++;
        }

        drawable->num_back++;
    }

    if (drawable->swap_interval == 0) {
        drawable->num_back++;
    }
}

/*
 * Process one Present event
 */
static void yagl_dri3_handle_present_event(struct yagl_dri3_drawable *drawable,
                                           xcb_present_generic_event_t *ge)
{
    YAGL_LOG_FUNC_SET(yagl_dri3_handle_present_event);

    switch (ge->evtype) {
    case XCB_PRESENT_CONFIGURE_NOTIFY: {
        xcb_present_configure_notify_event_t *ce = (void *)ge;

        YAGL_LOG_DEBUG("XCB_PRESENT_CONFIGURE_NOTIFY: %dx%d => %dx%d",
                       drawable->width,
                       drawable->height,
                       ce->width,
                       ce->height);

        drawable->width = ce->width;
        drawable->height = ce->height;

        break;
    }
    case XCB_PRESENT_COMPLETE_NOTIFY: {
        xcb_present_complete_notify_event_t *ce = (void *)ge;

        YAGL_LOG_DEBUG("XCB_PRESENT_COMPLETE_NOTIFY");

        /* Compute the processed SBC number from the received 32-bit serial number merged
         * with the upper 32-bits of the sent 64-bit serial number while checking for
         * wrap
         */
        if (ce->kind == XCB_PRESENT_COMPLETE_KIND_PIXMAP) {
            drawable->recv_sbc = (drawable->send_sbc & 0xffffffff00000000LL) | ce->serial;
            if (drawable->recv_sbc > drawable->send_sbc) {
                drawable->recv_sbc -= 0x100000000;
            }

            switch (ce->mode) {
            case XCB_PRESENT_COMPLETE_MODE_FLIP:
                drawable->flipping = true;
                break;
            case XCB_PRESENT_COMPLETE_MODE_COPY:
                drawable->flipping = false;
                break;
            }

            yagl_dri3_update_num_back(drawable);

            drawable->ust = ce->ust;
            drawable->msc = ce->msc;
        } else {
            drawable->recv_msc_serial = ce->serial;
            drawable->notify_ust = ce->ust;
            drawable->notify_msc = ce->msc;
        }

        break;
    }
    case XCB_PRESENT_EVENT_IDLE_NOTIFY: {
        xcb_present_idle_notify_event_t *ie = (void *)ge;
        int b;

        YAGL_LOG_DEBUG("XCB_PRESENT_EVENT_IDLE_NOTIFY");

        for (b = 0; b < sizeof(drawable->buffers) / sizeof(drawable->buffers[0]); b++) {
            struct yagl_dri3_buffer *buf = drawable->buffers[b];

            if (buf && buf->pixmap == ie->pixmap) {
                buf->busy = 0;

                if (drawable->num_back <= b && b < DRI3_MAX_BACK) {
                   yagl_dri3_free_render_buffer(drawable, buf);
                   drawable->buffers[b] = NULL;
                }

                break;
            }
        }

        break;
    }
    }

    free(ge);
}

/** yagl_dri3_flush_present_events
 *
 * Process any present events that have been received from the X server
 */
static void yagl_dri3_flush_present_events(struct yagl_dri3_drawable *drawable)
{
    Display *x_dpy = YAGL_X11_DPY(drawable->base.dpy->os_dpy);
    xcb_connection_t *c = XGetXCBConnection(x_dpy);

    /* Check to see if any configuration changes have occurred
     * since we were last invoked
     */
    if (drawable->special_event) {
        xcb_generic_event_t *ev;

        while ((ev = xcb_poll_for_special_event(c, drawable->special_event)) != NULL) {
            xcb_present_generic_event_t *ge = (void *)ev;
            yagl_dri3_handle_present_event(drawable, ge);
        }
    }
}

/** yagl_dri3_update_drawable
 *
 * Called the first time we use the drawable and then
 * after we receive present configure notify events to
 * track the geometry of the drawable
 */
static int yagl_dri3_update_drawable(struct yagl_dri3_drawable *drawable)
{
    Display *x_dpy = YAGL_X11_DPY(drawable->base.dpy->os_dpy);
    xcb_connection_t *c = XGetXCBConnection(x_dpy);

    YAGL_LOG_FUNC_SET(yagl_dri3_update_drawable);

    /* First time through, go get the current drawable geometry
     */
    if (drawable->width == 0 || drawable->height == 0 || drawable->depth == 0) {
        xcb_get_geometry_cookie_t geom_cookie;
        xcb_get_geometry_reply_t *geom_reply;
        xcb_void_cookie_t cookie;
        xcb_generic_error_t *error;
        xcb_present_query_capabilities_cookie_t present_capabilities_cookie;
        xcb_present_query_capabilities_reply_t *present_capabilities_reply;
        Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->base.os_drawable);

        /* Try to select for input on the window.
         *
         * If the drawable is a window, this will get our events
         * delivered.
         *
         * Otherwise, we'll get a BadWindow error back from this request which
         * will let us know that the drawable is a pixmap instead.
         */

        cookie = xcb_present_select_input_checked(c,
                                                  (drawable->eid = xcb_generate_id(c)),
                                                  x_drawable,
                                                  XCB_PRESENT_EVENT_MASK_CONFIGURE_NOTIFY|
                                                  XCB_PRESENT_EVENT_MASK_COMPLETE_NOTIFY|
                                                  XCB_PRESENT_EVENT_MASK_IDLE_NOTIFY);

        present_capabilities_cookie = xcb_present_query_capabilities(c, x_drawable);

        /* Create an XCB event queue to hold present events outside of the usual
         * application event queue
         */
        drawable->special_event = xcb_register_for_special_xge(c,
                                                               &xcb_present_id,
                                                               drawable->eid,
                                                               &drawable->base.stamp);

        geom_cookie = xcb_get_geometry(c, x_drawable);
        geom_reply = xcb_get_geometry_reply(c, geom_cookie, NULL);

        if (!geom_reply) {
            return false;
        }

        drawable->width = geom_reply->width;
        drawable->height = geom_reply->height;
        drawable->depth = geom_reply->depth;
        drawable->is_pixmap = false;

        free(geom_reply);

        /* Check to see if our select input call failed. If it failed with a
         * BadWindow error, then assume the drawable is a pixmap. Destroy the
         * special event queue created above and mark the drawable as a pixmap
         */

        error = xcb_request_check(c, cookie);

        present_capabilities_reply = xcb_present_query_capabilities_reply(c,
                                                                          present_capabilities_cookie,
                                                                          NULL);

        if (present_capabilities_reply) {
            drawable->present_capabilities = present_capabilities_reply->capabilities;
            free(present_capabilities_reply);
        } else {
            drawable->present_capabilities = 0;
        }

        if (error) {
            if (error->error_code != BadWindow) {
                free(error);
                return false;
            }

            drawable->is_pixmap = true;
            xcb_unregister_for_special_event(c, drawable->special_event);
            drawable->special_event = NULL;
        }

        YAGL_LOG_DEBUG("%s 0x%X initial geometry: %ux%u %u",
                       drawable->is_pixmap ? "Pixmap": "Window",
                       x_drawable,
                       drawable->width,
                       drawable->height,
                       drawable->depth);
    }

    yagl_dri3_flush_present_events(drawable);

    return true;
}

static xcb_gcontext_t yagl_dri3_drawable_gc(struct yagl_dri3_drawable *drawable)
{
    if (!drawable->gc) {
        Display *x_dpy = YAGL_X11_DPY(drawable->base.dpy->os_dpy);
        Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->base.os_drawable);
        xcb_connection_t *c = XGetXCBConnection(x_dpy);
        uint32_t v = 0;

        xcb_create_gc(c,
                      (drawable->gc = xcb_generate_id(c)),
                      x_drawable,
                      XCB_GC_GRAPHICS_EXPOSURES,
                      &v);
    }

    return drawable->gc;
}

static struct yagl_dri3_buffer *yagl_dri3_back_buffer(struct yagl_dri3_drawable *drawable)
{
   return drawable->buffers[DRI3_BACK_ID(drawable->cur_back)];
}

static struct yagl_dri3_buffer *yagl_dri3_front_buffer(struct yagl_dri3_drawable *drawable)
{
   return drawable->buffers[DRI3_FRONT_ID];
}

static void yagl_dri3_copy_area(xcb_connection_t *c,
                                xcb_drawable_t src_drawable,
                                xcb_drawable_t dst_drawable,
                                xcb_gcontext_t gc,
                                int16_t src_x,
                                int16_t src_y,
                                int16_t dst_x,
                                int16_t dst_y,
                                uint16_t width,
                                uint16_t height)
{
   xcb_void_cookie_t cookie;

   cookie = xcb_copy_area_checked(c,
                                  src_drawable,
                                  dst_drawable,
                                  gc,
                                  src_x,
                                  src_y,
                                  dst_x,
                                  dst_y,
                                  width,
                                  height);
   xcb_discard_reply(c, cookie.sequence);
}

static void yagl_dri3_copy_drawable(struct yagl_dri3_drawable *drawable,
                                    Drawable dest,
                                    Drawable src)
{
    Display *x_dpy = YAGL_X11_DPY(drawable->base.dpy->os_dpy);
    xcb_connection_t *c = XGetXCBConnection(x_dpy);
    struct yagl_dri3_buffer *front = yagl_dri3_front_buffer(drawable);

    yagl_dri3_fence_reset(c, front);
    yagl_dri3_copy_area(c,
                        src,
                        dest,
                        yagl_dri3_drawable_gc(drawable),
                        0,
                        0,
                        0,
                        0,
                        drawable->width,
                        drawable->height);
    yagl_dri3_fence_trigger(c, front);
    yagl_dri3_fence_await(c, front);
}

static inline int yagl_dri3_pixmap_buf_id(enum yagl_dri3_buffer_type buffer_type)
{
   if (buffer_type == yagl_dri3_buffer_back) {
       return DRI3_BACK_ID(0);
   } else {
       return DRI3_FRONT_ID;
   }
}

/** yagl_dri3_get_pixmap_buffer
 *
 * Get the DRM object for a pixmap from the X server
 */
static struct yagl_dri3_buffer *yagl_dri3_get_pixmap_buffer(struct yagl_dri3_drawable *drawable,
                                                            unsigned int format,
                                                            enum yagl_dri3_buffer_type buffer_type)
{
    Display *x_dpy = YAGL_X11_DPY(drawable->base.dpy->os_dpy);
    xcb_connection_t *c = XGetXCBConnection(x_dpy);
    int buf_id = yagl_dri3_pixmap_buf_id(buffer_type);
    struct yagl_dri3_buffer *buffer = drawable->buffers[buf_id];
    Pixmap pixmap = YAGL_X11_DRAWABLE(drawable->base.os_drawable);
    xcb_dri3_buffer_from_pixmap_cookie_t bp_cookie;
    xcb_dri3_buffer_from_pixmap_reply_t  *bp_reply;
    xcb_sync_fence_t sync_fence;
    struct xshmfence *shm_fence;
    struct vigs_drm_surface *sfc;
    int fence_fd, *fds;
    int ret;

    YAGL_LOG_FUNC_SET(yagl_dri3_get_pixmap_buffer);

    if (buffer) {
        return buffer;
    }

    buffer = yagl_malloc0(sizeof(*buffer));

    if (!buffer) {
        YAGL_LOG_ERROR("DRI3 buffer object allocation failure");
        return NULL;
    }

    fence_fd = xshmfence_alloc_shm();

    if (fence_fd < 0) {
        YAGL_LOG_ERROR("DRI3 Fence object allocation failure %s",
                       strerror(errno));
        goto err_alloc_fence;
    }

    shm_fence = xshmfence_map_shm(fence_fd);

    if (shm_fence == NULL) {
        YAGL_LOG_ERROR("DRI3 Fence object map failure %s",
                       strerror(errno));
        goto err_map_shm;
    }

    xcb_dri3_fence_from_fd(c,
                           pixmap,
                           (sync_fence = xcb_generate_id(c)),
                           false,
                           fence_fd);

    /* Get an FD for the pixmap object
     */
    bp_cookie = xcb_dri3_buffer_from_pixmap(c, pixmap);
    bp_reply = xcb_dri3_buffer_from_pixmap_reply(c, bp_cookie, NULL);

    if (!bp_reply) {
        YAGL_LOG_ERROR("DRI3 buffer_from_pixmap failed");
        goto err_get_fd;
    }

    fds = xcb_dri3_buffer_from_pixmap_reply_fds(c, bp_reply);

    /* Import exported FD */
    ret = vigs_drm_prime_import_fd(drawable->base.dpy->drm_dev,
                                   fds[0],
                                   &sfc);

    if (ret) {
        YAGL_LOG_ERROR("DRI3 fd import failure %s", strerror(errno));
        goto err_import;
    }

    close(fds[0]);

    buffer->pixmap = pixmap;
    buffer->own_pixmap = false;
    buffer->sync_fence = sync_fence;
    buffer->shm_fence = shm_fence;
    buffer->width = bp_reply->width;
    buffer->height = bp_reply->height;
    buffer->pitch = bp_reply->stride;
    buffer->buffer_type = buffer_type;
    buffer->sfc = sfc;

    drawable->buffers[buf_id] = buffer;

    return buffer;

err_import:
    close(fds[0]);

err_get_fd:
    /* nothing to do here */

err_map_shm:
    close(fence_fd);

err_alloc_fence:
    yagl_free(buffer);

    return NULL;
}

/** yagl_dri3_find_back
 *
 * Find an idle back buffer. If there isn't one, then
 * wait for a present idle notify event from the X server
 */
static int yagl_dri3_find_back(xcb_connection_t *c,
                               struct yagl_dri3_drawable *drawable)
{
   int  b;
   xcb_generic_event_t *ev;
   xcb_present_generic_event_t *ge;

   for (;;) {
       for (b = 0; b < drawable->num_back; b++) {
           int id = DRI3_BACK_ID((b + drawable->cur_back) % drawable->num_back);
           struct yagl_dri3_buffer *buffer = drawable->buffers[id];

           if (!buffer || !buffer->busy) {
               drawable->cur_back = id;
               return id;
           }
       }

       xcb_flush(c);
       ev = xcb_wait_for_special_event(c, drawable->special_event);

       if (!ev) {
          return -1;
       }

       ge = (void *)ev;
       yagl_dri3_handle_present_event(drawable, ge);
   }
}

/** yagl_dri3_get_buffer
 *
 * Find a front or back buffer, allocating new ones as necessary
 */
static struct yagl_dri3_buffer *yagl_dri3_get_buffer(struct yagl_dri3_drawable *drawable,
                                                     unsigned int format,
                                                     enum yagl_dri3_buffer_type buffer_type)
{
    Display *x_dpy = YAGL_X11_DPY(drawable->base.dpy->os_dpy);
    xcb_connection_t *c = XGetXCBConnection(x_dpy);
    struct yagl_dri3_buffer *buffer;
    int buf_id;

    YAGL_LOG_FUNC_SET(yagl_dri3_get_buffer);

    if (buffer_type == yagl_dri3_buffer_back) {
        buf_id = yagl_dri3_find_back(c, drawable);

        if (buf_id < 0) {
            return NULL;
        }
    } else {
        buf_id = DRI3_FRONT_ID;
    }

    buffer = drawable->buffers[buf_id];

    if (!buffer ||
        buffer->width != drawable->width ||
        buffer->height != drawable->height) {
        struct yagl_dri3_buffer *new_buffer = yagl_dri3_alloc_render_buffer(drawable,
                                                                            format,
                                                                            drawable->width,
                                                                            drawable->height,
                                                                            drawable->depth);

        if (!new_buffer) {
            YAGL_LOG_ERROR("DRI3 buffer object allocation failure");
            return NULL;
        }

        /* When resizing, copy the contents of the old buffer, waiting for that
         * copy to complete using our fences before proceeding
         */
        switch (buffer_type) {
        case yagl_dri3_buffer_back:
            if (buffer) {
                yagl_dri3_fence_reset(c, new_buffer);
                yagl_dri3_fence_await(c, buffer);
                yagl_dri3_copy_area(c,
                                    buffer->pixmap,
                                    new_buffer->pixmap,
                                    yagl_dri3_drawable_gc(drawable),
                                    0,
                                    0,
                                    0,
                                    0,
                                    drawable->width,
                                    drawable->height);
                yagl_dri3_fence_trigger(c, new_buffer);
                yagl_dri3_free_render_buffer(drawable, buffer);
            }
            break;
        case yagl_dri3_buffer_front:
            yagl_dri3_fence_reset(c, new_buffer);
            yagl_dri3_copy_area(c,
                                YAGL_X11_DRAWABLE(drawable->base.os_drawable),
                                new_buffer->pixmap,
                                yagl_dri3_drawable_gc(drawable),
                                0,
                                0,
                                0,
                                0,
                                drawable->width,
                                drawable->height);
            yagl_dri3_fence_trigger(c, new_buffer);
            break;
        }


        buffer = new_buffer;
        buffer->buffer_type = buffer_type;
        drawable->buffers[buf_id] = buffer;
    }

    yagl_dri3_fence_await(c, buffer);

    return buffer;
}

static int yagl_dri3_drawable_get_buffer(struct yagl_native_drawable *drawable,
                                         yagl_native_attachment attachment,
                                         uint32_t *buffer_name,
                                         struct vigs_drm_surface **buffer_sfc)
{
    struct yagl_dri3_drawable *dri3_drawable = (struct yagl_dri3_drawable *)drawable;
    struct yagl_dri3_buffer *buffer;
    vigs_drm_surface_format format;

    YAGL_LOG_FUNC_SET(yagl_dri3_drawable_get_buffer);

    YAGL_LOG_DEBUG("enter: attachment = %d", attachment);

    if (!yagl_dri3_update_drawable(dri3_drawable)) {
        YAGL_LOG_ERROR("DRI3 drawable update failure");
        return 0;
    }

    switch (dri3_drawable->depth) {
    case 24:
        format = vigs_drm_surface_bgrx8888;
        break;
    case 32:
        format = vigs_drm_surface_bgra8888;
        break;
    default:
        YAGL_LOG_ERROR("DRI3 bad drawable depth %d", dri3_drawable->depth);
        return 0;
    }

    switch (attachment) {
    case yagl_native_attachment_front:
        buffer = yagl_dri3_get_pixmap_buffer(dri3_drawable,
                                             format,
                                             yagl_dri3_buffer_front);
        break;
    case yagl_native_attachment_back:
        buffer = yagl_dri3_get_buffer(dri3_drawable,
                                      format,
                                      yagl_dri3_buffer_back);
        break;
    default:
        YAGL_LOG_ERROR("DRI3 bad attachment %u", attachment);
        return 0;
    }

    if (!buffer) {
        YAGL_LOG_ERROR("DRI3 get_buffer failed for attachement %d", attachment);
        return 0;
    }

    vigs_drm_gem_ref(&buffer->sfc->gem);
    *buffer_sfc = buffer->sfc;

    return 1;
}

static int yagl_dri3_drawable_get_buffer_age(struct yagl_native_drawable *drawable)
{
    Display *x_dpy = YAGL_X11_DPY(drawable->dpy->os_dpy);
    xcb_connection_t *c = XGetXCBConnection(x_dpy);
    struct yagl_dri3_drawable *dri3_drawable = (struct yagl_dri3_drawable *)drawable;
    int back_id = DRI3_BACK_ID(yagl_dri3_find_back(c, dri3_drawable));
    struct yagl_dri3_buffer *back = dri3_drawable->buffers[back_id];

    YAGL_LOG_FUNC_SET(yagl_dri3_drawable_get_buffer_age);

    YAGL_LOG_DEBUG("enter");

    if (back_id < 0 || !back) {
        return 0;
    }

    if (back->last_swap != 0) {
        return dri3_drawable->send_sbc - back->last_swap + 1;
    } else {
        return 0;
    }
}

static void yagl_dri3_drawable_swap_buffers(struct yagl_native_drawable *drawable)
{
    Display *x_dpy = YAGL_X11_DPY(drawable->dpy->os_dpy);
    xcb_connection_t *c = XGetXCBConnection(x_dpy);
    struct yagl_dri3_drawable *dri3_drawable = (struct yagl_dri3_drawable *)drawable;
    struct yagl_dri3_buffer *back = yagl_dri3_back_buffer(dri3_drawable);
    int64_t target_msc = 0;
    int64_t divisor = 0;
    int64_t remainder = 0;

    YAGL_LOG_FUNC_SET(yagl_dri3_drawable_swap_buffers);

    yagl_dri3_flush_present_events(dri3_drawable);

    if (back && !dri3_drawable->is_pixmap) {
        yagl_dri3_fence_reset(c, back);

        ++dri3_drawable->send_sbc;

        target_msc = dri3_drawable->msc + dri3_drawable->swap_interval *
                     (dri3_drawable->send_sbc - dri3_drawable->recv_sbc);

        YAGL_LOG_DEBUG("msc = %llu, swap_interval = %d, "
                       "send_sbc = %llu, recv_sbc = %llu, "
                       "target_msc = %llu",
                       dri3_drawable->msc,
                       dri3_drawable->swap_interval,
                       dri3_drawable->send_sbc,
                       dri3_drawable->recv_sbc,
                       target_msc);

        back->busy = 1;
        back->last_swap = dri3_drawable->send_sbc;

        xcb_present_pixmap(c,
                           YAGL_X11_DRAWABLE(drawable->os_drawable), /* dst*/
                           back->pixmap, /* src */
                           (uint32_t)dri3_drawable->send_sbc, /* serial */
                           0, /* valid */
                           0, /* update */
                           0, /* x_off */
                           0, /* y_off */
                           None, /* target_crtc */
                           None, /* wait_fence */
                           back->sync_fence, /* idle_fence */
                           XCB_PRESENT_OPTION_NONE, /* options */
                           target_msc, /* target_msc */
                           divisor, /* divisor */
                           remainder, /* remainder */
                           0, /* notifies_len */
                           NULL /* notifies */);

        xcb_flush(c);

        drawable->stamp++;
    }
}

static void yagl_dri3_drawable_wait(struct yagl_native_drawable *drawable,
                                    uint32_t width,
                                    uint32_t height)
{
    struct yagl_dri3_drawable *dri3_drawable = (struct yagl_dri3_drawable *)drawable;
    struct yagl_dri3_buffer *front;

    YAGL_LOG_FUNC_SET(yagl_dri3_drawable_wait);

    YAGL_LOG_DEBUG("enter");

    if (!dri3_drawable->is_pixmap) {
        return;
    }

    front = yagl_dri3_front_buffer(dri3_drawable);

    yagl_dri3_copy_drawable(dri3_drawable,
                            front->pixmap,
                            YAGL_X11_DRAWABLE(drawable->os_drawable));
}

static void yagl_dri3_drawable_copy_to_pixmap(struct yagl_native_drawable *drawable,
                                              yagl_os_pixmap os_pixmap,
                                              uint32_t from_x,
                                              uint32_t from_y,
                                              uint32_t to_x,
                                              uint32_t to_y,
                                              uint32_t width,
                                              uint32_t height)
{
    struct yagl_dri3_drawable *dri3_drawable = (struct yagl_dri3_drawable *)drawable;
    struct yagl_dri3_buffer *front = yagl_dri3_front_buffer(dri3_drawable);
    Display *x_dpy = YAGL_X11_DPY(drawable->dpy->os_dpy);
    xcb_connection_t *c = XGetXCBConnection(x_dpy);
    xcb_gcontext_t gc;
    uint32_t v = 0;

    YAGL_LOG_FUNC_SET(yagl_dri3_drawable_copy_to_pixmap);

    YAGL_LOG_DEBUG("enter");

    xcb_create_gc(c,
                  (gc = xcb_generate_id(c)),
                  YAGL_X11_DRAWABLE(os_pixmap),
                  XCB_GC_GRAPHICS_EXPOSURES,
                  &v);

    yagl_dri3_fence_reset(c, front);
    yagl_dri3_copy_area(c,
                        YAGL_X11_DRAWABLE(drawable->os_drawable),
                        YAGL_X11_DRAWABLE(os_pixmap),
                        gc,
                        from_x,
                        from_y,
                        to_x,
                        to_y,
                        width,
                        height);
    yagl_dri3_fence_trigger(c, front);
    yagl_dri3_fence_await(c, front);

    xcb_free_gc(c, gc);
}

static void yagl_dri3_drawable_set_swap_interval(struct yagl_native_drawable *drawable,
                                                 int interval)
{
    struct yagl_dri3_drawable *dri3_drawable = (struct yagl_dri3_drawable *)drawable;

    YAGL_LOG_FUNC_SET(yagl_dri3_drawable_set_swap_interval);

    YAGL_LOG_DEBUG("is_pixmap = %d, interval = %d",
                   dri3_drawable->is_pixmap,
                   interval);

    if (dri3_drawable->is_pixmap) {
        return;
    }

    dri3_drawable->swap_interval = interval;
    yagl_dri3_update_num_back(dri3_drawable);
}

static void yagl_dri3_drawable_get_geometry(struct yagl_native_drawable *drawable,
                                            uint32_t *width,
                                            uint32_t *height,
                                            uint32_t *depth)
{
    struct yagl_dri3_drawable *dri3_drawable = (struct yagl_dri3_drawable *)drawable;

    YAGL_LOG_FUNC_SET(yagl_dri3_drawable_get_geometry);

    YAGL_LOG_DEBUG("enter");

    *width = dri3_drawable->width;
    *height = dri3_drawable->height;
    *depth = dri3_drawable->depth;
}

static struct yagl_native_image *yagl_dri3_drawable_get_image(struct yagl_native_drawable *drawable,
                                                              uint32_t width,
                                                              uint32_t height)
{
    return NULL;
}

static void yagl_dri3_drawable_destroy(struct yagl_native_drawable *drawable)
{
    struct yagl_dri3_drawable *dri3_drawable = (struct yagl_dri3_drawable *)drawable;
    Display *x_dpy = YAGL_X11_DPY(drawable->dpy->os_dpy);
    Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->os_drawable);
    xcb_connection_t *c = XGetXCBConnection(x_dpy);
    int i;

    YAGL_LOG_FUNC_SET(yagl_dri3_drawable_destroy);

    YAGL_LOG_DEBUG("enter");

    for (i = 0; i < DRI3_NUM_BUFFERS; i++) {
        if (dri3_drawable->buffers[i]) {
            yagl_dri3_free_render_buffer(dri3_drawable,
                                         dri3_drawable->buffers[i]);
        }
    }

    if (dri3_drawable->special_event) {
        xcb_unregister_for_special_event(c, dri3_drawable->special_event);
    }

    if (dri3_drawable->gc) {
        xcb_free_gc(c, dri3_drawable->gc);
    }

    if (dri3_drawable->own_drawable) {
        if (dri3_drawable->is_pixmap) {
            xcb_free_pixmap(c, x_drawable);
        } else {
            xcb_destroy_window(c, x_drawable);
        }
    }

    yagl_native_drawable_cleanup(drawable);

    yagl_free(dri3_drawable);
}

struct yagl_native_drawable *yagl_dri3_drawable_create(struct yagl_native_display *dpy,
                                                       yagl_os_drawable os_drawable,
                                                       int own_drawable,
                                                       int is_pixmap)
{
    struct yagl_dri3_drawable *drawable;

    YAGL_LOG_FUNC_SET(yagl_dri3_drawable_create);

    drawable = yagl_malloc0(sizeof(*drawable));

    if (!drawable) {
        YAGL_LOG_ERROR("DRI3 Drawable object allocation failure %s",
                       strerror(errno));
        return NULL;
    }

    yagl_native_drawable_init(&drawable->base,
                              dpy,
                              os_drawable);

    drawable->base.get_buffer = &yagl_dri3_drawable_get_buffer;
    drawable->base.get_buffer_age = &yagl_dri3_drawable_get_buffer_age;
    drawable->base.swap_buffers = &yagl_dri3_drawable_swap_buffers;
    drawable->base.wait = &yagl_dri3_drawable_wait;
    drawable->base.copy_to_pixmap = &yagl_dri3_drawable_copy_to_pixmap;
    drawable->base.set_swap_interval = &yagl_dri3_drawable_set_swap_interval;
    drawable->base.get_geometry = &yagl_dri3_drawable_get_geometry;
    drawable->base.get_image = &yagl_dri3_drawable_get_image;
    drawable->base.destroy = &yagl_dri3_drawable_destroy;

    drawable->swap_interval = 1; /* default */
    drawable->own_drawable = own_drawable;
    drawable->is_pixmap = is_pixmap;

    yagl_dri3_update_num_back(drawable);

    YAGL_LOG_DEBUG("os_drawable = %p, is_pixmap = %d, own_drawable = %d",
                   (void *)os_drawable,
                   is_pixmap,
                   own_drawable);

    return &drawable->base;
}

static int yagl_dri3_display_check(Display *x_dpy)
{
    xcb_connection_t *c = XGetXCBConnection(x_dpy);
    xcb_dri3_query_version_cookie_t dri3_cookie;
    xcb_dri3_query_version_reply_t *dri3_reply;
    xcb_present_query_version_cookie_t present_cookie;
    xcb_present_query_version_reply_t *present_reply;
    xcb_generic_error_t *error;
    const xcb_query_extension_reply_t *extension;

    YAGL_LOG_FUNC_SET(yagl_dri3_display_check);

    xcb_prefetch_extension_data(c, &xcb_dri3_id);
    xcb_prefetch_extension_data(c, &xcb_present_id);

    extension = xcb_get_extension_data(c, &xcb_dri3_id);

    if (!(extension && extension->present)) {
        YAGL_LOG_DEBUG("dri3 extension not supported");
        return 0;
    }

    extension = xcb_get_extension_data(c, &xcb_present_id);

    if (!(extension && extension->present)) {
        YAGL_LOG_DEBUG("present extension not supported");
        return 0;
    }

    dri3_cookie = xcb_dri3_query_version(c,
                                         XCB_DRI3_MAJOR_VERSION,
                                         XCB_DRI3_MINOR_VERSION);


    present_cookie = xcb_present_query_version(c,
                                               XCB_PRESENT_MAJOR_VERSION,
                                               XCB_PRESENT_MINOR_VERSION);

    dri3_reply = xcb_dri3_query_version_reply(c, dri3_cookie, &error);

    if (!dri3_reply) {
        YAGL_LOG_DEBUG("dri3: version query failed");
        free(error);
        return 0;
    }

    YAGL_LOG_DEBUG("dri3: major = %d, minor = %d",
                   dri3_reply->major_version,
                   dri3_reply->minor_version);

    free(dri3_reply);

    present_reply = xcb_present_query_version_reply(c, present_cookie, &error);

    if (!present_reply) {
        YAGL_LOG_DEBUG("present: version query failed");
        free(error);
        return 0;
    }

    YAGL_LOG_DEBUG("present: major = %d, minor = %d",
                   present_reply->major_version,
                   present_reply->minor_version);

    free(present_reply);

    return 1;
}

int yagl_dri3_display_init(Display *x_dpy, char **dri_device)
{
    xcb_dri3_open_cookie_t cookie;
    xcb_dri3_open_reply_t *reply;
    xcb_connection_t *c = XGetXCBConnection(x_dpy);
    int fd;

    YAGL_LOG_FUNC_SET(yagl_x11_display_dri3_init);

    if (!yagl_dri3_display_check(x_dpy)) {
        YAGL_LOG_ERROR("Error: yagl_dri3_display_check failed\n");
        return -1;
    }

    cookie = xcb_dri3_open(c, RootWindow(x_dpy, DefaultScreen(x_dpy)), None);

    reply = xcb_dri3_open_reply(c, cookie, NULL);

    if (!reply) {
        YAGL_LOG_ERROR("Error: xcb_dri3_open_reply failed\n");
        return -1;
    }

    if (reply->nfd != 1) {
        YAGL_LOG_ERROR("Error: reply->nfd != 1 (%d)\n", reply->nfd);
        free(reply);
        return -1;
    }

    fd = xcb_dri3_open_reply_fds(c, reply)[0];
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    *dri_device = drmGetDeviceNameFromFd(fd);

    YAGL_LOG_DEBUG("dri3 display init: fd = %d, dri_device = %s",
                   fd,
                   *dri_device);

    return fd;
}
