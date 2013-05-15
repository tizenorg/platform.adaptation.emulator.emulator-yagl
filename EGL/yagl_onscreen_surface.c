#include "yagl_onscreen_surface.h"
#include "yagl_onscreen_display.h"
#include "yagl_host_egl_calls.h"
#include "yagl_egl_state.h"
#include "yagl_log.h"
#include "yagl_mem_egl.h"
#include "yagl_utils.h"
#include "yagl_malloc.h"
#include <assert.h>
#include <stdio.h>

static int yagl_onscreen_surface_reset(struct yagl_surface *sfc)
{
    return 1;
}

static void yagl_onscreen_surface_invalidate(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    struct yagl_onscreen_display *dpy = (struct yagl_onscreen_display*)sfc->dpy;
    yagl_DRI2Buffer *new_buffer;
    yagl_winsys_id new_id;
    uint32_t new_width;
    uint32_t new_height;

    if (osfc->last_stamp == osfc->stamp) {
        return;
    }

    assert(sfc->type == EGL_WINDOW_BIT);

    osfc->last_stamp = osfc->stamp;

    if (!yagl_onscreen_display_create_buffer(dpy,
                                             sfc->x_drawable.win,
                                             DRI2BufferBackLeft,
                                             &new_buffer,
                                             &new_id,
                                             &new_width,
                                             &new_height)) {
        return;
    }

    yagl_onscreen_display_destroy_buffer(osfc->buffer);
    osfc->buffer = new_buffer;
    osfc->width = new_width;
    osfc->height = new_height;

    YAGL_HOST_CALL_ASSERT(yagl_host_eglInvalidateOnscreenSurfaceYAGL(
        dpy->base.host_dpy, sfc->res.handle, new_id));
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

    x_gc = XCreateGC(sfc->dpy->x_dpy, target, 0, NULL);

    if (x_gc) {
        switch (sfc->type) {
        case EGL_PBUFFER_BIT:
            XCopyArea(sfc->dpy->x_dpy,
                      osfc->pbuffer_pixmap,
                      target,
                      x_gc,
                      0, 0, osfc->width, osfc->height,
                      0, 0);
            break;
        case EGL_PIXMAP_BIT:
            XCopyArea(sfc->dpy->x_dpy,
                      sfc->x_drawable.pixmap,
                      target,
                      x_gc,
                      0, 0, osfc->width, osfc->height,
                      0, 0);
            break;
        case EGL_WINDOW_BIT:
            XCopyArea(sfc->dpy->x_dpy,
                      sfc->x_drawable.win,
                      target,
                      x_gc,
                      0, 0, osfc->width, osfc->height,
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

    return 1;
}

static void yagl_onscreen_surface_destroy(struct yagl_ref *ref)
{
    struct yagl_onscreen_surface *sfc = (struct yagl_onscreen_surface*)ref;

    yagl_onscreen_display_destroy_buffer(sfc->buffer);
    switch (sfc->base.type) {
    case EGL_PBUFFER_BIT:
        yagl_DRI2DestroyDrawable(sfc->base.dpy->x_dpy,
                                 sfc->pbuffer_pixmap);
        XFreePixmap(sfc->base.dpy->x_dpy,
                    sfc->pbuffer_pixmap);
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
    yagl_DRI2Buffer *new_buffer = NULL;
    yagl_winsys_id new_id;
    uint32_t new_width;
    uint32_t new_height;
    yagl_host_handle host_surface = 0;

    YAGL_LOG_FUNC_SET(eglCreateWindowSurface);

    sfc = yagl_malloc0(sizeof(*sfc));

    yagl_DRI2CreateDrawable(dpy->x_dpy, x_win);

    if (!yagl_onscreen_display_create_buffer(odpy,
                                             x_win,
                                             DRI2BufferBackLeft,
                                             &new_buffer,
                                             &new_id,
                                             &new_width,
                                             &new_height)) {
        yagl_set_error(EGL_BAD_NATIVE_WINDOW);
        goto fail;
    }

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreateWindowSurfaceOnscreenYAGL(&host_surface,
        dpy->host_dpy,
        host_config,
        new_id,
        attrib_list));

    if (!host_surface) {
        goto fail;
    }

    yagl_surface_init_window(&sfc->base,
                             &yagl_onscreen_surface_destroy,
                             host_surface,
                             dpy,
                             x_win);

    sfc->base.reset = &yagl_onscreen_surface_reset;
    sfc->base.invalidate = &yagl_onscreen_surface_invalidate;
    sfc->base.swap_buffers = &yagl_onscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_onscreen_surface_copy_buffers;

    sfc->buffer = new_buffer;
    sfc->width = new_width;
    sfc->height = new_height;

    return sfc;

fail:
    if (new_buffer) {
        yagl_onscreen_display_destroy_buffer(new_buffer);
        yagl_DRI2DestroyDrawable(dpy->x_dpy, x_win);
    }
    yagl_free(sfc);

    return NULL;
}

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pixmap(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          Pixmap x_pixmap,
                                          const EGLint* attrib_list)
{
    struct yagl_onscreen_display *odpy = (struct yagl_onscreen_display*)dpy;
    struct yagl_onscreen_surface *sfc;
    yagl_DRI2Buffer *new_buffer = NULL;
    yagl_winsys_id new_id;
    uint32_t new_width;
    uint32_t new_height;
    yagl_host_handle host_surface = 0;

    YAGL_LOG_FUNC_SET(eglCreatePixmapSurface);

    sfc = yagl_malloc0(sizeof(*sfc));

    yagl_DRI2CreateDrawable(dpy->x_dpy, x_pixmap);

    if (!yagl_onscreen_display_create_buffer(odpy,
                                             x_pixmap,
                                             DRI2BufferFrontLeft,
                                             &new_buffer,
                                             &new_id,
                                             &new_width,
                                             &new_height)) {
        yagl_set_error(EGL_BAD_NATIVE_PIXMAP);
        goto fail;
    }

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreatePixmapSurfaceOnscreenYAGL(&host_surface,
        dpy->host_dpy,
        host_config,
        new_id,
        attrib_list));

    if (!host_surface) {
        goto fail;
    }

    yagl_surface_init_pixmap(&sfc->base,
                             &yagl_onscreen_surface_destroy,
                             host_surface,
                             dpy,
                             x_pixmap);

    sfc->base.reset = &yagl_onscreen_surface_reset;
    sfc->base.invalidate = &yagl_onscreen_surface_invalidate;
    sfc->base.swap_buffers = &yagl_onscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_onscreen_surface_copy_buffers;

    sfc->buffer = new_buffer;
    sfc->width = new_width;
    sfc->height = new_height;

    return sfc;

fail:
    if (new_buffer) {
        yagl_onscreen_display_destroy_buffer(new_buffer);
        yagl_DRI2DestroyDrawable(dpy->x_dpy, x_pixmap);
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
    yagl_DRI2Buffer *new_buffer = NULL;
    yagl_winsys_id new_id;
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

    sfc->pbuffer_pixmap = XCreatePixmap(dpy->x_dpy,
        RootWindow(dpy->x_dpy, DefaultScreen(dpy->x_dpy)),
        width, height, 24);

    if (!sfc->pbuffer_pixmap) {
        YAGL_LOG_ERROR("XCreatePixmap(%u,%u) failed", width, height);
        yagl_set_error(EGL_BAD_ALLOC);
        goto fail;
    }

    yagl_DRI2CreateDrawable(dpy->x_dpy, sfc->pbuffer_pixmap);

    if (!yagl_onscreen_display_create_buffer(odpy,
                                             sfc->pbuffer_pixmap,
                                             DRI2BufferFrontLeft,
                                             &new_buffer,
                                             &new_id,
                                             &width,
                                             &height)) {
        yagl_set_error(EGL_BAD_ALLOC);
        goto fail;
    }

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreatePbufferSurfaceOnscreenYAGL(&host_surface,
        dpy->host_dpy,
        host_config,
        new_id,
        attrib_list));

    if (!host_surface) {
        goto fail;
    }

    yagl_surface_init_pbuffer(&sfc->base,
                              &yagl_onscreen_surface_destroy,
                              host_surface,
                              dpy);

    sfc->base.reset = &yagl_onscreen_surface_reset;
    sfc->base.invalidate = &yagl_onscreen_surface_invalidate;
    sfc->base.swap_buffers = &yagl_onscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_onscreen_surface_copy_buffers;

    sfc->buffer = new_buffer;
    sfc->width = width;
    sfc->height = height;

    return sfc;

fail:
    if (sfc) {
        if (sfc->pbuffer_pixmap) {
            if (new_buffer) {
                yagl_onscreen_display_destroy_buffer(new_buffer);
                yagl_DRI2DestroyDrawable(dpy->x_dpy, sfc->pbuffer_pixmap);
            }
            XFreePixmap(dpy->x_dpy, sfc->pbuffer_pixmap);
        }
        yagl_free(sfc);
    }

    return NULL;
}
