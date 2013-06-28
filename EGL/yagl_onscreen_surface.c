#include "yagl_onscreen_surface.h"
#include "yagl_host_egl_calls.h"
#include "yagl_egl_state.h"
#include "yagl_log.h"
#include "yagl_mem_egl.h"
#include "yagl_utils.h"
#include "yagl_malloc.h"
#include <assert.h>
#include <stdio.h>

/*
 * When this is defined EGL pixmap surface will behave more like pbuffer
 * surface, i.e. it'll allocate additional pixmap and all OpenGL rendering
 * will go there. eglCopyBuffers will be the only way to get the rendering
 * results.
 *
 * This is a temporary workaround for Tizen, because it currently
 * behaves like if pixmap surfaces were pbuffer surfaces.
 *
 * One of the examples is WebKit, it renders WebGL like this:
 * + sfc = eglCreatePixmapSurface(... pixmap_id ...)
 * + eglMakeCurrent(... sfc ...);
 * + (render step)
 * + async: XGetImage(pixmap_id)
 * + eglCopyBuffers(... pixmap_id ...)
 *
 * I.e. it makes asynchronous calls to XGetImage for the pixmap which is
 * being rendered to. It doesn't call eglWaitX/glFlush/glFinish, thus, it
 * assumes that the image stored in 'pixmap_id' is complete and won't have any
 * intermediate rendering results and this is only possible if this pixmap
 * surface is a pbuffer surface. A eglCopyBuffers call at the end copies
 * the rendering results to the pixmap itself, which is really stupid thing
 * to do... unless this is a pbuffer surface...
 */
//#define YAGL_FAKE_PIXMAP_SURFACE

static void yagl_onscreen_surface_copy_drawable(struct yagl_onscreen_surface *sfc,
                                                int src, int dest)
{
    XRectangle xrect;
    XserverRegion region;

    xrect.x = 0;
    xrect.y = 0;
    xrect.width = sfc->buffer->drm_sfc->width;
    xrect.height = sfc->buffer->drm_sfc->height;

    region = XFixesCreateRegion(sfc->base.dpy->x_dpy, &xrect, 1);
    switch (sfc->base.type) {
    case EGL_PBUFFER_BIT:
        yagl_DRI2CopyRegion(sfc->base.dpy->x_dpy, sfc->tmp_pixmap,
                            region, dest, src);
        break;
    case EGL_PIXMAP_BIT:
        yagl_DRI2CopyRegion(sfc->base.dpy->x_dpy, sfc->base.x_drawable.pixmap,
                            region, dest, src);
        break;
    case EGL_WINDOW_BIT:
        yagl_DRI2CopyRegion(sfc->base.dpy->x_dpy, sfc->base.x_drawable.win,
                            region, dest, src);
        break;
    default:
        assert(0);
        break;
    }
    XFixesDestroyRegion(sfc->base.dpy->x_dpy, region);
}

static void yagl_onscreen_surface_invalidate(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    struct yagl_onscreen_display *dpy = (struct yagl_onscreen_display*)sfc->dpy;
    struct yagl_onscreen_buffer *new_buffer;

    if (osfc->last_stamp == osfc->stamp) {
        return;
    }

    assert(sfc->type == EGL_WINDOW_BIT);

    osfc->last_stamp = osfc->stamp;

    /*
     * We only process new buffer if it's different from
     * the current one.
     */
    new_buffer = yagl_onscreen_display_create_buffer(dpy,
                                                     sfc->x_drawable.win,
                                                     DRI2BufferBackLeft,
                                                     osfc->buffer->dri2_buffer->name);

    if (!new_buffer) {
        return;
    }

    yagl_onscreen_display_destroy_buffer(osfc->buffer);
    osfc->buffer = new_buffer;

    YAGL_HOST_CALL_ASSERT(yagl_host_eglInvalidateOnscreenSurfaceYAGL(
        dpy->base.host_dpy, sfc->res.handle, new_buffer->drm_sfc->id));
}

static void yagl_onscreen_surface_finish(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    int ret;

    YAGL_LOG_FUNC_SET(yagl_onscreen_surface_finish);

    ret = vigs_drm_surface_set_gpu_dirty(osfc->buffer->drm_sfc);

    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_surface_set_gpu_dirty failed: %s",
                       strerror(-ret));
    }
}

