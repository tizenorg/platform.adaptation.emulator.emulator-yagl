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

#ifndef _YAGL_SURFACE_H_
#define _YAGL_SURFACE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_native_types.h"
#include "yagl_resource.h"
#include "EGL/egl.h"

struct yagl_display;
struct yagl_native_drawable;
struct yagl_client_interface;
struct yagl_client_image;
struct yagl_tex_image_binding;

struct yagl_surface
{
    struct yagl_resource res;

    struct yagl_display *dpy;

    EGLenum type;

    /*
     * Non-NULL only for window and pixmap surfaces.
     */
    struct yagl_native_drawable *native_drawable;

    pthread_mutex_t mtx;

    /*
     * This is for EGL_KHR_lock_surface.
     *
     * 'lock_hint' is 0 when surface is not locked.
     * 'lock_hint' is a combination of EGL_READ_SURFACE_BIT_KHR and
     * EGL_WRITE_SURFACE_BIT_KHR when surface is locked.
     *
     * 'lock_ptr' is non-NULL when map has been called. This implies that the surface
     * is locked.
     */
    EGLint lock_hint;
    void *lock_ptr;
    uint32_t lock_stride;

    struct yagl_tex_image_binding *binding;

    int current;

    void (*invalidate)(struct yagl_surface */*sfc*/);

    int (*swap_buffers)(struct yagl_surface */*sfc*/);

    int (*copy_buffers)(struct yagl_surface */*sfc*/, yagl_os_pixmap /*target*/);

    void (*wait_x)(struct yagl_surface */*sfc*/);

    void (*wait_gl)(struct yagl_surface */*sfc*/);

    void (*map)(struct yagl_surface */*sfc*/);

    void (*unmap)(struct yagl_surface */*sfc*/);

    void (*set_swap_interval)(struct yagl_surface */*sfc*/, int /*interval*/);

    struct yagl_client_image *(*create_image)(struct yagl_surface */*sfc*/,
                                              struct yagl_client_interface */*iface*/);
};

/*
 * Takes ownership of 'native_window'.
 */
void yagl_surface_init_window(struct yagl_surface *sfc,
                              yagl_ref_destroy_func destroy_func,
                              yagl_host_handle handle,
                              struct yagl_display *dpy,
                              struct yagl_native_drawable *native_window);

/*
 * Takes ownership of 'native_pixmap'.
 */
void yagl_surface_init_pixmap(struct yagl_surface *sfc,
                              yagl_ref_destroy_func destroy_func,
                              yagl_host_handle handle,
                              struct yagl_display *dpy,
                              struct yagl_native_drawable *native_pixmap);

void yagl_surface_init_pbuffer(struct yagl_surface *sfc,
                               yagl_ref_destroy_func destroy_func,
                               yagl_host_handle handle,
                               struct yagl_display *dpy);

void yagl_surface_cleanup(struct yagl_surface *sfc);

/*
 * Surfaces cannot be simply referenced by 'sfc->res.handle', this is due to
 * Evas GL, it assumes that surface handles are same for same drawables. So,
 * in case of window and pixmap surfaces we'll use the drawable itself as
 * a handle, in case of pbuffer surface we'll simply use 'sfc->res.handle'.
 */
EGLSurface yagl_surface_get_handle(struct yagl_surface *sfc);

void yagl_surface_invalidate(struct yagl_surface *sfc);

int yagl_surface_lock(struct yagl_surface *sfc, EGLint hint);

int yagl_surface_locked(struct yagl_surface *sfc);

int yagl_surface_unlock(struct yagl_surface *sfc);

void *yagl_surface_map(struct yagl_surface *sfc, uint32_t *stride);

/*
 * Guaranteed to succeed.
 */
struct yagl_tex_image_binding
    *yagl_surface_create_tex_image_binding(struct yagl_surface *sfc,
                                           struct yagl_client_interface *iface);

/*
 * Takes ownership of 'binding'.
 */
int yagl_surface_bind_tex_image(struct yagl_surface *sfc,
                                struct yagl_tex_image_binding *binding);

void yagl_surface_release_tex_image(struct yagl_surface *sfc);

int yagl_surface_mark_current(struct yagl_surface *sfc, int current);

/*
 * Helper functions that simply acquire/release yagl_surface::res
 * @{
 */

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_surface_acquire(struct yagl_surface *sfc);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_surface_release(struct yagl_surface *sfc);

/*
 * @}
 */

#endif
