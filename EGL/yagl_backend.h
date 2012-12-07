#ifndef _YAGL_BACKEND_H_
#define _YAGL_BACKEND_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include <X11/X.h>
#include <EGL/egl.h>

struct yagl_display;
struct yagl_surface;
struct yagl_image;

struct yagl_backend
{
    struct yagl_surface *(*create_window_surface)(struct yagl_display */*dpy*/,
                                                  yagl_host_handle /*host_config*/,
                                                  Window /*x_win*/,
                                                  const EGLint* /*attrib_list*/);

    struct yagl_surface *(*create_pixmap_surface)(struct yagl_display */*dpy*/,
                                                  yagl_host_handle /*host_config*/,
                                                  Pixmap /*x_pixmap*/,
                                                  const EGLint* /*attrib_list*/);

    struct yagl_surface *(*create_pbuffer_surface)(struct yagl_display */*dpy*/,
                                                   yagl_host_handle /*host_config*/,
                                                   const EGLint* /*attrib_list*/);

    struct yagl_image *(*create_image)(struct yagl_display */*dpy*/,
                                       Pixmap /*x_pixmap*/,
                                       const EGLint */*attrib_list*/);

    void (*destroy)(struct yagl_backend */*backend*/);
};

struct yagl_backend *yagl_get_backend();

#endif
