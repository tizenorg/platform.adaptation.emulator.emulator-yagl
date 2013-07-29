#include "yagl_native_display.h"

void yagl_native_display_init(struct yagl_native_display *dpy,
                              struct yagl_native_platform *platform,
                              yagl_os_display os_dpy,
                              int drm_fd)
{
    dpy->platform = platform;
    dpy->os_dpy = os_dpy;
    dpy->drm_fd = drm_fd;
}

void yagl_native_display_cleanup(struct yagl_native_display *dpy)
{
    dpy->drm_fd = -1;
}
