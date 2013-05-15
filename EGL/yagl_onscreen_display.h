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

struct yagl_onscreen_display
    *yagl_onscreen_display_create(EGLNativeDisplayType display_id,
                                  Display *x_dpy,
                                  yagl_host_handle host_dpy);

int yagl_onscreen_display_create_buffer(struct yagl_onscreen_display* dpy,
                                        Drawable d,
                                        unsigned int attachment,
                                        yagl_DRI2Buffer **buffer,
                                        yagl_winsys_id *id,
                                        uint32_t *width,
                                        uint32_t *height);

void yagl_onscreen_display_destroy_buffer(yagl_DRI2Buffer *buffer);

#endif
