#include "yagl_onscreen_surface.h"
#include "yagl_onscreen_utils.h"
#include "yagl_host_egl_calls.h"
#include "yagl_egl_state.h"
#include "yagl_state.h"
#include "yagl_log.h"
#include "yagl_utils.h"
#include "yagl_malloc.h"
#include "yagl_display.h"
#include "yagl_native_display.h"
#include "yagl_native_drawable.h"
#include "yagl_transport_egl.h"
#include "yagl_client_interface.h"
#include "vigs.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct yagl_native_drawable
    *native_drawable(struct yagl_onscreen_surface *osfc)
{
    return osfc->tmp_pixmap ? osfc->tmp_pixmap : osfc->base.native_drawable;
}

static void yagl_onscreen_surface_invalidate(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    struct yagl_native_drawable *drawable = native_drawable(osfc);
    struct vigs_drm_surface *new_drm_sfc;

    if (osfc->last_stamp == drawable->stamp) {
        return;
    }

    assert(sfc->type == EGL_WINDOW_BIT);

    osfc->last_stamp = drawable->stamp;

    /*
     * We only process new buffer if it's different from
     * the current one.
     */
    new_drm_sfc = yagl_onscreen_buffer_create(drawable,
                                              yagl_native_attachment_back,
                                              osfc->drm_sfc);

    if (!new_drm_sfc) {
        return;
    }

    vigs_drm_gem_unref(&osfc->drm_sfc->gem);
    osfc->drm_sfc = new_drm_sfc;

    yagl_host_eglInvalidateOnscreenSurfaceYAGL(sfc->dpy->host_dpy,
                                               sfc->res.handle,
                                               new_drm_sfc->id);
}

static int yagl_onscreen_surface_swap_buffers(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    struct yagl_native_drawable *drawable = native_drawable(osfc);
    int ret;

    YAGL_LOG_FUNC_SET(eglSwapBuffers);

    yagl_host_eglSwapBuffers(sfc->dpy->host_dpy,
                             sfc->res.handle);

    ret = vigs_drm_surface_set_gpu_dirty(osfc->drm_sfc);

    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_surface_set_gpu_dirty failed: %s",
                       strerror(-ret));
    }

    drawable->swap_buffers(drawable);

    return 1;
}

static int yagl_onscreen_surface_copy_buffers(struct yagl_surface *sfc,
                                              yagl_os_pixmap target)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    struct yagl_native_drawable *drawable = native_drawable(osfc);
    int ret;

    YAGL_LOG_FUNC_SET(eglCopyBuffers);

    yagl_host_eglCopyBuffers(sfc->dpy->host_dpy,
                             sfc->res.handle);

    ret = vigs_drm_surface_set_gpu_dirty(osfc->drm_sfc);

    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_surface_set_gpu_dirty failed: %s",
                       strerror(-ret));
    }

    drawable->copy_to_pixmap(drawable,
                             target, 0, 0, 0, 0,
                             osfc->drm_sfc->width,
                             osfc->drm_sfc->height);

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
                                   osfc->drm_sfc->width,
                                   osfc->drm_sfc->height);
        break;
    default:
        assert(0);
        break;
    }
}

static void yagl_onscreen_surface_wait_gl(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    int ret;

    YAGL_LOG_FUNC_SET(yagl_onscreen_surface_wait_gl);

    yagl_host_eglWaitClient();

    ret = vigs_drm_surface_set_gpu_dirty(osfc->drm_sfc);

    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_surface_set_gpu_dirty failed: %s",
                       strerror(-ret));
    }
}

static void yagl_onscreen_surface_map(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    int ret;
    uint32_t saf = 0;

    YAGL_LOG_FUNC_SET(eglQuerySurface);

    ret = vigs_drm_gem_map(&osfc->drm_sfc->gem, 1);

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

    ret = vigs_drm_surface_start_access(osfc->drm_sfc, saf);
    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_surface_start_access failed: %s",
                       strerror(-ret));
    }

    sfc->lock_ptr = osfc->drm_sfc->gem.vaddr;
    sfc->lock_stride = osfc->drm_sfc->stride;
}

static void yagl_onscreen_surface_unmap(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    int ret;

    YAGL_LOG_FUNC_SET(eglUnlockSurfaceKHR);

    ret = vigs_drm_surface_end_access(osfc->drm_sfc, 1);
    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_surface_end_access failed: %s",
                       strerror(-ret));
    }
}

static void yagl_onscreen_surface_set_swap_interval(struct yagl_surface *sfc,
                                                    int interval)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    struct yagl_native_drawable *drawable = native_drawable(osfc);

    assert(sfc->type == EGL_WINDOW_BIT);

    drawable->set_swap_interval(drawable, interval);
}

static struct yagl_client_image
    *yagl_onscreen_surface_create_image(struct yagl_surface *sfc,
                                        struct yagl_client_interface *iface)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    EGLint error = 0;
    yagl_object_name tex_global_name = yagl_get_global_name();

    if (!yagl_host_eglCreateImageYAGL(tex_global_name,
                                      sfc->dpy->host_dpy,
                                      osfc->drm_sfc->id,
                                      &error)) {
        yagl_set_error(error);
        return NULL;
    }

    return iface->create_image(iface, tex_global_name);
}

