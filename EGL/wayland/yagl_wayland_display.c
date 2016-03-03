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

#include "yagl_wayland_display.h"
#include "yagl_wayland_window.h"
#include "yagl_wayland_pbuffer.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "wayland-drm-client-protocol.h"
#include "vigs.h"
#include <xf86drm.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void yagl_wayland_display_roundtrip_done(void *data,
                                                struct wl_callback *callback,
                                                uint32_t serial)
{
    int *done = data;

    *done = 1;

    wl_callback_destroy(callback);
}

static struct wl_callback_listener yagl_wayland_display_roundtrip_listener =
{
    yagl_wayland_display_roundtrip_done
};

static int yagl_wayland_display_roundtrip(struct wl_display *dpy,
                                          struct wl_event_queue *queue)
{
    struct wl_callback *callback;
    int done = 0, ret = 0;

    callback = wl_display_sync(dpy);
    wl_callback_add_listener(callback,
                             &yagl_wayland_display_roundtrip_listener,
                             &done);
    wl_proxy_set_queue((struct wl_proxy*)callback, queue);

    while ((ret != -1) && !done) {
        ret = wl_display_dispatch_queue(dpy, queue);
    }

    if (!done) {
        wl_callback_destroy(callback);
    }

    return ret >= 0;
}

static void yagl_wayland_display_drm_device(void *data,
                                            struct wl_drm *wl_drm,
                                            const char *name)
{
    struct yagl_wayland_display *dpy = data;
    drm_magic_t magic;
    int ret;

    YAGL_LOG_FUNC_ENTER(yagl_wayland_display_drm_device,
                        "dpy = %p, wl_drm = %p, name = %s",
                        dpy, wl_drm, name);

    if (dpy->drm_dev_name) {
        YAGL_LOG_FUNC_EXIT(NULL);
        return;
    }

    dpy->drm_dev_name = strdup(name);

    dpy->drm_fd = open(dpy->drm_dev_name, O_RDWR);

    if (dpy->drm_fd < 0) {
        fprintf(stderr, "Critical error! Failed to open(\"%s\"): %s\n",
                dpy->drm_dev_name, strerror(errno));
        goto fail;
    }

    memset(&magic, 0, sizeof(magic));

    ret = drmGetMagic(dpy->drm_fd, &magic);

    if (ret != 0) {
        fprintf(stderr, "Critical error! drmGetMagic failed: %s\n", strerror(-ret));
        goto fail;
    }

    wl_drm_authenticate(dpy->wl_drm, magic);

    YAGL_LOG_FUNC_EXIT(NULL);

    return;

fail:
    if (dpy->drm_fd >= 0) {
        close(dpy->drm_fd);
        dpy->drm_fd = -1;
    }
    free(dpy->drm_dev_name);
    dpy->drm_dev_name = NULL;

    YAGL_LOG_FUNC_EXIT(NULL);
}

static void yagl_wayland_display_drm_format(void *data,
                                            struct wl_drm *wl_drm,
                                            uint32_t format)
{
    YAGL_LOG_FUNC_ENTER(yagl_wayland_display_drm_format,
                        "dpy = %p, wl_drm = %p, format = %u",
                        data, wl_drm, format);

    YAGL_LOG_FUNC_EXIT(NULL);
}

static void yagl_wayland_display_drm_authenticated(void *data,
                                                   struct wl_drm *wl_drm)
{
    struct yagl_wayland_display *dpy = data;

    YAGL_LOG_FUNC_ENTER(yagl_wayland_display_drm_authenticated,
                        "dpy = %p, wl_drm = %p",
                        dpy, wl_drm);

    dpy->authenticated = 1;

    YAGL_LOG_FUNC_EXIT(NULL);
}

static struct wl_drm_listener yagl_wayland_display_drm_listener =
{
    yagl_wayland_display_drm_device,
    yagl_wayland_display_drm_format,
    yagl_wayland_display_drm_authenticated
};

