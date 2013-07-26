#ifndef _YAGL_DISPLAY_H_
#define _YAGL_DISPLAY_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_list.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include <pthread.h>

struct yagl_surface;
struct yagl_context;
struct yagl_image;

struct yagl_display
{
    struct yagl_list list;

    EGLNativeDisplayType display_id;

    Display *x_dpy;

    yagl_host_handle host_dpy;

    int xshm_images_supported;

    int xshm_pixmaps_supported;

    pthread_mutex_t mutex;

    int prepared;

    struct yagl_list surfaces;

    struct yagl_list contexts;

    struct yagl_list images;
};

void yagl_display_init(struct yagl_display *dpy,
                       EGLNativeDisplayType display_id,
                       Display *x_dpy,
                       yagl_host_handle host_dpy);

void yagl_display_cleanup(struct yagl_display *dpy);

struct yagl_display *yagl_display_get(EGLDisplay native_dpy);

struct yagl_display *yagl_display_get_x(Display *x_dpy);

struct yagl_display *yagl_display_add(EGLNativeDisplayType display_id,
                                      yagl_host_handle host_dpy);

void yagl_display_prepare(struct yagl_display *dpy);

int yagl_display_is_prepared(struct yagl_display *dpy);

void yagl_display_terminate(struct yagl_display *dpy);

/*
 * Surfaces.
 * @{
 */

/*
 * This acquires 'sfc', so the caller should
 * release 'sfc' if he doesn't want to use it and wants it to belong to the
 * display alone.
 */
int yagl_display_surface_add(struct yagl_display *dpy,
                             struct yagl_surface *sfc);

/*
 * Acquires a surface by handle. Be sure to release the surface when
 * you're done.
 */
struct yagl_surface *yagl_display_surface_acquire(struct yagl_display *dpy,
                                                  EGLSurface handle);

int yagl_display_surface_remove(struct yagl_display *dpy,
                                EGLSurface handle);

/*
 * @}
 */

/*
 * Contexts.
 * @{
 */

/*
 * This acquires 'ctx', so the caller should
 * release 'ctx' if he doesn't want to use it and wants it to belong to the
 * display alone.
 */
void yagl_display_context_add(struct yagl_display *dpy,
                              struct yagl_context *ctx);

/*
 * Acquires a context by handle. Be sure to release the context when
 * you're done.
 */
struct yagl_context *yagl_display_context_acquire(struct yagl_display *dpy,
                                                  EGLContext handle);

void yagl_display_context_remove(struct yagl_display *dpy,
                                 EGLContext handle);

/*
 * @}
 */

/*
 * Images.
 * @{
 */

/*
 * This acquires 'image', so the caller should
 * release 'image' if he doesn't want to use it and wants it to belong to the
 * display alone.
 */
int yagl_display_image_add(struct yagl_display *dpy,
                           struct yagl_image *image);

/*
 * Acquires an image by handle. Be sure to release the image when
 * you're done.
 */
struct yagl_image *yagl_display_image_acquire(struct yagl_display *dpy,
                                              EGLImageKHR handle);

int yagl_display_image_remove(struct yagl_display *dpy,
                              EGLImageKHR handle);

/*
 * @}
 */

#endif