static void yagl_onscreen_surface_destroy(struct yagl_ref *ref)
{
    struct yagl_onscreen_surface *sfc = (struct yagl_onscreen_surface*)ref;

    vigs_drm_gem_unref(&sfc->drm_sfc->gem);

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
    EGLint error = 0;
    struct yagl_onscreen_surface *sfc;
    struct vigs_drm_surface *drm_sfc = NULL;
    yagl_host_handle host_surface = 0;

    sfc = yagl_malloc0(sizeof(*sfc));

    drm_sfc = yagl_onscreen_buffer_create(native_window,
                                          yagl_native_attachment_back,
                                          NULL);

    if (!drm_sfc) {
        yagl_set_error(EGL_BAD_NATIVE_WINDOW);
        goto fail;
    }

    host_surface = yagl_host_eglCreateWindowSurfaceOnscreenYAGL(dpy->host_dpy,
                                                                host_config,
                                                                drm_sfc->id,
                                                                attrib_list,
                                                                yagl_transport_attrib_list_count(attrib_list),
                                                                &error);

    if (!host_surface) {
        yagl_set_error(error);
        goto fail;
    }

    yagl_surface_init_window(&sfc->base,
                             &yagl_onscreen_surface_destroy,
                             host_surface,
                             dpy,
                             native_window);

    sfc->base.invalidate = &yagl_onscreen_surface_invalidate;
    sfc->base.swap_buffers = &yagl_onscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_onscreen_surface_copy_buffers;
    sfc->base.wait_x = &yagl_onscreen_surface_wait_x;
    sfc->base.wait_gl = &yagl_onscreen_surface_wait_gl;
    sfc->base.map = &yagl_onscreen_surface_map;
    sfc->base.unmap = &yagl_onscreen_surface_unmap;
    sfc->base.set_swap_interval = &yagl_onscreen_surface_set_swap_interval;
    sfc->base.create_image = &yagl_onscreen_surface_create_image;

    sfc->drm_sfc = drm_sfc;

    return sfc;

fail:
    if (drm_sfc) {
        vigs_drm_gem_unref(&drm_sfc->gem);
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
    EGLint error = 0;
    struct yagl_onscreen_surface *sfc;
    struct vigs_drm_surface *drm_sfc = NULL;
    yagl_host_handle host_surface = 0;

    sfc = yagl_malloc0(sizeof(*sfc));

    drm_sfc = yagl_onscreen_buffer_create(native_pixmap,
                                          yagl_native_attachment_front,
                                          NULL);

    if (!drm_sfc) {
        yagl_set_error(EGL_BAD_NATIVE_PIXMAP);
        goto fail;
    }

    host_surface = yagl_host_eglCreatePixmapSurfaceOnscreenYAGL(dpy->host_dpy,
                                                                host_config,
                                                                drm_sfc->id,
                                                                attrib_list,
                                                                yagl_transport_attrib_list_count(attrib_list),
                                                                &error);

    if (!host_surface) {
        yagl_set_error(error);
        goto fail;
    }

    yagl_surface_init_pixmap(&sfc->base,
                             &yagl_onscreen_surface_destroy,
                             host_surface,
                             dpy,
                             native_pixmap);

    sfc->base.invalidate = &yagl_onscreen_surface_invalidate;
    sfc->base.swap_buffers = &yagl_onscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_onscreen_surface_copy_buffers;
    sfc->base.wait_x = &yagl_onscreen_surface_wait_x;
    sfc->base.wait_gl = &yagl_onscreen_surface_wait_gl;
    sfc->base.map = &yagl_onscreen_surface_map;
    sfc->base.unmap = &yagl_onscreen_surface_unmap;
    sfc->base.set_swap_interval = &yagl_onscreen_surface_set_swap_interval;
    sfc->base.create_image = &yagl_onscreen_surface_create_image;

    sfc->drm_sfc = drm_sfc;

    return sfc;

fail:
    if (drm_sfc) {
        vigs_drm_gem_unref(&drm_sfc->gem);
    }
    yagl_free(sfc);

    return NULL;
}

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pbuffer(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          const EGLint* attrib_list)
{
    EGLint error = 0;
    struct yagl_onscreen_surface *sfc;
    uint32_t width = 0;
    uint32_t height = 0;
    int i = 0;
    struct vigs_drm_surface *drm_sfc = NULL;
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

    drm_sfc = yagl_onscreen_buffer_create(sfc->tmp_pixmap,
                                          yagl_native_attachment_front,
                                          NULL);

    if (!drm_sfc) {
        yagl_set_error(EGL_BAD_ALLOC);
        goto fail;
    }

    host_surface = yagl_host_eglCreatePbufferSurfaceOnscreenYAGL(dpy->host_dpy,
                                                                 host_config,
                                                                 drm_sfc->id,
                                                                 attrib_list,
                                                                 yagl_transport_attrib_list_count(attrib_list),
                                                                 &error);

    if (!host_surface) {
        yagl_set_error(error);
        goto fail;
    }

    yagl_surface_init_pbuffer(&sfc->base,
                              &yagl_onscreen_surface_destroy,
                              host_surface,
                              dpy);

    sfc->base.invalidate = &yagl_onscreen_surface_invalidate;
    sfc->base.swap_buffers = &yagl_onscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_onscreen_surface_copy_buffers;
    sfc->base.wait_x = &yagl_onscreen_surface_wait_x;
    sfc->base.wait_gl = &yagl_onscreen_surface_wait_gl;
    sfc->base.map = &yagl_onscreen_surface_map;
    sfc->base.unmap = &yagl_onscreen_surface_unmap;
    sfc->base.set_swap_interval = &yagl_onscreen_surface_set_swap_interval;
    sfc->base.create_image = &yagl_onscreen_surface_create_image;

    sfc->drm_sfc = drm_sfc;

    return sfc;

fail:
    if (sfc) {
        if (sfc->tmp_pixmap) {
            if (drm_sfc) {
                vigs_drm_gem_unref(&drm_sfc->gem);
            }
            sfc->tmp_pixmap->destroy(sfc->tmp_pixmap);
            sfc->tmp_pixmap = NULL;
        }
        yagl_free(sfc);
    }

    return NULL;
}
