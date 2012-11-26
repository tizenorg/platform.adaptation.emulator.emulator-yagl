#include "yagl_display.h"
#include "yagl_utils.h"
#include "yagl_malloc.h"
#include "yagl_log.h"
#include "yagl_surface.h"
#include "yagl_context.h"
#include "yagl_image.h"
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static pthread_once_t g_displays_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_displays_mutex;
static YAGL_DECLARE_LIST(g_displays);

static void yagl_displays_init_once(void)
{
    yagl_mutex_init(&g_displays_mutex);
}

static void yagl_displays_init(void)
{
    pthread_once(&g_displays_init, yagl_displays_init_once);
}

struct yagl_display *yagl_display_get(EGLDisplay native_dpy)
{
    yagl_host_handle host_dpy = (yagl_host_handle)native_dpy;
    struct yagl_display *dpy;

    yagl_displays_init();

    pthread_mutex_lock(&g_displays_mutex);

    yagl_list_for_each(struct yagl_display, dpy, &g_displays, list) {
        if (dpy->host_dpy == host_dpy) {
            pthread_mutex_unlock(&g_displays_mutex);

            return dpy;
        }
    }

    pthread_mutex_unlock(&g_displays_mutex);

    return NULL;
}

struct yagl_display *yagl_display_add(EGLNativeDisplayType display_id,
                                      yagl_host_handle host_dpy)
{
    struct yagl_display *dpy;
    Display *x_dpy;
    int xmajor;
    int xminor;
    Bool pixmaps;

    YAGL_LOG_FUNC_ENTER_SPLIT2( yagl_display_add,
                                EGLNativeDisplayType,
                                yagl_host_handle,
                                display_id,
                                host_dpy );

    yagl_displays_init();

    pthread_mutex_lock(&g_displays_mutex);

    yagl_list_for_each(struct yagl_display, dpy, &g_displays, list) {
        if (dpy->host_dpy == host_dpy) {
            pthread_mutex_unlock(&g_displays_mutex);

            if (dpy->display_id != display_id) {
                YAGL_LOG_ERROR(
                    "display id mismatch, host handle is %u, corresponding display id is %p, passed display id is %p",
                     dpy->host_dpy,
                     dpy->display_id,
                     display_id );

                YAGL_LOG_FUNC_EXIT(NULL);

                return NULL;
            } else {
                return dpy;
            }
        }
    }

    if (display_id == EGL_DEFAULT_DISPLAY) {
        x_dpy = XOpenDisplay(0);

        if (!x_dpy) {
            pthread_mutex_unlock(&g_displays_mutex);

            YAGL_LOG_ERROR("unable to open display 0");

            YAGL_LOG_FUNC_EXIT(NULL);

            return NULL;
        }
    } else {
        x_dpy = display_id;
    }

    dpy = yagl_malloc0(sizeof(*dpy));

    yagl_list_init(&dpy->list);
    dpy->display_id = display_id;
    dpy->x_dpy = x_dpy;
    dpy->host_dpy = host_dpy;
    yagl_mutex_init(&dpy->mutex);
    dpy->initialized = 0;
    dpy->xshm_images_supported = XShmQueryVersion(dpy->x_dpy,
                                                  &xmajor,
                                                  &xminor,
                                                  &pixmaps);
    dpy->xshm_pixmaps_supported = pixmaps;

    YAGL_LOG_DEBUG("XShm images are%s supported, version %d, %d (pixmaps = %d)",
                   (dpy->xshm_images_supported ? "" : " NOT"),
                   xmajor,
                   xminor,
                   pixmaps);

    yagl_list_init(&dpy->surfaces);
    yagl_list_init(&dpy->contexts);
    yagl_list_init(&dpy->images);

    yagl_list_add(&g_displays, &dpy->list);

    pthread_mutex_unlock(&g_displays_mutex);

    YAGL_LOG_FUNC_EXIT(NULL);

    return dpy;
}

void yagl_display_initialize(struct yagl_display *dpy)
{
    pthread_mutex_lock(&dpy->mutex);
    dpy->initialized = 1;
    pthread_mutex_unlock(&dpy->mutex);
}

int yagl_display_is_initialized(struct yagl_display *dpy)
{
    int ret;

    pthread_mutex_lock(&dpy->mutex);
    ret = dpy->initialized;
    pthread_mutex_unlock(&dpy->mutex);

    return ret;
}

