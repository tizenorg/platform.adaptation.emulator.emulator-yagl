#ifndef _YAGL_GBM_PIXMAP_H_
#define _YAGL_GBM_PIXMAP_H_

#include "yagl_export.h"
#include "yagl_native_types.h"

#define YAGL_GBM_PIXMAP(os_pixmap) ((struct gbm_bo*)(os_pixmap))

struct yagl_native_display;
struct yagl_native_drawable;

struct yagl_native_drawable
    *yagl_gbm_pixmap_create(struct yagl_native_display *dpy,
                            yagl_os_pixmap os_pixmap);

#endif