static void yagl_wayland_display_registry_global(void *data,
                                                 struct wl_registry *registry,
                                                 uint32_t name,
                                                 const char* interface,
                                                 uint32_t version)
{
    struct yagl_wayland_display *dpy = data;

    if (dpy->wl_drm) {
        return;
    }

    if (strcmp(interface, "wl_drm") == 0) {
        dpy->wl_drm = wl_registry_bind(registry,
                                       name,
                                       &wl_drm_interface,
                                       1);
        wl_drm_add_listener(dpy->wl_drm,
                            &yagl_wayland_display_drm_listener,
                            dpy);
    }
}

static void
    yagl_wayland_display_registry_global_remove(void *data,
                                                struct wl_registry *registry,
                                                uint32_t name)
{
}

static struct wl_registry_listener yagl_wayland_display_registry_listener =
{
    yagl_wayland_display_registry_global,
    yagl_wayland_display_registry_global_remove
};

static int yagl_wayland_display_authenticate(struct yagl_native_display *dpy,
                                             uint32_t id)
{
    struct yagl_wayland_display *wayland_dpy = (struct yagl_wayland_display*)dpy;
    struct wl_display *wl_dpy = YAGL_WAYLAND_DPY(dpy->os_dpy);
    int ret;

    /*
     * Set 'authenticated' temporary to 0, send
     * an authenticate request, wait for the reply to set
     * 'authenticated' to 1, in this case we're authenticated,
     * otherwise an error has occurred and 'authenticated'
     * will remain 0. Restore 'authenticated' to 1 afterwards.
     */

    wayland_dpy->authenticated = 0;

    wl_drm_authenticate(wayland_dpy->wl_drm, id);
    yagl_wayland_display_roundtrip(wl_dpy, wayland_dpy->queue);

    ret = wayland_dpy->authenticated;

    wayland_dpy->authenticated = 1;

    return ret;
}

static struct yagl_native_drawable
    *yagl_wayland_display_wrap_window(struct yagl_native_display *dpy,
                                      yagl_os_window os_window)
{
    return yagl_wayland_window_create(dpy, os_window);
}

static struct yagl_native_drawable
    *yagl_wayland_display_wrap_pixmap(struct yagl_native_display *dpy,
                                      yagl_os_pixmap os_pixmap)
{
    return NULL;
}

static struct yagl_native_drawable
    *yagl_wayland_display_create_pixmap(struct yagl_native_display *dpy,
                                        uint32_t width,
                                        uint32_t height,
                                        uint32_t depth)
{
    /*
     * Wayland actaully has no pixmap type. This is just for creating
     * a pbuffer and to make Tizen conformance tests happy.
     */
    return yagl_wayland_pbuffer_create(dpy, width, height, depth);
}

static struct yagl_native_image
    *yagl_wayland_display_create_image(struct yagl_native_display *dpy,
                                       uint32_t width,
                                       uint32_t height,
                                       uint32_t depth)
{
    return NULL;
}

static int yagl_wayland_display_get_visual(struct yagl_native_display *dpy,
                                           int *visual_id,
                                           int *visual_type)
{
    *visual_id = 0;
    *visual_type = 0;

    return 1;
}

static void yagl_wayland_display_update_wl_server(struct yagl_native_display *dpy)
{
    return ;
}

static void yagl_wayland_display_destroy(struct yagl_native_display *dpy)
{
    struct yagl_wayland_display *wayland_dpy = (struct yagl_wayland_display*)dpy;
    struct wl_display *wl_dpy = YAGL_WAYLAND_DPY(dpy->os_dpy);

    free(wayland_dpy->drm_dev_name);
    wayland_dpy->drm_dev_name = NULL;

    close(wayland_dpy->drm_fd);
    wayland_dpy->drm_fd = -1;

    wl_drm_destroy(wayland_dpy->wl_drm);
    wayland_dpy->wl_drm = NULL;

    wl_event_queue_destroy(wayland_dpy->queue);
    wayland_dpy->queue = NULL;

    yagl_native_display_cleanup(dpy);

    if (wayland_dpy->own_dpy) {
        wl_display_disconnect(wl_dpy);
    }

    yagl_free(wayland_dpy);
}

