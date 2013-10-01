#include "yagl_offscreen_image_pixmap.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_display.h"
#include "yagl_state.h"
#include "yagl_host_egl_calls.h"
#include "yagl_native_drawable.h"
#include "yagl_native_image.h"
#include "yagl_transport_egl.h"
#include "yagl_client_interface.h"
#include "yagl_client_image.h"
#include "yagl_egl_state.h"
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

    oimage->base.client_image->update(oimage->base.client_image,
                                      native_image->width,
                                      native_image->height,
                                      native_image->bpp,
                                      native_image->pixels);

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
                                        struct yagl_native_drawable *native_pixmap,
                                        struct yagl_client_interface *iface)
{
    EGLint error = 0;
    yagl_object_name tex_global_name = yagl_get_global_name();
    struct yagl_client_image *client_image;
    struct yagl_offscreen_image_pixmap *image;
    uint32_t depth;

    if (!yagl_host_eglCreateImageYAGL(tex_global_name,
                                      dpy->host_dpy,
                                      0,
                                      &error)) {
        yagl_set_error(error);
        return NULL;
    }

    client_image = iface->create_image(iface, tex_global_name);

    image = yagl_malloc0(sizeof(*image));

    yagl_image_init(&image->base,
                    &yagl_offscreen_image_pixmap_destroy,
                    dpy,
                    (EGLImageKHR)native_pixmap->os_drawable,
                    client_image);

    yagl_client_image_release(client_image);

    image->base.update = &yagl_offscreen_image_pixmap_update;

    image->native_pixmap = native_pixmap;

    native_pixmap->get_geometry(native_pixmap,
                                &image->width,
                                &image->height,
                                &depth);

    return image;
}
