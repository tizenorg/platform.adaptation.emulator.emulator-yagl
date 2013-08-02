#include "yagl_gbm_display.h"
#include "yagl_gbm_window.h"
#include "yagl_gbm_pixmap.h"
#include "yagl_native_display.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_gbm.h"

static struct yagl_native_drawable
    *yagl_gbm_display_wrap_window(struct yagl_native_display *dpy,
                                  yagl_os_window os_window)
{
    return yagl_gbm_window_create(dpy, os_window);
}

static struct yagl_native_drawable
    *yagl_gbm_display_wrap_pixmap(struct yagl_native_display *dpy,
                                  yagl_os_pixmap os_pixmap)
{
    return yagl_gbm_pixmap_create(dpy, os_pixmap);
}

static struct yagl_native_drawable
    *yagl_gbm_display_create_pixmap(struct yagl_native_display *dpy,
                                    uint32_t width,
                                    uint32_t height,
                                    uint32_t depth)
{
    return NULL;
}

static struct yagl_native_image
    *yagl_gbm_display_create_image(struct yagl_native_display *dpy,
                                   uint32_t width,
                                   uint32_t height,
                                   uint32_t depth)
{
    return NULL;
}

static int yagl_gbm_display_get_visual(struct yagl_native_display *dpy,
                                       int *visual_id,
                                       int *visual_type)
{
    return 0;
}

static void yagl_gbm_display_destroy(struct yagl_native_display *dpy)
{
    yagl_native_display_cleanup(dpy);

    yagl_free(dpy);
}

struct yagl_native_display
    *yagl_gbm_display_create(struct yagl_native_platform *platform,
                             yagl_os_display os_dpy)
{
    struct gbm_device *gbm_dpy = YAGL_GBM_DPY(os_dpy);
    struct yagl_native_display *dpy;

    dpy = yagl_malloc0(sizeof(*dpy));

    yagl_native_display_init(dpy,
                             platform,
                             os_dpy,
                             gbm_dpy->drm_dev);

    dpy->wrap_window = &yagl_gbm_display_wrap_window;
    dpy->wrap_pixmap = &yagl_gbm_display_wrap_pixmap;
    dpy->create_pixmap = &yagl_gbm_display_create_pixmap;
    dpy->create_image = &yagl_gbm_display_create_image;
    dpy->get_visual = &yagl_gbm_display_get_visual;
    dpy->destroy = &yagl_gbm_display_destroy;

    return dpy;
}
