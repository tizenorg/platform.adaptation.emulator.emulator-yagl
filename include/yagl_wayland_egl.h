#ifndef _YAGL_WAYLAND_EGL_H_
#define _YAGL_WAYLAND_EGL_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include <wayland-egl.h>

struct wl_egl_window
{
    struct wl_surface *surface;

    int width;
    int height;
    int dx;
    int dy;

    int attached_width;
    int attached_height;

    void *user_data;
    void (*resize_callback)(struct wl_egl_window */*egl_window*/,
                            void */*user_data*/);
};

#endif
