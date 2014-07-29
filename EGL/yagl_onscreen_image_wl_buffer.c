#include "yagl_onscreen_image_wl_buffer.h"
#include "yagl_display.h"
#include "yagl_malloc.h"
#include "yagl_host_egl_calls.h"
#include "yagl_egl_state.h"
#include "yagl_state.h"
#include "yagl_transport_egl.h"
#include "wayland-drm.h"
#include "yagl_client_interface.h"
#include "yagl_client_image.h"
#include "vigs.h"

static void yagl_onscreen_image_wl_buffer_update(struct yagl_image *image)
{
}

static void yagl_onscreen_image_wl_buffer_destroy(struct yagl_ref *ref)
{
    struct yagl_onscreen_image_wl_buffer *image = (struct yagl_onscreen_image_wl_buffer*)ref;

    yagl_image_cleanup(&image->base);

    yagl_free(image);
}

struct yagl_onscreen_image_wl_buffer
    *yagl_onscreen_image_wl_buffer_create(struct yagl_display *dpy,
                                          struct wl_resource *buffer,
                                          struct yagl_client_interface *iface)
{
    EGLint error = 0;
    yagl_object_name tex_global_name = yagl_get_global_name();
    struct yagl_client_image *client_image;
    struct yagl_onscreen_image_wl_buffer *image;
    struct wl_drm_buffer *drm_buffer;
    struct vigs_drm_surface *drm_sfc;

    image = yagl_malloc0(sizeof(*image));

    drm_buffer = wayland_drm_get_buffer(buffer);

    if (!drm_buffer) {
        /*
         * Or is it some other error ?
         */
        yagl_set_error(EGL_BAD_NATIVE_PIXMAP);
        goto fail;
    }

    drm_sfc = wayland_drm_buffer_get_sfc(drm_buffer);

    if (!yagl_host_eglCreateImageYAGL(tex_global_name,
                                      dpy->host_dpy,
                                      drm_sfc->id,
                                      &error)) {
        yagl_set_error(error);
        goto fail;
    }

    client_image = iface->create_image(iface, tex_global_name);

    yagl_image_init(&image->base,
                    &yagl_onscreen_image_wl_buffer_destroy,
                    dpy,
                    (EGLImageKHR)drm_sfc->gem.name,
                    client_image);

    yagl_client_image_release(client_image);

    image->base.update = &yagl_onscreen_image_wl_buffer_update;

    image->buffer = drm_buffer;

    return image;

fail:
    yagl_free(image);

    return NULL;
}
