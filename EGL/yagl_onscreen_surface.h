#ifndef _YAGL_onscreen_SURFACE_H_
#define _YAGL_onscreen_SURFACE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_surface.h"
#include <X11/extensions/Xfixes.h>

struct yagl_onscreen_surface
{
    struct yagl_surface base;

    XserverRegion x_region;
    int last_width;
    int last_height;
};

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_window(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         Window x_win,
                                         const EGLint* attrib_list);

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pixmap(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         Pixmap x_pixmap,
                                         const EGLint* attrib_list);

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pbuffer(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          const EGLint* attrib_list);

#endif
