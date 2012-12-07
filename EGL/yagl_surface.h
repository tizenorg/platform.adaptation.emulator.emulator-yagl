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

    int (*invalidate)(struct yagl_surface */*sfc*/);

    int (*swap_buffers)(struct yagl_surface */*sfc*/);

    int (*copy_buffers)(struct yagl_surface */*sfc*/, Pixmap /*target*/);
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
