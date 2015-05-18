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

#include "yagl_display.h"
#include "yagl_utils.h"
#include "yagl_backend.h"
#include "yagl_log.h"
#include "yagl_surface.h"
#include "yagl_context.h"
#include "yagl_image.h"
#include "yagl_malloc.h"
#include "yagl_fence.h"
#include "yagl_native_display.h"
#include "yagl_native_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define YAGL_EGL_BASE_EXTENSIONS "EGL_KHR_image_base " \
                                 "EGL_KHR_lock_surface " \
                                 "EGL_KHR_surfaceless_context "

#define YAGL_EGL_PIXMAPS_EXTENSIONS "EGL_KHR_image " \
                                    "EGL_KHR_image_pixmap " \
                                    "EGL_NOK_texture_from_pixmap "

#define YAGL_EGL_WL_BIND_WAYLAND_DISPLAY_EXTENSIONS "EGL_WL_bind_wayland_display "

#define YAGL_EGL_GL_TEXTURE_EXTENSIONS "EGL_KHR_gl_texture_2D_image "

#define YAGL_EGL_BUFFER_AGE_EXTENSIONS "EGL_EXT_buffer_age "

#define YAGL_EGL_FENCE_EXTENSIONS "EGL_KHR_fence_sync "

void yagl_set_fence_display(struct yagl_display *fence_dpy);

static pthread_once_t g_displays_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_displays_mutex;
static YAGL_DECLARE_LIST(g_displays);

static void yagl_displays_init_once(void)
{
    /*
     * We need a recursive mutex here because in DRI2 an event can come in
     * during any X.Org function call and we need to access displays in
     * DRI2 event handler.
     */
    yagl_recursive_mutex_init(&g_displays_mutex);
}

static void yagl_displays_init(void)
{
    pthread_once(&g_displays_init, yagl_displays_init_once);
}

void yagl_display_init(struct yagl_display *dpy,
                       yagl_os_display display_id,
                       struct yagl_native_display *native_dpy,
                       yagl_host_handle host_dpy)
{
    yagl_list_init(&dpy->list);
    dpy->display_id = display_id;
    dpy->native_dpy = native_dpy;
    dpy->host_dpy = host_dpy;
    /*
     * Need recursive mutex here,
     * see 'g_displays_mutex' initialization for reasons.
     */
    yagl_recursive_mutex_init(&dpy->mutex);
    dpy->prepared = 0;

    yagl_list_init(&dpy->surfaces);
    yagl_list_init(&dpy->contexts);
    yagl_list_init(&dpy->images);
    yagl_list_init(&dpy->fences);
}

void yagl_display_atfork(void)
{
    /*
     * Don't actually destroy displays, just
     * reset the list. See yagl_state.c:yagl_state_atfork.
     */

    yagl_list_init(&g_displays);
}

struct yagl_display *yagl_display_get(EGLDisplay handle)
{
    yagl_host_handle host_dpy = (yagl_host_handle)handle;
    struct yagl_display *dpy;

    yagl_displays_init();

    pthread_mutex_lock(&g_displays_mutex);

    yagl_list_for_each(struct yagl_display, dpy, &g_displays, list) {
        if (dpy->host_dpy == host_dpy) {
            pthread_mutex_unlock(&g_displays_mutex);

            return dpy;
        }
    }

    pthread_mutex_unlock(&g_displays_mutex);

    return NULL;
}

struct yagl_display *yagl_display_get_os(yagl_os_display os_dpy)
{
    struct yagl_display *dpy;

    yagl_displays_init();

    pthread_mutex_lock(&g_displays_mutex);

    yagl_list_for_each(struct yagl_display, dpy, &g_displays, list) {
        if (dpy->native_dpy->os_dpy == os_dpy) {
            pthread_mutex_unlock(&g_displays_mutex);

            return dpy;
        }
    }

    pthread_mutex_unlock(&g_displays_mutex);

    return NULL;
}

struct yagl_display *yagl_display_add(struct yagl_native_platform *platform,
                                      yagl_os_display display_id)
{
    struct yagl_display *dpy;

    yagl_displays_init();

    pthread_mutex_lock(&g_displays_mutex);

    yagl_list_for_each(struct yagl_display, dpy, &g_displays, list) {
        if (dpy->display_id == display_id) {
            pthread_mutex_unlock(&g_displays_mutex);
            return dpy;
        }
    }

    dpy = yagl_get_backend()->create_display(platform, display_id);

    if (!dpy) {
        pthread_mutex_unlock(&g_displays_mutex);
        return NULL;
    }

    yagl_list_add(&g_displays, &dpy->list);

    pthread_mutex_unlock(&g_displays_mutex);

    yagl_set_fence_display(dpy);

    return dpy;
}

