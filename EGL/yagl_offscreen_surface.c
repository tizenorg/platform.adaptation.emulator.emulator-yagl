#include "yagl_offscreen_surface.h"
#include "yagl_host_egl_calls.h"
#include "yagl_egl_state.h"
#include "yagl_bimage.h"
#include "yagl_malloc.h"
#include "yagl_log.h"
#include "yagl_display.h"
#include "yagl_mem_egl.h"
#include "yagl_utils.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

static int yagl_offscreen_surface_resize(struct yagl_offscreen_surface *surface)
{
    int res = 0;
    EGLBoolean retval = EGL_FALSE;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int depth = 0;
    union { Window w; int i; unsigned int ui; } tmp_geom;
    unsigned int captured_width = 0;
    unsigned int captured_height = 0;
    unsigned int captured_depth = 0;
    struct yagl_bimage *bi = NULL;

    YAGL_LOG_FUNC_SET(yagl_offscreen_surface_resize);

    if (surface->base.type != EGL_WINDOW_BIT) {
        return 1;
    }

    memset(&tmp_geom, 0, sizeof(tmp_geom));

    XGetGeometry(surface->base.dpy->x_dpy,
                 surface->base.x_drawable.win,
                 &tmp_geom.w,
                 &tmp_geom.i,
                 &tmp_geom.i,
                 &width,
                 &height,
                 &tmp_geom.ui,
                 &depth);

    pthread_mutex_lock(&surface->bi_mtx);

    if (!surface->bi) {
        pthread_mutex_unlock(&surface->bi_mtx);
        YAGL_LOG_ERROR("surface->bi is NULL");
        yagl_set_error(EGL_BAD_SURFACE);
        goto out;
    }

    captured_width = surface->bi->width;
    captured_height = surface->bi->height;
    captured_depth = surface->bi->depth;

    pthread_mutex_unlock(&surface->bi_mtx);

    if ((width != captured_width) ||
        (height != captured_height) ||
        (depth != captured_depth)) {
        YAGL_LOG_DEBUG("Surface resizing from %ux%ux%u to %ux%ux%u",
                       captured_width, captured_height, captured_depth,
                       width, height, depth);
        /*
         * First of all, create new backing image.
         */

        bi = yagl_bimage_create(surface->base.dpy, width, height, depth);

        if (!bi) {
            YAGL_LOG_ERROR("yagl_bimage_create failed");
            yagl_set_error(EGL_BAD_ALLOC);
            goto out;
        }

        /*
         * Tell the host that it should use new backing image from now on.
         * No need to probe in 'bi->pixels', it's already 'mlock'ed.
         */

        YAGL_HOST_CALL_ASSERT(yagl_host_eglResizeOffscreenSurfaceYAGL(&retval,
            surface->base.dpy->host_dpy,
            surface->base.res.handle,
            bi->width,
            bi->height,
            bi->bpp,
            bi->pixels));

        if (!retval) {
            YAGL_LOG_ERROR("eglResizeOffscreenSurfaceYAGL failed");
            goto out;
        }

        /*
         * Now that the host accepted us we can be sure that 'surface' is
         * attached to current context, so no race conditions will occur and
         * we can safely replace surface's backing image with a new one.
         */

        pthread_mutex_lock(&surface->bi_mtx);

        if (surface->bi) {
            yagl_bimage_destroy(surface->bi);
            surface->bi = bi;
            bi = NULL;
        } else {
            pthread_mutex_unlock(&surface->bi_mtx);
            YAGL_LOG_ERROR("surface->bi is NULL");
            yagl_set_error(EGL_BAD_SURFACE);
            goto out;
        }

        pthread_mutex_unlock(&surface->bi_mtx);
    }

    res = 1;

out:
    if (bi) {
        yagl_bimage_destroy(bi);
    }

    return res;
}

