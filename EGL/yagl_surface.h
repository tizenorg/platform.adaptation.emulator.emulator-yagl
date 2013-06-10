#ifndef _YAGL_SURFACE_H_
#define _YAGL_SURFACE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_resource.h"
#include <EGL/egl.h>

struct yagl_display;

struct yagl_surface
{
    struct yagl_resource res;

    struct yagl_display *dpy;

    EGLenum type;

    union
    {
        Window win;
        Pixmap pixmap;
    } x_drawable;

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

    void (*invalidate)(struct yagl_surface */*sfc*/);

    void (*finish)(struct yagl_surface */*sfc*/);

    int (*swap_buffers)(struct yagl_surface */*sfc*/);

    int (*copy_buffers)(struct yagl_surface */*sfc*/, Pixmap /*target*/);

    void (*wait_x)(struct yagl_surface */*sfc*/);

    void (*wait_gl)(struct yagl_surface */*sfc*/);

    void (*map)(struct yagl_surface */*sfc*/);

    void (*unmap)(struct yagl_surface */*sfc*/);
};

void yagl_surface_init_window(struct yagl_surface *sfc,
                              yagl_ref_destroy_func destroy_func,
                              yagl_host_handle handle,
                              struct yagl_display *dpy,
                              Window x_win);

void yagl_surface_init_pixmap(struct yagl_surface *sfc,
                              yagl_ref_destroy_func destroy_func,
                              yagl_host_handle handle,
                              struct yagl_display *dpy,
                              Pixmap x_pixmap);

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
