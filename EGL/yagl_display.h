#ifndef _YAGL_DISPLAY_H_
#define _YAGL_DISPLAY_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_list.h"
#include "yagl_native_types.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include <pthread.h>

struct yagl_surface;
struct yagl_context;
struct yagl_image;
struct yagl_native_platform;
struct yagl_native_display;

struct yagl_display
{
    struct yagl_list list;

    yagl_os_display display_id;

    struct yagl_native_display *native_dpy;

    yagl_host_handle host_dpy;

    pthread_mutex_t mutex;

    int prepared;

    char *extensions;

    struct yagl_list surfaces;

    struct yagl_list contexts;

    struct yagl_list images;
};

/*
 * Takes ownership of 'native_dpy'.
 */
void yagl_display_init(struct yagl_display *dpy,
                       yagl_os_display display_id,
                       struct yagl_native_display *native_dpy,
                       yagl_host_handle host_dpy);

struct yagl_display *yagl_display_get(EGLDisplay handle);

struct yagl_display *yagl_display_get_os(yagl_os_display os_dpy);

struct yagl_display *yagl_display_add(struct yagl_native_platform *platform,
                                      yagl_os_display display_id,
                                      yagl_host_handle host_dpy);

void yagl_display_prepare(struct yagl_display *dpy);

int yagl_display_is_prepared(struct yagl_display *dpy);

void yagl_display_terminate(struct yagl_display *dpy);

/*
 * 'dpy' can be NULL here.
 */
const char *yagl_display_get_extensions(struct yagl_display *dpy);

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
