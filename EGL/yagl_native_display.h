#ifndef _YAGL_NATIVE_DISPLAY_H_
#define _YAGL_NATIVE_DISPLAY_H_

#include "yagl_export.h"
#include "yagl_native_types.h"

struct yagl_native_platform;
struct yagl_native_drawable;
struct yagl_native_image;
struct vigs_drm_device;

struct yagl_native_display
{
    struct yagl_native_platform *platform;

    yagl_os_display os_dpy;

    struct vigs_drm_device *drm_dev;

    struct yagl_native_drawable *(*wrap_window)(struct yagl_native_display */*dpy*/,
                                                yagl_os_window /*os_window*/);

    struct yagl_native_drawable *(*wrap_pixmap)(struct yagl_native_display */*dpy*/,
                                                yagl_os_pixmap /*os_pixmap*/);

    struct yagl_native_drawable *(*create_pixmap)(struct yagl_native_display */*dpy*/,
                                                  uint32_t /*width*/,
                                                  uint32_t /*height*/,
                                                  uint32_t /*depth*/);

    struct yagl_native_image *(*create_image)(struct yagl_native_display */*dpy*/,
                                              uint32_t /*width*/,
                                              uint32_t /*height*/,
                                              uint32_t /*depth*/);

    int (*get_visual)(struct yagl_native_display */*dpy*/,
                      int */*visual_id*/,
                      int */*visual_type*/);

    void (*destroy)(struct yagl_native_display */*dpy*/);
};

void yagl_native_display_init(struct yagl_native_display *dpy,
                              struct yagl_native_platform *platform,
                              yagl_os_display os_dpy,
                              struct vigs_drm_device *drm_dev);

void yagl_native_display_cleanup(struct yagl_native_display *dpy);

#endif
