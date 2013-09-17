#include "yagl_host_egl_calls.h"
#include "yagl_impl.h"
#include "yagl_display.h"
#include "yagl_log.h"
#include "yagl_egl_state.h"
#include "yagl_malloc.h"
#include "yagl_surface.h"
#include "yagl_context.h"
#include "yagl_image.h"
#include "yagl_backend.h"
#include "yagl_render.h"
#include "yagl_native_platform.h"
#include "yagl_native_display.h"
#include "yagl_native_drawable.h"
#include "yagl_transport_egl.h"
#include "EGL/eglext.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <dlfcn.h>
#include <stdlib.h>

#define YAGL_SET_ERR(err) \
    yagl_set_error(err); \
    YAGL_LOG_ERROR("error = 0x%X", err)

#define YAGL_UNIMPLEMENTED(func, ret) \
    YAGL_LOG_FUNC_SET(func); \
    YAGL_LOG_WARN("NOT IMPLEMENTED!!!"); \
    return ret;

static __inline int yagl_validate_display(EGLDisplay dpy_,
                                          struct yagl_display **dpy)
{
    YAGL_LOG_FUNC_SET(yagl_validate_display);

    *dpy = yagl_display_get(dpy_);

    if (!*dpy) {
        YAGL_SET_ERR(EGL_BAD_DISPLAY);
        return 0;
    }

    if (!yagl_display_is_prepared(*dpy)) {
        YAGL_SET_ERR(EGL_NOT_INITIALIZED);
        return 0;
    }

    return 1;
}

static __inline int yagl_validate_surface(struct yagl_display *dpy,
                                          EGLSurface sfc_,
                                          struct yagl_surface **sfc)
{
    YAGL_LOG_FUNC_SET(yagl_validate_surface);

    *sfc = yagl_display_surface_acquire(dpy, sfc_);

    if (!*sfc) {
        YAGL_SET_ERR(EGL_BAD_SURFACE);
        return 0;
    }

    return 1;
}

static __inline int yagl_validate_image(struct yagl_display *dpy,
                                        EGLImageKHR image_,
                                        struct yagl_image **image)
{
    YAGL_LOG_FUNC_SET(yagl_validate_image);

    *image = yagl_display_image_acquire(dpy, image_);

    if (!*image) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        return 0;
    }

    return 1;
}

YAGL_API EGLint eglGetError()
{
    EGLint retval;

    YAGL_LOG_FUNC_ENTER(eglGetError, NULL);

    retval = yagl_get_error();

    if (retval == EGL_SUCCESS) {
        retval = yagl_host_eglGetError();
    }

    YAGL_LOG_FUNC_EXIT_SPLIT(EGLint, retval);

    return retval;
}

YAGL_API EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id)
{
    struct yagl_native_platform *platform;
    yagl_host_handle host_dpy;
    struct yagl_display *dpy;
    EGLDisplay ret = EGL_NO_DISPLAY;

    YAGL_LOG_FUNC_ENTER_SPLIT1(eglGetDisplay, EGLNativeDisplayType, display_id);

    platform = yagl_guess_platform((yagl_os_display)display_id);

    if (!platform) {
        goto out;
    }

    host_dpy = yagl_host_eglGetDisplay((uint32_t)display_id);

    if (!host_dpy) {
        YAGL_LOG_ERROR("unable to get host display for %p",
                       (yagl_os_display)display_id);
        goto out;
    }

    dpy = yagl_display_add(platform, (yagl_os_display)display_id, host_dpy);

    if (!dpy) {
        YAGL_SET_ERR(EGL_BAD_DISPLAY);
        YAGL_LOG_ERROR("unable to add display %p, %u",
                       (yagl_os_display)display_id, host_dpy);
        goto out;
    }

    ret = (EGLDisplay)dpy->host_dpy;

    YAGL_LOG_FUNC_EXIT_SPLIT(yagl_host_handle, dpy->host_dpy);

    return ret;

out:
    YAGL_LOG_FUNC_EXIT(NULL);

    return ret;
}

YAGL_API EGLBoolean eglInitialize(EGLDisplay dpy_, EGLint* major, EGLint* minor)
{
    struct yagl_display *dpy;

    YAGL_LOG_FUNC_ENTER(eglInitialize, "dpy = %u", (yagl_host_handle)dpy_);

    dpy = yagl_display_get(dpy_);

    if (!dpy) {
        YAGL_SET_ERR(EGL_BAD_DISPLAY);
        goto fail;
    }

    if (!yagl_host_eglInitialize(dpy->host_dpy, major, minor)) {
        goto fail;
    }

    yagl_display_prepare(dpy);

    YAGL_LOG_FUNC_EXIT("major = %d, minor = %d",
                       (major ? *major : -1),
                       (minor ? *minor : -1));

    return EGL_TRUE;

fail:
    YAGL_LOG_FUNC_EXIT(NULL);

    return EGL_FALSE;
}

YAGL_API EGLBoolean eglTerminate(EGLDisplay dpy_)
{
    struct yagl_display *dpy;
    EGLBoolean ret = EGL_FALSE;

    YAGL_LOG_FUNC_ENTER(eglTerminate, "dpy = %u", (yagl_host_handle)dpy_);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    ret = yagl_host_eglTerminate(dpy->host_dpy);

    if (ret) {
        yagl_display_terminate(dpy);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);

    return ret;
}

