#ifndef _YAGL_OFFSCREEN_SURFACE_H_
#define _YAGL_OFFSCREEN_SURFACE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_surface.h"
#include <pthread.h>

struct yagl_bimage;

struct yagl_offscreen_surface
{
    struct yagl_surface base;

    pthread_mutex_t bi_mtx;

    struct yagl_bimage *bi;

    GC x_gc;
};

struct yagl_offscreen_surface
    *yagl_offscreen_surface_create_window(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          Window x_win,
                                          const EGLint* attrib_list);

struct yagl_offscreen_surface
    *yagl_offscreen_surface_create_pixmap(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          Pixmap x_pixmap,
                                          const EGLint* attrib_list);

struct yagl_offscreen_surface
    *yagl_offscreen_surface_create_pbuffer(struct yagl_display *dpy,
                                           yagl_host_handle host_config,
                                           const EGLint* attrib_list);

#endif
