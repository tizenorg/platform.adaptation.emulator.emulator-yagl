#ifndef _YAGL_GLES_IMAGE_H_
#define _YAGL_GLES_IMAGE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include <X11/X.h>
#include <X11/Xlib.h>

struct yagl_gles_image
{
    void *opaque;

    Display *x_dpy;

    Pixmap x_pixmap;

    uint32_t width;
    uint32_t height;
};

YAGL_API struct yagl_gles_image *yagl_gles_image_acquire(GLeglImageOES image);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_gles_image_release(struct yagl_gles_image *image);

#endif
