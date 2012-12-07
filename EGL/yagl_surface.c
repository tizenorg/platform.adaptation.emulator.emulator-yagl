#include "yagl_surface.h"
#include <assert.h>

void yagl_surface_init_window(struct yagl_surface *sfc,
                              yagl_ref_destroy_func destroy_func,
                              yagl_host_handle handle,
                              struct yagl_display *dpy,
                              Window x_win)
{
    yagl_resource_init(&sfc->res, destroy_func, handle);

    sfc->dpy = dpy;
    sfc->type = EGL_WINDOW_BIT;
    sfc->x_drawable.win = x_win;
}

void yagl_surface_init_pixmap(struct yagl_surface *sfc,
                              yagl_ref_destroy_func destroy_func,
                              yagl_host_handle handle,
                              struct yagl_display *dpy,
                              Pixmap x_pixmap)
{
    yagl_resource_init(&sfc->res, destroy_func, handle);

    sfc->dpy = dpy;
    sfc->type = EGL_PIXMAP_BIT;
    sfc->x_drawable.pixmap = x_pixmap;
}

void yagl_surface_init_pbuffer(struct yagl_surface *sfc,
                               yagl_ref_destroy_func destroy_func,
                               yagl_host_handle handle,
                               struct yagl_display *dpy)
{
    yagl_resource_init(&sfc->res, destroy_func, handle);

    sfc->dpy = dpy;
    sfc->type = EGL_PBUFFER_BIT;
}

void yagl_surface_cleanup(struct yagl_surface *sfc)
{
    yagl_resource_cleanup(&sfc->res);
}

EGLSurface yagl_surface_get_handle(struct yagl_surface *sfc)
{
    switch (sfc->type) {
    case EGL_PBUFFER_BIT:
        return (EGLSurface)sfc->res.handle;
    case EGL_PIXMAP_BIT:
        return (EGLSurface)sfc->x_drawable.pixmap;
    case EGL_WINDOW_BIT:
        return (EGLSurface)sfc->x_drawable.win;
    default:
        assert(0);
        return NULL;
    }
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
