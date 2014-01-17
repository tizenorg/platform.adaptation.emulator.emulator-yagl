#include "yagl_gbm_window.h"
#include "yagl_native_drawable.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_gbm.h"

static int yagl_gbm_window_get_buffer(struct yagl_native_drawable *drawable,
                                      yagl_native_attachment attachment,
                                      uint32_t *buffer_name,
                                      struct vigs_drm_surface **buffer_sfc)
{
    struct gbm_surface *sfc = YAGL_GBM_WINDOW(drawable->os_drawable);

    YAGL_LOG_FUNC_SET(yagl_gbm_window_get_buffer);

    switch (attachment) {
    case yagl_native_attachment_back:
        break;
    case yagl_native_attachment_front:
    default:
        YAGL_LOG_ERROR("Bad attachment %u", attachment);
        return 0;
    }

    *buffer_sfc = sfc->acquire_back(sfc);

    if (!*buffer_sfc) {
        YAGL_LOG_ERROR("Cannot get back for drawable %p",
                       drawable->os_drawable);
        return 0;
    }

    return 1;
}

static int yagl_gbm_window_get_buffer_age(struct yagl_native_drawable *drawable)
{
    struct gbm_surface *sfc = YAGL_GBM_WINDOW(drawable->os_drawable);

    return sfc->get_buffer_age(sfc);
}

static void yagl_gbm_window_swap_buffers(struct yagl_native_drawable *drawable)
{
    struct gbm_surface *sfc = YAGL_GBM_WINDOW(drawable->os_drawable);

    sfc->swap_buffers(sfc);

    ++drawable->stamp;
}

static void yagl_gbm_window_wait(struct yagl_native_drawable *drawable,
                                 uint32_t width,
                                 uint32_t height)
{
}

static void yagl_gbm_window_copy_to_pixmap(struct yagl_native_drawable *drawable,
                                           yagl_os_pixmap os_pixmap,
                                           uint32_t from_x,
                                           uint32_t from_y,
                                           uint32_t to_x,
                                           uint32_t to_y,
                                           uint32_t width,
                                           uint32_t height)
{
}

static void yagl_gbm_window_set_swap_interval(struct yagl_native_drawable *drawable,
                                              int interval)
{
}

static void yagl_gbm_window_get_geometry(struct yagl_native_drawable *drawable,
                                         uint32_t *width,
                                         uint32_t *height,
                                         uint32_t *depth)
{
    struct gbm_surface *sfc = YAGL_GBM_WINDOW(drawable->os_drawable);

    *width = sfc->width;
    *height = sfc->height;
    *depth = sfc->depth;
}

static struct yagl_native_image
    *yagl_gbm_window_get_image(struct yagl_native_drawable *drawable,
                               uint32_t width,
                               uint32_t height)
{
    return NULL;
}

static void yagl_gbm_window_destroy(struct yagl_native_drawable *drawable)
{
    yagl_native_drawable_cleanup(drawable);

    yagl_free(drawable);
}

struct yagl_native_drawable
    *yagl_gbm_window_create(struct yagl_native_display *dpy,
                            yagl_os_window os_window)
{
    struct yagl_native_drawable *window;

    window = yagl_malloc0(sizeof(*window));

    yagl_native_drawable_init(window, dpy, os_window);

    window->get_buffer = &yagl_gbm_window_get_buffer;
    window->get_buffer_age = &yagl_gbm_window_get_buffer_age;
    window->swap_buffers = &yagl_gbm_window_swap_buffers;
    window->wait = &yagl_gbm_window_wait;
    window->copy_to_pixmap = &yagl_gbm_window_copy_to_pixmap;
    window->set_swap_interval = &yagl_gbm_window_set_swap_interval;
    window->get_geometry = &yagl_gbm_window_get_geometry;
    window->get_image = &yagl_gbm_window_get_image;
    window->destroy = &yagl_gbm_window_destroy;

    return window;
}