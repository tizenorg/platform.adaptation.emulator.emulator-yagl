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

#include "yagl_surface.h"
#include "yagl_utils.h"
#include "yagl_native_drawable.h"
#include "yagl_malloc.h"
#include "yagl_client_interface.h"
#include "yagl_tex_image_binding.h"
#include <assert.h>
#include "EGL/eglext.h"

struct yagl_surface_tex_image_binding
{
    struct yagl_tex_image_binding base;

    struct yagl_client_interface *iface;

    struct yagl_surface *sfc; /* Weak pointer. */
};

static void yagl_surface_tex_image_unbind(struct yagl_tex_image_binding *binding)
{
    struct yagl_surface_tex_image_binding *sfc_binding =
        (struct yagl_surface_tex_image_binding*)binding;
    struct yagl_surface *sfc = sfc_binding->sfc;

    pthread_mutex_lock(&sfc->mtx);

    yagl_free(sfc->binding);
    sfc->binding = NULL;

    pthread_mutex_unlock(&sfc->mtx);
}

void yagl_surface_init_window(struct yagl_surface *sfc,
                              yagl_ref_destroy_func destroy_func,
                              yagl_host_handle handle,
                              struct yagl_display *dpy,
                              struct yagl_native_drawable *native_window)
{
    yagl_resource_init(&sfc->res, destroy_func, handle);

    sfc->dpy = dpy;
    sfc->type = EGL_WINDOW_BIT;
    sfc->native_drawable = native_window;

    yagl_recursive_mutex_init(&sfc->mtx);
}

void yagl_surface_init_pixmap(struct yagl_surface *sfc,
                              yagl_ref_destroy_func destroy_func,
                              yagl_host_handle handle,
                              struct yagl_display *dpy,
                              struct yagl_native_drawable *native_pixmap)
{
    yagl_resource_init(&sfc->res, destroy_func, handle);

    sfc->dpy = dpy;
    sfc->type = EGL_PIXMAP_BIT;
    sfc->native_drawable = native_pixmap;

    yagl_recursive_mutex_init(&sfc->mtx);
}

void yagl_surface_init_pbuffer(struct yagl_surface *sfc,
                               yagl_ref_destroy_func destroy_func,
                               yagl_host_handle handle,
                               struct yagl_display *dpy)
{
    yagl_resource_init(&sfc->res, destroy_func, handle);

    sfc->dpy = dpy;
    sfc->type = EGL_PBUFFER_BIT;

    yagl_recursive_mutex_init(&sfc->mtx);
}

void yagl_surface_cleanup(struct yagl_surface *sfc)
{
    yagl_surface_release_tex_image(sfc);

    if (sfc->native_drawable) {
        sfc->native_drawable->destroy(sfc->native_drawable);
        sfc->native_drawable = NULL;
    }
    yagl_resource_cleanup(&sfc->res);
    pthread_mutex_destroy(&sfc->mtx);
}

EGLSurface yagl_surface_get_handle(struct yagl_surface *sfc)
{
    switch (sfc->type) {
    case EGL_PBUFFER_BIT:
        return (EGLSurface)sfc->res.handle;
    case EGL_PIXMAP_BIT:
    case EGL_WINDOW_BIT:
        return (EGLSurface)sfc->native_drawable->os_drawable;
    default:
        assert(0);
        return NULL;
    }
}

void yagl_surface_invalidate(struct yagl_surface *sfc)
{
    pthread_mutex_lock(&sfc->mtx);

    if (sfc->lock_hint == 0) {
        sfc->invalidate(sfc);
    }

    pthread_mutex_unlock(&sfc->mtx);
}

