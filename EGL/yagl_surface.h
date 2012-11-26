#ifndef _YAGL_SURFACE_H_
#define _YAGL_SURFACE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_resource.h"
#include <EGL/egl.h>
#include <pthread.h>

struct yagl_display;
struct yagl_bimage;

struct yagl_surface
{
    struct yagl_resource res;

    struct yagl_display *dpy;

    EGLenum type;

    /*
     * See host:yagl_egl_surface.h:bimage_mtx for explanation.
     */
    pthread_mutex_t bi_mtx;

    struct yagl_bimage *bi;

    union
    {
        Window win;
        Pixmap pixmap;
    } x_drawable;

    GC x_gc;
};

/*
 * These take ownership of 'bi'. Guaranteed to succeed.
 * @{
 */

struct yagl_surface *yagl_surface_create_window(yagl_host_handle handle,
                                                struct yagl_bimage *bi,
                                                Window x_win);

struct yagl_surface *yagl_surface_create_pixmap(yagl_host_handle handle,
                                                struct yagl_bimage *bi,
                                                Pixmap x_pixmap);

struct yagl_surface *yagl_surface_create_pbuffer(yagl_host_handle handle,
                                                 struct yagl_bimage *bi);

/*
 * Surfaces cannot be simply referenced by 'sfc->res.handle', this is due to
 * Evas GL, it assumes that surface handles are same for same drawables. So,
 * in case of window and pixmap surfaces we'll use the drawable itself as
 * a handle, in case of pbuffer surface we'll simply use 'sfc->res.handle'.
 */
EGLSurface yagl_surface_get_handle(struct yagl_surface *sfc);

/*
 * Lock/unlock surface for 'bi' access, all 'bi' operations
 * must be carried out while surface is locked.
 * @{
 */

void yagl_surface_lock(struct yagl_surface *sfc);

void yagl_surface_unlock(struct yagl_surface *sfc);

/*
 * @}
 */

void yagl_surface_update(struct yagl_surface *sfc,
                         struct yagl_bimage *bi);

/*
 * @}
 */

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