static int yagl_offscreen_surface_invalidate(struct yagl_surface *sfc)
{
    struct yagl_offscreen_surface *osfc = (struct yagl_offscreen_surface*)sfc;

    YAGL_LOG_FUNC_SET(eglDestroySurface);

    pthread_mutex_lock(&osfc->bi_mtx);

    assert(osfc->bi);

    if (!osfc->bi) {
        pthread_mutex_unlock(&osfc->bi_mtx);
        YAGL_LOG_ERROR("we're the one who destroy the surface, but bi isn't there!");
        yagl_set_error(EGL_BAD_SURFACE);
        return 0;
    }

    yagl_bimage_destroy(osfc->bi);
    osfc->bi = NULL;

    pthread_mutex_unlock(&osfc->bi_mtx);

    return 1;
}

static int yagl_offscreen_surface_swap_buffers(struct yagl_surface *sfc)
{
    struct yagl_offscreen_surface *osfc = (struct yagl_offscreen_surface*)sfc;
    EGLBoolean retval = EGL_FALSE;

    YAGL_LOG_FUNC_SET(eglSwapBuffers);

    if (!yagl_offscreen_surface_resize(osfc)) {
        return 0;
    }

    YAGL_HOST_CALL_ASSERT(yagl_host_eglSwapBuffers(&retval,
                                                   sfc->dpy->host_dpy,
                                                   sfc->res.handle));

    if (!retval) {
        YAGL_LOG_ERROR("eglSwapBuffers failed");
        return 0;
    }

    /*
     * Host has updated our image, update the window.
     */

    pthread_mutex_lock(&osfc->bi_mtx);

    if (osfc->bi) {
        yagl_bimage_draw(osfc->bi, sfc->x_drawable.win, osfc->x_gc);
        pthread_mutex_unlock(&osfc->bi_mtx);
        return 1;
    } else {
        pthread_mutex_unlock(&osfc->bi_mtx);
        YAGL_LOG_ERROR("osfc->bi is NULL");
        yagl_set_error(EGL_BAD_SURFACE);
        return 0;
    }
}

static int yagl_offscreen_surface_copy_buffers(struct yagl_surface *sfc,
                                               Pixmap target)
{
    struct yagl_offscreen_surface *osfc = (struct yagl_offscreen_surface*)sfc;
    EGLBoolean retval = EGL_FALSE;

    YAGL_LOG_FUNC_SET(eglCopyBuffers);

    if (sfc->type == EGL_WINDOW_BIT) {
        unsigned int width = 0;
        unsigned int height = 0;
        unsigned int depth = 0;
        union { Window w; int i; unsigned int ui; } tmp_geom;
        unsigned int captured_width = 0;
        unsigned int captured_height = 0;
        unsigned int captured_depth = 0;

        memset(&tmp_geom, 0, sizeof(tmp_geom));

        XGetGeometry(sfc->dpy->x_dpy,
                     sfc->x_drawable.win,
                     &tmp_geom.w,
                     &tmp_geom.i,
                     &tmp_geom.i,
                     &width,
                     &height,
                     &tmp_geom.ui,
                     &depth);

        pthread_mutex_lock(&osfc->bi_mtx);

        if (!osfc->bi) {
            pthread_mutex_unlock(&osfc->bi_mtx);
            YAGL_LOG_ERROR("osfc->bi is NULL");
            yagl_set_error(EGL_BAD_SURFACE);
            return 0;
        }

        captured_width = osfc->bi->width;
        captured_height = osfc->bi->height;
        captured_depth = osfc->bi->depth;

        pthread_mutex_unlock(&osfc->bi_mtx);

        if ((width != captured_width) ||
            (height != captured_height) ||
            (depth != captured_depth)) {
            /*
             * Don't allow copying from window surfaces that
             * just changed their size, user must first update
             * the surface with eglSwapBuffers.
             */

            YAGL_LOG_ERROR("Not allowed");
            yagl_set_error(EGL_BAD_MATCH);
            return 0;
        }
    }

    YAGL_HOST_CALL_ASSERT(yagl_host_eglCopyBuffers(&retval,
                                                   sfc->dpy->host_dpy,
                                                   sfc->res.handle,
                                                   target));

    if (!retval) {
        YAGL_LOG_ERROR("eglCopyBuffers failed");
        return 0;
    }

    /*
     * Host has updated our image, update the surface.
     */

    pthread_mutex_lock(&osfc->bi_mtx);

    if (osfc->bi) {
        GC x_gc = XCreateGC(sfc->dpy->x_dpy, target, 0, NULL);

        if (x_gc) {
            yagl_bimage_draw(osfc->bi, target, x_gc);
            XFreeGC(sfc->dpy->x_dpy, x_gc);
        } else {
            YAGL_LOG_ERROR("XCreateGC failed");
            yagl_set_error(EGL_BAD_ALLOC);
        }

        pthread_mutex_unlock(&osfc->bi_mtx);

        return x_gc ? 1 : 0;
    } else {
        pthread_mutex_unlock(&osfc->bi_mtx);
        YAGL_LOG_ERROR("osfc->bi is NULL");
        yagl_set_error(EGL_BAD_SURFACE);
        return 0;
    }
}

