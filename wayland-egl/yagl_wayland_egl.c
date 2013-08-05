#include "yagl_export.h"
#include "yagl_wayland_egl.h"
#include <stdlib.h>
#include <string.h>

YAGL_API struct wl_egl_window *wl_egl_window_create(struct wl_surface *surface,
                                                    int width, int height)
{
    struct wl_egl_window *egl_window;

    egl_window = malloc(sizeof(*egl_window));

    if (!egl_window) {
        return NULL;
    }

    memset(egl_window, 0, sizeof(*egl_window));

    egl_window->surface = surface;

    wl_egl_window_resize(egl_window, width, height, 0, 0);

    return egl_window;
}

YAGL_API void wl_egl_window_destroy(struct wl_egl_window *egl_window)
{
    free(egl_window);
}

YAGL_API void wl_egl_window_resize(struct wl_egl_window *egl_window,
                                   int width, int height,
                                   int dx, int dy)
{
    egl_window->width  = width;
    egl_window->height = height;
    egl_window->dx     = dx;
    egl_window->dy     = dy;

    if (egl_window->resize_callback) {
        egl_window->resize_callback(egl_window, egl_window->user_data);
    }
}

YAGL_API void wl_egl_window_get_attached_size(struct wl_egl_window *egl_window,
                                              int *width, int *height)
{
    if (width) {
        *width = egl_window->attached_width;
    }
    if (height) {
        *height = egl_window->attached_height;
    }
}
