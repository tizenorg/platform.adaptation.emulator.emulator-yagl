#include "yagl_offscreen_image_pixmap.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_display.h"
#include "yagl_state.h"
#include "yagl_host_egl_calls.h"
#include "yagl_mem_egl.h"
#include "yagl_native_drawable.h"
#include "yagl_native_image.h"
#include <string.h>

static void yagl_offscreen_image_pixmap_update(struct yagl_image *image)
{
    struct yagl_offscreen_image_pixmap *oimage = (struct yagl_offscreen_image_pixmap*)image;
    struct yagl_native_image *native_image = NULL;

    YAGL_LOG_FUNC_SET(yagl_offscreen_image_pixmap_update);

    native_image = oimage->native_pixmap->get_image(oimage->native_pixmap,
                                                    oimage->width,
                                                    oimage->height);

    if (!native_image) {
        YAGL_LOG_ERROR("get_image failed for image %u", image->res.handle);
        return;
    }

    while (!yagl_host_eglUpdateOffscreenImageYAGL(image->dpy->host_dpy,
            image->res.handle,
            native_image->width,
            native_image->height,
            native_image->bpp,
            yagl_batch_put(native_image->pixels, (native_image->width *
                                                  native_image->height *
                                                  native_image->bpp)))) {}

    native_image->destroy(native_image);
}

static void yagl_offscreen_image_pixmap_destroy(struct yagl_ref *ref)
{
    struct yagl_offscreen_image_pixmap *image = (struct yagl_offscreen_image_pixmap*)ref;

    image->native_pixmap->destroy(image->native_pixmap);
    image->native_pixmap = NULL;

    yagl_image_cleanup(&image->base);

    yagl_free(image);
}

struct yagl_offscreen_image_pixmap
    *yagl_offscreen_image_pixmap_create(struct yagl_display *dpy,
                                        yagl_host_handle host_context,
                                        struct yagl_native_drawable *native_pixmap,
                                        const EGLint* attrib_list)
{
    yagl_host_handle host_image = 0;
    struct yagl_offscreen_image_pixmap *image;
    uint32_t depth;

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreateImageKHR(&host_image,
        dpy->host_dpy,
        host_context,
        EGL_NATIVE_PIXMAP_KHR,
        0,
        attrib_list));

    if (!host_image) {
        return NULL;
    }

    image = yagl_malloc0(sizeof(*image));

    yagl_image_init(&image->base,
                    &yagl_offscreen_image_pixmap_destroy,
                    host_image,
                    dpy,
                    (EGLImageKHR)native_pixmap->os_drawable);

    image->base.update = &yagl_offscreen_image_pixmap_update;

    image->native_pixmap = native_pixmap;

    native_pixmap->get_geometry(native_pixmap,
                                &image->width,
                                &image->height,
                                &depth);

    return image;
}