YAGL_API const char *eglQueryString(EGLDisplay dpy_, EGLint name)
{
    const char *str = NULL;

    YAGL_LOG_FUNC_ENTER(eglQueryString,
                        "dpy = %u, name = %d",
                        (yagl_host_handle)dpy_,
                        name);

    switch (name) {
    case EGL_VENDOR:
        str = "Samsung";
        break;
    case EGL_VERSION:
        str = "1.4";
        break;
    case EGL_CLIENT_APIS:
        str = "OpenGL_ES OpenGL_ES2";
        break;
    case EGL_EXTENSIONS:
        str = yagl_display_get_extensions(yagl_display_get(dpy_));
        break;
    default:
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
    }

    YAGL_LOG_FUNC_EXIT(NULL);

    return str;
}

YAGL_API EGLBoolean eglGetConfigs(EGLDisplay dpy,
                                  EGLConfig *configs,
                                  EGLint config_size,
                                  EGLint *num_config)
{
    YAGL_LOG_FUNC_ENTER(eglGetConfigs,
                        "dpy = %u, configs = %p, config_size = %d",
                        (yagl_host_handle)dpy,
                        configs,
                        config_size);

    if (!num_config) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto fail;
    }

    if (!yagl_host_eglGetConfigs((yagl_host_handle)dpy,
                                 (yagl_host_handle*)configs,
                                 config_size,
                                 num_config)) {
        goto fail;
    }

    YAGL_LOG_FUNC_EXIT("num_config = %d", *num_config);

    return EGL_TRUE;

fail:
    YAGL_LOG_FUNC_EXIT(NULL);

    return EGL_FALSE;
}

YAGL_API EGLBoolean eglChooseConfig(EGLDisplay dpy,
                                    const EGLint *attrib_list,
                                    EGLConfig *configs,
                                    EGLint config_size,
                                    EGLint *num_config)
{
    YAGL_LOG_FUNC_ENTER(eglChooseConfig, "dpy = %u", (yagl_host_handle)dpy);

    if (!num_config) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto fail;
    }

    if (!yagl_host_eglChooseConfig((yagl_host_handle)dpy,
                                   attrib_list,
                                   yagl_transport_attrib_list_count(attrib_list),
                                   (yagl_host_handle*)configs,
                                   config_size,
                                   num_config)) {
        goto fail;
    }

    YAGL_LOG_FUNC_EXIT("num_config = %d", *num_config);

    return EGL_TRUE;

fail:
    YAGL_LOG_FUNC_EXIT(NULL);

    return EGL_FALSE;
}

YAGL_API EGLBoolean eglGetConfigAttrib(EGLDisplay dpy_,
                                       EGLConfig config,
                                       EGLint attribute,
                                       EGLint *value)
{
    struct yagl_display *dpy = NULL;
    EGLBoolean ret = EGL_FALSE;
    int visual_id = 0, visual_type = 0;

    YAGL_LOG_FUNC_ENTER(eglGetConfigAttrib,
                        "dpy = %u, config = %u, attribute = 0x%X",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)config,
                        attribute);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto fail;
    }

    switch (attribute) {
    case EGL_NATIVE_VISUAL_ID:
    case EGL_NATIVE_VISUAL_TYPE:
        if (!dpy->native_dpy->get_visual(dpy->native_dpy,
                                         &visual_id,
                                         &visual_type)) {
            YAGL_SET_ERR(EGL_BAD_CONFIG);
            YAGL_LOG_ERROR("get_visual failed");
            goto fail;
        }

        if (attribute == EGL_NATIVE_VISUAL_ID) {
            *value = visual_id;
        } else {
            *value = visual_type;
        }

        ret = EGL_TRUE;

        break;
    case EGL_Y_INVERTED_NOK:
        if (dpy->native_dpy->platform->pixmaps_supported) {
            *value = yagl_get_backend()->y_inverted;
            ret = EGL_TRUE;
            break;
        }
        /* Fall through. */
    default:
        ret = yagl_host_eglGetConfigAttrib((yagl_host_handle)dpy_,
                                           (yagl_host_handle)config,
                                           attribute,
                                           value);

        break;
    }

    if (!ret) {
        goto fail;
    }

    YAGL_LOG_FUNC_EXIT("value = 0x%X", (value ? *value : -1));

    return ret;

fail:
    YAGL_LOG_FUNC_EXIT(NULL);

    return ret;
}

YAGL_API EGLSurface eglCreateWindowSurface(EGLDisplay dpy_,
                                           EGLConfig config,
                                           EGLNativeWindowType win,
                                           const EGLint *attrib_list)
{
    struct yagl_display *dpy = NULL;
    struct yagl_native_drawable *native_win = NULL;
    struct yagl_surface *surface = NULL;

    YAGL_LOG_FUNC_ENTER(eglCreateWindowSurface,
                        "dpy = %u, config = %u",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)config);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto fail;
    }

    native_win = dpy->native_dpy->wrap_window(dpy->native_dpy,
                                              (yagl_os_window)win);

    if (!native_win) {
        goto fail;
    }

    surface = yagl_get_backend()->create_window_surface(dpy,
                                                        (yagl_host_handle)config,
                                                        native_win,
                                                        attrib_list);

    if (!surface) {
        native_win->destroy(native_win);
        native_win = NULL;
        goto fail;
    }

    if (!yagl_display_surface_add(dpy, surface)) {
        yagl_host_eglDestroySurface(dpy->host_dpy, surface->res.handle);
        YAGL_SET_ERR(EGL_BAD_ALLOC);
        goto fail;
    }

    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%p", yagl_surface_get_handle(surface));

    return yagl_surface_get_handle(surface);

