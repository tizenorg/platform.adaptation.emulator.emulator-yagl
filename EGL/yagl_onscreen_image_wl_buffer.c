#include "yagl_onscreen_image_wl_buffer.h"
#include "yagl_display.h"
#include "yagl_malloc.h"
#include "yagl_host_egl_calls.h"
#include "yagl_mem_egl.h"
#include "yagl_egl_state.h"
#include "wayland-drm.h"
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
                                          yagl_host_handle host_context,
                                          struct wl_resource *buffer,
                                          const EGLint* attrib_list)
{
    struct yagl_onscreen_image_wl_buffer *image;
    struct wl_drm_buffer *drm_buffer;
    struct vigs_drm_surface *drm_sfc;
    yagl_host_handle host_image = 0;

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

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreateImageKHR(&host_image,
        dpy->host_dpy,
        host_context,
        EGL_WAYLAND_BUFFER_WL,
        drm_sfc->id,
        attrib_list));

    if (!host_image) {
        goto fail;
    }

    yagl_image_init(&image->base,
                    &yagl_onscreen_image_wl_buffer_destroy,
                    host_image,
                    dpy,
                    (EGLImageKHR)drm_sfc->gem.name);

    image->base.update = &yagl_onscreen_image_wl_buffer_update;

    image->buffer = drm_buffer;

    return image;

fail:
    yagl_free(image);

    return NULL;
}
