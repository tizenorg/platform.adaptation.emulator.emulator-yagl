#include "yagl_onscreen_display.h"
#include "yagl_malloc.h"
#include "yagl_native_display.h"
#include "vigs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct yagl_onscreen_display
    *yagl_onscreen_display_create(yagl_os_display display_id,
                                  struct yagl_native_display *native_dpy,
                                  yagl_host_handle host_dpy)
{
    int ret;
    struct vigs_drm_device *drm_dev = NULL;
    struct yagl_onscreen_display *dpy = NULL;

    ret = vigs_drm_device_create(native_dpy->drm_fd, &drm_dev);

    if (ret != 0) {
        fprintf(stderr, "Critical error! vigs_drm_device_create failed: %s\n", strerror(-ret));
        return NULL;
    }

    dpy = yagl_malloc0(sizeof(*dpy));

    yagl_display_init(&dpy->base, display_id, native_dpy, host_dpy);

    dpy->drm_dev = drm_dev;

    return dpy;
}