fail:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT(NULL);

    return EGL_NO_SURFACE;
}

YAGL_API EGLSurface eglCreatePbufferSurface(EGLDisplay dpy_,
                                            EGLConfig config,
                                            const EGLint *attrib_list)
{
    struct yagl_display *dpy = NULL;
    struct yagl_surface *surface = NULL;

    YAGL_LOG_FUNC_ENTER(eglCreatePbufferSurface,
                        "dpy = %u, config = %u",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)config);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto fail;
    }

    surface = yagl_get_backend()->create_pbuffer_surface(dpy,
                                                         (yagl_host_handle)config,
                                                         attrib_list);

    if (!surface) {
        goto fail;
    }

    if (!yagl_display_surface_add(dpy, surface)) {
        yagl_host_eglDestroySurface(dpy->host_dpy, surface->res.handle);
        YAGL_SET_ERR(EGL_BAD_ALLOC);
        goto fail;
    }

    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%p", yagl_surface_get_handle(surface));

    return yagl_surface_get_handle(surface);

fail:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT(NULL);

    return EGL_NO_SURFACE;
}

YAGL_API EGLSurface eglCreatePixmapSurface(EGLDisplay dpy_,
                                           EGLConfig config,
                                           EGLNativePixmapType pixmap,
                                           const EGLint *attrib_list)
{
    struct yagl_display *dpy = NULL;
    struct yagl_native_drawable *native_pixmap = NULL;
    struct yagl_surface *surface = NULL;

    YAGL_LOG_FUNC_ENTER(eglCreatePixmapSurface,
                        "dpy = %u, config = %u",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)config);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto fail;
    }

    native_pixmap = dpy->native_dpy->wrap_pixmap(dpy->native_dpy,
                                                 (yagl_os_pixmap)pixmap);

    if (!native_pixmap) {
        goto fail;
    }

    surface = yagl_get_backend()->create_pixmap_surface(dpy,
                                                        (yagl_host_handle)config,
                                                        native_pixmap,
                                                        attrib_list);

    if (!surface) {
        native_pixmap->destroy(native_pixmap);
        native_pixmap = NULL;
        goto fail;
    }

    if (!yagl_display_surface_add(dpy, surface)) {
        yagl_host_eglDestroySurface(dpy->host_dpy, surface->res.handle);
        YAGL_SET_ERR(EGL_BAD_ALLOC);
        goto fail;
    }

    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%p", yagl_surface_get_handle(surface));

    return yagl_surface_get_handle(surface);

fail:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT(NULL);

    return EGL_NO_SURFACE;
}

YAGL_API EGLBoolean eglDestroySurface(EGLDisplay dpy_, EGLSurface surface_)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_surface *surface = NULL;

    YAGL_LOG_FUNC_ENTER(eglDestroySurface,
                        "dpy = %u, surface = %p",
                        (yagl_host_handle)dpy_,
                        surface_);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_surface(dpy, surface_, &surface)) {
        goto out;
    }

    if (!yagl_host_eglDestroySurface(dpy->host_dpy, surface->res.handle)) {
        goto out;
    }

    if (!yagl_display_surface_remove(dpy, surface_)) {
        YAGL_LOG_ERROR("we're the one who destroy the surface, but surface isn't there!");
        YAGL_SET_ERR(EGL_BAD_SURFACE);
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", ((res == EGL_TRUE) ? 1 : 0));

    return res;
}

YAGL_API EGLBoolean eglQuerySurface(EGLDisplay dpy_,
                                    EGLSurface surface_,
                                    EGLint attribute,
                                    EGLint *value)
{
    EGLBoolean retval = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_surface *surface = NULL;
    void *ptr;
    uint32_t stride;

    YAGL_LOG_FUNC_ENTER(eglQuerySurface,
                        "dpy = %u, surface = %p, attribute = 0x%X, value = %p",
                        (yagl_host_handle)dpy_,
                        surface_,
                        attribute,
                        value);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_surface(dpy, surface_, &surface)) {
        goto out;
    }

    switch (attribute) {
    case EGL_BITMAP_POINTER_KHR:
        ptr = yagl_surface_map(surface, &stride);
        if (ptr) {
            *value = (EGLint)ptr;
            retval = EGL_TRUE;
        } else {
            YAGL_SET_ERR(EGL_BAD_ACCESS);
        }
        break;
    case EGL_BITMAP_PITCH_KHR:
        ptr = yagl_surface_map(surface, &stride);
        if (ptr) {
            *value = stride;
            retval = EGL_TRUE;
        } else {
            YAGL_SET_ERR(EGL_BAD_ACCESS);
        }
        break;
    case EGL_BITMAP_ORIGIN_KHR:
        *value = EGL_UPPER_LEFT_KHR;
        retval = EGL_TRUE;
        break;
    case EGL_BITMAP_PIXEL_RED_OFFSET_KHR:
        *value = 16;
        retval = EGL_TRUE;
        break;
    case EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR:
        *value = 8;
        retval = EGL_TRUE;
        break;
    case EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR:
        *value = 0;
        retval = EGL_TRUE;
        break;
    case EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR:
        *value = 24;
        retval = EGL_TRUE;
        break;
    case EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR:
        *value = 0;
        retval = EGL_TRUE;
        break;
    case EGL_BUFFER_AGE_EXT:
        if (dpy->native_dpy->platform->buffer_age_supported &&
            surface->native_drawable) {
            *value = surface->native_drawable->get_buffer_age(surface->native_drawable);
            retval = EGL_TRUE;
            break;
        }
        /* Fall through. */
    default:
        retval = yagl_host_eglQuerySurface(dpy->host_dpy,
                                           surface->res.handle,
                                           attribute,
                                           value);
        break;
    }

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", ((retval == EGL_TRUE) ? 1 : 0));

    return retval;
}

