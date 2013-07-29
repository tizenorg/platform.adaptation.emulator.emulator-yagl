#ifndef _YAGL_ONSCREEN_DISPLAY_H_
#define _YAGL_ONSCREEN_DISPLAY_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_display.h"

struct vigs_drm_device;

struct yagl_onscreen_display
{
    struct yagl_display base;

    struct vigs_drm_device *drm_dev;
};

struct yagl_onscreen_display
    *yagl_onscreen_display_create(yagl_os_display display_id,
                                  struct yagl_native_display *native_dpy,
                                  yagl_host_handle host_dpy);

#endif
