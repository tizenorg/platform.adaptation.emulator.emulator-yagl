#include "yagl_host_egl_calls.h"
#include "yagl_impl.h"
#include "yagl_display.h"
#include "yagl_log.h"
#include "yagl_egl_state.h"
#include "yagl_malloc.h"
#include "yagl_surface.h"
#include "yagl_context.h"
#include "yagl_image.h"
#include "yagl_mem_egl.h"
#include "yagl_backend.h"
#include "yagl_render.h"
#include <EGL/eglext.h>
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
        YAGL_HOST_CALL_ASSERT(yagl_host_eglGetError(&retval));
    }

    YAGL_LOG_FUNC_EXIT_SPLIT(EGLint, retval);

    return retval;
}

YAGL_API EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id)
{
    yagl_host_handle host_dpy;
    struct yagl_display *dpy;
    EGLDisplay ret = EGL_NO_DISPLAY;

    YAGL_LOG_FUNC_ENTER_SPLIT1(eglGetDisplay, EGLNativeDisplayType, display_id);

    YAGL_HOST_CALL_ASSERT(yagl_host_eglGetDisplay(&host_dpy, display_id));

    if (!host_dpy) {
        YAGL_LOG_ERROR("unable to get host display for %p", display_id);
        goto out;
    }

    dpy = yagl_display_add(display_id, host_dpy);

    if (!dpy) {
        YAGL_SET_ERR(EGL_BAD_DISPLAY);
        YAGL_LOG_ERROR("unable to add display %p, %u", display_id, host_dpy);
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
    EGLBoolean retval = EGL_FALSE;
    struct yagl_display *dpy;

    YAGL_LOG_FUNC_ENTER(eglInitialize, "dpy = %u", (yagl_host_handle)dpy_);

    dpy = yagl_display_get(dpy_);

    if (!dpy) {
        YAGL_SET_ERR(EGL_BAD_DISPLAY);
        goto fail;
    }

    do {
        yagl_mem_probe_write_EGLint(major);
        yagl_mem_probe_write_EGLint(minor);
    } while (!yagl_host_eglInitialize(&retval, dpy->host_dpy, major, minor));

    if (!retval) {
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

    YAGL_HOST_CALL_ASSERT(yagl_host_eglTerminate(&ret, dpy->host_dpy));

    if (ret) {
        yagl_display_terminate(dpy);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);

    return ret;
}

YAGL_API const char* eglQueryString(EGLDisplay dpy, EGLint name)
{
    const char *str = NULL;

    YAGL_LOG_FUNC_ENTER(eglQueryString,
                        "dpy = %u, name = %d",
                        (yagl_host_handle)dpy,
                        name);

    switch (name) {
    case EGL_VENDOR:
        str = "Samsung";
        break;
    case EGL_VERSION:
        str = "1.4";
        break;
    case EGL_CLIENT_APIS:
        str = "OpenGL_ES";
        break;
    case EGL_EXTENSIONS:
        str = "EGL_KHR_image_base "
              "EGL_KHR_image "
              "EGL_KHR_image_pixmap ";
        break;
    default:
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
    }

    YAGL_LOG_FUNC_EXIT(NULL);

    return str;
}

YAGL_API EGLBoolean eglGetConfigs( EGLDisplay dpy_,
                                   EGLConfig* configs_,
                                   EGLint config_size,
                                   EGLint* num_config )
{
    EGLBoolean retval = EGL_FALSE;
    yagl_host_handle* configs = NULL;
    EGLint i;

    YAGL_LOG_FUNC_ENTER(eglGetConfigs,
                        "dpy = %u, configs = %p, config_size = %d",
                        (yagl_host_handle)dpy_,
                        configs_,
                        config_size);

    if (configs_) {
        configs = yagl_malloc(config_size * sizeof(yagl_host_handle));
    }

    do {
        yagl_mem_probe_write(configs, config_size * sizeof(yagl_host_handle));
        yagl_mem_probe_write_EGLint(num_config);
    } while (!yagl_host_eglGetConfigs(&retval,
                                      (yagl_host_handle)dpy_,
                                      configs,
                                      config_size,
                                      num_config));

    if (!retval) {
        goto fail;
    }

    if (configs_) {
        for (i = 0; i < config_size; ++i) {
            configs_[i] = (EGLConfig)configs[i];
        }
    }

    YAGL_LOG_FUNC_EXIT("num_config = %d",
                       (num_config ? *num_config : -1));

    yagl_free(configs);

    return EGL_TRUE;

fail:
    YAGL_LOG_FUNC_EXIT(NULL);

    yagl_free(configs);

    return EGL_FALSE;
}

YAGL_API EGLBoolean eglChooseConfig( EGLDisplay dpy,
                                     const EGLint* attrib_list,
                                     EGLConfig* configs_,
                                     EGLint config_size,
                                     EGLint* num_config )
{
    EGLBoolean retval = EGL_FALSE;
    yagl_host_handle* configs = NULL;
    EGLint i;

    YAGL_LOG_FUNC_ENTER(eglChooseConfig, "dpy = %u", (yagl_host_handle)dpy);

    if (configs_) {
        configs = yagl_malloc(config_size * sizeof(yagl_host_handle));
    }

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
        yagl_mem_probe_write(configs, config_size * sizeof(yagl_host_handle));
        yagl_mem_probe_write_EGLint(num_config);
    } while(!yagl_host_eglChooseConfig(&retval,
                                       (yagl_host_handle)dpy,
                                       attrib_list,
                                       configs,
                                       config_size,
                                       num_config));

    if (!retval) {
        goto fail;
    }

    if (configs_) {
        for (i = 0; i < config_size; ++i) {
            configs_[i] = (EGLConfig)configs[i];
        }
    }

    YAGL_LOG_FUNC_EXIT("num_config = %d",
                       (num_config ? *num_config : -1));

    yagl_free(configs);

    return EGL_TRUE;

fail:
    YAGL_LOG_FUNC_EXIT(NULL);

    yagl_free(configs);

    return EGL_FALSE;
}

YAGL_API EGLBoolean eglGetConfigAttrib( EGLDisplay dpy_,
                                        EGLConfig config,
                                        EGLint attribute,
                                        EGLint* value )
{
    struct yagl_display *dpy = NULL;
    EGLBoolean ret = EGL_FALSE;
    int screen;
    XVisualInfo vi;

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
        screen = XScreenNumberOfScreen(XDefaultScreenOfDisplay(dpy->x_dpy));

        /*
         * 24-bit is the highest supported by soft framebuffer.
         */
        if (!XMatchVisualInfo(dpy->x_dpy, screen, 24, TrueColor, &vi))
        {
            YAGL_SET_ERR(EGL_BAD_CONFIG);
            YAGL_LOG_ERROR("XMatchVisualInfo failed");
            goto fail;
        }

        if (attribute == EGL_NATIVE_VISUAL_ID) {
            *value = XVisualIDFromVisual(vi.visual);
        } else {
            *value = TrueColor;
        }

        ret = EGL_TRUE;

        break;
    default:
        do {
            yagl_mem_probe_write_EGLint(value);
        } while (!yagl_host_eglGetConfigAttrib(&ret,
                                               (yagl_host_handle)dpy_,
                                               (yagl_host_handle)config,
                                               attribute,
                                               value));

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

YAGL_API EGLSurface eglCreateWindowSurface( EGLDisplay dpy_,
                                            EGLConfig config,
                                            EGLNativeWindowType win,
                                            const EGLint* attrib_list)
{
    struct yagl_display *dpy = NULL;
    struct yagl_surface *surface = NULL;

    YAGL_LOG_FUNC_ENTER(eglCreateWindowSurface,
                        "dpy = %u, config = %u",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)config);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto fail;
    }

    surface = yagl_get_backend()->create_window_surface(dpy,
                                                        (yagl_host_handle)config,
                                                        win,
                                                        attrib_list);

    if (!surface) {
        goto fail;
    }

    if (!yagl_display_surface_add(dpy, surface)) {
        EGLBoolean retval;
        YAGL_HOST_CALL_ASSERT(yagl_host_eglDestroySurface(&retval,
                                                          dpy->host_dpy,
                                                          surface->res.handle));
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

YAGL_API EGLSurface eglCreatePbufferSurface( EGLDisplay dpy_,
                                             EGLConfig config,
                                             const EGLint* attrib_list )
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
        EGLBoolean retval;
        YAGL_HOST_CALL_ASSERT(yagl_host_eglDestroySurface(&retval,
                                                          dpy->host_dpy,
                                                          surface->res.handle));
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

YAGL_API EGLSurface eglCreatePixmapSurface( EGLDisplay dpy_,
                                            EGLConfig config,
                                            EGLNativePixmapType pixmap,
                                            const EGLint* attrib_list )
{
    struct yagl_display *dpy = NULL;
    struct yagl_surface *surface = NULL;

    YAGL_LOG_FUNC_ENTER(eglCreatePixmapSurface,
                        "dpy = %u, config = %u",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)config);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto fail;
    }

    surface = yagl_get_backend()->create_pixmap_surface(dpy,
                                                        (yagl_host_handle)config,
                                                        pixmap,
                                                        attrib_list);

    if (!surface) {
        goto fail;
    }

    if (!yagl_display_surface_add(dpy, surface)) {
        EGLBoolean retval;
        YAGL_HOST_CALL_ASSERT(yagl_host_eglDestroySurface(&retval,
                                                          dpy->host_dpy,
                                                          surface->res.handle));
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
    EGLBoolean retval = EGL_FALSE;
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

    YAGL_HOST_CALL_ASSERT(yagl_host_eglDestroySurface(&retval,
                                                      dpy->host_dpy,
                                                      surface->res.handle));

    if (!retval) {
        goto out;
    }

    if (!yagl_display_surface_remove(dpy, surface_)) {
        YAGL_LOG_ERROR("we're the one who destroy the surface, but surface isn't there!");
        YAGL_SET_ERR(EGL_BAD_SURFACE);
        goto out;
    }

    if (!surface->reset(surface)) {
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", ((res == EGL_TRUE) ? 1 : 0));

    return res;
}

YAGL_API EGLBoolean eglQuerySurface( EGLDisplay dpy_,
                                     EGLSurface surface_,
                                     EGLint attribute,
                                     EGLint* value )
{
    EGLBoolean res = EGL_FALSE;
    EGLBoolean retval = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_surface *surface = NULL;

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

    do {
        yagl_mem_probe_write_EGLint(value);
    } while (!yagl_host_eglQuerySurface(&retval,
                                        dpy->host_dpy,
                                        surface->res.handle,
                                        attribute,
                                        value));

    if (!retval) {
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", ((res == EGL_TRUE) ? 1 : 0));

    return res;
}

YAGL_API EGLBoolean eglBindAPI(EGLenum api)
{
    EGLBoolean ret;

    YAGL_LOG_FUNC_ENTER_SPLIT1(eglBindAPI, EGLenum, api);

    YAGL_HOST_CALL_ASSERT(yagl_host_eglBindAPI(&ret, api));

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
    EGLBoolean retval;

    YAGL_LOG_FUNC_ENTER(eglReleaseThread, NULL);

    YAGL_HOST_CALL_ASSERT(yagl_host_eglReleaseThread(&retval));

    if (!retval) {
        goto fail;
    }

    yagl_reset_state();

    YAGL_LOG_FUNC_EXIT("1");

    return EGL_TRUE;

fail:
    YAGL_LOG_FUNC_EXIT(NULL);

    return EGL_FALSE;
}

YAGL_API EGLSurface eglCreatePbufferFromClientBuffer( EGLDisplay dpy,
                                                      EGLenum buftype,
                                                      EGLClientBuffer buffer,
                                                      EGLConfig config,
                                                      const EGLint* attrib_list )
{
    YAGL_UNIMPLEMENTED(eglCreatePbufferFromClientBuffer, EGL_NO_SURFACE);
}

YAGL_API EGLBoolean eglSurfaceAttrib( EGLDisplay dpy_,
                                      EGLSurface surface_,
                                      EGLint attribute,
                                      EGLint value )
{
    EGLBoolean res = EGL_FALSE;
    EGLBoolean retval = EGL_FALSE;
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

    YAGL_HOST_CALL_ASSERT(yagl_host_eglSurfaceAttrib(&retval,
        dpy->host_dpy,
        surface->res.handle,
        attribute,
        value));

    if (!retval) {
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", ((res == EGL_TRUE) ? 1 : 0));

    return res;
}

YAGL_API EGLBoolean eglBindTexImage( EGLDisplay dpy,
                                     EGLSurface surface,
                                     EGLint buffer )
{
    YAGL_UNIMPLEMENTED(eglBindTexImage, EGL_FALSE);
}

YAGL_API EGLBoolean eglReleaseTexImage( EGLDisplay dpy,
                                        EGLSurface surface,
                                        EGLint buffer )
{
    YAGL_UNIMPLEMENTED(eglReleaseTexImage, EGL_FALSE);
}

YAGL_API EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
    YAGL_LOG_FUNC_ENTER(eglSwapInterval,
                        "dpy = %u, interval = %d",
                        (yagl_host_handle)dpy,
                        interval);

    /*
     * We don't care about this for now.
     */

    YAGL_LOG_FUNC_EXIT("1");

    return EGL_TRUE;
}

YAGL_API EGLContext eglCreateContext( EGLDisplay dpy_,
                                      EGLConfig config,
                                      EGLContext share_context,
                                      const EGLint* attrib_list )
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

    do {
        yagl_mem_probe_read_attrib_list(attrib_list);
    } while (!yagl_host_eglCreateContext(&host_context,
        dpy->host_dpy,
        (yagl_host_handle)config,
        (yagl_host_handle)share_context,
        attrib_list));

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

YAGL_API EGLBoolean eglDestroyContext( EGLDisplay dpy_,
                                       EGLContext ctx )
{
    EGLBoolean retval = EGL_FALSE;
    struct yagl_display *dpy = NULL;

    YAGL_LOG_FUNC_ENTER(eglDestroyContext,
                        "dpy = %u, ctx = %u",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)ctx);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto fail;
    }

    YAGL_HOST_CALL_ASSERT(yagl_host_eglDestroyContext(&retval,
                                                      dpy->host_dpy,
                                                      (yagl_host_handle)ctx));

    if (!retval) {
        goto fail;
    }

    yagl_display_context_remove(dpy, ctx);

    YAGL_LOG_FUNC_EXIT("1");

    return EGL_TRUE;

fail:
    YAGL_LOG_FUNC_EXIT(NULL);

    return EGL_FALSE;
}

YAGL_API EGLBoolean eglMakeCurrent( EGLDisplay dpy_,
                                    EGLSurface draw_,
                                    EGLSurface read_,
                                    EGLContext ctx_ )
{
    EGLBoolean res = EGL_FALSE;
    EGLBoolean retval = EGL_FALSE;
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

    if (draw) {
        draw->invalidate(draw);
    }

    if (read && (draw != read)) {
        read->invalidate(read);
    }

    YAGL_HOST_CALL_ASSERT(yagl_host_eglMakeCurrent(&retval,
        (yagl_host_handle)dpy_,
        (draw ? draw->res.handle : 0),
        (read ? read->res.handle : 0),
        (ctx ? ctx->res.handle : 0)));

    if (!retval) {
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

YAGL_API EGLBoolean eglQueryContext( EGLDisplay dpy_,
                                     EGLContext ctx,
                                     EGLint attribute,
                                     EGLint* value )
{
    EGLBoolean res = EGL_FALSE;
    EGLBoolean retval = EGL_FALSE;
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

    do {
        yagl_mem_probe_write_EGLint(value);
    } while (!yagl_host_eglQueryContext(&retval,
        dpy->host_dpy,
        (yagl_host_handle)ctx,
        attribute,
        value));

    if (!retval) {
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

    if (surface->type != EGL_WINDOW_BIT) {
        res = EGL_TRUE;
        goto out;
    }

    if (!surface->swap_buffers(surface)) {
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_surface_release(surface);

    yagl_render_invalidate();

    YAGL_LOG_FUNC_EXIT("%d", res);

    return res;
}

YAGL_API EGLBoolean eglCopyBuffers( EGLDisplay dpy_,
                                    EGLSurface surface_,
                                    EGLNativePixmapType target )
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

    if (!surface->copy_buffers(surface, target)) {
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", res);

    return res;
}

YAGL_API EGLImageKHR eglCreateImageKHR( EGLDisplay dpy_,
                                        EGLContext ctx,
                                        EGLenum target,
                                        EGLClientBuffer buffer,
                                        const EGLint *attrib_list )
{
    EGLImageKHR ret = EGL_NO_IMAGE_KHR;
    struct yagl_display *dpy = NULL;
    struct yagl_image *image = NULL;

    YAGL_LOG_FUNC_ENTER(eglCreateImageKHR,
                        "dpy = %u, ctx = %u, target = %u, buffer = %p",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)ctx,
                        target,
                        buffer);

    if (target != EGL_NATIVE_PIXMAP_KHR) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    if (!buffer) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    image = yagl_get_backend()->create_image(dpy,
                                             (yagl_host_handle)ctx,
                                             (Pixmap)buffer,
                                             attrib_list);

    if (!image) {
        goto out;
    }

    if (!yagl_display_image_add(dpy, image)) {
        EGLBoolean retval;
        YAGL_HOST_CALL_ASSERT(yagl_host_eglDestroyImageKHR(&retval,
                                                           dpy->host_dpy,
                                                           image->res.handle));
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    ret = yagl_image_get_handle(image);

out:
    yagl_image_release(image);

    YAGL_LOG_FUNC_EXIT("%p", ret);

    return ret;
}

YAGL_API EGLBoolean eglDestroyImageKHR( EGLDisplay dpy_,
                                        EGLImageKHR image_ )
{
    EGLBoolean res = EGL_FALSE;
    EGLBoolean retval = EGL_FALSE;
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

    YAGL_HOST_CALL_ASSERT(yagl_host_eglDestroyImageKHR(&retval,
                                                       dpy->host_dpy,
                                                       image->res.handle));

    if (!retval) {
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

/* We assume that if procname ends with 'ARB', 'EXT' or 'OES' then its an
 * extension */
static int yagl_procname_is_extension(const char* procname)
{
    size_t len = strlen(procname);

    procname = &procname[len - 3];

    if (strcmp("OES", procname) == 0 ||
        strcmp("ARB", procname) == 0 ||
        strcmp("EXT", procname) == 0) {
        return 1;
    }

    return 0;
}

YAGL_API __eglMustCastToProperFunctionPointerType eglGetProcAddress(const char* procname)
{
    __eglMustCastToProperFunctionPointerType ret = NULL;

    YAGL_LOG_FUNC_ENTER(eglGetProcAddress, "procname = %s", procname);

    if (procname) {
        if (strcmp("eglCreateImageKHR", procname) == 0) {
            ret = (__eglMustCastToProperFunctionPointerType)&eglCreateImageKHR;
        } else if (strcmp("eglDestroyImageKHR", procname) == 0) {
            ret = (__eglMustCastToProperFunctionPointerType)&eglDestroyImageKHR;
        } else if (yagl_procname_is_extension(procname)) {

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
        }
    }

    YAGL_LOG_FUNC_EXIT("%p", ret);

    return ret;
}