static int yagl_onscreen_surface_swap_buffers(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    EGLBoolean retval = EGL_FALSE;
    CARD64 count = 0;

    YAGL_LOG_FUNC_SET(eglSwapBuffers);

    YAGL_HOST_CALL_ASSERT(yagl_host_eglSwapBuffers(&retval,
                                                   sfc->dpy->host_dpy,
                                                   sfc->res.handle));

    if (!retval) {
        YAGL_LOG_ERROR("eglSwapBuffers failed");
        return 0;
    }

    yagl_onscreen_surface_finish(sfc);

    yagl_DRI2SwapBuffers(sfc->dpy->x_dpy,
                         sfc->x_drawable.win, 0, 0, 0,
                         &count);

    return 1;
}

static int yagl_onscreen_surface_copy_buffers(struct yagl_surface *sfc,
                                              Pixmap target)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    EGLBoolean retval = EGL_FALSE;
    GC x_gc;

    YAGL_LOG_FUNC_SET(eglCopyBuffers);

    YAGL_HOST_CALL_ASSERT(yagl_host_eglCopyBuffers(&retval,
                                                   sfc->dpy->host_dpy,
                                                   sfc->res.handle));

    if (!retval) {
        YAGL_LOG_ERROR("eglCopyBuffers failed");
        return 0;
    }

    yagl_onscreen_surface_finish(sfc);

    x_gc = XCreateGC(sfc->dpy->x_dpy, target, 0, NULL);

    if (x_gc) {
        switch (sfc->type) {
        case EGL_PBUFFER_BIT:
            XCopyArea(sfc->dpy->x_dpy,
                      osfc->tmp_pixmap,
                      target,
                      x_gc,
                      0, 0,
                      osfc->buffer->drm_sfc->width,
                      osfc->buffer->drm_sfc->height,
                      0, 0);
            break;
        case EGL_PIXMAP_BIT:
#ifdef YAGL_FAKE_PIXMAP_SURFACE
            XCopyArea(sfc->dpy->x_dpy,
                      osfc->tmp_pixmap,
                      target,
                      x_gc,
                      0, 0,
                      osfc->buffer->drm_sfc->width,
                      osfc->buffer->drm_sfc->height,
                      0, 0);
#else
            XCopyArea(sfc->dpy->x_dpy,
                      sfc->x_drawable.pixmap,
                      target,
                      x_gc,
                      0, 0,
                      osfc->buffer->drm_sfc->width,
                      osfc->buffer->drm_sfc->height,
                      0, 0);
#endif
            break;
        case EGL_WINDOW_BIT:
            XCopyArea(sfc->dpy->x_dpy,
                      sfc->x_drawable.win,
                      target,
                      x_gc,
                      0, 0,
                      osfc->buffer->drm_sfc->width,
                      osfc->buffer->drm_sfc->height,
                      0, 0);
            break;
        default:
            assert(0);
            YAGL_LOG_ERROR("Bad surface type");
        }
    } else {
        YAGL_LOG_ERROR("XCreateGC failed");
    }

    XFreeGC(sfc->dpy->x_dpy, x_gc);

    XSync(sfc->dpy->x_dpy, 0);

    return 1;
}

static void yagl_onscreen_surface_wait_x(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;

    switch (sfc->type) {
    case EGL_PBUFFER_BIT:
    case EGL_WINDOW_BIT:
        /*
         * Currently our window surfaces are always double-buffered, so
         * this is a no-op.
         */
        break;
    case EGL_PIXMAP_BIT:
#ifndef YAGL_FAKE_PIXMAP_SURFACE
        yagl_onscreen_surface_copy_drawable(osfc,
                                            DRI2BufferFrontLeft,
                                            DRI2BufferFakeFrontLeft);
#endif
        break;
    default:
        assert(0);
        break;
    }
}

static void yagl_onscreen_surface_wait_gl(struct yagl_surface *sfc)
{
    EGLBoolean retval;

    /*
     * There used to be DRI2CopyRegion call here, but it's not needed.
     * This call must not be distinguishable from glFinish, so just do
     * the stuff glFinish does here and not more.
     */
    YAGL_HOST_CALL_ASSERT(yagl_host_eglWaitClient(&retval));
    yagl_onscreen_surface_finish(sfc);
}

