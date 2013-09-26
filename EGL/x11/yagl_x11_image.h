#ifndef _YAGL_X11_IMAGE_H_
#define _YAGL_X11_IMAGE_H_

#include "yagl_export.h"
#include "yagl_native_image.h"
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

struct yagl_x11_image
{
    struct yagl_native_image base;

    XImage *x_image; /* X11 image */
    XShmSegmentInfo x_shm; /* X11 shared memory segment */

    int is_wrapped;
};

struct yagl_native_image
    *yagl_x11_image_create(struct yagl_native_display *dpy,
                           uint32_t width,
                           uint32_t height,
                           uint32_t depth);

/*
 * Takes ownership of 'x_image'.
 */
struct yagl_native_image
    *yagl_x11_image_wrap(struct yagl_native_display *dpy,
                         XImage *x_image);

#endif
