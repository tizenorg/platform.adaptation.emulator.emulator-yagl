#ifndef _YAGL_NATIVE_IMAGE_H_
#define _YAGL_NATIVE_IMAGE_H_

#include "yagl_export.h"
#include "yagl_native_types.h"

struct yagl_native_drawable;

struct yagl_native_image
{
    struct yagl_native_display *dpy;

    uint32_t width;
    uint32_t height;
    uint32_t depth; /* bit-depth. e.g.: 24 */

    uint32_t bpp; /* bytes-per-pixel. e.g.: 3 */
    void *pixels; /* pixel data */

    void (*draw)(struct yagl_native_image */*image*/,
                 struct yagl_native_drawable */*drawable*/);

    void (*draw_to_pixmap)(struct yagl_native_image */*image*/,
                           yagl_os_pixmap /*os_pixmap*/);

    void (*destroy)(struct yagl_native_image */*image*/);
};

void yagl_native_image_init(struct yagl_native_image *image,
                            struct yagl_native_display *dpy,
                            uint32_t width,
                            uint32_t height,
                            uint32_t depth,
                            uint32_t bpp,
                            void *pixels);

void yagl_native_image_cleanup(struct yagl_native_image *image);

#endif
