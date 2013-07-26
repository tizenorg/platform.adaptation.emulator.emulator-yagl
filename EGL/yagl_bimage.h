#ifndef _YAGL_BIMAGE_H_
#define _YAGL_BIMAGE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "EGL/egl.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

/*
 * Backing image for offscreen surfaces.
 */

struct yagl_display;

struct yagl_bimage
{
    uint32_t width;
    uint32_t height;
    uint32_t depth; /* bit-depth. e.g.: 24 */

    uint32_t bpp; /* bytes-per-pixel. e.g.: 3 */
    void *pixels; /* pixel data */

    struct yagl_display *dpy; /* The owning display */
    XShmSegmentInfo x_shm; /* X11 shared memory segment */
    XImage* x_image; /* X11 image */
};

struct yagl_bimage *yagl_bimage_create(struct yagl_display *dpy,
                                       uint32_t width,
                                       uint32_t height,
                                       uint32_t depth);

void yagl_bimage_destroy(struct yagl_bimage *bi);

void yagl_bimage_draw(struct yagl_bimage *bi, Drawable target, GC gc);

#endif