int yagl_surface_lock(struct yagl_surface *sfc, EGLint hint)
{
    int ret = 0;

    pthread_mutex_lock(&sfc->mtx);

    if (sfc->lock_hint == 0) {
        sfc->lock_hint = (hint & EGL_READ_SURFACE_BIT_KHR) |
                         (hint & EGL_WRITE_SURFACE_BIT_KHR);
        if (sfc->lock_hint == 0) {
            sfc->lock_hint = EGL_READ_SURFACE_BIT_KHR |
                             EGL_WRITE_SURFACE_BIT_KHR;
        }

        ret = 1;
    }

    pthread_mutex_unlock(&sfc->mtx);

    return ret;
}

int yagl_surface_locked(struct yagl_surface *sfc)
{
    int ret;

    pthread_mutex_lock(&sfc->mtx);

    ret = (sfc->lock_hint != 0);

    pthread_mutex_unlock(&sfc->mtx);

    return ret;
}

int yagl_surface_unlock(struct yagl_surface *sfc)
{
    int ret = 0;

    pthread_mutex_lock(&sfc->mtx);

    if (sfc->lock_hint != 0) {
        if (sfc->lock_ptr) {
            sfc->unmap(sfc);
        }

        sfc->lock_hint = 0;
        sfc->lock_ptr = NULL;
        sfc->lock_stride = 0;
        ret = 1;
    }

    pthread_mutex_unlock(&sfc->mtx);

    return ret;
}

void *yagl_surface_map(struct yagl_surface *sfc, uint32_t *stride)
{
    void *ret = NULL;

    pthread_mutex_lock(&sfc->mtx);

    if (sfc->lock_hint != 0) {
        if (!sfc->lock_ptr) {
            sfc->map(sfc);
        }
        if (sfc->lock_ptr) {
            ret = sfc->lock_ptr;
            *stride = sfc->lock_stride;
        }
    }

    pthread_mutex_unlock(&sfc->mtx);

    return ret;
}

struct yagl_tex_image_binding
    *yagl_surface_create_tex_image_binding(struct yagl_surface *sfc,
                                           struct yagl_client_interface *iface)
{
    struct yagl_surface_tex_image_binding *binding;

    binding = yagl_malloc0(sizeof(*binding));

    binding->base.unbind = &yagl_surface_tex_image_unbind;
    binding->iface = iface;
    binding->sfc = sfc;

    return &binding->base;
}

int yagl_surface_bind_tex_image(struct yagl_surface *sfc,
                                struct yagl_tex_image_binding *binding)
{
    struct yagl_surface_tex_image_binding *sfc_binding =
        (struct yagl_surface_tex_image_binding*)binding;
    int ret = 0;

    pthread_mutex_lock(&sfc->mtx);

    if (sfc->binding || (sfc_binding->sfc != sfc)) {
        goto out;
    }

    sfc->binding = binding;

    ret = 1;

out:
    pthread_mutex_unlock(&sfc->mtx);

    return ret;
}

void yagl_surface_release_tex_image(struct yagl_surface *sfc)
{
    struct yagl_surface_tex_image_binding *binding;

    pthread_mutex_lock(&sfc->mtx);

    if (sfc->binding) {
        binding = (struct yagl_surface_tex_image_binding*)sfc->binding;

        binding->iface->release_tex_image(binding->iface, binding->base.cookie);

        yagl_free(sfc->binding);
        sfc->binding = NULL;
    }

    pthread_mutex_unlock(&sfc->mtx);
}

int yagl_surface_mark_current(struct yagl_surface *sfc, int current)
{
    int ret = 1;

    pthread_mutex_lock(&sfc->mtx);

    if ((!current ^ !sfc->current) != 0) {
        sfc->current = current;
    } else {
        ret = 0;
    }

    pthread_mutex_unlock(&sfc->mtx);

    return ret;
}

void yagl_surface_acquire(struct yagl_surface *sfc)
{
    if (sfc) {
        yagl_resource_acquire(&sfc->res);
    }
}

void yagl_surface_release(struct yagl_surface *sfc)
{
    if (sfc) {
        yagl_resource_release(&sfc->res);
    }
}