YAGL_API EGLBoolean eglBindAPI(EGLenum api)
{
    EGLBoolean ret;

    YAGL_LOG_FUNC_ENTER_SPLIT1(eglBindAPI, EGLenum, api);

    ret = yagl_host_eglBindAPI(api);

    if (ret) {
        yagl_set_api(api);
    }

    YAGL_LOG_FUNC_EXIT_SPLIT(EGLBoolean, ret);

    return ret;
}

YAGL_API EGLenum eglQueryAPI()
{
    EGLenum ret;

    YAGL_LOG_FUNC_ENTER_SPLIT0(eglQueryAPI);

    ret = yagl_get_api();

    YAGL_LOG_FUNC_EXIT_SPLIT(EGLenum, ret);

    return ret;
}

YAGL_API EGLBoolean eglWaitClient()
{
    struct yagl_surface *draw_sfc;

    YAGL_LOG_FUNC_ENTER_SPLIT0(eglWaitClient);

    draw_sfc = yagl_get_draw_surface();

    if (draw_sfc) {
        draw_sfc->wait_gl(draw_sfc);
    }

    YAGL_LOG_FUNC_EXIT(NULL);

    return EGL_TRUE;
}

YAGL_API EGLBoolean eglReleaseThread()
{
    YAGL_LOG_FUNC_ENTER(eglReleaseThread, NULL);

    if (!yagl_host_eglReleaseThread()) {
        goto fail;
    }

    yagl_reset_state();

    YAGL_LOG_FUNC_EXIT("1");

    return EGL_TRUE;

fail:
    YAGL_LOG_FUNC_EXIT(NULL);

    return EGL_FALSE;
}

YAGL_API EGLSurface eglCreatePbufferFromClientBuffer(EGLDisplay dpy,
                                                     EGLenum buftype,
                                                     EGLClientBuffer buffer,
                                                     EGLConfig config,
                                                     const EGLint *attrib_list)
{
    YAGL_UNIMPLEMENTED(eglCreatePbufferFromClientBuffer, EGL_NO_SURFACE);
}

YAGL_API EGLBoolean eglSurfaceAttrib(EGLDisplay dpy_,
                                     EGLSurface surface_,
                                     EGLint attribute,
                                     EGLint value)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_surface *surface = NULL;

    YAGL_LOG_FUNC_ENTER(eglSurfaceAttrib,
                        "dpy = %u, surface = %p, attribute = 0x%X, value = 0x%X",
                        (yagl_host_handle)dpy_,
                        surface_,
                        attribute,
                        value);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_surface(dpy, surface_, &surface)) {
        goto out;
    }

    if (!yagl_host_eglSurfaceAttrib(dpy->host_dpy,
                                    surface->res.handle,
                                    attribute,
                                    value)) {
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", ((res == EGL_TRUE) ? 1 : 0));

    return res;
}

YAGL_API EGLBoolean eglBindTexImage(EGLDisplay dpy_,
                                    EGLSurface surface_,
                                    EGLint buffer)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_surface *surface = NULL;

    YAGL_LOG_FUNC_ENTER(eglBindTexImage,
                        "dpy = %u, surface = %p, buffer = 0x%X",
                        (yagl_host_handle)dpy_,
                        surface_,
                        buffer);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_surface(dpy, surface_, &surface)) {
        goto out;
    }

    if (!yagl_host_eglBindTexImage(dpy->host_dpy,
                                   surface->res.handle,
                                   buffer)) {
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", ((res == EGL_TRUE) ? 1 : 0));

    return res;
}

YAGL_API EGLBoolean eglReleaseTexImage(EGLDisplay dpy_,
                                       EGLSurface surface_,
                                       EGLint buffer)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_surface *surface = NULL;

    YAGL_LOG_FUNC_ENTER(eglReleaseTexImage,
                        "dpy = %u, surface = %p, buffer = 0x%X",
                        (yagl_host_handle)dpy_,
                        surface_,
                        buffer);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_surface(dpy, surface_, &surface)) {
        goto out;
    }

    if (!yagl_host_eglReleaseTexImage(dpy->host_dpy,
                                      surface->res.handle,
                                      buffer)) {
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", ((res == EGL_TRUE) ? 1 : 0));

    return res;
}

