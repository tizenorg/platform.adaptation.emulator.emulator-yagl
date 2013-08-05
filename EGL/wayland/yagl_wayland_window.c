#include "yagl_wayland_window.h"
#include "yagl_wayland_display.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_wayland_egl.h"
#include "wayland-drm.h"
#include "wayland-drm-client-protocol.h"
#include "vigs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void yagl_wayland_window_buffer_release(void *data,
                                               struct wl_buffer *buffer)
{
    struct yagl_wayland_window *window = data;
    int i;

    for (i = 0;
         i < sizeof(window->color_buffers)/sizeof(window->color_buffers[0]);
         ++i) {
        if (window->color_buffers[i].wl_buffer == buffer) {
            /*
             * The buffer is still in the pool, just unlock,
             * don't destroy it.
             */
            window->color_buffers[i].locked = 0;
            return;
        }
    }

    /*
     * Buffer is not in the pool, drop it.
     */
    wl_buffer_destroy(buffer);
}

static struct wl_buffer_listener yagl_wayland_window_buffer_listener =
{
    yagl_wayland_window_buffer_release
};

static void yagl_wayland_window_frame_done(void *data,
                                           struct wl_callback *callback,
                                           uint32_t time)
{
    struct yagl_wayland_window *window = data;

    window->frame_callback = NULL;
    wl_callback_destroy(callback);
}

static struct wl_callback_listener yagl_wayland_window_frame_listener =
{
    yagl_wayland_window_frame_done
};

static int yagl_wayland_window_lock_back(struct yagl_wayland_window *window)
{
    struct yagl_wayland_display *dpy = (struct yagl_wayland_display*)window->base.dpy;
    struct wl_display *wl_dpy = YAGL_WAYLAND_DPY(dpy->base.os_dpy);
    int i, ret;

    /*
     * There might be a buffer release already queued that wasn't processed.
     */
    wl_display_dispatch_queue_pending(wl_dpy, dpy->queue);

    if (!window->back) {
        for (i = 0;
             i < sizeof(window->color_buffers)/sizeof(window->color_buffers[0]);
             ++i) {
            if (window->color_buffers[i].locked) {
                continue;
            }
            if (!window->back) {
                window->back = &window->color_buffers[i];
            } else if (!window->back->drm_sfc) {
                /*
                 * Prefer buffers with DRM surfaces allocated.
                 */
                window->back = &window->color_buffers[i];
            }
        }
    }

    if (!window->back) {
        return 0;
    }

    if (!window->back->drm_sfc) {
        ret = vigs_drm_surface_create(dpy->base.drm_dev,
                                      window->width,
                                      window->height,
                                      (window->width * 4),
                                      vigs_drm_surface_bgrx8888,
                                      &window->back->drm_sfc);

        if (ret != 0) {
            fprintf(stderr,
                    "wayland: Unable to create DRM surface(%ux%u): %s\n",
                    window->width, window->height,
                    strerror(-ret));
            window->back->drm_sfc = NULL;
        }
    }

    if (!window->back->drm_sfc) {
        return 0;
    }

    window->back->locked = 1;

    return 1;
}

static void yagl_wayland_window_resize(struct wl_egl_window *egl_window,
                                       void *user_data)
{
    struct yagl_wayland_window *window = user_data;

    YAGL_LOG_FUNC_SET(yagl_wayland_window_resize);

    ++window->base.stamp;

    YAGL_LOG_DEBUG("window %p resized, stamp = %u",
                   egl_window, window->base.stamp);
}

static int yagl_wayland_window_get_buffer(struct yagl_native_drawable *drawable,
                                          yagl_native_attachment attachment,
                                          uint32_t *buffer_name,
                                          struct vigs_drm_surface **buffer_sfc)
{
    struct yagl_wayland_window *window = (struct yagl_wayland_window*)drawable;
    struct wl_egl_window *egl_window = YAGL_WAYLAND_WINDOW(drawable->os_drawable);
    int i;

    YAGL_LOG_FUNC_SET(yagl_wayland_window_get_buffer);

    switch (attachment) {
    case yagl_native_attachment_back:
        break;
    case yagl_native_attachment_front:
    default:
        YAGL_LOG_ERROR("Bad attachment %u", attachment);
        return 0;
    }

