#ifndef _YAGL_ONSCREEN_IMAGE_H_
#define _YAGL_ONSCREEN_IMAGE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_image.h"

struct yagl_onscreen_buffer;
struct yagl_native_drawable;

struct yagl_onscreen_image
{
    struct yagl_image base;

    struct yagl_onscreen_buffer *buffer;
};

struct yagl_onscreen_image
    *yagl_onscreen_image_create(struct yagl_display *dpy,
                                yagl_host_handle host_context,
                                struct yagl_native_drawable *native_pixmap,
                                const EGLint* attrib_list);

#endif
