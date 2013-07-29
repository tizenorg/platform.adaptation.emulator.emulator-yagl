#include "yagl_onscreen_image.h"
#include "yagl_onscreen_buffer.h"
#include "yagl_onscreen_display.h"
#include "yagl_display.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_host_egl_calls.h"
#include "yagl_mem_egl.h"
#include "yagl_egl_state.h"
#include "vigs.h"

static void yagl_onscreen_image_update(struct yagl_image *image)
{
}

static void yagl_onscreen_image_destroy(struct yagl_ref *ref)
{
    struct yagl_onscreen_image *image = (struct yagl_onscreen_image*)ref;

    yagl_onscreen_buffer_destroy(image->buffer);

    yagl_image_cleanup(&image->base);

    yagl_free(image);
}

struct yagl_onscreen_image
    *yagl_onscreen_image_create(struct yagl_display *dpy,
                                yagl_host_handle host_context,
                                struct yagl_native_drawable *native_pixmap,
                                const EGLint* attrib_list)
{
    struct yagl_onscreen_display *odpy = (struct yagl_onscreen_display*)dpy;
    struct yagl_onscreen_image *image;
    struct yagl_onscreen_buffer *new_buffer = NULL;
    yagl_host_handle host_image = 0;

    image = yagl_malloc0(sizeof(*image));

    new_buffer = yagl_onscreen_buffer_create(odpy,
                                             native_pixmap,
                                             yagl_native_attachment_front,
                                             0);

    if (!new_buffer) {
        yagl_set_error(EGL_BAD_NATIVE_PIXMAP);
        goto fail;
    }

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreateImageKHR(&host_image,
        dpy->host_dpy,
        host_context,
        EGL_NATIVE_PIXMAP_KHR,
        new_buffer->drm_sfc->id,
        attrib_list));

    if (!host_image) {
        goto fail;
    }

    yagl_image_init(&image->base,
                    &yagl_onscreen_image_destroy,
                    host_image,
                    dpy,
                    native_pixmap);

    image->base.update = &yagl_onscreen_image_update;

    image->buffer = new_buffer;

    return image;

fail:
    if (new_buffer) {
        yagl_onscreen_buffer_destroy(new_buffer);
    }
    yagl_free(image);

    return NULL;
}
