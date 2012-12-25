#include "yagl_onscreen_surface.h"
#include "yagl_host_egl_calls.h"
#include "yagl_egl_state.h"
#include "yagl_log.h"
#include "yagl_display.h"
#include "yagl_mem_egl.h"
#include "yagl_utils.h"
#include "yagl_malloc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/extensions/Xdamage.h>

static int yagl_onscreen_surface_invalidate(struct yagl_surface *sfc)
{
    return 1;
}

static int yagl_onscreen_surface_swap_buffers(struct yagl_surface *sfc)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    EGLBoolean retval = EGL_FALSE;
    XWindowAttributes x_wa;

    YAGL_LOG_FUNC_SET(eglSwapBuffers);

    if (!XGetWindowAttributes(sfc->dpy->x_dpy, sfc->x_drawable.win, &x_wa)) {
        YAGL_LOG_ERROR("XGetWindowAttributes failed");
        return 0;
    }

    if (x_wa.map_state != IsViewable) {
        return 1;
    }

    YAGL_HOST_CALL_ASSERT(yagl_host_eglSwapBuffers(&retval,
                                                   sfc->dpy->host_dpy,
                                                   sfc->res.handle));

    if (!retval) {
        YAGL_LOG_ERROR("eglSwapBuffers failed");
        return 0;
    }

    if ((x_wa.width != osfc->last_width) ||
        (x_wa.height != osfc->last_height) ||
        (osfc->x_region == 0)) {
        XRectangle x_rect;

        if (osfc->x_region != 0) {
            XFixesDestroyRegion(sfc->dpy->x_dpy, osfc->x_region);
        }

        x_rect.x = 0;
        x_rect.y = 0;
        x_rect.width = x_wa.width;
        x_rect.height = x_wa.height;

        osfc->x_region = XFixesCreateRegion(sfc->dpy->x_dpy, &x_rect, 1);
        osfc->last_width = x_wa.width;
        osfc->last_height = x_wa.height;
    }

    XDamageAdd(sfc->dpy->x_dpy, sfc->x_drawable.win, osfc->x_region);

    return 1;
}

static int yagl_onscreen_surface_copy_buffers(struct yagl_surface *sfc,
                                              Pixmap target)
{
    struct yagl_onscreen_surface *osfc = (struct yagl_onscreen_surface*)sfc;
    EGLBoolean retval = EGL_FALSE;

    YAGL_LOG_FUNC_SET(eglCopyBuffers);

    YAGL_HOST_CALL_ASSERT(yagl_host_eglCopyBuffers(&retval,
                                                   sfc->dpy->host_dpy,
                                                   sfc->res.handle,
                                                   target));

    if (!retval) {
        YAGL_LOG_ERROR("eglCopyBuffers failed");
        return 0;
    }

    return 1;
}

static void yagl_onscreen_surface_destroy(struct yagl_ref *ref)
{
    struct yagl_onscreen_surface *sfc = (struct yagl_onscreen_surface*)ref;

    if (sfc->x_region != 0) {
        XFixesDestroyRegion(sfc->base.dpy->x_dpy, sfc->x_region);
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
    struct yagl_onscreen_surface *sfc;
    yagl_host_handle host_surface = 0;

    YAGL_LOG_FUNC_SET(eglCreateWindowSurface);

    sfc = yagl_malloc0(sizeof(*sfc));

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreateWindowSurfaceOnscreenYAGL(&host_surface,
        dpy->host_dpy,
        host_config,
        x_win,
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
    sfc->base.swap_buffers = &yagl_onscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_onscreen_surface_copy_buffers;

    return sfc;

fail:
    yagl_free(sfc);

    return NULL;
}

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pixmap(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          Pixmap x_pixmap,
                                          const EGLint* attrib_list)
{
    struct yagl_onscreen_surface *sfc;
    yagl_host_handle host_surface = 0;

    YAGL_LOG_FUNC_SET(eglCreatePixmapSurface);

    sfc = yagl_malloc0(sizeof(*sfc));

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreatePixmapSurfaceOnscreenYAGL(&host_surface,
        dpy->host_dpy,
        host_config,
        x_pixmap,
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
    sfc->base.swap_buffers = &yagl_onscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_onscreen_surface_copy_buffers;

    return sfc;

fail:
    yagl_free(sfc);

    return NULL;
}

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pbuffer(struct yagl_display *dpy,
                                           yagl_host_handle host_config,
                                           const EGLint* attrib_list)
{
    struct yagl_onscreen_surface *sfc;
    yagl_host_handle host_surface = 0;

    YAGL_LOG_FUNC_SET(eglCreatePbufferSurface);

    sfc = yagl_malloc0(sizeof(*sfc));

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreatePbufferSurfaceOnscreenYAGL(&host_surface,
        dpy->host_dpy,
        host_config,
        attrib_list));

    if (!host_surface) {
        goto fail;
    }

    yagl_surface_init_pbuffer(&sfc->base,
                              &yagl_onscreen_surface_destroy,
                              host_surface,
                              dpy);

    sfc->base.invalidate = &yagl_onscreen_surface_invalidate;
    sfc->base.swap_buffers = &yagl_onscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_onscreen_surface_copy_buffers;

    return sfc;

fail:
    yagl_free(sfc);

    return NULL;
}
