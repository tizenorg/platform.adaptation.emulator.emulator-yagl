#ifndef _YAGL_WAYLAND_DISPLAY_H_
#define _YAGL_WAYLAND_DISPLAY_H_

#include "yagl_export.h"
#include "yagl_native_display.h"
#include <wayland-client.h>

#define YAGL_WAYLAND_DPY(os_dpy) ((struct wl_display*)(os_dpy))

struct wl_drm;

struct yagl_wayland_display
{
    struct yagl_native_display base;

    struct wl_event_queue *queue;

    struct wl_registry *registry;

    struct wl_drm *wl_drm;

    char *drm_dev_name;

    int drm_fd;

    int authenticated;
};

struct yagl_native_display
    *yagl_wayland_display_create(struct yagl_native_platform *platform,
                                 yagl_os_display os_dpy);

#endif