static void yagl_onscreen_surface_map(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    int ret;

    YAGL_LOG_FUNC_SET(eglQuerySurface);

    ret = vigs_drm_gem_map(&osfc->buffer->drm_sfc->gem);

    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_gem_map failed: %s",
                       strerror(-ret));
        return;
    }

    if ((sfc->lock_hint & EGL_READ_SURFACE_BIT_KHR) != 0) {
        ret = vigs_drm_surface_update_vram(osfc->buffer->drm_sfc);
        if (ret != 0) {
            YAGL_LOG_ERROR("vigs_drm_surface_update_vram failed: %s",
                           strerror(-ret));
        }
    }

    sfc->lock_ptr = osfc->buffer->drm_sfc->gem.vaddr;
    sfc->lock_stride = osfc->buffer->drm_sfc->stride;
}

static void yagl_onscreen_surface_unmap(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    int ret;

    YAGL_LOG_FUNC_SET(eglUnlockSurfaceKHR);

    if ((sfc->lock_hint & EGL_WRITE_SURFACE_BIT_KHR) != 0) {
        ret = vigs_drm_surface_update_gpu(osfc->buffer->drm_sfc);
        if (ret != 0) {
            YAGL_LOG_ERROR("vigs_drm_surface_update_gpu failed: %s",
                           strerror(-ret));
        }
    }
}

static void yagl_onscreen_surface_set_swap_interval(struct yagl_surface *sfc,
                                                    int interval)
{
    assert(sfc->type == EGL_WINDOW_BIT);

    yagl_DRI2SwapInterval(sfc->dpy->x_dpy, sfc->x_drawable.win, interval);
}

static void yagl_onscreen_surface_destroy(struct yagl_ref *ref)
{
    struct yagl_onscreen_surface *sfc = (struct yagl_onscreen_surface*)ref;

    yagl_onscreen_display_destroy_buffer(sfc->buffer);
    switch (sfc->base.type) {
    case EGL_PBUFFER_BIT:
        yagl_DRI2DestroyDrawable(sfc->base.dpy->x_dpy,
                                 sfc->tmp_pixmap);
        break;
    case EGL_PIXMAP_BIT:
        yagl_DRI2DestroyDrawable(sfc->base.dpy->x_dpy,
                                 sfc->base.x_drawable.pixmap);
        break;
    case EGL_WINDOW_BIT:
        yagl_DRI2DestroyDrawable(sfc->base.dpy->x_dpy,
                                 sfc->base.x_drawable.win);
        break;
    default:
        assert(0);
        break;
    }

    if (sfc->tmp_pixmap) {
        XFreePixmap(sfc->base.dpy->x_dpy,
                    sfc->tmp_pixmap);
        sfc->tmp_pixmap = 0;
    }

    yagl_surface_cleanup(&sfc->base);

    yagl_free(sfc);
}

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_window(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         Window x_win,
                                         const EGLint* attrib_list)
{
    struct yagl_onscreen_display *odpy = (struct yagl_onscreen_display*)dpy;
    struct yagl_onscreen_surface *sfc;
    struct yagl_onscreen_buffer *new_buffer = NULL;
    yagl_host_handle host_surface = 0;

    YAGL_LOG_FUNC_SET(eglCreateWindowSurface);

    sfc = yagl_malloc0(sizeof(*sfc));

    yagl_DRI2CreateDrawable(dpy->x_dpy, x_win);

    new_buffer = yagl_onscreen_display_create_buffer(odpy,
                                                     x_win,
                                                     DRI2BufferBackLeft,
                                                     0);

    if (!new_buffer) {
        yagl_set_error(EGL_BAD_NATIVE_WINDOW);
        goto fail;
    }

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreateWindowSurfaceOnscreenYAGL(&host_surface,
        dpy->host_dpy,
        host_config,
        new_buffer->drm_sfc->id,
        attrib_list));

    if (!host_surface) {
        goto fail;
    }

    yagl_surface_init_window(&sfc->base,
                             &yagl_onscreen_surface_destroy,
                             host_surface,
                             dpy,
                             x_win);

    sfc->base.invalidate = &yagl_onscreen_surface_invalidate;
    sfc->base.finish = &yagl_onscreen_surface_finish;
    sfc->base.swap_buffers = &yagl_onscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_onscreen_surface_copy_buffers;
    sfc->base.wait_x = &yagl_onscreen_surface_wait_x;
    sfc->base.wait_gl = &yagl_onscreen_surface_wait_gl;
    sfc->base.map = &yagl_onscreen_surface_map;
    sfc->base.unmap = &yagl_onscreen_surface_unmap;
    sfc->base.set_swap_interval = &yagl_onscreen_surface_set_swap_interval;

    sfc->buffer = new_buffer;

    return sfc;

