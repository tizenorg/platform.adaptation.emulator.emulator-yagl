#include "yagl_x11_platform.h"
#include "yagl_x11_display.h"
#include "yagl_native_platform.h"
#include "yagl_log.h"
#include "EGL/egl.h"
#include <X11/Xlib.h>

static int yagl_x11_platform_probe(yagl_os_display os_dpy)
{
    /*
     * Can't probe for X11, so assume probe passed.
     */

    return 1;
}

static struct yagl_native_display
    *yagl_x11_wrap_display(yagl_os_display os_dpy,
                           int enable_drm)
{
    struct yagl_native_display *dpy;

    YAGL_LOG_FUNC_SET(eglGetDisplay);

    if (os_dpy == (yagl_os_display)EGL_DEFAULT_DISPLAY) {
        Display *x_dpy = XOpenDisplay(0);

        if (!x_dpy) {
            YAGL_LOG_ERROR("unable to open display 0");
            return NULL;
        }

        dpy = yagl_x11_display_create(&yagl_x11_platform,
                                      (yagl_os_display)x_dpy,
                                      1,
                                      enable_drm);

        if (!dpy) {
            XCloseDisplay(x_dpy);
        }
    } else {
        dpy = yagl_x11_display_create(&yagl_x11_platform,
                                      os_dpy,
                                      0,
                                      enable_drm);
    }

    return dpy;
}

struct yagl_native_platform yagl_x11_platform =
{
    .probe = yagl_x11_platform_probe,
    .wrap_display = yagl_x11_wrap_display
};
