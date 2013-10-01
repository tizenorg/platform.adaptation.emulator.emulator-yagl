#ifndef _YAGL_CLIENT_IMAGE_H_
#define _YAGL_CLIENT_IMAGE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_object.h"

struct yagl_client_image
{
    struct yagl_object base;

    void (*update)(struct yagl_client_image */*image*/,
                   uint32_t /*width*/,
                   uint32_t /*height*/,
                   uint32_t /*bpp*/,
                   const void */*pixels*/);
};

YAGL_API void yagl_client_image_init(struct yagl_client_image *image,
                                     yagl_ref_destroy_func destroy_func);

YAGL_API void yagl_client_image_cleanup(struct yagl_client_image *image);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_client_image_acquire(struct yagl_client_image *image);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_client_image_release(struct yagl_client_image *image);

YAGL_API struct yagl_client_image *yagl_acquire_client_image(yagl_host_handle handle);

#endif
