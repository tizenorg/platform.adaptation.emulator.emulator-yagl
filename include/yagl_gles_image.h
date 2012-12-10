#ifndef _YAGL_GLES_IMAGE_H_
#define _YAGL_GLES_IMAGE_H_

#include "yagl_export.h"
#include "yagl_types.h"

struct yagl_gles_image
{
    yagl_host_handle host_image;

    void (*update)(struct yagl_gles_image */*image*/);
};

YAGL_API struct yagl_gles_image *yagl_gles_image_acquire(GLeglImageOES image);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_gles_image_release(struct yagl_gles_image *image);

#endif
