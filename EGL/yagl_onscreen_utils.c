#include "yagl_onscreen_utils.h"
#include "yagl_log.h"
#include "yagl_native_display.h"
#include "yagl_native_drawable.h"
#include "vigs.h"
#include <string.h>

struct vigs_drm_surface
    *yagl_onscreen_buffer_create(struct yagl_native_drawable *native_drawable,
                                 yagl_native_attachment attachment,
                                 struct vigs_drm_surface *check_sfc)
{
    int ret;
    uint32_t buffer_name;
    struct vigs_drm_surface *buffer_sfc = NULL;

    YAGL_LOG_FUNC_ENTER(yagl_onscreen_buffer_create,
                        "dpy = %p, d = %p, attachment = %u, check_sfc = %u",
                        native_drawable->dpy->os_dpy,
                        native_drawable->os_drawable,
                        attachment,
                        (check_sfc ? check_sfc->id : 0));

    if (!native_drawable->get_buffer(native_drawable,
                                     attachment,
                                     &buffer_name,
                                     &buffer_sfc)) {
        goto fail;
    }

    if (check_sfc) {
        if (buffer_name && (buffer_name == check_sfc->gem.name)) {
            /*
             * Buffer matches check_sfc, fail.
             */
            goto fail;
        }

        if (buffer_sfc && (buffer_sfc->id == check_sfc->id)) {
            /*
             * Buffer matches check_sfc, fail.
             */
            goto fail;
        }
    }

    if (buffer_sfc) {
        YAGL_LOG_FUNC_EXIT(NULL);

        return buffer_sfc;
    }

    ret = vigs_drm_surface_open(native_drawable->dpy->drm_dev,
                                buffer_name,
                                &buffer_sfc);

    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_surface_open failed for drawable %p: %s",
                       native_drawable->os_drawable,
                       strerror(-ret));
        goto fail;
    }

    YAGL_LOG_FUNC_EXIT(NULL);

    return buffer_sfc;

fail:
    if (buffer_sfc) {
        vigs_drm_gem_unref(&buffer_sfc->gem);
    }

    YAGL_LOG_FUNC_EXIT(NULL);

    return NULL;
}
