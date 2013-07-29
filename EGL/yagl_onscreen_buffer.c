#include "yagl_onscreen_buffer.h"
#include "yagl_onscreen_display.h"
#include "yagl_native_display.h"
#include "yagl_native_drawable.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "vigs.h"
#include <string.h>

struct yagl_onscreen_buffer
    *yagl_onscreen_buffer_create(struct yagl_onscreen_display *dpy,
                                 struct yagl_native_drawable *native_drawable,
                                 yagl_native_attachment attachment,
                                 uint32_t check_name)
{
    int ret;
    uint32_t name;
    struct vigs_drm_surface *drm_sfc;
    struct yagl_onscreen_buffer *buffer;

    YAGL_LOG_FUNC_ENTER(yagl_onscreen_buffer_create,
                        "dpy = %p, d = %p, attachment = %u, check_name = %u",
                        native_drawable->dpy->os_dpy,
                        native_drawable->os_drawable,
                        attachment,
                        check_name);

    name = native_drawable->get_buffer(native_drawable, attachment);

    if (!name) {
        YAGL_LOG_FUNC_EXIT(NULL);
        return NULL;
    }

    if (name == check_name) {
        YAGL_LOG_FUNC_EXIT(NULL);
        return NULL;
    }

    ret = vigs_drm_surface_open(dpy->drm_dev, name, &drm_sfc);

    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_surface_open failed for drawable %p: %s",
                       native_drawable->os_drawable,
                       strerror(-ret));
        YAGL_LOG_FUNC_EXIT(NULL);
        return NULL;
    }

    buffer = yagl_malloc0(sizeof(*buffer));

    buffer->name = name;
    buffer->drm_sfc = drm_sfc;

    YAGL_LOG_FUNC_EXIT(NULL);

    return buffer;
}

void yagl_onscreen_buffer_destroy(struct yagl_onscreen_buffer *buffer)
{
    vigs_drm_gem_unref(&buffer->drm_sfc->gem);
    yagl_free(buffer);
}
