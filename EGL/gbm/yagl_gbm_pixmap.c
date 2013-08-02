#include "yagl_gbm_pixmap.h"
#include "yagl_native_drawable.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_gbm.h"
#include "vigs.h"

static int yagl_gbm_pixmap_get_buffer(struct yagl_native_drawable *drawable,
                                      yagl_native_attachment attachment,
                                      uint32_t *buffer_name,
                                      struct vigs_drm_surface **buffer_sfc)
{
    struct gbm_bo *bo = YAGL_GBM_PIXMAP(drawable->os_drawable);

    YAGL_LOG_FUNC_SET(yagl_gbm_pixmap_get_buffer);

    switch (attachment) {
    case yagl_native_attachment_front:
        break;
    case yagl_native_attachment_back:
    default:
        YAGL_LOG_ERROR("Bad attachment %u", attachment);
        return 0;
    }

    vigs_drm_gem_ref(&bo->drm_sfc->gem);

    *buffer_sfc = bo->drm_sfc;

    return 1;
}

static void yagl_gbm_pixmap_swap_buffers(struct yagl_native_drawable *drawable)
{
}

static void yagl_gbm_pixmap_wait(struct yagl_native_drawable *drawable,
                                 uint32_t width,
                                 uint32_t height)
{
}

static void yagl_gbm_pixmap_copy_to_pixmap(struct yagl_native_drawable *drawable,
                                           yagl_os_pixmap os_pixmap,
                                           uint32_t from_x,
                                           uint32_t from_y,
                                           uint32_t to_x,
                                           uint32_t to_y,
                                           uint32_t width,
                                           uint32_t height)
{
}

static void yagl_gbm_pixmap_set_swap_interval(struct yagl_native_drawable *drawable,
                                              int interval)
{
}

static void yagl_gbm_pixmap_get_geometry(struct yagl_native_drawable *drawable,
                                         uint32_t *width,
                                         uint32_t *height,
                                         uint32_t *depth)
{
    struct gbm_bo *bo = YAGL_GBM_PIXMAP(drawable->os_drawable);

    *width = bo->drm_sfc->width;
    *height = bo->drm_sfc->height;
    *depth = bo->depth;
}

static struct yagl_native_image
    *yagl_gbm_pixmap_get_image(struct yagl_native_drawable *drawable,
                               uint32_t width,
                               uint32_t height)
{
    return NULL;
}

static void yagl_gbm_pixmap_destroy(struct yagl_native_drawable *drawable)
{
    yagl_native_drawable_cleanup(drawable);

    yagl_free(drawable);
}

struct yagl_native_drawable
    *yagl_gbm_pixmap_create(struct yagl_native_display *dpy,
                            yagl_os_pixmap os_pixmap)
{
    struct yagl_native_drawable *pixmap;

    pixmap = yagl_malloc0(sizeof(*pixmap));

    yagl_native_drawable_init(pixmap, dpy, os_pixmap);

    pixmap->get_buffer = &yagl_gbm_pixmap_get_buffer;
    pixmap->swap_buffers = &yagl_gbm_pixmap_swap_buffers;
    pixmap->wait = &yagl_gbm_pixmap_wait;
    pixmap->copy_to_pixmap = &yagl_gbm_pixmap_copy_to_pixmap;
    pixmap->set_swap_interval = &yagl_gbm_pixmap_set_swap_interval;
    pixmap->get_geometry = &yagl_gbm_pixmap_get_geometry;
    pixmap->get_image = &yagl_gbm_pixmap_get_image;
    pixmap->destroy = &yagl_gbm_pixmap_destroy;

    return pixmap;
}
