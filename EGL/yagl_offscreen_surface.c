#include "yagl_offscreen_surface.h"
#include "yagl_host_egl_calls.h"
#include "yagl_egl_state.h"
#include "yagl_malloc.h"
#include "yagl_log.h"
#include "yagl_display.h"
#include "yagl_mem_egl.h"
#include "yagl_native_display.h"
#include "yagl_native_drawable.h"
#include "yagl_native_image.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>

static struct yagl_native_image
    *yagl_offscreen_surface_create_bi(struct yagl_native_display *dpy,
                                      uint32_t width,
                                      uint32_t height,
                                      uint32_t depth)
{
    struct yagl_native_image *bi = dpy->create_image(dpy,
                                                     width,
                                                     height,
                                                     depth);

    if (!bi) {
        return NULL;
    }

    if (mlock(bi->pixels, bi->width * bi->height * bi->bpp) == -1) {
        fprintf(stderr, "Critical error! Unable to lock YaGL bi memory: %s!\n", strerror(errno));
        exit(1);
    }

    /*
     * Probe in immediately.
     */

    yagl_mem_probe_write(bi->pixels, bi->width * bi->height * bi->bpp);

    return bi;
}

static void yagl_offscreen_surface_destroy_bi(struct yagl_native_image *bi)
{
    if (munlock(bi->pixels, bi->width * bi->height * bi->bpp) == -1) {
        fprintf(stderr, "Critical error! Unable to unlock YaGL bi memory: %s!\n", strerror(errno));
        exit(1);
    }

    bi->destroy(bi);
}