YAGL_API EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
    struct yagl_surface *draw_sfc;

    YAGL_LOG_FUNC_ENTER(eglSwapInterval,
                        "dpy = %u, interval = %d",
                        (yagl_host_handle)dpy,
                        interval);

    draw_sfc = yagl_get_draw_surface();

    if (draw_sfc && (draw_sfc->type == EGL_WINDOW_BIT)) {
        if (interval <= 0) {
            /*
             * Always make sure that swap interval is at least 1. Setting
             * to 0 makes little sense with tizen since tizen compositor
             * will still use a value of 1 and this will cause jagged rendering
             */
            interval = 1;
        }

        draw_sfc->set_swap_interval(draw_sfc, interval);
    }

    YAGL_LOG_FUNC_EXIT("1");

    return EGL_TRUE;
}

YAGL_API EGLContext eglCreateContext(EGLDisplay dpy_,
                                     EGLConfig config,
                                     EGLContext share_context,
                                     const EGLint *attrib_list)
{
    struct yagl_display *dpy = NULL;
    yagl_host_handle host_context = 0;
    struct yagl_context *ctx = NULL;
    int i = 0;
    EGLint version = 1;

    YAGL_LOG_FUNC_ENTER(eglCreateContext,
                        "dpy = %u, config = %u, share_context = %u",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)config,
                        (yagl_host_handle)share_context);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto fail;
    }

    if (attrib_list) {
        while (attrib_list[i] != EGL_NONE) {
            switch (attrib_list[i]) {
            case EGL_CONTEXT_CLIENT_VERSION:
                version = attrib_list[i + 1];
                break;
            default:
                break;
            }

            i += 2;
        }
    }

    host_context =
        yagl_host_eglCreateContext(dpy->host_dpy,
                                   (yagl_host_handle)config,
                                   (yagl_host_handle)share_context,
                                   attrib_list,
                                   yagl_transport_attrib_list_count(attrib_list));

    if (!host_context) {
        goto fail;
    }

    ctx = yagl_context_create(host_context, dpy, yagl_get_api(), version);
    assert(ctx);

    yagl_display_context_add(dpy, ctx);

    yagl_context_release(ctx);

    YAGL_LOG_FUNC_EXIT("%u", host_context);

    return (EGLContext)host_context;

fail:
    YAGL_LOG_FUNC_EXIT(NULL);

    return EGL_NO_CONTEXT;
}

YAGL_API EGLBoolean eglDestroyContext(EGLDisplay dpy_, EGLContext ctx)
{
    struct yagl_display *dpy = NULL;

    YAGL_LOG_FUNC_ENTER(eglDestroyContext,
                        "dpy = %u, ctx = %u",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)ctx);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto fail;
    }

    if (!yagl_host_eglDestroyContext(dpy->host_dpy, (yagl_host_handle)ctx)) {
        goto fail;
    }

    yagl_display_context_remove(dpy, ctx);

    YAGL_LOG_FUNC_EXIT("1");

    return EGL_TRUE;

fail:
    YAGL_LOG_FUNC_EXIT(NULL);

    return EGL_FALSE;
}

YAGL_API EGLBoolean eglMakeCurrent(EGLDisplay dpy_,
                                   EGLSurface draw_,
                                   EGLSurface read_,
                                   EGLContext ctx_)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_surface *draw = NULL;
    struct yagl_surface *read = NULL;
    struct yagl_context *ctx = NULL;

    YAGL_LOG_FUNC_ENTER(eglMakeCurrent,
                        "dpy = %u, draw = %p, read = %p, ctx = %u",
                        (yagl_host_handle)dpy_,
                        draw_,
                        read_,
                        (yagl_host_handle)ctx_);

    dpy = yagl_display_get(dpy_);

    if (dpy) {
        draw = yagl_display_surface_acquire(dpy, draw_);
        read = yagl_display_surface_acquire(dpy, read_);
        ctx = yagl_display_context_acquire(dpy, ctx_);

        /*
         * A workaround for EffectsApp. It incorrectly calls
         * eglMakeCurrent(dpy, draw, read, NULL) which is not allowed
         * according to EGL standard. i.e. non-NULL surfaces and NULL context
         * is not allowed.
         */
        if (draw && read && !ctx) {
            ctx = yagl_get_context();
            yagl_context_acquire(ctx);
        }
    }

    if (ctx &&
        (ctx == yagl_get_context()) &&
        (ctx->dpy == dpy) &&
        (draw == yagl_get_draw_surface()) &&
        (read == yagl_get_read_surface())) {
        res = EGL_TRUE;

        goto out;
    }

    if ((draw && yagl_surface_locked(draw)) ||
        (read && yagl_surface_locked(read))) {
        YAGL_SET_ERR(EGL_BAD_ACCESS);
        goto out;
    }

    if (draw) {
        yagl_surface_invalidate(draw);
    }

    if (read && (draw != read)) {
        yagl_surface_invalidate(read);
    }

    if (!yagl_host_eglMakeCurrent((yagl_host_handle)dpy_,
                                  (draw ? draw->res.handle : 0),
                                  (read ? read->res.handle : 0),
                                  (ctx ? ctx->res.handle : 0))) {
        goto out;
    }

    yagl_set_context(ctx, draw, read);

    res = EGL_TRUE;

