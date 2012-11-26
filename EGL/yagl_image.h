#ifndef _YAGL_IMAGE_H_
#define _YAGL_IMAGE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_resource.h"
#include <X11/X.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

struct yagl_display;
struct yagl_gles_image;

struct yagl_image
{
    struct yagl_resource res;

    struct yagl_display *dpy;

    struct yagl_gles_image *gles_image;
};

struct yagl_image *yagl_image_create(Pixmap x_pixmap,
                                     struct yagl_display *dpy);

EGLImageKHR yagl_image_get_handle(struct yagl_image *image);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_image_acquire(struct yagl_image *image);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_image_release(struct yagl_image *image);

#endif