    if ((window->width != egl_window->width) ||
        (window->height != egl_window->height)) {
        for (i = 0;
             i < sizeof(window->color_buffers)/sizeof(window->color_buffers[0]);
             ++i) {
            if (window->color_buffers[i].wl_buffer &&
                !window->color_buffers[i].locked) {
                wl_buffer_destroy(window->color_buffers[i].wl_buffer);
            }
            if (window->color_buffers[i].drm_sfc) {
                vigs_drm_gem_unref(&window->color_buffers[i].drm_sfc->gem);
            }
            window->color_buffers[i].wl_buffer = NULL;
            window->color_buffers[i].drm_sfc = NULL;
            window->color_buffers[i].locked = 0;
        }

        window->width = egl_window->width;
        window->height = egl_window->height;
    }

    window->dx = egl_window->dx;
    window->dy = egl_window->dy;

    if (!yagl_wayland_window_lock_back(window)) {
        YAGL_LOG_ERROR("Cannot lock back for egl_window %p", egl_window);
        return 0;
    }

    /*
     * What this does is handles the case when there was triple buffering
     * for some period of time and now we're back to double buffering.
     * Since we're back to double buffering we can free buffers
     * in order not to waste memory.
     *
     * Some detailed explanation since it's not that trivial: When triple
     * buffering is in place we'll allocate all 3 buffers in 'color_buffers',
     * the release code only sets 'locked' to 0, it doesn't destroy the buffers,
     * so they could be reused. So, with triple buffering all 3 buffers will be
     * locked all the time, but once the "pressure" on the compositor will
     * lower it'll release the excess buffer and here we just check for that
     * excess buffer and destroy it, so we won't waste memory in vain.
     */
    for (i = 0;
         i < sizeof(window->color_buffers)/sizeof(window->color_buffers[0]);
         ++i) {
        if (window->color_buffers[i].wl_buffer &&
            !window->color_buffers[i].locked) {
            wl_buffer_destroy(window->color_buffers[i].wl_buffer);
            vigs_drm_gem_unref(&window->color_buffers[i].drm_sfc->gem);
            window->color_buffers[i].wl_buffer = NULL;
            window->color_buffers[i].drm_sfc = NULL;
        }
    }

    vigs_drm_gem_ref(&window->back->drm_sfc->gem);

    *buffer_sfc = window->back->drm_sfc;

    return 1;
}

static void yagl_wayland_window_swap_buffers(struct yagl_native_drawable *drawable)
{
    struct yagl_wayland_window *window = (struct yagl_wayland_window*)drawable;
    struct yagl_wayland_display *dpy = (struct yagl_wayland_display*)drawable->dpy;
    struct wl_display *wl_dpy = YAGL_WAYLAND_DPY(drawable->dpy->os_dpy);
    struct wl_egl_window *egl_window = YAGL_WAYLAND_WINDOW(drawable->os_drawable);
    int ret = 0;

    YAGL_LOG_FUNC_SET(yagl_wayland_window_swap_buffers);

    /*
     * Throttle.
     */
    while (window->frame_callback && (ret != -1)) {
        ret = wl_display_dispatch_queue(wl_dpy, dpy->queue);
    }

    if (ret < 0) {
        /*
         * Is this possible ?
         */
        return;
    }

    window->frame_callback = wl_surface_frame(egl_window->surface);
    wl_callback_add_listener(window->frame_callback,
                             &yagl_wayland_window_frame_listener,
                             window);
    wl_proxy_set_queue((struct wl_proxy*)window->frame_callback, dpy->queue);

    /*
     * Make sure we have a back buffer in case we're swapping without ever
     * rendering.
     */
    if (!yagl_wayland_window_lock_back(window)) {
        YAGL_LOG_ERROR("Cannot lock back for egl_window %p", egl_window);
        return;
    }

    window->front = window->back;
    window->back = NULL;

    if (!window->front->wl_buffer) {
        ret = vigs_drm_gem_get_name(&window->front->drm_sfc->gem);

        if (ret != 0) {
            fprintf(stderr,
                    "wayland: Unable to get GEM name: %s\n", strerror(-ret));
        }

        window->front->wl_buffer =
            wl_drm_create_buffer(dpy->wl_drm,
                                 window->front->drm_sfc->gem.name,
                                 window->width,
                                 window->height,
                                 window->front->drm_sfc->stride,
                                 WL_DRM_FORMAT_XRGB8888);
        wl_proxy_set_queue((struct wl_proxy*)window->front->wl_buffer,
                           dpy->queue);
        wl_buffer_add_listener(window->front->wl_buffer,
                               &yagl_wayland_window_buffer_listener,
                               window);
    }

    wl_surface_attach(egl_window->surface,
                      window->front->wl_buffer,
                      window->dx,
                      window->dy);

    egl_window->attached_width = window->width;
    egl_window->attached_height = window->height;

    /*
     * Reset resize growing parameters.
     */
    window->dx = 0;
    window->dy = 0;

    wl_surface_damage(egl_window->surface, 0, 0,
                      window->width,
                      window->height);

    wl_surface_commit(egl_window->surface);

    ++drawable->stamp;
}