static void yagl_offscreen_surface_destroy(struct yagl_ref *ref)
{
    struct yagl_offscreen_surface *sfc = (struct yagl_offscreen_surface*)ref;

    if (sfc->x_gc) {
        XFreeGC(sfc->base.dpy->x_dpy, sfc->x_gc);
        sfc->x_gc = 0;
    }

    if (sfc->bi) {
        yagl_bimage_destroy(sfc->bi);
        sfc->bi = NULL;
    }

    pthread_mutex_destroy(&sfc->bi_mtx);

    yagl_surface_cleanup(&sfc->base);

    yagl_free(sfc);
}

struct yagl_offscreen_surface
    *yagl_offscreen_surface_create_window(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          Window x_win,
                                          const EGLint* attrib_list)
{
    struct yagl_offscreen_surface *sfc;
    XWindowAttributes x_wa;
    yagl_host_handle host_surface = 0;
    struct yagl_bimage *bi = NULL;
    GC x_gc = NULL;

    YAGL_LOG_FUNC_SET(eglCreateWindowSurface);

    sfc = yagl_malloc0(sizeof(*sfc));

    if (!XGetWindowAttributes(dpy->x_dpy, x_win, &x_wa)) {
        YAGL_LOG_ERROR("XGetWindowAttributes failed");
        yagl_set_error(EGL_BAD_NATIVE_WINDOW);
        goto fail;
    }

    bi = yagl_bimage_create(dpy, x_wa.width, x_wa.height, x_wa.depth);

    if (!bi) {
        YAGL_LOG_ERROR("yagl_bimage_create failed");
        yagl_set_error(EGL_BAD_ALLOC);
        goto fail;
    }

    x_gc = XCreateGC(dpy->x_dpy, x_win, 0, NULL);

    if (!x_gc) {
        YAGL_LOG_ERROR("XCreateGC failed");
        yagl_set_error(EGL_BAD_ALLOC);
        goto fail;
    }

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreateWindowSurfaceOffscreenYAGL(&host_surface,
        dpy->host_dpy,
        host_config,
        bi->width,
        bi->height,
        bi->bpp,
        bi->pixels,
        attrib_list));

    if (!host_surface) {
        goto fail;
    }

    yagl_surface_init_window(&sfc->base,
                             &yagl_offscreen_surface_destroy,
                             host_surface,
                             dpy,
                             x_win);

    sfc->base.invalidate = &yagl_offscreen_surface_invalidate;
    sfc->base.swap_buffers = &yagl_offscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_offscreen_surface_copy_buffers;

    yagl_mutex_init(&sfc->bi_mtx);
    sfc->bi = bi;
    sfc->x_gc = x_gc;

    return sfc;

fail:
    if (bi) {
        yagl_bimage_destroy(bi);
    }
    if (x_gc) {
        XFreeGC(dpy->x_dpy, x_gc);
    }
    yagl_free(sfc);

    return NULL;
}

