#ifndef _YAGL_ONSCREEN_DISPLAY_H_
#define _YAGL_ONSCREEN_DISPLAY_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_display.h"
#include "yagl_dri2.h"
#include "vigs.h"

struct yagl_onscreen_display
{
    struct yagl_display base;

    int drm_fd;
    struct vigs_drm_device *drm_dev;
};

struct yagl_onscreen_buffer
{
    yagl_DRI2Buffer *dri2_buffer;

    struct vigs_drm_surface *drm_sfc;
};

struct yagl_onscreen_display
    *yagl_onscreen_display_create(EGLNativeDisplayType display_id,
                                  Display *x_dpy,
                                  yagl_host_handle host_dpy);

struct yagl_onscreen_buffer
    *yagl_onscreen_display_create_buffer(struct yagl_onscreen_display* dpy,
                                         Drawable d,
                                         unsigned int attachment,
                                         uint32_t check_name);

void yagl_onscreen_display_destroy_buffer(struct yagl_onscreen_buffer *buffer);

#endif
