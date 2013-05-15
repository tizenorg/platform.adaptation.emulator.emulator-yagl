#include "yagl_onscreen.h"
#include "yagl_onscreen_display.h"
#include "yagl_onscreen_surface.h"
#include "yagl_onscreen_image.h"
#include "yagl_backend.h"
#include "yagl_malloc.h"

static struct yagl_display
    *yagl_onscreen_create_display(EGLNativeDisplayType display_id,
                                  Display *x_dpy,
                                  yagl_host_handle host_dpy)
{
    struct yagl_onscreen_display *dpy =
        yagl_onscreen_display_create(display_id, x_dpy, host_dpy);

    return dpy ? &dpy->base : NULL;
}

static struct yagl_surface
    *yagl_onscreen_create_window_surface(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          Window x_win,
                                          const EGLint* attrib_list)
{
    struct yagl_onscreen_surface *sfc =
        yagl_onscreen_surface_create_window(dpy, host_config, x_win, attrib_list);

    return sfc ? &sfc->base : NULL;
}

static struct yagl_surface
    *yagl_onscreen_create_pixmap_surface(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          Pixmap x_pixmap,
                                          const EGLint* attrib_list)
{
    struct yagl_onscreen_surface *sfc =
        yagl_onscreen_surface_create_pixmap(dpy, host_config, x_pixmap, attrib_list);

    return sfc ? &sfc->base : NULL;
}

static struct yagl_surface
    *yagl_onscreen_create_pbuffer_surface(struct yagl_display *dpy,
                                           yagl_host_handle host_config,
                                           const EGLint* attrib_list)
{
    struct yagl_onscreen_surface *sfc =
        yagl_onscreen_surface_create_pbuffer(dpy, host_config, attrib_list);

    return sfc ? &sfc->base : NULL;
}

static struct yagl_image
    *yagl_onscreen_create_image(struct yagl_display *dpy,
                                yagl_host_handle host_context,
                                Pixmap x_pixmap,
                                const EGLint* attrib_list)
{
    struct yagl_onscreen_image *image =
        yagl_onscreen_image_create(dpy, host_context, x_pixmap, attrib_list);

    return image ? &image->base : NULL;
}

static void yagl_onscreen_destroy(struct yagl_backend *backend)
{
    yagl_free(backend);
}

struct yagl_backend *yagl_onscreen_create()
{
    struct yagl_backend *backend;

    backend = yagl_malloc0(sizeof(*backend));

    backend->create_display = &yagl_onscreen_create_display;
    backend->create_window_surface = &yagl_onscreen_create_window_surface;
    backend->create_pixmap_surface = &yagl_onscreen_create_pixmap_surface;
    backend->create_pbuffer_surface = &yagl_onscreen_create_pbuffer_surface;
    backend->create_image = &yagl_onscreen_create_image;
    backend->destroy = &yagl_onscreen_destroy;

    return backend;
}