struct yagl_offscreen_surface
    *yagl_offscreen_surface_create_pixmap(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          Pixmap x_pixmap,
                                          const EGLint* attrib_list)
{
    struct yagl_offscreen_surface *sfc;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int depth = 0;
    union { Window w; int i; unsigned int ui; } tmp_geom;
    yagl_host_handle host_surface = 0;
    struct yagl_bimage *bi = NULL;
    GC x_gc = NULL;

    YAGL_LOG_FUNC_SET(eglCreatePixmapSurface);

    sfc = yagl_malloc0(sizeof(*sfc));

    memset(&tmp_geom, 0, sizeof(tmp_geom));

    XGetGeometry(dpy->x_dpy,
                 x_pixmap,
                 &tmp_geom.w,
                 &tmp_geom.i,
                 &tmp_geom.i,
                 &width,
                 &height,
                 &tmp_geom.ui,
                 &depth);

    bi = yagl_bimage_create(dpy, width, height, depth);

    if (!bi) {
        YAGL_LOG_ERROR("yagl_bimage_create failed");
        yagl_set_error(EGL_BAD_ALLOC);
        goto fail;
    }

    x_gc = XCreateGC(dpy->x_dpy, x_pixmap, 0, NULL);

    if (!x_gc) {
        YAGL_LOG_ERROR("XCreateGC failed");
        yagl_set_error(EGL_BAD_ALLOC);
        goto fail;
    }

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreatePixmapSurfaceOffscreenYAGL(&host_surface,
        dpy->host_dpy,
        host_config,
        bi->width,
        bi->height,
        bi->bpp,
        bi->pixels,
        attrib_list));

    if (!host_surface) {
        goto fail;
    }

    yagl_surface_init_pixmap(&sfc->base,
                             &yagl_offscreen_surface_destroy,
                             host_surface,
                             dpy,
                             x_pixmap);

    sfc->base.invalidate = &yagl_offscreen_surface_invalidate;
    sfc->base.swap_buffers = &yagl_offscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_offscreen_surface_copy_buffers;

    yagl_mutex_init(&sfc->bi_mtx);
    sfc->bi = bi;
    sfc->x_gc = x_gc;

    return sfc;

fail:
    if (bi) {
        yagl_bimage_destroy(bi);
    }
    if (x_gc) {
        XFreeGC(dpy->x_dpy, x_gc);
    }
    yagl_free(sfc);

    return NULL;
}

struct yagl_offscreen_surface
    *yagl_offscreen_surface_create_pbuffer(struct yagl_display *dpy,
                                           yagl_host_handle host_config,
                                           const EGLint* attrib_list)
{
    struct yagl_offscreen_surface *sfc;
    uint32_t width = 0;
    uint32_t height = 0;
    yagl_host_handle host_surface = 0;
    struct yagl_bimage *bi = NULL;
    int i = 0;

    YAGL_LOG_FUNC_SET(eglCreatePbufferSurface);

    sfc = yagl_malloc0(sizeof(*sfc));

    if (attrib_list) {
        while (attrib_list[i] != EGL_NONE) {
            switch (attrib_list[i]) {
            case EGL_WIDTH:
                width = attrib_list[i + 1];
                break;
            case EGL_HEIGHT:
                height = attrib_list[i + 1];
                break;
            default:
                break;
            }

            i += 2;
        }
    }

    bi = yagl_bimage_create(dpy, width, height, 24);

    if (!bi) {
        YAGL_LOG_ERROR("yagl_bimage_create failed");
        yagl_set_error(EGL_BAD_ALLOC);
        goto fail;
    }

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreatePbufferSurfaceOffscreenYAGL(&host_surface,
        dpy->host_dpy,
        host_config,
        bi->width,
        bi->height,
        bi->bpp,
        bi->pixels,
        attrib_list));

    if (!host_surface) {
        goto fail;
    }

    yagl_surface_init_pbuffer(&sfc->base,
                              &yagl_offscreen_surface_destroy,
                              host_surface,
                              dpy);

    sfc->base.invalidate = &yagl_offscreen_surface_invalidate;
    sfc->base.swap_buffers = &yagl_offscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_offscreen_surface_copy_buffers;

    yagl_mutex_init(&sfc->bi_mtx);
    sfc->bi = bi;

    return sfc;

fail:
    if (bi) {
        yagl_bimage_destroy(bi);
    }
    yagl_free(sfc);

    return NULL;
}