void yagl_display_prepare(struct yagl_display *dpy)
{
    pthread_mutex_lock(&dpy->mutex);
    dpy->prepared = 1;
    pthread_mutex_unlock(&dpy->mutex);
}

int yagl_display_is_prepared(struct yagl_display *dpy)
{
    int ret;

    pthread_mutex_lock(&dpy->mutex);
    ret = dpy->prepared;
    pthread_mutex_unlock(&dpy->mutex);

    return ret;
}

void yagl_display_terminate(struct yagl_display *dpy)
{
    struct yagl_list tmp_list;
    struct yagl_resource *res, *next;

    /*
     * First, move all surfaces, contexts and images to a
     * temporary list to release later.
     */

    yagl_list_init(&tmp_list);

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each_safe(struct yagl_resource,
                            res,
                            next,
                            &dpy->surfaces,
                            list) {
        yagl_list_remove(&res->list);
        yagl_list_add_tail(&tmp_list, &res->list);
    }

    yagl_list_for_each_safe(struct yagl_resource,
                            res,
                            next,
                            &dpy->contexts,
                            list) {
        yagl_list_remove(&res->list);
        yagl_list_add_tail(&tmp_list, &res->list);
    }

    yagl_list_for_each_safe(struct yagl_resource,
                            res,
                            next,
                            &dpy->images,
                            list) {
        yagl_list_remove(&res->list);
        yagl_list_add_tail(&tmp_list, &res->list);
    }

    yagl_list_for_each_safe(struct yagl_resource,
                            res,
                            next,
                            &dpy->fences,
                            list) {
        yagl_list_remove(&res->list);
        yagl_list_add_tail(&tmp_list, &res->list);
    }

    assert(yagl_list_empty(&dpy->surfaces));
    assert(yagl_list_empty(&dpy->contexts));
    assert(yagl_list_empty(&dpy->images));
    assert(yagl_list_empty(&dpy->fences));

    dpy->prepared = 0;

    pthread_mutex_unlock(&dpy->mutex);

    /*
     * We release here because we don't want the resources to be released
     * when display mutex is held.
     */

    yagl_list_for_each_safe(struct yagl_resource, res, next, &tmp_list, list) {
        yagl_list_remove(&res->list);
        yagl_resource_release(res);
    }

    assert(yagl_list_empty(&tmp_list));
}

const char *yagl_display_get_extensions(struct yagl_display *dpy)
{
    if (!dpy) {
        return YAGL_EGL_BASE_EXTENSIONS;
    }

    pthread_mutex_lock(&dpy->mutex);

    if (!dpy->extensions) {
        uint32_t len = strlen(YAGL_EGL_BASE_EXTENSIONS);

        if (dpy->native_dpy->platform->pixmaps_supported) {
            len += strlen(YAGL_EGL_PIXMAPS_EXTENSIONS);
        }

        if (dpy->native_dpy->WL_bind_wayland_display_supported) {
            len += strlen(YAGL_EGL_WL_BIND_WAYLAND_DISPLAY_EXTENSIONS);
        }

        if (dpy->native_dpy->platform->buffer_age_supported) {
            len += strlen(YAGL_EGL_BUFFER_AGE_EXTENSIONS);
        }

        if (yagl_egl_fence_supported()) {
            len += strlen(YAGL_EGL_FENCE_EXTENSIONS);
        }

        len += strlen(YAGL_EGL_GL_TEXTURE_EXTENSIONS);

        dpy->extensions = yagl_malloc(len + 1);

        strcpy(dpy->extensions, YAGL_EGL_BASE_EXTENSIONS);

        if (dpy->native_dpy->platform->pixmaps_supported) {
            strcat(dpy->extensions, YAGL_EGL_PIXMAPS_EXTENSIONS);
        }

        if (dpy->native_dpy->WL_bind_wayland_display_supported) {
            strcat(dpy->extensions, YAGL_EGL_WL_BIND_WAYLAND_DISPLAY_EXTENSIONS);
        }

        if (dpy->native_dpy->platform->buffer_age_supported) {
            strcat(dpy->extensions, YAGL_EGL_BUFFER_AGE_EXTENSIONS);
        }

        if (yagl_egl_fence_supported()) {
            strcat(dpy->extensions, YAGL_EGL_FENCE_EXTENSIONS);
        }

        strcat(dpy->extensions, YAGL_EGL_GL_TEXTURE_EXTENSIONS);
    }

    pthread_mutex_unlock(&dpy->mutex);

    return dpy->extensions;
}

