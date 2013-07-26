#include "yagl_surface.h"
#include "yagl_utils.h"
#include <assert.h>
#include "EGL/eglext.h"

void yagl_surface_init_window(struct yagl_surface *sfc,
                              yagl_ref_destroy_func destroy_func,
                              yagl_host_handle handle,
                              struct yagl_display *dpy,
                              Window x_win)
{
    yagl_resource_init(&sfc->res, destroy_func, handle);

    sfc->dpy = dpy;
    sfc->type = EGL_WINDOW_BIT;
    sfc->x_drawable.win = x_win;

    yagl_recursive_mutex_init(&sfc->mtx);
}

void yagl_surface_init_pixmap(struct yagl_surface *sfc,
                              yagl_ref_destroy_func destroy_func,
                              yagl_host_handle handle,
                              struct yagl_display *dpy,
                              Pixmap x_pixmap)
{
    yagl_resource_init(&sfc->res, destroy_func, handle);

    sfc->dpy = dpy;
    sfc->type = EGL_PIXMAP_BIT;
    sfc->x_drawable.pixmap = x_pixmap;

    yagl_recursive_mutex_init(&sfc->mtx);
}

void yagl_surface_init_pbuffer(struct yagl_surface *sfc,
                               yagl_ref_destroy_func destroy_func,
                               yagl_host_handle handle,
                               struct yagl_display *dpy)
{
    yagl_resource_init(&sfc->res, destroy_func, handle);

    sfc->dpy = dpy;
    sfc->type = EGL_PBUFFER_BIT;

    yagl_recursive_mutex_init(&sfc->mtx);
}

void yagl_surface_cleanup(struct yagl_surface *sfc)
{
    yagl_resource_cleanup(&sfc->res);
    pthread_mutex_destroy(&sfc->mtx);
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

void yagl_surface_invalidate(struct yagl_surface *sfc)
{
    pthread_mutex_lock(&sfc->mtx);

    if (sfc->lock_hint == 0) {
        sfc->invalidate(sfc);
    }

    pthread_mutex_unlock(&sfc->mtx);
}

int yagl_surface_lock(struct yagl_surface *sfc, EGLint hint)
{
    int ret = 0;

    pthread_mutex_lock(&sfc->mtx);

    if (sfc->lock_hint == 0) {
        sfc->lock_hint = (hint & EGL_READ_SURFACE_BIT_KHR) |
                         (hint & EGL_WRITE_SURFACE_BIT_KHR);
        if (sfc->lock_hint == 0) {
            sfc->lock_hint = EGL_READ_SURFACE_BIT_KHR |
                             EGL_WRITE_SURFACE_BIT_KHR;
        }

        ret = 1;
    }

    pthread_mutex_unlock(&sfc->mtx);

    return ret;
}

int yagl_surface_locked(struct yagl_surface *sfc)
{
    int ret;

    pthread_mutex_lock(&sfc->mtx);

    ret = (sfc->lock_hint != 0);

    pthread_mutex_unlock(&sfc->mtx);

    return ret;
}

int yagl_surface_unlock(struct yagl_surface *sfc)
{
    int ret = 0;

    pthread_mutex_lock(&sfc->mtx);

    if (sfc->lock_hint != 0) {
        if (sfc->lock_ptr) {
            sfc->unmap(sfc);
        }

        sfc->lock_hint = 0;
        sfc->lock_ptr = NULL;
        sfc->lock_stride = 0;
        ret = 1;
    }

    pthread_mutex_unlock(&sfc->mtx);

    return ret;
}

void *yagl_surface_map(struct yagl_surface *sfc, uint32_t *stride)
{
    void *ret = NULL;

    pthread_mutex_lock(&sfc->mtx);

    if (sfc->lock_hint != 0) {
        if (!sfc->lock_ptr) {
            sfc->map(sfc);
        }
        if (sfc->lock_ptr) {
            ret = sfc->lock_ptr;
            *stride = sfc->lock_stride;
        }
    }

    pthread_mutex_unlock(&sfc->mtx);

    return ret;
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
