#ifndef _YAGL_BACKEND_H_
#define _YAGL_BACKEND_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_native_types.h"
#include "EGL/egl.h"

struct yagl_display;
struct yagl_surface;
struct yagl_image;
struct yagl_native_platform;
struct yagl_native_drawable;

struct yagl_backend
{
    struct yagl_display *(*create_display)(struct yagl_native_platform */*platform*/,
                                           yagl_os_display /*os_dpy*/,
                                           yagl_host_handle /*host_dpy*/);

    /*
     * Takes ownership of 'native_window'.
     */
    struct yagl_surface *(*create_window_surface)(struct yagl_display */*dpy*/,
                                                  yagl_host_handle /*host_config*/,
                                                  struct yagl_native_drawable */*native_window*/,
                                                  const EGLint */*attrib_list*/);

    /*
     * Takes ownership of 'native_pixmap'.
     */
    struct yagl_surface *(*create_pixmap_surface)(struct yagl_display */*dpy*/,
                                                  yagl_host_handle /*host_config*/,
                                                  struct yagl_native_drawable */*native_pixmap*/,
                                                  const EGLint */*attrib_list*/);

    struct yagl_surface *(*create_pbuffer_surface)(struct yagl_display */*dpy*/,
                                                   yagl_host_handle /*host_config*/,
                                                   const EGLint */*attrib_list*/);

    /*
     * Takes ownership of 'native_pixmap'.
     */
    struct yagl_image *(*create_image)(struct yagl_display */*dpy*/,
                                       yagl_host_handle /*host_context*/,
                                       struct yagl_native_drawable */*native_pixmap*/,
                                       const EGLint */*attrib_list*/);

    void (*destroy)(struct yagl_backend */*backend*/);

    EGLint y_inverted;
};

struct yagl_backend *yagl_get_backend();

#endif