static void yagl_wayland_window_wait(struct yagl_native_drawable *drawable,
                                     uint32_t width,
                                     uint32_t height)
{
}

static void yagl_wayland_window_copy_to_pixmap(struct yagl_native_drawable *drawable,
                                               yagl_os_pixmap os_pixmap,
                                               uint32_t from_x,
                                               uint32_t from_y,
                                               uint32_t to_x,
                                               uint32_t to_y,
                                               uint32_t width,
                                               uint32_t height)
{
}

static void yagl_wayland_window_set_swap_interval(struct yagl_native_drawable *drawable,
                                                  int interval)
{
}

static void yagl_wayland_window_get_geometry(struct yagl_native_drawable *drawable,
                                             uint32_t *width,
                                             uint32_t *height,
                                             uint32_t *depth)
{
}

static struct yagl_native_image
    *yagl_wayland_window_get_image(struct yagl_native_drawable *drawable,
                                   uint32_t width,
                                   uint32_t height)
{
    return NULL;
}

static void yagl_wayland_window_destroy(struct yagl_native_drawable *drawable)
{
    struct yagl_wayland_window *window = (struct yagl_wayland_window*)drawable;
    struct wl_egl_window *egl_window = YAGL_WAYLAND_WINDOW(drawable->os_drawable);
    int i;

    for (i = 0;
         i < sizeof(window->color_buffers)/sizeof(window->color_buffers[0]);
         ++i) {
        if (window->color_buffers[i].wl_buffer) {
            wl_buffer_destroy(window->color_buffers[i].wl_buffer);
        }
        if (window->color_buffers[i].drm_sfc) {
            vigs_drm_gem_unref(&window->color_buffers[i].drm_sfc->gem);
        }
        window->color_buffers[i].wl_buffer = NULL;
        window->color_buffers[i].drm_sfc = NULL;
        window->color_buffers[i].locked = 0;
    }

    if (window->frame_callback) {
        wl_callback_destroy(window->frame_callback);
    }

    yagl_native_drawable_cleanup(drawable);

    egl_window->resize_callback = NULL;
    egl_window->user_data = NULL;

    yagl_free(drawable);
}

struct yagl_native_drawable
    *yagl_wayland_window_create(struct yagl_native_display *dpy,
                                yagl_os_window os_window)
{
    struct wl_egl_window *egl_window = YAGL_WAYLAND_WINDOW(os_window);
    struct yagl_wayland_window *window;

    YAGL_LOG_FUNC_SET(yagl_wayland_window_create);

    if (egl_window->resize_callback || egl_window->user_data) {
        YAGL_LOG_ERROR("wl_egl_window %p already wrapped", egl_window);
        return NULL;
    }

    window = yagl_malloc0(sizeof(*window));

    yagl_native_drawable_init(&window->base, dpy, os_window);

    window->base.get_buffer = &yagl_wayland_window_get_buffer;
    window->base.swap_buffers = &yagl_wayland_window_swap_buffers;
    window->base.wait = &yagl_wayland_window_wait;
    window->base.copy_to_pixmap = &yagl_wayland_window_copy_to_pixmap;
    window->base.set_swap_interval = &yagl_wayland_window_set_swap_interval;
    window->base.get_geometry = &yagl_wayland_window_get_geometry;
    window->base.get_image = &yagl_wayland_window_get_image;
    window->base.destroy = &yagl_wayland_window_destroy;

    /*
     * First comparison against real window
     * dimensions must always fail.
     */
    window->width = -1;
    window->height = -1;

    egl_window->resize_callback = &yagl_wayland_window_resize;
    egl_window->user_data = window;

    return &window->base;
}
