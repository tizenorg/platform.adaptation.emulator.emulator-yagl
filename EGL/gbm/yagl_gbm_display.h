#ifndef _YAGL_GBM_DISPLAY_H_
#define _YAGL_GBM_DISPLAY_H_

#include "yagl_export.h"
#include "yagl_native_types.h"

#define YAGL_GBM_DPY(os_dpy) ((struct gbm_device*)(os_dpy))

struct yagl_native_platform;
struct yagl_native_display;

struct yagl_native_display
    *yagl_gbm_display_create(struct yagl_native_platform *platform,
                             yagl_os_display os_dpy);

#endif
