#include "yagl_native_platform.h"
#ifdef YAGL_PLATFORM_X11
#include "x11/yagl_x11_platform.h"
#endif
#ifdef YAGL_PLATFORM_GBM
#include "gbm/yagl_gbm_platform.h"
#endif
#ifdef YAGL_PLATFORM_WAYLAND
#include "wayland/yagl_wayland_platform.h"
#endif
#include "yagl_log.h"
#include <stdlib.h>
#include <string.h>

static struct
{
   struct yagl_native_platform *platform;
   const char *name;
} g_platforms[] =
{
#ifdef YAGL_PLATFORM_WAYLAND
    {&yagl_wayland_platform, "wayland"},
#endif
#ifdef YAGL_PLATFORM_GBM
    {&yagl_gbm_platform, "gbm"},
#endif
#ifdef YAGL_PLATFORM_X11
    {&yagl_x11_platform, "x11"},
#endif
};

struct yagl_native_platform *yagl_guess_platform(yagl_os_display os_dpy)
{
    const char *platform_name = NULL;
    uint32_t i;

    YAGL_LOG_FUNC_SET(yagl_guess_platform);

    platform_name = getenv("EGL_PLATFORM");

    if (!platform_name || !platform_name[0]) {
        platform_name = getenv("EGL_DISPLAY");
    }

    if (platform_name && platform_name[0]) {
        for (i = 0; i < sizeof(g_platforms)/sizeof(g_platforms[0]); ++i) {
            if (strcmp(g_platforms[i].name, platform_name) == 0) {
                YAGL_LOG_INFO("EGL platform %s chosen by environment variable",
                              g_platforms[i].name);
                return g_platforms[i].platform;
            }
        }
    }

    for (i = 0; i < sizeof(g_platforms)/sizeof(g_platforms[0]); ++i) {
        if (g_platforms[i].platform->probe(os_dpy)) {
            YAGL_LOG_INFO("EGL platform %s chosen by probing",
                          g_platforms[i].name);
            return g_platforms[i].platform;
        }
    }

    YAGL_LOG_ERROR("No EGL platform chosen");

    return NULL;
}
