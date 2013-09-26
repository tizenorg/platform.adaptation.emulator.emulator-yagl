#ifndef _YAGL_WAYLAND_WINDOW_H_
#define _YAGL_WAYLAND_WINDOW_H_

#include "yagl_export.h"
#include "yagl_native_drawable.h"
#include <wayland-client.h>

#define YAGL_WAYLAND_WINDOW(os_window) ((struct wl_egl_window*)(os_window))

struct vigs_drm_surface;

struct yagl_wayland_window
{
    struct yagl_native_drawable base;

    struct
    {
        struct vigs_drm_surface *drm_sfc;
        struct wl_buffer *wl_buffer;
        int locked;
        int age;
    } color_buffers[3], *back, *front;

    int width;
    int height;

    int dx;
    int dy;

    struct wl_callback *frame_callback;
};

struct yagl_native_drawable
    *yagl_wayland_window_create(struct yagl_native_display *dpy,
                                yagl_os_drawable os_drawable);

#endif
