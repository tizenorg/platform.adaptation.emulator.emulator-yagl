#ifndef _YAGL_IMAGE_H_
#define _YAGL_IMAGE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_resource.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"

struct yagl_display;
struct yagl_client_image;

struct yagl_image
{
    struct yagl_resource res;

    struct yagl_display *dpy;

    EGLImageKHR client_handle;

    struct yagl_client_image *client_image;

    void (*update)(struct yagl_image */*image*/);
};

void yagl_image_init(struct yagl_image *image,
                     yagl_ref_destroy_func destroy_func,
                     struct yagl_display *dpy,
                     EGLImageKHR client_handle,
                     struct yagl_client_image *client_image);

void yagl_image_cleanup(struct yagl_image *image);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_image_acquire(struct yagl_image *image);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_image_release(struct yagl_image *image);

#endif