int yagl_display_surface_add(struct yagl_display *dpy,
                             struct yagl_surface *sfc)
{
    struct yagl_resource *res = NULL;
    EGLSurface handle = yagl_surface_get_handle(sfc);

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->surfaces, list) {
        if (yagl_surface_get_handle((struct yagl_surface*)res) == handle) {
            pthread_mutex_unlock(&dpy->mutex);
            return 0;
        }
    }

    yagl_resource_acquire(&sfc->res);
    yagl_list_add_tail(&dpy->surfaces, &sfc->res.list);

    pthread_mutex_unlock(&dpy->mutex);

    return 1;
}

struct yagl_surface *yagl_display_surface_acquire(struct yagl_display *dpy,
                                                  EGLSurface handle)
{
    struct yagl_resource *res = NULL;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->surfaces, list) {
        if (yagl_surface_get_handle((struct yagl_surface*)res) == handle) {
            yagl_resource_acquire(res);
            pthread_mutex_unlock(&dpy->mutex);
            return (struct yagl_surface*)res;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);

    return NULL;
}

int yagl_display_surface_remove(struct yagl_display *dpy,
                                EGLSurface handle)
{
    struct yagl_resource *res;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->surfaces, list) {
        if (yagl_surface_get_handle((struct yagl_surface*)res) == handle) {
            yagl_list_remove(&res->list);
            yagl_resource_release(res);
            pthread_mutex_unlock(&dpy->mutex);
            return 1;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);

    return 0;
}

void yagl_display_context_add(struct yagl_display *dpy,
                              struct yagl_context *ctx)
{
    pthread_mutex_lock(&dpy->mutex);

    yagl_resource_acquire(&ctx->res);
    yagl_list_add_tail(&dpy->contexts, &ctx->res.list);

    pthread_mutex_unlock(&dpy->mutex);
}

struct yagl_context *yagl_display_context_acquire(struct yagl_display *dpy,
                                                  EGLContext handle)
{
    struct yagl_resource *res = NULL;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->contexts, list) {
        if (res->handle == (yagl_host_handle)handle) {
            yagl_resource_acquire(res);
            pthread_mutex_unlock(&dpy->mutex);
            return (struct yagl_context*)res;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);

    return NULL;
}

void yagl_display_context_remove(struct yagl_display *dpy,
                                 EGLContext handle)
{
    struct yagl_resource *res;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->contexts, list) {
        if (res->handle == (yagl_host_handle)handle) {
            yagl_list_remove(&res->list);
            yagl_resource_release(res);
            break;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);
}

int yagl_display_image_add(struct yagl_display *dpy,
                           struct yagl_image *image)
{
    struct yagl_resource *res = NULL;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->images, list) {
        if (((struct yagl_image*)res)->client_handle == image->client_handle) {
            pthread_mutex_unlock(&dpy->mutex);
            return 0;
        }
    }

    yagl_resource_acquire(&image->res);
    yagl_list_add_tail(&dpy->images, &image->res.list);

    pthread_mutex_unlock(&dpy->mutex);

    return 1;
}

struct yagl_image *yagl_display_image_acquire(struct yagl_display *dpy,
                                              EGLImageKHR handle)
{
    struct yagl_resource *res = NULL;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->images, list) {
        if (((struct yagl_image*)res)->client_handle == handle) {
            yagl_resource_acquire(res);
            pthread_mutex_unlock(&dpy->mutex);
            return (struct yagl_image*)res;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);

    return NULL;
}

int yagl_display_image_remove(struct yagl_display *dpy,
                              EGLImageKHR handle)
{
    struct yagl_resource *res;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->images, list) {
        if (((struct yagl_image*)res)->client_handle == handle) {
            yagl_list_remove(&res->list);
            yagl_resource_release(res);
            pthread_mutex_unlock(&dpy->mutex);
            return 1;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);

    return 0;
}

void yagl_display_fence_add(struct yagl_display *dpy,
                            struct yagl_fence *fence)
{
    pthread_mutex_lock(&dpy->mutex);

    yagl_resource_acquire(&fence->base.res);
    yagl_list_add_tail(&dpy->fences, &fence->base.res.list);

    pthread_mutex_unlock(&dpy->mutex);
}

struct yagl_fence *yagl_display_fence_acquire(struct yagl_display *dpy,
                                              EGLSyncKHR handle)
{
    struct yagl_resource *res = NULL;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->fences, list) {
        if ((EGLSyncKHR)res == handle) {
            yagl_resource_acquire(res);
            pthread_mutex_unlock(&dpy->mutex);
            return (struct yagl_fence*)res;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);

    return NULL;
}

int yagl_display_fence_remove(struct yagl_display *dpy,
                              EGLSyncKHR handle)
{
    struct yagl_resource *res;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->fences, list) {
        if ((EGLSyncKHR)res == handle) {
            yagl_list_remove(&res->list);
            yagl_resource_release(res);
            pthread_mutex_unlock(&dpy->mutex);
            return 1;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);

    return 0;
}