out:
    yagl_surface_release(draw);
    yagl_surface_release(read);
    yagl_context_release(ctx);

    YAGL_LOG_FUNC_EXIT("%d", ((res == EGL_TRUE) ? 1 : 0));

    return res;
}

YAGL_API EGLContext eglGetCurrentContext(void)
{
    struct yagl_context *ctx;

    YAGL_LOG_FUNC_ENTER(eglGetCurrentContext, NULL);

    ctx = yagl_get_context();

    YAGL_LOG_FUNC_EXIT("%u", (ctx ? ctx->res.handle : 0));

    return (ctx ? (EGLContext)ctx->res.handle : EGL_NO_CONTEXT);
}

YAGL_API EGLSurface eglGetCurrentSurface(EGLint readdraw)
{
    struct yagl_surface *sfc;
    EGLSurface ret = EGL_NO_SURFACE;

    YAGL_LOG_FUNC_ENTER(eglGetCurrentSurface, NULL);

    if (readdraw == EGL_READ) {
        sfc = yagl_get_read_surface();
        ret = (sfc ? yagl_surface_get_handle(sfc) : EGL_NO_SURFACE);
    } else if (readdraw == EGL_DRAW) {
        sfc = yagl_get_draw_surface();
        ret = (sfc ? yagl_surface_get_handle(sfc) : EGL_NO_SURFACE);
    } else {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
    }

    YAGL_LOG_FUNC_EXIT("%p", ret);

    return ret;
}

YAGL_API EGLDisplay eglGetCurrentDisplay(void)
{
    struct yagl_context *ctx;

    YAGL_LOG_FUNC_ENTER(eglGetCurrentDisplay, NULL);

    ctx = yagl_get_context();

    YAGL_LOG_FUNC_EXIT("%u", (ctx ? ctx->dpy->host_dpy : 0));

    return (ctx ? (EGLDisplay)ctx->dpy->host_dpy : EGL_NO_DISPLAY);
}

YAGL_API EGLBoolean eglQueryContext(EGLDisplay dpy_,
                                    EGLContext ctx,
                                    EGLint attribute,
                                    EGLint *value)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;

    YAGL_LOG_FUNC_ENTER(eglQueryContext,
                        "dpy = %u, ctx = %u, attribute = 0x%X, value = %p",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)ctx,
                        attribute,
                        value);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_host_eglQueryContext(dpy->host_dpy,
                                   (yagl_host_handle)ctx,
                                   attribute,
                                   value)) {
        goto out;
    }

    res = EGL_TRUE;

out:
    YAGL_LOG_FUNC_EXIT("%d", ((res == EGL_TRUE) ? 1 : 0));

    return res;
}

YAGL_API EGLBoolean eglWaitGL()
{
    EGLBoolean ret;

    YAGL_LOG_FUNC_ENTER_SPLIT0(eglWaitGL);

    ret = eglWaitClient();

    YAGL_LOG_FUNC_EXIT(NULL);

    return ret;
}

YAGL_API EGLBoolean eglWaitNative(EGLint engine)
{
    struct yagl_surface *draw_sfc;

    YAGL_LOG_FUNC_ENTER_SPLIT1(eglWaitNative, EGLint, engine);

    draw_sfc = yagl_get_draw_surface();

    if (draw_sfc) {
        draw_sfc->wait_x(draw_sfc);
    }

    YAGL_LOG_FUNC_EXIT(NULL);

    return EGL_TRUE;
}

YAGL_API EGLBoolean eglSwapBuffers(EGLDisplay dpy_, EGLSurface surface_)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_surface *surface = NULL;

    YAGL_LOG_FUNC_ENTER(eglSwapBuffers,
                        "dpy = %u, surface = %p",
                        (yagl_host_handle)dpy_,
                        surface_);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_surface(dpy, surface_, &surface)) {
        goto out;
    }

    if ((yagl_get_draw_surface() != surface) &&
        (yagl_get_read_surface() != surface)) {
        YAGL_SET_ERR(EGL_BAD_SURFACE);
        goto out;
    }

    if (yagl_surface_locked(surface)) {
        YAGL_SET_ERR(EGL_BAD_ACCESS);
        goto out;
    }

    if (surface->type != EGL_WINDOW_BIT) {
        res = EGL_TRUE;
        goto out;
    }

    if (!surface->swap_buffers(surface)) {
        goto out;
    }

    res = EGL_TRUE;

    yagl_render_invalidate();

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", res);

    return res;
}

YAGL_API EGLBoolean eglCopyBuffers(EGLDisplay dpy_,
                                   EGLSurface surface_,
                                   EGLNativePixmapType target)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_surface *surface = NULL;

    YAGL_LOG_FUNC_ENTER(eglCopyBuffers,
                        "dpy = %u, surface = %p, target = %u",
                        (yagl_host_handle)dpy_,
                        surface_,
                        (uint32_t)target);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_surface(dpy, surface_, &surface)) {
        goto out;
    }

    if ((yagl_get_draw_surface() != surface) &&
        (yagl_get_read_surface() != surface)) {
        YAGL_SET_ERR(EGL_BAD_SURFACE);
        goto out;
    }

    if (yagl_surface_locked(surface)) {
        YAGL_SET_ERR(EGL_BAD_ACCESS);
        goto out;
    }

    if (!surface->copy_buffers(surface, target)) {
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", res);

    return res;
}

