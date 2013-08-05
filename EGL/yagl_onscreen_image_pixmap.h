#ifndef _YAGL_ONSCREEN_IMAGE_PIXMAP_H_
#define _YAGL_ONSCREEN_IMAGE_PIXMAP_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_image.h"

struct yagl_native_drawable;
struct vigs_drm_surface;

struct yagl_onscreen_image_pixmap
{
    struct yagl_image base;

    struct yagl_native_drawable *native_pixmap;

    struct vigs_drm_surface *drm_sfc;
};

struct yagl_onscreen_image_pixmap
    *yagl_onscreen_image_pixmap_create(struct yagl_display *dpy,
                                       yagl_host_handle host_context,
                                       struct yagl_native_drawable *native_pixmap,
                                       const EGLint* attrib_list);

#endif
