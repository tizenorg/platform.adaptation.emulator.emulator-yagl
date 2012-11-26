#include "yagl_surface.h"
#include "yagl_malloc.h"
#include "yagl_bimage.h"
#include "yagl_display.h"
#include "yagl_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void yagl_surface_destroy(struct yagl_ref *ref)
{
    struct yagl_surface *sfc = (struct yagl_surface*)ref;

    if (sfc->x_gc) {
        XFreeGC(sfc->dpy->x_dpy, sfc->x_gc);
        sfc->x_gc = 0;
    }

    if (sfc->bi) {
        yagl_bimage_destroy(sfc->bi);
        sfc->bi = NULL;
    }

    pthread_mutex_destroy(&sfc->bi_mtx);

    yagl_resource_cleanup(&sfc->res);

    yagl_free(sfc);
}

struct yagl_surface *yagl_surface_create_window(yagl_host_handle handle,
                                                struct yagl_bimage *bi,
                                                Window x_win)
{
    struct yagl_surface *sfc;

    sfc = yagl_malloc0(sizeof(*sfc));

    yagl_resource_init(&sfc->res, &yagl_surface_destroy, handle);

    sfc->dpy = bi->dpy;
    sfc->type = EGL_WINDOW_BIT;
    sfc->bi = bi;
    sfc->x_drawable.win = x_win;
    sfc->x_gc = XCreateGC(sfc->dpy->x_dpy, x_win, 0, NULL);

    if (!sfc->x_gc) {
        fprintf(stderr, "Critical error! XCreateGC failed!\n");
        exit(1);
    }

    yagl_mutex_init(&sfc->bi_mtx);

    return sfc;
}

struct yagl_surface *yagl_surface_create_pixmap(yagl_host_handle handle,
                                                struct yagl_bimage *bi,
                                                Pixmap x_pixmap)
{
    struct yagl_surface *sfc;

    sfc = yagl_malloc0(sizeof(*sfc));

    yagl_resource_init(&sfc->res, &yagl_surface_destroy, handle);

    sfc->dpy = bi->dpy;
    sfc->type = EGL_PIXMAP_BIT;
    sfc->bi = bi;
    sfc->x_drawable.pixmap = x_pixmap;
    sfc->x_gc = XCreateGC(sfc->dpy->x_dpy, x_pixmap, 0, NULL);

    if (!sfc->x_gc) {
        fprintf(stderr, "Critical error! XCreateGC failed!\n");
        exit(1);
    }

    yagl_mutex_init(&sfc->bi_mtx);

    return sfc;
}

struct yagl_surface *yagl_surface_create_pbuffer(yagl_host_handle handle,
                                                 struct yagl_bimage *bi)
{
    struct yagl_surface *sfc;

    sfc = yagl_malloc0(sizeof(*sfc));

    yagl_resource_init(&sfc->res, &yagl_surface_destroy, handle);

    sfc->dpy = bi->dpy;
    sfc->type = EGL_PBUFFER_BIT;
    sfc->bi = bi;
    yagl_mutex_init(&sfc->bi_mtx);

    return sfc;
}

EGLSurface yagl_surface_get_handle(struct yagl_surface *sfc)
{
    switch (sfc->type) {
    case EGL_PBUFFER_BIT:
        return (EGLSurface)sfc->res.handle;
    case EGL_PIXMAP_BIT:
        return (EGLSurface)sfc->x_drawable.pixmap;
    case EGL_WINDOW_BIT:
        return (EGLSurface)sfc->x_drawable.win;
    default:
        assert(0);
        return NULL;
    }
}

void yagl_surface_lock(struct yagl_surface *sfc)
{
    pthread_mutex_lock(&sfc->bi_mtx);
}

void yagl_surface_unlock(struct yagl_surface *sfc)
{
    pthread_mutex_unlock(&sfc->bi_mtx);
}

void yagl_surface_update(struct yagl_surface *sfc,
                         struct yagl_bimage *bi)
{
    assert(sfc->bi);

    yagl_bimage_destroy(sfc->bi);
    sfc->bi = bi;
}

void yagl_surface_acquire(struct yagl_surface *sfc)
{
    if (sfc) {
        yagl_resource_acquire(&sfc->res);
    }
}

void yagl_surface_release(struct yagl_surface *sfc)
{
    if (sfc) {
        yagl_resource_release(&sfc->res);
    }
}
