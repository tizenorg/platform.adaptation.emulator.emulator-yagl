#include "yagl_onscreen_image_pixmap.h"
#include "yagl_onscreen_utils.h"
#include "yagl_display.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_host_egl_calls.h"
#include "yagl_mem_egl.h"
#include "yagl_egl_state.h"
#include "yagl_native_drawable.h"
#include "vigs.h"

static void yagl_onscreen_image_pixmap_update(struct yagl_image *image)
{
}

static void yagl_onscreen_image_pixmap_destroy(struct yagl_ref *ref)
{
    struct yagl_onscreen_image_pixmap *image = (struct yagl_onscreen_image_pixmap*)ref;

    vigs_drm_gem_unref(&image->drm_sfc->gem);

    image->native_pixmap->destroy(image->native_pixmap);
    image->native_pixmap = NULL;

    yagl_image_cleanup(&image->base);

    yagl_free(image);
}

struct yagl_onscreen_image_pixmap
    *yagl_onscreen_image_pixmap_create(struct yagl_display *dpy,
                                       yagl_host_handle host_context,
                                       struct yagl_native_drawable *native_pixmap,
                                       const EGLint* attrib_list)
{
    struct yagl_onscreen_image_pixmap *image;
    struct vigs_drm_surface *drm_sfc = NULL;
    yagl_host_handle host_image = 0;

    image = yagl_malloc0(sizeof(*image));

    drm_sfc = yagl_onscreen_buffer_create(native_pixmap,
                                          yagl_native_attachment_front,
                                          NULL);

    if (!drm_sfc) {
        yagl_set_error(EGL_BAD_NATIVE_PIXMAP);
        goto fail;
    }

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreateImageKHR(&host_image,
        dpy->host_dpy,
        host_context,
        EGL_NATIVE_PIXMAP_KHR,
        drm_sfc->id,
        attrib_list));

    if (!host_image) {
        goto fail;
    }

    yagl_image_init(&image->base,
                    &yagl_onscreen_image_pixmap_destroy,
                    host_image,
                    dpy,
                    (EGLImageKHR)native_pixmap->os_drawable);

    image->base.update = &yagl_onscreen_image_pixmap_update;

    image->native_pixmap = native_pixmap;
    image->drm_sfc = drm_sfc;

    return image;

fail:
    if (drm_sfc) {
        vigs_drm_gem_unref(&drm_sfc->gem);
    }
    yagl_free(image);

    return NULL;
}
