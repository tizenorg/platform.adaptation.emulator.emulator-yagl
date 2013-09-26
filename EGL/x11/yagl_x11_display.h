#ifndef _YAGL_X11_DISPLAY_H_
#define _YAGL_X11_DISPLAY_H_

#include "yagl_export.h"
#include "yagl_native_display.h"

#define YAGL_X11_DPY(os_dpy) ((Display*)(os_dpy))

struct yagl_x11_display
{
    struct yagl_native_display base;

    int own_dpy;

    int xshm_images_supported;

    int xshm_pixmaps_supported;
};

struct yagl_native_display *yagl_x11_display_create(struct yagl_native_platform *platform,
                                                    yagl_os_display os_dpy,
                                                    int own_dpy,
                                                    int enable_drm);

#endif
