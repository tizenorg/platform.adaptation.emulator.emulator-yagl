#include "yagl_native_display.h"

void yagl_native_display_init(struct yagl_native_display *dpy,
                              struct yagl_native_platform *platform,
                              yagl_os_display os_dpy,
                              struct vigs_drm_device *drm_dev)
{
    dpy->platform = platform;
    dpy->os_dpy = os_dpy;
    dpy->drm_dev = drm_dev;
}

void yagl_native_display_cleanup(struct yagl_native_display *dpy)
{
    dpy->drm_dev = NULL;
}
