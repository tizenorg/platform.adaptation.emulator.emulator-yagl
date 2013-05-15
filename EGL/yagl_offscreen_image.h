#ifndef _YAGL_OFFSCREEN_IMAGE_H_
#define _YAGL_OFFSCREEN_IMAGE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_image.h"

struct yagl_offscreen_image
{
    struct yagl_image base;

    uint32_t width;
    uint32_t height;
};

struct yagl_offscreen_image
    *yagl_offscreen_image_create(struct yagl_display *dpy,
                                 yagl_host_handle host_context,
                                 Pixmap x_pixmap,
                                 const EGLint* attrib_list);

#endif
