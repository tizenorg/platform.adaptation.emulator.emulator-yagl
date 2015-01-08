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

#include "yagl_onscreen.h"
#include "yagl_onscreen_surface.h"
#include "yagl_onscreen_image_pixmap.h"
#ifdef YAGL_PLATFORM_WAYLAND
#include "yagl_onscreen_image_wl_buffer.h"
#endif
#include "yagl_onscreen_fence.h"
#include "yagl_backend.h"
#include "yagl_malloc.h"
#include "yagl_display.h"
#include "yagl_native_platform.h"
#include "yagl_native_display.h"
#include "yagl_egl_state.h"
#include "yagl_host_egl_calls.h"

static struct yagl_display
    *yagl_onscreen_create_display(struct yagl_native_platform *platform,
                                  yagl_os_display os_dpy)
{
    EGLint error = 0;
    struct yagl_native_display *native_dpy;
    yagl_host_handle host_dpy;
    struct yagl_display *dpy;

    native_dpy = platform->wrap_display(os_dpy, 1);

    if (!native_dpy) {
        return NULL;
    }

    host_dpy = yagl_host_eglGetDisplay((uint32_t)os_dpy, &error);

    if (!host_dpy) {
        yagl_set_error(error);
        native_dpy->destroy(native_dpy);
        return NULL;
    }

    dpy = yagl_malloc0(sizeof(*dpy));

    yagl_display_init(dpy, os_dpy, native_dpy, host_dpy);

    return dpy;
}

static struct yagl_surface
    *yagl_onscreen_create_window_surface(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         struct yagl_native_drawable *native_window,
                                         const EGLint *attrib_list)
{
    struct yagl_onscreen_surface *sfc =
        yagl_onscreen_surface_create_window(dpy,
                                            host_config,
                                            native_window,
                                            attrib_list);

    return sfc ? &sfc->base : NULL;
}

static struct yagl_surface
    *yagl_onscreen_create_pixmap_surface(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         struct yagl_native_drawable *native_pixmap,
                                         const EGLint *attrib_list)
{
    struct yagl_onscreen_surface *sfc =
        yagl_onscreen_surface_create_pixmap(dpy,
                                            host_config,
                                            native_pixmap,
                                            attrib_list);

    return sfc ? &sfc->base : NULL;
}

static struct yagl_surface
    *yagl_onscreen_create_pbuffer_surface(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          const EGLint *attrib_list)
{
    struct yagl_onscreen_surface *sfc =
        yagl_onscreen_surface_create_pbuffer(dpy, host_config, attrib_list);

    return sfc ? &sfc->base : NULL;
}

static struct yagl_image
    *yagl_onscreen_create_image_pixmap(struct yagl_display *dpy,
                                       struct yagl_native_drawable *native_pixmap,
                                       struct yagl_client_interface *iface)
{
    struct yagl_onscreen_image_pixmap *image =
        yagl_onscreen_image_pixmap_create(dpy,
                                          native_pixmap,
                                          iface);

    return image ? &image->base : NULL;
}

static struct yagl_image
    *yagl_onscreen_create_image_wl_buffer(struct yagl_display *dpy,
                                          struct wl_resource *buffer,
                                          struct yagl_client_interface *iface)
{
#ifdef YAGL_PLATFORM_WAYLAND
    struct yagl_onscreen_image_wl_buffer *image =
        yagl_onscreen_image_wl_buffer_create(dpy,
                                             buffer,
                                             iface);

    return image ? &image->base : NULL;
#else
    return NULL;
#endif
}

static struct yagl_fence
    *yagl_onscreen_create_fence(struct yagl_display *dpy)
{
    struct yagl_onscreen_fence *fence = yagl_onscreen_fence_create(dpy);

    return fence ? &fence->base : NULL;
}

static void yagl_onscreen_destroy(struct yagl_backend *backend)
{
    yagl_free(backend);
}

struct yagl_backend *yagl_onscreen_create()
{
    struct yagl_backend *backend;

    backend = yagl_malloc0(sizeof(*backend));

    backend->create_display = &yagl_onscreen_create_display;
    backend->create_window_surface = &yagl_onscreen_create_window_surface;
    backend->create_pixmap_surface = &yagl_onscreen_create_pixmap_surface;
    backend->create_pbuffer_surface = &yagl_onscreen_create_pbuffer_surface;
    backend->create_image_pixmap = &yagl_onscreen_create_image_pixmap;
    backend->create_image_wl_buffer = &yagl_onscreen_create_image_wl_buffer;
    backend->create_fence = &yagl_onscreen_create_fence;
    backend->destroy = &yagl_onscreen_destroy;
    backend->y_inverted = 0;
    backend->fence_supported = 1;

    return backend;
}