static int yagl_offscreen_surface_resize(struct yagl_offscreen_surface *surface)
{
    int res = 0;
    EGLBoolean retval = EGL_FALSE;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
    struct yagl_native_image *bi = NULL;

    YAGL_LOG_FUNC_SET(yagl_offscreen_surface_resize);

    if (surface->base.type != EGL_WINDOW_BIT) {
        return 1;
    }

    surface->base.native_drawable->get_geometry(surface->base.native_drawable,
                                                &width,
                                                &height,
                                                &depth);

    if ((width != surface->bi->width) ||
        (height != surface->bi->height) ||
        (depth != surface->bi->depth)) {
        YAGL_LOG_DEBUG("Surface resizing from %ux%ux%u to %ux%ux%u",
                       surface->bi->width, surface->bi->height,
                       surface->bi->depth,
                       width, height, depth);
        /*
         * First of all, create new backing image.
         */

        bi = yagl_offscreen_surface_create_bi(surface->base.dpy->native_dpy,
                                              width, height, depth);

        if (!bi) {
            YAGL_LOG_ERROR("create_bi failed");
            yagl_set_error(EGL_BAD_ALLOC);
            goto out;
        }

        /*
         * Tell the host that it should use new backing image from now on.
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

        yagl_offscreen_surface_destroy_bi(surface->bi);
        surface->bi = bi;
        bi = NULL;
    }

    res = 1;

out:
    if (bi) {
        yagl_offscreen_surface_destroy_bi(bi);
    }

    return res;
}

static void yagl_offscreen_surface_invalidate(struct yagl_surface *sfc)
{
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

    if (sfc->native_drawable) {
        osfc->bi->draw(osfc->bi, sfc->native_drawable);
    }

    return 1;
}

static int yagl_offscreen_surface_copy_buffers(struct yagl_surface *sfc,
                                               yagl_os_pixmap target)
{
    struct yagl_offscreen_surface *osfc = (struct yagl_offscreen_surface*)sfc;
    EGLBoolean retval = EGL_FALSE;

    YAGL_LOG_FUNC_SET(eglCopyBuffers);

    if (sfc->type == EGL_WINDOW_BIT) {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 0;

        sfc->native_drawable->get_geometry(sfc->native_drawable,
                                           &width,
                                           &height,
                                           &depth);

        if ((width != osfc->bi->width) ||
            (height != osfc->bi->height) ||
            (depth != osfc->bi->depth)) {
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
                                                   sfc->res.handle));

    if (!retval) {
        YAGL_LOG_ERROR("eglCopyBuffers failed");
        return 0;
    }

    /*
     * Host has updated our image, update the target.
     */

    osfc->bi->draw_to_pixmap(osfc->bi, target);

    return 1;
}

static void yagl_offscreen_surface_wait_x(struct yagl_surface *sfc)
{
}

static void yagl_offscreen_surface_wait_gl(struct yagl_surface *sfc)
{
}

static void yagl_offscreen_surface_map(struct yagl_surface *sfc)
{
    struct yagl_offscreen_surface *osfc = (struct yagl_offscreen_surface*)sfc;

    sfc->lock_ptr = osfc->bi->pixels;
    sfc->lock_stride = osfc->bi->width * osfc->bi->bpp;
}

static void yagl_offscreen_surface_unmap(struct yagl_surface *sfc)
{
    struct yagl_offscreen_surface *osfc = (struct yagl_offscreen_surface*)sfc;

    switch (sfc->type) {
    case EGL_PBUFFER_BIT:
        break;
    case EGL_WINDOW_BIT:
    case EGL_PIXMAP_BIT:
        osfc->bi->draw(osfc->bi, sfc->native_drawable);
        break;
    default:
        assert(0);
        break;
    }
}

static void yagl_offscreen_surface_set_swap_interval(struct yagl_surface *sfc,
                                                     int interval)
{
}

static void yagl_offscreen_surface_destroy(struct yagl_ref *ref)
{
    struct yagl_offscreen_surface *sfc = (struct yagl_offscreen_surface*)ref;

    yagl_offscreen_surface_destroy_bi(sfc->bi);
    sfc->bi = NULL;

    yagl_surface_cleanup(&sfc->base);

    yagl_free(sfc);
}

struct yagl_offscreen_surface
    *yagl_offscreen_surface_create_window(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          struct yagl_native_drawable *native_window,
                                          const EGLint* attrib_list)
{
    struct yagl_offscreen_surface *sfc;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
    yagl_host_handle host_surface = 0;
    struct yagl_native_image *bi = NULL;

    YAGL_LOG_FUNC_SET(eglCreateWindowSurface);

    sfc = yagl_malloc0(sizeof(*sfc));

    native_window->get_geometry(native_window,
                                &width,
                                &height,
                                &depth);

    bi = yagl_offscreen_surface_create_bi(dpy->native_dpy, width, height, depth);

    if (!bi) {
        YAGL_LOG_ERROR("create_bi failed");
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
                             native_window);

    sfc->base.invalidate = &yagl_offscreen_surface_invalidate;
    sfc->base.swap_buffers = &yagl_offscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_offscreen_surface_copy_buffers;
    sfc->base.wait_x = &yagl_offscreen_surface_wait_x;
    sfc->base.wait_gl = &yagl_offscreen_surface_wait_gl;
    sfc->base.map = &yagl_offscreen_surface_map;
    sfc->base.unmap = &yagl_offscreen_surface_unmap;
    sfc->base.set_swap_interval = &yagl_offscreen_surface_set_swap_interval;

    sfc->bi = bi;

    return sfc;

fail:
    if (bi) {
        yagl_offscreen_surface_destroy_bi(bi);
        bi = NULL;
    }

    yagl_free(sfc);

    return NULL;
}

struct yagl_offscreen_surface
    *yagl_offscreen_surface_create_pixmap(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          struct yagl_native_drawable *native_pixmap,
                                          const EGLint* attrib_list)
{
    struct yagl_offscreen_surface *sfc;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
    yagl_host_handle host_surface = 0;
    struct yagl_native_image *bi = NULL;

    YAGL_LOG_FUNC_SET(eglCreatePixmapSurface);

    sfc = yagl_malloc0(sizeof(*sfc));

    native_pixmap->get_geometry(native_pixmap,
                                &width,
                                &height,
                                &depth);

    bi = yagl_offscreen_surface_create_bi(dpy->native_dpy, width, height, depth);

    if (!bi) {
        YAGL_LOG_ERROR("create_bi failed");
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
                             native_pixmap);

    sfc->base.invalidate = &yagl_offscreen_surface_invalidate;
    sfc->base.swap_buffers = &yagl_offscreen_surface_swap_buffers;
    sfc->base.copy_buffers = &yagl_offscreen_surface_copy_buffers;
    sfc->base.wait_x = &yagl_offscreen_surface_wait_x;
    sfc->base.wait_gl = &yagl_offscreen_surface_wait_gl;
    sfc->base.map = &yagl_offscreen_surface_map;
    sfc->base.unmap = &yagl_offscreen_surface_unmap;
    sfc->base.set_swap_interval = &yagl_offscreen_surface_set_swap_interval;

    sfc->bi = bi;

    return sfc;

fail:
    if (bi) {
        yagl_offscreen_surface_destroy_bi(bi);
        bi = NULL;
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
    struct yagl_native_image *bi = NULL;
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

    bi = yagl_offscreen_surface_create_bi(dpy->native_dpy, width, height, 24);

    if (!bi) {
        YAGL_LOG_ERROR("create_bi failed");
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
    sfc->base.wait_x = &yagl_offscreen_surface_wait_x;
    sfc->base.wait_gl = &yagl_offscreen_surface_wait_gl;
    sfc->base.map = &yagl_offscreen_surface_map;
    sfc->base.unmap = &yagl_offscreen_surface_unmap;
    sfc->base.set_swap_interval = &yagl_offscreen_surface_set_swap_interval;

    sfc->bi = bi;

    return sfc;

fail:
    if (bi) {
        yagl_offscreen_surface_destroy_bi(bi);
        bi = NULL;
    }

    yagl_free(sfc);

    return NULL;
}
