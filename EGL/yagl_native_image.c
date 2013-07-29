#include "yagl_native_image.h"

void yagl_native_image_init(struct yagl_native_image *image,
                            struct yagl_native_display *dpy,
                            uint32_t width,
                            uint32_t height,
                            uint32_t depth,
                            uint32_t bpp,
                            void *pixels)
{
    image->dpy = dpy;
    image->width = width;
    image->height = height;
    image->depth = depth;
    image->bpp = bpp;
    image->pixels = pixels;
}

void yagl_native_image_cleanup(struct yagl_native_image *image)
{
}
