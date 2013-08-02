#ifndef _YAGL_GBM_WINDOW_H_
#define _YAGL_GBM_WINDOW_H_

#include "yagl_export.h"
#include "yagl_native_types.h"

#define YAGL_GBM_WINDOW(os_window) ((struct gbm_surface*)(os_window))

struct yagl_native_display;
struct yagl_native_drawable;

struct yagl_native_drawable
    *yagl_gbm_window_create(struct yagl_native_display *dpy,
                            yagl_os_window os_window);

#endif
