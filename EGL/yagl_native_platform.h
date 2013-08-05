#ifndef _YAGL_NATIVE_PLATFORM_H_
#define _YAGL_NATIVE_PLATFORM_H_

#include "yagl_export.h"
#include "yagl_native_types.h"

struct yagl_native_display;

struct yagl_native_platform
{
    int pixmaps_supported;

    int (*probe)(yagl_os_display /*os_dpy*/);

    struct yagl_native_display *(*wrap_display)(yagl_os_display /*os_dpy*/,
                                                int /*enable_drm*/);
};

/*
 * Attempts to guess the platform based on 'os_dpy'.
 * Returns NULL if unsuccessful.
 */
struct yagl_native_platform *yagl_guess_platform(yagl_os_display os_dpy);

#endif