fail:
    if (new_buffer) {
        yagl_onscreen_display_destroy_buffer(new_buffer);
        yagl_DRI2DestroyDrawable(dpy->x_dpy, x_win);
    }
    yagl_free(sfc);

    return NULL;
}

#ifdef YAGL_FAKE_PIXMAP_SURFACE
struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pixmap(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         Pixmap x_pixmap,
                                         const EGLint* attrib_list)
{
    struct yagl_onscreen_display *odpy = (struct yagl_onscreen_display*)dpy;
    struct yagl_onscreen_surface *sfc;
    uint32_t width = 0;
    uint32_t height = 0;
    unsigned int depth = 0;
    union { Window w; int i; unsigned int ui; } tmp_geom;
    struct yagl_onscreen_buffer *new_buffer = NULL;
    yagl_host_handle host_surface = 0;

    YAGL_LOG_FUNC_SET(eglCreatePixmapSurface);

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

    sfc = yagl_malloc0(sizeof(*sfc));

    sfc->tmp_pixmap = XCreatePixmap(dpy->x_dpy,
        RootWindow(dpy->x_dpy, DefaultScreen(dpy->x_dpy)),
        width, height, depth);

    if (!sfc->tmp_pixmap) {
        YAGL_LOG_ERROR("XCreatePixmap(%u,%u) failed", width, height);
        yagl_set_error(EGL_BAD_ALLOC);
        goto fail;
    }

    yagl_DRI2CreateDrawable(dpy->x_dpy, sfc->tmp_pixmap);

    new_buffer = yagl_onscreen_display_create_buffer(odpy,
                                                     sfc->tmp_pixmap,
                                                     DRI2BufferFrontLeft,
                                                     0);

    if (!new_buffer) {
        yagl_set_error(EGL_BAD_NATIVE_PIXMAP);
        goto fail;
    }

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreatePixmapSurfaceOnscreenYAGL(&host_surface,
        dpy->host_dpy,
        host_config,
        new_buffer->drm_sfc->id,
        attrib_list));

    if (!host_surface) {
        goto fail;
    }

    yagl_surface_init_pixmap(&sfc->base,
                             &yagl_onscreen_surface_destroy,
                             host_surface,
                             dpy,
                             x_pixmap);

    sfc->base.invalidate = &yagl_onscreen_surface_invalidate;
    sfc->base.finish = &yagl_onscreen_surface_finish;
    sfc->base.swap_buffers = &yagl_onscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_onscreen_surface_copy_buffers;
    sfc->base.wait_x = &yagl_onscreen_surface_wait_x;
    sfc->base.wait_gl = &yagl_onscreen_surface_wait_gl;
    sfc->base.map = &yagl_onscreen_surface_map;
    sfc->base.unmap = &yagl_onscreen_surface_unmap;
    sfc->base.set_swap_interval = &yagl_onscreen_surface_set_swap_interval;

    sfc->buffer = new_buffer;

    return sfc;