YAGL_API EGLImageKHR eglCreateImageKHR(EGLDisplay dpy_,
                                       EGLContext ctx,
                                       EGLenum target,
                                       EGLClientBuffer buffer,
                                       const EGLint *attrib_list)
{
    EGLImageKHR ret = EGL_NO_IMAGE_KHR;
    struct yagl_display *dpy = NULL;
    struct yagl_native_drawable *native_buffer = NULL;
    struct yagl_image *image = NULL;
    int i = 0;

    YAGL_LOG_FUNC_ENTER(eglCreateImageKHR,
                        "dpy = %u, ctx = %u, target = %u, buffer = %p",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)ctx,
                        target,
                        buffer);

    if (!buffer) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    switch (target) {
    case EGL_NATIVE_PIXMAP_KHR:
        if (!dpy->native_dpy->platform->pixmaps_supported) {
            YAGL_SET_ERR(EGL_BAD_PARAMETER);
            goto out;
        }

        if (attrib_list) {
            while (attrib_list[i] != EGL_NONE) {
                switch (attrib_list[i]) {
                case EGL_IMAGE_PRESERVED_KHR:
                    break;
                default:
                    YAGL_SET_ERR(EGL_BAD_ATTRIBUTE);
                    goto out;
                }

                i += 2;
            }
        }

        native_buffer = dpy->native_dpy->wrap_pixmap(dpy->native_dpy,
                                                     (yagl_os_pixmap)buffer);

        if (!native_buffer) {
            goto out;
        }

        image = yagl_get_backend()->create_image_pixmap(dpy,
                                                        (yagl_host_handle)ctx,
                                                        native_buffer,
                                                        attrib_list);

        if (!image) {
            native_buffer->destroy(native_buffer);
            goto out;
        }

        break;
    case EGL_WAYLAND_BUFFER_WL:
        if (!dpy->native_dpy->WL_bind_wayland_display_supported) {
            YAGL_SET_ERR(EGL_BAD_PARAMETER);
            goto out;
        }

        if (attrib_list) {
            while (attrib_list[i] != EGL_NONE) {
                switch (attrib_list[i]) {
                case EGL_IMAGE_PRESERVED_KHR:
                case EGL_WAYLAND_PLANE_WL:
                    break;
                default:
                    YAGL_SET_ERR(EGL_BAD_ATTRIBUTE);
                    goto out;
                }

                i += 2;
            }
        }

        image = yagl_get_backend()->create_image_wl_buffer(dpy,
                                                           (yagl_host_handle)ctx,
                                                           (struct wl_resource*)buffer,
                                                           attrib_list);

        if (!image) {
            goto out;
        }

        break;
    default:
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    if (!yagl_display_image_add(dpy, image)) {
        yagl_host_eglDestroyImageKHR(dpy->host_dpy, image->res.handle);
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    ret = image->client_handle;

out:
    yagl_image_release(image);

    YAGL_LOG_FUNC_EXIT("%p", ret);

    return ret;
}

YAGL_API EGLBoolean eglDestroyImageKHR( EGLDisplay dpy_,
                                        EGLImageKHR image_ )
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_image *image = NULL;

    YAGL_LOG_FUNC_ENTER(eglDestroyImageKHR,
                        "dpy = %u, image = %p",
                        (yagl_host_handle)dpy_,
                        image_);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_image(dpy, image_, &image)) {
        goto out;
    }

    if (!yagl_host_eglDestroyImageKHR(dpy->host_dpy, image->res.handle)) {
        goto out;
    }

    if (!yagl_display_image_remove(dpy, image_)) {
        YAGL_LOG_ERROR("we're the one who destroy the image, but it isn't there!");
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_image_release(image);

    YAGL_LOG_FUNC_EXIT("%d", ((res == EGL_TRUE) ? 1 : 0));

    return res;
}

YAGL_API EGLBoolean eglLockSurfaceKHR(EGLDisplay dpy_,
                                      EGLSurface surface_,
                                      const EGLint *attrib_list)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_surface *surface = NULL;
    int i = 0;
    int preserve = 0;
    EGLint hint = 0;

    YAGL_LOG_FUNC_ENTER(eglLockSurfaceKHR,
                        "dpy = %u, surface = %p",
                        (yagl_host_handle)dpy_,
                        surface_);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_surface(dpy, surface_, &surface)) {
        goto out;
    }

    if (attrib_list) {
        while (attrib_list[i] != EGL_NONE) {
            switch (attrib_list[i]) {
            case EGL_MAP_PRESERVE_PIXELS_KHR:
                preserve = attrib_list[i + 1];
                break;
            case EGL_LOCK_USAGE_HINT_KHR:
                break;
            default:
                YAGL_SET_ERR(EGL_BAD_ATTRIBUTE);
                goto out;
            }

            i += 2;
        }
    }

    if (preserve) {
        hint = EGL_READ_SURFACE_BIT_KHR | EGL_WRITE_SURFACE_BIT_KHR;
    } else {
        hint = EGL_WRITE_SURFACE_BIT_KHR;
    }

    if (!yagl_surface_lock(surface, hint)) {
        YAGL_SET_ERR(EGL_BAD_ACCESS);
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", res);

    return res;
}

