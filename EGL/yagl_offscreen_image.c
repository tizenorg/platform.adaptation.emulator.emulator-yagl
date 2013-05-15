#include "yagl_offscreen_image.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_display.h"
#include "yagl_state.h"
#include "yagl_host_egl_calls.h"
#include "yagl_mem_egl.h"
#include <string.h>

static void yagl_offscreen_image_update(struct yagl_image *image)
{
    struct yagl_offscreen_image *oimage = (struct yagl_offscreen_image*)image;
    XImage *x_image = NULL;

    YAGL_LOG_FUNC_SET(yagl_offscreen_image_update);

    x_image = XGetImage(image->dpy->x_dpy,
                        image->x_pixmap,
                        0,
                        0,
                        oimage->width,
                        oimage->height,
                        AllPlanes,
                        ZPixmap);

    if (!x_image) {
        YAGL_LOG_ERROR("XGetImage failed for image %u", image->res.handle);
        return;
    }

    while (!yagl_host_eglUpdateOffscreenImageYAGL(image->dpy->host_dpy,
            image->res.handle,
            oimage->width,
            oimage->height,
            (x_image->bits_per_pixel / 8),
            yagl_batch_put(x_image->data, (oimage->width *
                                           oimage->height *
                                           (x_image->bits_per_pixel / 8))))) {}

    XDestroyImage(x_image);
}

static void yagl_offscreen_image_destroy(struct yagl_ref *ref)
{
    struct yagl_offscreen_image *image = (struct yagl_offscreen_image*)ref;

    yagl_image_cleanup(&image->base);

    yagl_free(image);
}

struct yagl_offscreen_image
    *yagl_offscreen_image_create(struct yagl_display *dpy,
                                 yagl_host_handle host_context,
                                 Pixmap x_pixmap,
                                 const EGLint* attrib_list)
{
    struct yagl_offscreen_image *image;
    unsigned int depth = 0;
    union { Window w; int i; unsigned int ui; } tmp_geom;
    yagl_host_handle host_image = 0;

    image = yagl_malloc0(sizeof(*image));

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreateImageKHR(&host_image,
        dpy->host_dpy,
        host_context,
        EGL_NATIVE_PIXMAP_KHR,
        x_pixmap,
        attrib_list));

    if (!host_image) {
        goto fail;
    }

    memset(&tmp_geom, 0, sizeof(tmp_geom));

    XGetGeometry(dpy->x_dpy,
                 x_pixmap,
                 &tmp_geom.w,
                 &tmp_geom.i,
                 &tmp_geom.i,
                 &image->width,
                 &image->height,
                 &tmp_geom.ui,
                 &depth);

    yagl_image_init(&image->base,
                    &yagl_offscreen_image_destroy,
                    host_image,
                    dpy,
                    x_pixmap);

    image->base.update = &yagl_offscreen_image_update;

    return image;

fail:
    yagl_free(image);

    return NULL;
}
