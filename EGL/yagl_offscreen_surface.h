#ifndef _YAGL_OFFSCREEN_SURFACE_H_
#define _YAGL_OFFSCREEN_SURFACE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_surface.h"

struct yagl_native_image;
struct yagl_native_drawable;

struct yagl_offscreen_surface
{
    struct yagl_surface base;

    struct yagl_native_image *bi;
};

struct yagl_offscreen_surface
    *yagl_offscreen_surface_create_window(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          struct yagl_native_drawable *native_window,
                                          const EGLint* attrib_list);

struct yagl_offscreen_surface
    *yagl_offscreen_surface_create_pixmap(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          struct yagl_native_drawable *native_pixmap,
                                          const EGLint* attrib_list);

struct yagl_offscreen_surface
    *yagl_offscreen_surface_create_pbuffer(struct yagl_display *dpy,
                                           yagl_host_handle host_config,
                                           const EGLint* attrib_list);

#endif