YAGL_API EGLBoolean eglUnlockSurfaceKHR(EGLDisplay dpy_,
                                        EGLSurface surface_)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_surface *surface = NULL;

    YAGL_LOG_FUNC_ENTER(eglUnlockSurfaceKHR,
                        "dpy = %u, surface = %p",
                        (yagl_host_handle)dpy_,
                        surface_);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_surface(dpy, surface_, &surface)) {
        goto out;
    }

    if (!yagl_surface_unlock(surface)) {
        YAGL_SET_ERR(EGL_BAD_ACCESS);
        goto out;
    }

    yagl_surface_invalidate(surface);

    res = EGL_TRUE;

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", res);

    return res;
}

#ifdef YAGL_PLATFORM_WAYLAND
struct wl_display;
struct wl_resource;

YAGL_API EGLBoolean eglBindWaylandDisplayWL(EGLDisplay dpy_,
                                            struct wl_display *display)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;

    YAGL_LOG_FUNC_ENTER(eglBindWaylandDisplayWL,
                        "dpy = %u, display = %p",
                        (yagl_host_handle)dpy_,
                        display);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!display) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    if (!dpy->native_dpy->WL_bind_wayland_display_supported) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    res = yagl_native_display_bind_wl_display(dpy->native_dpy, display);

out:
    YAGL_LOG_FUNC_EXIT("%d", res);

    return res;
}

YAGL_API EGLBoolean eglUnbindWaylandDisplayWL(EGLDisplay dpy_,
                                              struct wl_display *display)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;

    YAGL_LOG_FUNC_ENTER(eglUnbindWaylandDisplayWL,
                        "dpy = %u, display = %p",
                        (yagl_host_handle)dpy_,
                        display);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!display) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    if (!dpy->native_dpy->WL_bind_wayland_display_supported) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    res = yagl_native_display_unbind_wl_display(dpy->native_dpy);

out:
    YAGL_LOG_FUNC_EXIT("%d", res);

    return res;
}

YAGL_API EGLBoolean eglQueryWaylandBufferWL(EGLDisplay dpy_,
                                            struct wl_resource *buffer,
                                            EGLint attribute,
                                            EGLint *value)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;

    YAGL_LOG_FUNC_ENTER(eglQueryWaylandBufferWL,
                        "dpy = %u, buffer = %p, attribute = 0x%X",
                        (yagl_host_handle)dpy_,
                        buffer,
                        attribute);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!buffer) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    if (!dpy->native_dpy->WL_bind_wayland_display_supported) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    res = yagl_native_display_query_wl_buffer(dpy->native_dpy,
                                              buffer,
                                              attribute,
                                              value);

out:
    YAGL_LOG_FUNC_EXIT("%d", res);

    return res;
}
#endif

static __eglMustCastToProperFunctionPointerType yagl_get_gles1_proc_address(const char* procname)
{
    void *handle = dlopen("libGLESv1_CM.so.1", RTLD_NOW|RTLD_GLOBAL);
    if (!handle) {
        handle = dlopen("libGLESv1_CM.so", RTLD_NOW|RTLD_GLOBAL);
    }
    if (handle) {
        return dlsym(handle, procname);
    }
    return NULL;
}

static __eglMustCastToProperFunctionPointerType yagl_get_gles2_proc_address(const char* procname)
{
    void *handle = dlopen("libGLESv2.so.1", RTLD_NOW|RTLD_GLOBAL);
    if (!handle) {
        handle = dlopen("libGLESv2.so", RTLD_NOW|RTLD_GLOBAL);
    }
    if (handle) {
        return dlsym(handle, procname);
    }
    return NULL;
}

YAGL_API __eglMustCastToProperFunctionPointerType eglGetProcAddress(const char* procname)
{
    __eglMustCastToProperFunctionPointerType ret = NULL;

    YAGL_LOG_FUNC_ENTER(eglGetProcAddress, "procname = %s", procname);

    if (procname) {
        if (strncmp(procname, "gl", 2) == 0) {

            struct yagl_context *ctx = yagl_get_context();

            if (ctx) {
                switch (ctx->api) {
                case EGL_OPENGL_ES_API:
                    switch (ctx->version) {
                    case 1:
                        ret = yagl_get_gles1_proc_address(procname);
                        break;
                    case 2:
                        ret = yagl_get_gles2_proc_address(procname);
                        break;
                    default:
                        break;
                    }
                    break;
                case EGL_OPENVG_API:
                case EGL_OPENGL_API:
                default:
                    break;
                }
            } else {
                /*
                 * Workaround for evas, which foolishly calls this without
                 * context being set.
                 */
                ret = yagl_get_gles2_proc_address(procname);
            }
        } else if (strncmp(procname, "egl", 3) == 0) {
            ret = dlsym(NULL, procname);
        }
    }

    YAGL_LOG_FUNC_EXIT("%p", ret);

    return ret;
}