void yagl_display_terminate(struct yagl_display *dpy)
{
    struct yagl_list tmp_list;
    struct yagl_resource *res, *next;

    /*
     * First, move all surfaces, contexts and images to a
     * temporary list to release later.
     */

    yagl_list_init(&tmp_list);

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each_safe(struct yagl_resource,
                            res,
                            next,
                            &dpy->surfaces,
                            list) {
        yagl_list_remove(&res->list);
        yagl_list_add_tail(&tmp_list, &res->list);
    }

    yagl_list_for_each_safe(struct yagl_resource,
                            res,
                            next,
                            &dpy->contexts,
                            list) {
        yagl_list_remove(&res->list);
        yagl_list_add_tail(&tmp_list, &res->list);
    }

    yagl_list_for_each_safe(struct yagl_resource,
                            res,
                            next,
                            &dpy->images,
                            list) {
        yagl_list_remove(&res->list);
        yagl_list_add_tail(&tmp_list, &res->list);
    }

    assert(yagl_list_empty(&dpy->surfaces));
    assert(yagl_list_empty(&dpy->contexts));
    assert(yagl_list_empty(&dpy->images));

    dpy->initialized = 0;

    pthread_mutex_unlock(&dpy->mutex);

    /*
     * We release here because we don't want the resources to be released
     * when display mutex is held.
     */

    yagl_list_for_each_safe(struct yagl_resource, res, next, &tmp_list, list) {
        yagl_list_remove(&res->list);
        yagl_resource_release(res);
    }

    assert(yagl_list_empty(&tmp_list));
}

int yagl_display_surface_add(struct yagl_display *dpy,
                              struct yagl_surface *sfc)
{
    struct yagl_resource *res = NULL;
    EGLSurface handle = yagl_surface_get_handle(sfc);

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->surfaces, list) {
        if (yagl_surface_get_handle((struct yagl_surface*)res) == handle) {
            pthread_mutex_unlock(&dpy->mutex);
            return 0;
        }
    }

    yagl_resource_acquire(&sfc->res);
    yagl_list_add_tail(&dpy->surfaces, &sfc->res.list);

    pthread_mutex_unlock(&dpy->mutex);

    return 1;
}

struct yagl_surface *yagl_display_surface_acquire(struct yagl_display *dpy,
                                                  EGLSurface handle)
{
    struct yagl_resource *res = NULL;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->surfaces, list) {
        if (yagl_surface_get_handle((struct yagl_surface*)res) == handle) {
            yagl_resource_acquire(res);
            pthread_mutex_unlock(&dpy->mutex);
            return (struct yagl_surface*)res;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);

    return NULL;
}

int yagl_display_surface_remove(struct yagl_display *dpy,
                                EGLSurface handle)
{
    struct yagl_resource *res;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->surfaces, list) {
        if (yagl_surface_get_handle((struct yagl_surface*)res) == handle) {
            yagl_list_remove(&res->list);
            yagl_resource_release(res);
            pthread_mutex_unlock(&dpy->mutex);
            return 1;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);

    return 0;
}

void yagl_display_context_add(struct yagl_display *dpy,
                              struct yagl_context *ctx)
{
    pthread_mutex_lock(&dpy->mutex);

    yagl_resource_acquire(&ctx->res);
    yagl_list_add_tail(&dpy->contexts, &ctx->res.list);

    pthread_mutex_unlock(&dpy->mutex);
}

struct yagl_context *yagl_display_context_acquire(struct yagl_display *dpy,
                                                  EGLContext handle)
{
    struct yagl_resource *res = NULL;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->contexts, list) {
        if (res->handle == (yagl_host_handle)handle) {
            yagl_resource_acquire(res);
            pthread_mutex_unlock(&dpy->mutex);
            return (struct yagl_context*)res;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);

    return NULL;
}

void yagl_display_context_remove(struct yagl_display *dpy,
                                 EGLContext handle)
{
    struct yagl_resource *res;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->contexts, list) {
        if (res->handle == (yagl_host_handle)handle) {
            yagl_list_remove(&res->list);
            yagl_resource_release(res);
            break;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);
}

int yagl_display_image_add(struct yagl_display *dpy,
                           struct yagl_image *image)
{
    struct yagl_resource *res = NULL;
    EGLImageKHR handle = yagl_image_get_handle(image);

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->images, list) {
        if (yagl_image_get_handle((struct yagl_image*)res) == handle) {
            pthread_mutex_unlock(&dpy->mutex);
            return 0;
        }
    }

    yagl_resource_acquire(&image->res);
    yagl_list_add_tail(&dpy->images, &image->res.list);

    pthread_mutex_unlock(&dpy->mutex);

    return 1;
}

struct yagl_image *yagl_display_image_acquire(struct yagl_display *dpy,
                                              EGLImageKHR handle)
{
    struct yagl_resource *res = NULL;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->images, list) {
        if (yagl_image_get_handle((struct yagl_image*)res) == handle) {
            yagl_resource_acquire(res);
            pthread_mutex_unlock(&dpy->mutex);
            return (struct yagl_image*)res;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);

    return NULL;
}

int yagl_display_image_remove(struct yagl_display *dpy,
                              EGLImageKHR handle)
{
    struct yagl_resource *res;

    pthread_mutex_lock(&dpy->mutex);

    yagl_list_for_each(struct yagl_resource, res, &dpy->images, list) {
        if (yagl_image_get_handle((struct yagl_image*)res) == handle) {
            yagl_list_remove(&res->list);
            yagl_resource_release(res);
            pthread_mutex_unlock(&dpy->mutex);
            return 1;
        }
    }

    pthread_mutex_unlock(&dpy->mutex);

    return 0;
}
