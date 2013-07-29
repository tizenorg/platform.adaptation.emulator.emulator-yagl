#ifndef _YAGL_X11_DRAWABLE_H_
#define _YAGL_X11_DRAWABLE_H_

#include "yagl_export.h"
#include "yagl_native_drawable.h"
#include <X11/Xlib.h>
#include <pthread.h>

#define YAGL_X11_DRAWABLE(os_drawable) ((Drawable)(os_drawable))

struct yagl_x11_drawable
{
    struct yagl_native_drawable base;

    int own_drawable;

    int is_pixmap;

    pthread_mutex_t mtx;

    /*
     * Allocated on first request.
     */
    GC x_gc;

    /*
     * For pixmaps only, filled on first request.
     * @{
     */
    int is_geom_acquired;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    /*
     * @}
     */
};

struct yagl_native_drawable
    *yagl_x11_drawable_create(struct yagl_native_display *dpy,
                              yagl_os_drawable os_drawable,
                              int own_drawable,
                              int is_pixmap);

GC yagl_x11_drawable_get_gc(struct yagl_x11_drawable *drawable);

#endif