fail:
    if (sfc) {
        if (sfc->tmp_pixmap) {
            if (new_buffer) {
                yagl_onscreen_display_destroy_buffer(new_buffer);
                yagl_DRI2DestroyDrawable(dpy->x_dpy, sfc->tmp_pixmap);
            }
            XFreePixmap(dpy->x_dpy, sfc->tmp_pixmap);
        }
        yagl_free(sfc);
    }

    return NULL;
}
#else
struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pixmap(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         Pixmap x_pixmap,
                                         const EGLint* attrib_list)
{
    struct yagl_onscreen_display *odpy = (struct yagl_onscreen_display*)dpy;
    struct yagl_onscreen_surface *sfc;
    struct yagl_onscreen_buffer *new_buffer = NULL;
    yagl_host_handle host_surface = 0;

    YAGL_LOG_FUNC_SET(eglCreatePixmapSurface);

    sfc = yagl_malloc0(sizeof(*sfc));

    yagl_DRI2CreateDrawable(dpy->x_dpy, x_pixmap);

    new_buffer = yagl_onscreen_display_create_buffer(odpy,
                                                     x_pixmap,
                                                     DRI2BufferFrontLeft,
                                                     0);

    if (!new_buffer) {
        yagl_set_error(EGL_BAD_NATIVE_PIXMAP);
        goto fail;
    }

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreatePixmapSurfaceOnscreenYAGL(&host_surface,
        dpy->host_dpy,
        host_config,
        new_buffer->drm_sfc->id,
        attrib_list));

    if (!host_surface) {
        goto fail;
    }

    yagl_surface_init_pixmap(&sfc->base,
                             &yagl_onscreen_surface_destroy,
                             host_surface,
                             dpy,
                             x_pixmap);

    sfc->base.invalidate = &yagl_onscreen_surface_invalidate;
    sfc->base.finish = &yagl_onscreen_surface_finish;
    sfc->base.swap_buffers = &yagl_onscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_onscreen_surface_copy_buffers;
    sfc->base.wait_x = &yagl_onscreen_surface_wait_x;
    sfc->base.wait_gl = &yagl_onscreen_surface_wait_gl;
    sfc->base.map = &yagl_onscreen_surface_map;
    sfc->base.unmap = &yagl_onscreen_surface_unmap;
    sfc->base.set_swap_interval = &yagl_onscreen_surface_set_swap_interval;

    sfc->buffer = new_buffer;

    return sfc;

fail:
    if (new_buffer) {
        yagl_onscreen_display_destroy_buffer(new_buffer);
        yagl_DRI2DestroyDrawable(dpy->x_dpy, x_pixmap);
    }
    yagl_free(sfc);

    return NULL;
}
#endif

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pbuffer(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          const EGLint* attrib_list)
{
    struct yagl_onscreen_display *odpy = (struct yagl_onscreen_display*)dpy;
    struct yagl_onscreen_surface *sfc;
    uint32_t width = 0;
    uint32_t height = 0;
    int i = 0;
    struct yagl_onscreen_buffer *new_buffer = NULL;
    yagl_host_handle host_surface = 0;

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

    sfc->tmp_pixmap = XCreatePixmap(dpy->x_dpy,
        RootWindow(dpy->x_dpy, DefaultScreen(dpy->x_dpy)),
        width, height, 24);

    if (!sfc->tmp_pixmap) {
        YAGL_LOG_ERROR("XCreatePixmap(%u,%u) failed", width, height);
        yagl_set_error(EGL_BAD_ALLOC);
        goto fail;
    }

    yagl_DRI2CreateDrawable(dpy->x_dpy, sfc->tmp_pixmap);

    new_buffer = yagl_onscreen_display_create_buffer(odpy,
                                                     sfc->tmp_pixmap,
                                                     DRI2BufferFrontLeft,
                                                     0);

    if (!new_buffer) {
        yagl_set_error(EGL_BAD_ALLOC);
        goto fail;
    }

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreatePbufferSurfaceOnscreenYAGL(&host_surface,
        dpy->host_dpy,
        host_config,
        new_buffer->drm_sfc->id,
        attrib_list));

    if (!host_surface) {
        goto fail;
    }

    yagl_surface_init_pbuffer(&sfc->base,
                              &yagl_onscreen_surface_destroy,
                              host_surface,
                              dpy);

    sfc->base.invalidate = &yagl_onscreen_surface_invalidate;
    sfc->base.finish = &yagl_onscreen_surface_finish;
    sfc->base.swap_buffers = &yagl_onscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_onscreen_surface_copy_buffers;
    sfc->base.wait_x = &yagl_onscreen_surface_wait_x;
    sfc->base.wait_gl = &yagl_onscreen_surface_wait_gl;
    sfc->base.map = &yagl_onscreen_surface_map;
    sfc->base.unmap = &yagl_onscreen_surface_unmap;
    sfc->base.set_swap_interval = &yagl_onscreen_surface_set_swap_interval;

    sfc->buffer = new_buffer;

    return sfc;

fail:
    if (sfc) {
        if (sfc->tmp_pixmap) {
            if (new_buffer) {
                yagl_onscreen_display_destroy_buffer(new_buffer);
                yagl_DRI2DestroyDrawable(dpy->x_dpy, sfc->tmp_pixmap);
            }
            XFreePixmap(dpy->x_dpy, sfc->tmp_pixmap);
        }
        yagl_free(sfc);
    }

    return NULL;
}
