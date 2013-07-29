#include "yagl_gbm_platform.h"
#include "yagl_native_platform.h"

static int yagl_gbm_platform_probe(yagl_os_display os_dpy)
{
    return 0;
}

static struct yagl_native_display
    *yagl_gbm_wrap_display(yagl_os_display os_dpy,
                           int enable_drm)
{
    return NULL;
}

struct yagl_native_platform yagl_gbm_platform =
{
    .probe = yagl_gbm_platform_probe,
    .wrap_display = yagl_gbm_wrap_display
};
