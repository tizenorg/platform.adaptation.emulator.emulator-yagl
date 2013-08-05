#ifndef _WAYLAND_DRM_H_
#define _WAYLAND_DRM_H_

#include "yagl_export.h"
#include "yagl_types.h"

struct wl_drm;
struct wl_resource;
struct wl_display;
struct vigs_drm_surface;

struct wl_drm_buffer
{
    struct wl_resource *resource;

    struct wl_drm *drm;

    uint32_t format;

    struct vigs_drm_surface *drm_sfc;
};

struct wayland_drm_callbacks
{
    int (*authenticate)(void */*user_data*/,
                        uint32_t /*id*/);

    struct vigs_drm_surface *(*acquire_buffer)(void */*user_data*/,
                                               uint32_t /*name*/);
};

struct wl_drm *wayland_drm_create(struct wl_display *display,
                                  char *device_name,
                                  struct wayland_drm_callbacks *callbacks,
                                  void *user_data);

void wayland_drm_destroy(struct wl_drm *drm);

struct wl_drm_buffer *wayland_drm_get_buffer(struct wl_resource *resource);

#endif
