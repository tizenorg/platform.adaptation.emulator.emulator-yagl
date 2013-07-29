#include "yagl_onscreen_surface.h"
#include "yagl_onscreen_buffer.h"
#include "yagl_onscreen_display.h"
#include "yagl_host_egl_calls.h"
#include "yagl_egl_state.h"
#include "yagl_log.h"
#include "yagl_mem_egl.h"
#include "yagl_utils.h"
#include "yagl_malloc.h"
#include "yagl_native_display.h"
#include "yagl_native_drawable.h"
#include "vigs.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void yagl_onscreen_surface_invalidate(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    struct yagl_onscreen_display *dpy = (struct yagl_onscreen_display*)sfc->dpy;
    struct yagl_onscreen_buffer *new_buffer;

    if (osfc->last_stamp == sfc->native_drawable->stamp) {
        return;
    }

    assert(sfc->type == EGL_WINDOW_BIT);

    osfc->last_stamp = sfc->native_drawable->stamp;

    /*
     * We only process new buffer if it's different from
     * the current one.
     */
    new_buffer = yagl_onscreen_buffer_create(dpy,
                                             sfc->native_drawable,
                                             yagl_native_attachment_back,
                                             osfc->buffer->name);

    if (!new_buffer) {
        return;
    }

    yagl_onscreen_buffer_destroy(osfc->buffer);
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
    EGLBoolean retval = EGL_FALSE;

    YAGL_LOG_FUNC_SET(eglSwapBuffers);

    YAGL_HOST_CALL_ASSERT(yagl_host_eglSwapBuffers(&retval,
                                                   sfc->dpy->host_dpy,
                                                   sfc->res.handle));

    if (!retval) {
        YAGL_LOG_ERROR("eglSwapBuffers failed");
        return 0;
    }

    yagl_onscreen_surface_finish(sfc);

    sfc->native_drawable->swap_buffers(sfc->native_drawable);

    return 1;
}

static int yagl_onscreen_surface_copy_buffers(struct yagl_surface *sfc,
                                              yagl_os_pixmap target)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    EGLBoolean retval = EGL_FALSE;

    YAGL_LOG_FUNC_SET(eglCopyBuffers);

    YAGL_HOST_CALL_ASSERT(yagl_host_eglCopyBuffers(&retval,
                                                   sfc->dpy->host_dpy,
                                                   sfc->res.handle));

    if (!retval) {
        YAGL_LOG_ERROR("eglCopyBuffers failed");
        return 0;
    }

    yagl_onscreen_surface_finish(sfc);

    sfc->native_drawable->copy_to_pixmap(sfc->native_drawable,
                                         target, 0, 0, 0, 0,
                                         osfc->buffer->drm_sfc->width,
                                         osfc->buffer->drm_sfc->height);

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
        sfc->native_drawable->wait(sfc->native_drawable,
                                   osfc->buffer->drm_sfc->width,
                                   osfc->buffer->drm_sfc->height);
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
    uint32_t saf = 0;

    YAGL_LOG_FUNC_SET(eglQuerySurface);

    ret = vigs_drm_gem_map(&osfc->buffer->drm_sfc->gem, 1);

    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_gem_map failed: %s",
                       strerror(-ret));
        return;
    }

    if ((sfc->lock_hint & EGL_READ_SURFACE_BIT_KHR) != 0) {
        saf |= VIGS_DRM_SAF_READ;
    }

    if ((sfc->lock_hint & EGL_WRITE_SURFACE_BIT_KHR) != 0) {
        saf |= VIGS_DRM_SAF_WRITE;
    }

    ret = vigs_drm_surface_start_access(osfc->buffer->drm_sfc, saf);
    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_surface_start_access failed: %s",
                       strerror(-ret));
    }

    sfc->lock_ptr = osfc->buffer->drm_sfc->gem.vaddr;
    sfc->lock_stride = osfc->buffer->drm_sfc->stride;
}

static void yagl_onscreen_surface_unmap(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    int ret;

    YAGL_LOG_FUNC_SET(eglUnlockSurfaceKHR);

    ret = vigs_drm_surface_end_access(osfc->buffer->drm_sfc, 1);
    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_surface_end_access failed: %s",
                       strerror(-ret));
    }
}

static void yagl_onscreen_surface_set_swap_interval(struct yagl_surface *sfc,
                                                    int interval)
{
    assert(sfc->type == EGL_WINDOW_BIT);

    sfc->native_drawable->set_swap_interval(sfc->native_drawable, interval);
}

static void yagl_onscreen_surface_destroy(struct yagl_ref *ref)
{
    struct yagl_onscreen_surface *sfc = (struct yagl_onscreen_surface*)ref;

    yagl_onscreen_buffer_destroy(sfc->buffer);

    if (sfc->tmp_pixmap) {
        sfc->tmp_pixmap->destroy(sfc->tmp_pixmap);
        sfc->tmp_pixmap = NULL;
    }

    yagl_surface_cleanup(&sfc->base);

    yagl_free(sfc);
}

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_window(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         struct yagl_native_drawable *native_window,
                                         const EGLint* attrib_list)
{
    struct yagl_onscreen_display *odpy = (struct yagl_onscreen_display*)dpy;
    struct yagl_onscreen_surface *sfc;
    struct yagl_onscreen_buffer *new_buffer = NULL;
    yagl_host_handle host_surface = 0;

    sfc = yagl_malloc0(sizeof(*sfc));

    new_buffer = yagl_onscreen_buffer_create(odpy,
                                             native_window,
                                             yagl_native_attachment_back,
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
                             native_window);

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
        yagl_onscreen_buffer_destroy(new_buffer);
    }
    yagl_free(sfc);

    return NULL;
}

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pixmap(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         struct yagl_native_drawable *native_pixmap,
                                         const EGLint* attrib_list)
{
    struct yagl_onscreen_display *odpy = (struct yagl_onscreen_display*)dpy;
    struct yagl_onscreen_surface *sfc;
    struct yagl_onscreen_buffer *new_buffer = NULL;
    yagl_host_handle host_surface = 0;

    sfc = yagl_malloc0(sizeof(*sfc));

    new_buffer = yagl_onscreen_buffer_create(odpy,
                                             native_pixmap,
                                             yagl_native_attachment_front,
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
                             native_pixmap);

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
        yagl_onscreen_buffer_destroy(new_buffer);
    }
    yagl_free(sfc);

    return NULL;
}

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

    sfc->tmp_pixmap = dpy->native_dpy->create_pixmap(dpy->native_dpy,
                                                     width,
                                                     height,
                                                     24);

    if (!sfc->tmp_pixmap) {
        YAGL_LOG_ERROR("create_pixmap(%u,%u) failed", width, height);
        yagl_set_error(EGL_BAD_ALLOC);
        goto fail;
    }

    new_buffer = yagl_onscreen_buffer_create(odpy,
                                             sfc->tmp_pixmap,
                                             yagl_native_attachment_front,
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
                yagl_onscreen_buffer_destroy(new_buffer);
            }
            sfc->tmp_pixmap->destroy(sfc->tmp_pixmap);
            sfc->tmp_pixmap = NULL;
        }
        yagl_free(sfc);
    }

    return NULL;
}
