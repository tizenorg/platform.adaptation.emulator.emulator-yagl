#include "yagl_image.h"
#include "yagl_malloc.h"
#include "yagl_display.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <string.h>
#include "yagl_gles_image.h"

static void yagl_image_destroy(struct yagl_ref *ref)
{
    struct yagl_image *image = (struct yagl_image*)ref;

    yagl_free(image->gles_image);

    yagl_resource_cleanup(&image->res);

    yagl_free(image);
}

struct yagl_image *yagl_image_create(Pixmap x_pixmap,
                                     struct yagl_display *dpy)
{
    struct yagl_image *image;
    unsigned int depth = 0;
    union { Window w; int i; unsigned int ui; } tmp_geom;

    image = yagl_malloc0(sizeof(*image));

    yagl_resource_init(&image->res, &yagl_image_destroy, 0);

    image->dpy = dpy;

    image->gles_image = yagl_malloc0(sizeof(*image->gles_image));

    image->gles_image->opaque = image;

    image->gles_image->x_dpy = dpy->x_dpy;

    image->gles_image->x_pixmap = x_pixmap;

    memset(&tmp_geom, 0, sizeof(tmp_geom));

    XGetGeometry(dpy->x_dpy,
                 x_pixmap,
                 &tmp_geom.w,
                 &tmp_geom.i,
                 &tmp_geom.i,
                 &image->gles_image->width,
                 &image->gles_image->height,
                 &tmp_geom.ui,
                 &depth);

    return image;
}

EGLImageKHR yagl_image_get_handle(struct yagl_image *image)
{
    return (EGLImageKHR)image->gles_image->x_pixmap;
}

void yagl_image_acquire(struct yagl_image *image)
{
    if (image) {
        yagl_resource_acquire(&image->res);
    }
}

void yagl_image_release(struct yagl_image *image)
{
    if (image) {
        yagl_resource_release(&image->res);
    }
}