struct yagl_native_display
    *yagl_wayland_display_create(struct yagl_native_platform *platform,
                                 yagl_os_display os_dpy,
                                 int own_dpy)
{
    struct wl_display *wl_dpy = YAGL_WAYLAND_DPY(os_dpy);
    struct yagl_wayland_display *dpy;
    int ret;
    struct vigs_drm_device *drm_dev = NULL;

    YAGL_LOG_FUNC_ENTER(yagl_wayland_display_create,
                        "os_dpy = %p", os_dpy);

    dpy = yagl_malloc0(sizeof(*dpy));

    dpy->own_dpy = own_dpy;

    dpy->drm_fd = -1;

    dpy->queue = wl_display_create_queue(wl_dpy);

    if (!dpy->queue) {
        YAGL_LOG_ERROR("Unable to create event queue");
        goto fail;
    }

    if (own_dpy) {
        wl_display_dispatch_pending(wl_dpy);
    }

    dpy->registry = wl_display_get_registry(wl_dpy);

    wl_proxy_set_queue((struct wl_proxy*)dpy->registry, dpy->queue);

    wl_registry_add_listener(dpy->registry,
                             &yagl_wayland_display_registry_listener,
                             dpy);

    if (!yagl_wayland_display_roundtrip(wl_dpy, dpy->queue) ||
        !dpy->wl_drm) {
        YAGL_LOG_ERROR("Unable to obtain wl_drm interface");
        goto fail;
    }

    if (!yagl_wayland_display_roundtrip(wl_dpy, dpy->queue) ||
        (dpy->drm_fd == -1)) {
        YAGL_LOG_ERROR("Unable to open wl_drm device");
        goto fail;
    }

    if (!yagl_wayland_display_roundtrip(wl_dpy, dpy->queue) ||
        !dpy->authenticated) {
        YAGL_LOG_ERROR("Unable to authenticate on wl_drm interface");
        goto fail;
    }

    ret = vigs_drm_device_create(dpy->drm_fd, &drm_dev);

    if (ret != 0) {
        fprintf(stderr,
                "Critical error! vigs_drm_device_create failed: %s\n",
                strerror(-ret));
        goto fail;
    }

    yagl_native_display_init(&dpy->base,
                             platform,
                             os_dpy,
                             drm_dev,
                             dpy->drm_dev_name);

    dpy->base.authenticate = &yagl_wayland_display_authenticate;
    dpy->base.wrap_window = &yagl_wayland_display_wrap_window;
    dpy->base.wrap_pixmap = &yagl_wayland_display_wrap_pixmap;
    dpy->base.create_pixmap = &yagl_wayland_display_create_pixmap;
    dpy->base.create_image = &yagl_wayland_display_create_image;
    dpy->base.get_visual = &yagl_wayland_display_get_visual;
    dpy->base.update_wl_server = &yagl_wayland_display_update_wl_server;
    dpy->base.destroy = &yagl_wayland_display_destroy;

    YAGL_LOG_FUNC_EXIT("display %p created", dpy);

    return &dpy->base;

fail:
    free(dpy->drm_dev_name);
    dpy->drm_dev_name = NULL;

    if (dpy->drm_fd >= 0) {
        close(dpy->drm_fd);
        dpy->drm_fd = -1;
    }

    if (dpy->wl_drm) {
        wl_drm_destroy(dpy->wl_drm);
        dpy->wl_drm = NULL;
    }

    if (dpy->queue) {
        wl_event_queue_destroy(dpy->queue);
        dpy->queue = NULL;
    }

    yagl_free(dpy);

    YAGL_LOG_FUNC_EXIT(NULL);

    return NULL;
}
