#ifndef _YAGL_GLES_IMAGE_H_
#define _YAGL_GLES_IMAGE_H_

#include "yagl_types.h"
#include "yagl_client_image.h"

struct yagl_gles_image
{
    struct yagl_client_image base;

    yagl_object_name tex_global_name;
};

struct yagl_gles_image *yagl_gles_image_create(yagl_object_name tex_global_name);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_image_acquire(struct yagl_gles_image *image);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_image_release(struct yagl_gles_image *image);

#endif
