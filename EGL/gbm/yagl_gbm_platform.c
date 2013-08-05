#include "yagl_gbm_platform.h"
#include "yagl_gbm_display.h"
#include "yagl_native_platform.h"
#include "yagl_gbm.h"
#include "EGL/egl.h"

static int yagl_gbm_platform_probe(yagl_os_display os_dpy)
{
    void *first_pointer;

    if (os_dpy == (yagl_os_display)EGL_DEFAULT_DISPLAY) {
        return 0;
    }

    first_pointer = *(void**)os_dpy;

    return (first_pointer == &gbm_create_device);
}

static struct yagl_native_display
    *yagl_gbm_wrap_display(yagl_os_display os_dpy,
                           int enable_drm)
{
    return yagl_gbm_display_create(&yagl_gbm_platform, os_dpy);
}

struct yagl_native_platform yagl_gbm_platform =
{
    .pixmaps_supported = 1,
    .probe = yagl_gbm_platform_probe,
    .wrap_display = yagl_gbm_wrap_display
};
