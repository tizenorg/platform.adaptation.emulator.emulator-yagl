/*
 * YaGL
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Stanislav Vorobiov <s.vorobiov@samsung.com>
 * Jinhyung Jo <jinhyung.jo@samsung.com>
 * YeongKyoon Lee <yeongkyoon.lee@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * - S-Core Co., Ltd
 *
 */

#include "yagl_host_egl_calls.h"
#include "yagl_impl.h"
#include "yagl_display.h"
#include "yagl_log.h"
#include "yagl_egl_state.h"
#include "yagl_state.h"
#include "yagl_malloc.h"
#include "yagl_surface.h"
#include "yagl_context.h"
#include "yagl_image.h"
#include "yagl_fence.h"
#include "yagl_backend.h"
#include "yagl_render.h"
#include "yagl_native_platform.h"
#include "yagl_native_display.h"
#include "yagl_native_drawable.h"
#include "yagl_transport_egl.h"
#include "yagl_client_interface.h"
#include "yagl_client_image.h"
#include "yagl_client_context.h"
#include "yagl_sharegroup.h"
#include "yagl_tex_image_binding.h"
#ifdef YAGL_PLATFORM_X11
#include "x11/yagl_x11_platform.h"
#endif
#include "EGL/eglmesaext.h"
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

static int yagl_get_client_api(const EGLint *attrib_list, yagl_client_api *client_api)
{
    int i = 0;

    switch (yagl_get_api()) {
    case EGL_OPENGL_ES_API:
        *client_api = yagl_client_api_gles1;
        if (attrib_list) {
            while (attrib_list[i] != EGL_NONE) {
                switch (attrib_list[i]) {
                case EGL_CONTEXT_CLIENT_VERSION:
                    if (attrib_list[i + 1] == 1) {
                        *client_api = yagl_client_api_gles1;
                    } else if (attrib_list[i + 1] == 2) {
                        *client_api = yagl_client_api_gles2;
                    } else if (attrib_list[i + 1] == 3) {
                        *client_api = yagl_client_api_gles3;
                    } else {
                        return 0;
                    }
                    break;
                default:
                    return 0;
                }

                i += 2;
            }
        }
        break;
    case EGL_OPENVG_API:
        *client_api = yagl_client_api_ovg;
        break;
    case EGL_OPENGL_API:
        *client_api = yagl_client_api_ogl;
        break;
    default:
        return 0;
    }
    return 1;
}

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

static __inline int yagl_validate_context(struct yagl_display *dpy,
                                          EGLContext ctx_,
                                          struct yagl_context **ctx)
{
    YAGL_LOG_FUNC_SET(yagl_validate_context);

    *ctx = yagl_display_context_acquire(dpy, ctx_);

    if (!*ctx) {
        YAGL_SET_ERR(EGL_BAD_CONTEXT);
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

static __inline int yagl_validate_fence(struct yagl_display *dpy,
                                        EGLSyncKHR sync_,
                                        struct yagl_fence **fence)
{
    YAGL_LOG_FUNC_SET(yagl_validate_fence);

    *fence = yagl_display_fence_acquire(dpy, sync_);

    if (!*fence) {
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

    YAGL_LOG_FUNC_EXIT_SPLIT(EGLint, retval);

    return retval;
}

YAGL_API EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id)
{
    struct yagl_native_platform *platform;
    struct yagl_display *dpy;
    EGLDisplay ret = EGL_NO_DISPLAY;

    YAGL_LOG_FUNC_ENTER_SPLIT1(eglGetDisplay, EGLNativeDisplayType, display_id);

    platform = yagl_guess_platform((yagl_os_display)display_id);

    if (!platform) {
        goto out;
    }

    dpy = yagl_display_add(platform, (yagl_os_display)display_id);

    if (!dpy) {
        YAGL_SET_ERR(EGL_BAD_DISPLAY);
        YAGL_LOG_ERROR("unable to add display %p",
                       (yagl_os_display)display_id);
        goto out;
    }

    ret = (EGLDisplay)INT2VOIDP(dpy->host_dpy);

    YAGL_LOG_FUNC_EXIT_SPLIT(yagl_host_handle, dpy->host_dpy);

    return ret;

out:
    YAGL_LOG_FUNC_EXIT(NULL);

    return ret;
}

YAGL_API EGLBoolean eglInitialize(EGLDisplay dpy_, EGLint* major, EGLint* minor)
{
    EGLint error = 0;
    struct yagl_display *dpy;

    YAGL_LOG_FUNC_ENTER(eglInitialize, "dpy = %u", (yagl_host_handle)dpy_);

    dpy = yagl_display_get(dpy_);

    if (!dpy) {
        YAGL_SET_ERR(EGL_BAD_DISPLAY);
        goto fail;
    }

    if (!yagl_host_eglInitialize(dpy->host_dpy, major, minor, &error)) {
        YAGL_SET_ERR(error);
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
    EGLint error = 0;
    struct yagl_display *dpy;
    EGLBoolean ret = EGL_FALSE;

    YAGL_LOG_FUNC_ENTER(eglTerminate, "dpy = %u", (yagl_host_handle)dpy_);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    ret = yagl_host_eglTerminate(dpy->host_dpy, &error);

    if (ret) {
        yagl_display_terminate(dpy);
    } else {
        YAGL_SET_ERR(error);
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
        if (yagl_get_host_gl_version() < yagl_gl_3_1_es3) {
            str = "OpenGL_ES OpenGL_ES2";
        } else {
            str = "OpenGL_ES OpenGL_ES2 OpenGL_ES3";
        }
        break;
    case EGL_EXTENSIONS:
        str = yagl_display_get_extensions(yagl_display_get(dpy_));
        break;
    default:
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
    }

    YAGL_LOG_FUNC_EXIT("%s", str);

    return str;
}

YAGL_API EGLBoolean eglGetConfigs(EGLDisplay dpy,
                                  EGLConfig *configs,
                                  EGLint config_size,
                                  EGLint *num_config)
{
    EGLint error = 0;

    YAGL_LOG_FUNC_ENTER(eglGetConfigs,
                        "dpy = %u, configs = %p, config_size = %d",
                        (yagl_host_handle)dpy,
                        configs,
                        config_size);

    if (!num_config) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto fail;
    }

    if (!yagl_host_eglGetConfigs((yagl_host_handle)VOIDP2INT(dpy),
                                 (yagl_host_handle*)configs,
                                 config_size,
                                 num_config,
                                 &error)) {
        YAGL_SET_ERR(error);
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
    EGLint error = 0;

    YAGL_LOG_FUNC_ENTER(eglChooseConfig, "dpy = %u", (yagl_host_handle)dpy);

    if (!num_config) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto fail;
    }

    if (!yagl_host_eglChooseConfig((yagl_host_handle)VOIDP2INT(dpy),
                                   attrib_list,
                                   yagl_transport_attrib_list_count(attrib_list),
                                   (yagl_host_handle*)configs,
                                   config_size,
                                   num_config,
                                   &error)) {
        YAGL_SET_ERR(error);
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
    EGLint error = 0;
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
        ret = yagl_host_eglGetConfigAttrib((yagl_host_handle)VOIDP2INT(dpy_),
                                           (yagl_host_handle)VOIDP2INT(config),
                                           attribute,
                                           value,
                                           &error);

        if (!ret) {
            YAGL_SET_ERR(error);
        }

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
    EGLint error = 0;
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
                                                        (yagl_host_handle)VOIDP2INT(config),
                                                        native_win,
                                                        attrib_list);

    if (!surface) {
        native_win->destroy(native_win);
        native_win = NULL;
        goto fail;
    }

    if (!yagl_display_surface_add(dpy, surface)) {
        yagl_host_eglDestroySurface(dpy->host_dpy, surface->res.handle, &error);
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
    EGLint error = 0;
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
                                                         (yagl_host_handle)VOIDP2INT(config),
                                                         attrib_list);

    if (!surface) {
        goto fail;
    }

    if (!yagl_display_surface_add(dpy, surface)) {
        yagl_host_eglDestroySurface(dpy->host_dpy, surface->res.handle, &error);
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
    EGLint error = 0;
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
                                                        (yagl_host_handle)VOIDP2INT(config),
                                                        native_pixmap,
                                                        attrib_list);

    if (!surface) {
        native_pixmap->destroy(native_pixmap);
        native_pixmap = NULL;
        goto fail;
    }

    if (!yagl_display_surface_add(dpy, surface)) {
        yagl_host_eglDestroySurface(dpy->host_dpy, surface->res.handle, &error);
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
    EGLint error = 0;
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

    if (!yagl_host_eglDestroySurface(dpy->host_dpy, surface->res.handle, &error)) {
        YAGL_SET_ERR(error);
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
    EGLint error = 0;
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
            *value = (EGLint)VOIDP2INT(ptr);
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
                                           value,
                                           &error);

        if (!retval) {
            YAGL_SET_ERR(error);
        }

        break;
    }

out:
    yagl_surface_release(surface);

    YAGL_LOG_FUNC_EXIT("%d", ((retval == EGL_TRUE) ? 1 : 0));

    return retval;
}

YAGL_API EGLBoolean eglBindAPI(EGLenum api)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(eglBindAPI, EGLenum, api);

    if (api != EGL_OPENGL_ES_API) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        return EGL_FALSE;
    }

    yagl_host_eglBindAPI(api);
    yagl_set_api(api);

    YAGL_LOG_FUNC_EXIT_SPLIT(EGLBoolean, EGL_TRUE);

    return EGL_TRUE;
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
    EGLint error = 0;

    YAGL_LOG_FUNC_ENTER(eglReleaseThread, NULL);

    if (!yagl_host_eglReleaseThread(&error)) {
        YAGL_SET_ERR(error);
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
    EGLint error = 0;
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
                                    value,
                                    &error)) {
        YAGL_SET_ERR(error);
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
    struct yagl_context *ctx = NULL;
    struct yagl_client_interface *iface = NULL;
    struct yagl_client_image *image = NULL;
    struct yagl_tex_image_binding *binding = NULL;

    YAGL_LOG_FUNC_ENTER(eglBindTexImage,
                        "dpy = %u, surface = %p, buffer = 0x%X",
                        (yagl_host_handle)dpy_,
                        surface_,
                        buffer);

    iface = yagl_get_any_client_interface();

    if (!iface) {
        YAGL_SET_ERR(EGL_BAD_ALLOC);
        goto out;
    }

    ctx = yagl_get_context();

    if (!ctx) {
        YAGL_LOG_WARN("No context");
        res = EGL_TRUE;
        goto out;
    }

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_surface(dpy, surface_, &surface)) {
        goto out;
    }

    if (buffer != EGL_BACK_BUFFER) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    if (surface->type != EGL_PBUFFER_BIT) {
        YAGL_SET_ERR(EGL_BAD_SURFACE);
        goto out;
    }

    image = surface->create_image(surface, iface);

    if (!image) {
        goto out;
    }

    binding = yagl_surface_create_tex_image_binding(surface, iface);

    if (!ctx->client_ctx->bind_tex_image(ctx->client_ctx, image, binding)) {
        yagl_free(binding);
        YAGL_SET_ERR(EGL_BAD_ACCESS);
        goto out;
    }

    if (!yagl_surface_bind_tex_image(surface, binding)) {
        iface->release_tex_image(iface, binding->cookie);
        yagl_free(binding);
        YAGL_SET_ERR(EGL_BAD_ACCESS);
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_surface_release(surface);
    yagl_client_image_release(image);

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

    if (buffer != EGL_BACK_BUFFER) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    if (surface->type != EGL_PBUFFER_BIT) {
        YAGL_SET_ERR(EGL_BAD_SURFACE);
        goto out;
    }

    yagl_surface_release_tex_image(surface);

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
                                     EGLContext share_context_,
                                     const EGLint *attrib_list)
{
    EGLint error = 0;
    struct yagl_display *dpy = NULL;
    struct yagl_context *share_context = NULL;
    struct yagl_sharegroup *sg = NULL;
    yagl_client_api client_api;
    struct yagl_client_interface *iface = NULL;
    yagl_host_handle host_context = 0;
    struct yagl_client_context *client_ctx = NULL;
    struct yagl_context *ctx = NULL;

    YAGL_LOG_FUNC_ENTER(eglCreateContext,
                        "dpy = %u, config = %u, share_context = %u",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)config,
                        (yagl_host_handle)share_context_);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (share_context_) {
        if (!yagl_validate_context(dpy, share_context_, &share_context)) {
            goto out;
        }
        sg = share_context->client_ctx->sg;
        yagl_sharegroup_acquire(sg);
    } else {
        sg = yagl_sharegroup_create();
    }

    if (!yagl_get_client_api(attrib_list, &client_api)) {
        YAGL_SET_ERR(EGL_BAD_ATTRIBUTE);
        goto out;
    }

    if ((client_api == yagl_client_api_gles3) &&
        (yagl_get_host_gl_version() < yagl_gl_3_1_es3)) {
        YAGL_SET_ERR(EGL_BAD_ATTRIBUTE);
        goto out;
    }

    iface = yagl_get_client_interface(client_api);

    if (!iface) {
        YAGL_SET_ERR(EGL_BAD_ATTRIBUTE);
        goto out;
    }

    host_context =
        yagl_host_eglCreateContext(dpy->host_dpy,
                                   (yagl_host_handle)VOIDP2INT(config),
                                   (yagl_host_handle)VOIDP2INT(share_context_),
                                   attrib_list,
                                   yagl_transport_attrib_list_count(attrib_list),
                                   &error);

    if (!host_context) {
        YAGL_SET_ERR(error);
        goto out;
    }

    client_ctx = iface->create_ctx(iface, client_api, sg);

    ctx = yagl_context_create(host_context, dpy, client_ctx);
    assert(ctx);

    yagl_display_context_add(dpy, ctx);

    yagl_context_release(ctx);

out:
    yagl_sharegroup_release(sg);
    yagl_context_release(share_context);

    YAGL_LOG_FUNC_EXIT("%u", host_context);

    return (EGLContext)INT2VOIDP(host_context);
}

YAGL_API EGLBoolean eglDestroyContext(EGLDisplay dpy_, EGLContext ctx)
{
    EGLint error = 0;
    struct yagl_display *dpy = NULL;

    YAGL_LOG_FUNC_ENTER(eglDestroyContext,
                        "dpy = %u, ctx = %u",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)ctx);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto fail;
    }

    if (!yagl_host_eglDestroyContext(dpy->host_dpy, (yagl_host_handle)VOIDP2INT(ctx), &error)) {
        YAGL_SET_ERR(error);
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
    int bad_match = ctx_ ? (!draw_ ^ !read_) : (draw_ || read_);
    struct yagl_display *dpy = NULL;
    struct yagl_surface *draw = NULL;
    struct yagl_surface *read = NULL;
    struct yagl_context *ctx = NULL;
    struct yagl_context *prev_ctx = NULL;

    YAGL_LOG_FUNC_ENTER(eglMakeCurrent,
                        "dpy = %u, draw = %p, read = %p, ctx = %u",
                        (yagl_host_handle)dpy_,
                        draw_,
                        read_,
                        (yagl_host_handle)ctx_);

    if (bad_match) {
        YAGL_SET_ERR(EGL_BAD_MATCH);
        goto out;
    }

    prev_ctx = yagl_get_context();

    if (!draw_ && !read_ && !ctx_) {
        /*
         * Releasing context.
         */

        if (prev_ctx) {
            if (dpy_ == EGL_NO_DISPLAY) {
                /*
                 * Workaround for the case when dpy
                 * passed is EGL_NO_DISPLAY and we're releasing
                 * the current context.
                 */

                dpy_ = (EGLDisplay)INT2VOIDP(prev_ctx->dpy->host_dpy);
                dpy = prev_ctx->dpy;
            } else {
                if (!yagl_validate_display(dpy_, &dpy)) {
                    goto out;
                }
            }
        } else {
            /*
             * No current context and we're releasing, a no-op.
             */
            res = EGL_TRUE;
            goto out;
        }
    } else {
        /*
         * Making some context current.
         */

        if (!yagl_validate_display(dpy_, &dpy)) {
            goto out;
        }

        if (draw_ && !yagl_validate_surface(dpy, draw_, &draw)) {
            goto out;
        }

        if (read_ && !yagl_validate_surface(dpy, read_, &read)) {
            goto out;
        }

        ctx = yagl_display_context_acquire(dpy, ctx_);

        /*
         * A workaround for EffectsApp. It incorrectly calls
         * eglMakeCurrent(dpy, draw, read, NULL) which is not allowed
         * according to EGL standard. i.e. non-NULL surfaces and NULL context
         * is not allowed.
         */
        if (draw && read && !ctx) {
            ctx = prev_ctx;
            yagl_context_acquire(ctx);
        }

        if (!ctx) {
            YAGL_SET_ERR(EGL_BAD_CONTEXT);
            goto out;
        }
    }

    if (ctx &&
        (ctx == prev_ctx) &&
        (ctx->dpy == dpy) &&
        (draw == yagl_get_draw_surface()) &&
        (read == yagl_get_read_surface())) {
        /*
         * Absolutely no change, skip.
         */
        res = EGL_TRUE;

        goto out;
    }

    if ((draw && yagl_surface_locked(draw)) ||
        (read && yagl_surface_locked(read))) {
        YAGL_SET_ERR(EGL_BAD_ACCESS);
        goto out;
    }

    if (!yagl_set_context(ctx, draw, read)) {
        /*
         * Context and/or surfaces are already
         * current to some other thread.
         */
        YAGL_SET_ERR(EGL_BAD_ACCESS);
        goto out;
    }

    yagl_render_invalidate(0);

    yagl_host_eglMakeCurrent((yagl_host_handle)VOIDP2INT(dpy_),
                             (draw ? draw->res.handle : 0),
                             (read ? read->res.handle : 0),
                             (ctx ? ctx->res.handle : 0));

    if (ctx && !ctx->client_ctx_prepared) {
        ctx->client_ctx->prepare(ctx->client_ctx);
        ctx->client_ctx_prepared = 1;
    }

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

    return (ctx ? (EGLContext)INT2VOIDP(ctx->res.handle) : EGL_NO_CONTEXT);
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

    return (ctx ? (EGLDisplay)INT2VOIDP(ctx->dpy->host_dpy) : EGL_NO_DISPLAY);
}

YAGL_API EGLBoolean eglQueryContext(EGLDisplay dpy_,
                                    EGLContext ctx_,
                                    EGLint attribute,
                                    EGLint *value)
{
    EGLint error = 0;
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_context *ctx = NULL;

    YAGL_LOG_FUNC_ENTER(eglQueryContext,
                        "dpy = %u, ctx = %u, attribute = 0x%X, value = %p",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)ctx_,
                        attribute,
                        value);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_context(dpy, ctx_, &ctx)) {
        goto out;
    }

    res = EGL_TRUE;

    switch (attribute) {
    case EGL_CONTEXT_CLIENT_TYPE:
        switch (ctx->client_ctx->client_api) {
        case yagl_client_api_gles1:
        case yagl_client_api_gles2:
        case yagl_client_api_gles3:
            if (value) {
                *value = EGL_OPENGL_ES_API;
            }
            break;
        case yagl_client_api_ogl:
            if (value) {
                *value = EGL_OPENGL_API;
            }
            break;
        case yagl_client_api_ovg:
            if (value) {
                *value = EGL_OPENVG_API;
            }
            break;
        default:
            if (value) {
                *value = EGL_NONE;
            }
            break;
        }
        break;
    case EGL_CONTEXT_CLIENT_VERSION:
        switch (ctx->client_ctx->client_api) {
        case yagl_client_api_gles1:
            if (value) {
                *value = 1;
            }
            break;
        case yagl_client_api_gles2:
            if (value) {
                *value = 2;
            }
            break;
        case yagl_client_api_gles3:
            if (value) {
                *value = 3;
            }
            break;
        case yagl_client_api_ogl:
        case yagl_client_api_ovg:
        default:
            if (value) {
                *value = 0;
            }
            break;
        }
        break;
    default:
        res = yagl_host_eglQueryContext(dpy->host_dpy,
                                        (yagl_host_handle)VOIDP2INT(ctx_),
                                        attribute,
                                        value,
                                        &error);

        if (!res) {
            YAGL_SET_ERR(error);
        }

        break;
    }

out:
    yagl_context_release(ctx);

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

#ifdef YAGL_PLATFORM_X11
    if (dpy->native_dpy->platform == &yagl_x11_platform) {
        /*
         * Normally one should not invalidate right after eglSwapBuffers,
         * instead invalidate should be called on first rendering call
         * after eglSwapBuffers (see yagl_render.h:yagl_render_invalidate).
         * But this doesn't work on Tizen mobile, without this invalidate
         * Tizen compositor may miss surface updates.
         * The problem is in "efl_sync" option of the compositor, which is 0
         * by default in upstream and 1 in Tizen. Setting it to 0 in
         * config doesn't seem to help, one had to manually change code:
         * e17-extra-modules/comp-tizen/src/e_mod_comp_cfdata.c:
         * cfg->efl_sync = 1;
         * to:
         * cfg->efl_sync = 0;
         * AND set efl_sync to 0 in config to make this work.
         * When efl_sync is set to 1 the compositor acts like this:
         * + For normal X11 windows nothing is changed
         * + For e17 windows (created via ecore_x_window_new) it doesn't redraw
         *   the screen on damage events, but only on special SYNC_DRAW_DONE
         *   events that are sent from _ecore_evas_x_flush_post, which is called
         *   after eglSwapBuffers, that's why things work when invalidate
         *   is inside eglSwapBuffers, it finishes the swap before sending
         *   SYNC_DRAW_DONE, but once we move invalidate to first rendering call
         *   after eglSwapBuffers SYNC_DRAW_DONE starts to come before
         *   uxa copy, thus, missing last frame.
         */
        yagl_render_invalidate(0);
    }
#endif

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

#ifndef EGL_NATIVE_SURFACE_TIZEN
#define EGL_NATIVE_SURFACE_TIZEN 0x32A1
#endif

YAGL_API EGLImageKHR eglCreateImageKHR(EGLDisplay dpy_,
                                       EGLContext ctx_,
                                       EGLenum target,
                                       EGLClientBuffer buffer,
                                       const EGLint *attrib_list)
{
    EGLImageKHR ret = EGL_NO_IMAGE_KHR;
    struct yagl_client_interface *iface = NULL;
    struct yagl_display *dpy = NULL;
    struct yagl_context *ctx = NULL;
    struct yagl_native_drawable *native_buffer = NULL;
    struct yagl_image *image = NULL;
    int i = 0;

    YAGL_LOG_FUNC_ENTER(eglCreateImageKHR,
                        "dpy = %u, ctx = %u, target = %u, buffer = %p",
                        (yagl_host_handle)dpy_,
                        (yagl_host_handle)ctx_,
                        target,
                        buffer);

    iface = yagl_get_any_client_interface();

    if (!iface) {
        YAGL_SET_ERR(EGL_BAD_ALLOC);
        goto out;
    }

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
                                                        native_buffer,
                                                        iface);

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
#ifdef YAGL_PLATFORM_WAYLAND
        image = yagl_get_backend()->create_image_wl_buffer(dpy,
                                                           (struct wl_resource*)buffer,
                                                           iface);
#elif YAGL_PLATFORM_TIZEN
        image = yagl_get_backend()->create_image_wl_buffer(dpy,
                                                           buffer,
                                                           iface);
#endif

        if (!image) {
            goto out;
        }

        break;
    case EGL_GL_TEXTURE_2D_KHR:
        if (attrib_list) {
            while (attrib_list[i] != EGL_NONE) {
                switch (attrib_list[i]) {
                case EGL_IMAGE_PRESERVED_KHR:
                case EGL_GL_TEXTURE_LEVEL_KHR:
                    break;
                default:
                    YAGL_SET_ERR(EGL_BAD_ATTRIBUTE);
                    goto out;
                }

                i += 2;
            }
        }

        if (!yagl_validate_context(dpy, ctx_, &ctx)) {
            goto out;
        }

        image = yagl_get_backend()->create_image_gl_texture_2d(dpy,
                                                               ctx,
                                                               (yagl_object_name)VOIDP2INT(buffer),
                                                               iface);

        if (!image) {
            goto out;
        }

        break;
    case EGL_NATIVE_SURFACE_TIZEN:
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

        image = yagl_get_backend()->create_image_tizen_sfc(dpy,
                                                           buffer,
                                                           iface);

        if (!image) {
            goto out;
        }

        break;
    default:
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    if (!yagl_display_image_add(dpy, image)) {
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    ret = image->client_handle;

out:
    yagl_image_release(image);

    yagl_context_release(ctx);

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

YAGL_API EGLSyncKHR eglCreateSyncKHR(EGLDisplay dpy_, EGLenum type, const EGLint *attrib_list)
{
    EGLSyncKHR ret = EGL_NO_SYNC_KHR;
    struct yagl_display *dpy = NULL;
    struct yagl_fence *fence = NULL;

    YAGL_LOG_FUNC_ENTER(eglCreateSyncKHR, "dpy = %u, type = %u",
                        (yagl_host_handle)dpy_, type);

    if (type != EGL_SYNC_FENCE_KHR) {
        YAGL_SET_ERR(EGL_BAD_ATTRIBUTE);
        goto out;
    }

    if (attrib_list && (attrib_list[0] != EGL_NONE)) {
        YAGL_SET_ERR(EGL_BAD_ATTRIBUTE);
        goto out;
    }

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    fence = yagl_get_backend()->create_fence(dpy);

    if (!fence) {
        YAGL_SET_ERR(EGL_BAD_ACCESS);
        goto out;
    }

    yagl_display_fence_add(dpy, fence);

    yagl_transport_flush(yagl_get_transport(), &fence->base);

    ret = fence;

out:
    yagl_fence_release(fence);

    YAGL_LOG_FUNC_EXIT("%p", ret);

    return ret;
}

YAGL_API EGLBoolean eglDestroySyncKHR(EGLDisplay dpy_, EGLSyncKHR sync_)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_fence *fence = NULL;

    YAGL_LOG_FUNC_ENTER(eglDestroySyncKHR, "dpy = %u, sync = %p",
                        (yagl_host_handle)dpy_, sync_);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_fence(dpy, sync_, &fence)) {
        goto out;
    }

    if (!yagl_display_fence_remove(dpy, sync_)) {
        YAGL_LOG_ERROR("we're the one who destroy the fence, but it isn't there!");
        YAGL_SET_ERR(EGL_BAD_PARAMETER);
        goto out;
    }

    res = EGL_TRUE;

out:
    yagl_fence_release(fence);

    YAGL_LOG_FUNC_EXIT("%d", res);

    return res;
}

YAGL_API EGLint eglClientWaitSyncKHR(EGLDisplay dpy_, EGLSyncKHR sync_, EGLint flags, EGLTimeKHR timeout)
{
    EGLint res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_fence *fence = NULL;

    YAGL_LOG_FUNC_ENTER(eglClientWaitSyncKHR, "dpy = %u, sync = %p, flags = 0x%X, timeout = %u",
                        (yagl_host_handle)dpy_, sync_, flags, (uint32_t)timeout);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_fence(dpy, sync_, &fence)) {
        goto out;
    }

    if (timeout == 0) {
        res = fence->base.signaled(&fence->base) ? EGL_CONDITION_SATISFIED_KHR
                                                 : EGL_TIMEOUT_EXPIRED_KHR;
    } else if (fence->base.wait(&fence->base)) {
        res = EGL_CONDITION_SATISFIED_KHR;
    } else {
        YAGL_SET_ERR(EGL_BAD_ACCESS);
    }

out:
    yagl_fence_release(fence);

    YAGL_LOG_FUNC_EXIT("%d", res);

    return res;
}

YAGL_API EGLBoolean eglSignalSyncKHR(EGLDisplay dpy_, EGLSyncKHR sync_, EGLenum mode)
{
    YAGL_LOG_FUNC_ENTER(eglSignalSyncKHR, "dpy = %u, sync = %p, mode = 0x%X",
                        (yagl_host_handle)dpy_, sync_, mode);

    YAGL_SET_ERR(EGL_BAD_MATCH);

    YAGL_LOG_FUNC_EXIT("%d", EGL_FALSE);

    return EGL_FALSE;
}

YAGL_API EGLBoolean eglGetSyncAttribKHR(EGLDisplay dpy_, EGLSyncKHR sync_, EGLint attribute, EGLint *value)
{
    EGLBoolean res = EGL_FALSE;
    struct yagl_display *dpy = NULL;
    struct yagl_fence *fence = NULL;

    YAGL_LOG_FUNC_ENTER(eglGetSyncAttribKHR, "dpy = %u, sync = %p, attribute = 0x%X",
                        (yagl_host_handle)dpy_, sync_, attribute);

    if (!yagl_validate_display(dpy_, &dpy)) {
        goto out;
    }

    if (!yagl_validate_fence(dpy, sync_, &fence)) {
        goto out;
    }

    switch (attribute) {
    case EGL_SYNC_TYPE_KHR:
        *value = EGL_SYNC_FENCE_KHR;
        res = EGL_TRUE;
        break;
    case EGL_SYNC_STATUS_KHR:
        *value = fence->base.signaled(&fence->base) ? EGL_SIGNALED_KHR
                                                    : EGL_UNSIGNALED_KHR;
        res = EGL_TRUE;
        break;
    case EGL_SYNC_CONDITION_KHR:
        *value = EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR;
        res = EGL_TRUE;
        break;
    default:
        YAGL_SET_ERR(EGL_BAD_ATTRIBUTE);
        break;
    }

out:
    yagl_fence_release(fence);

    YAGL_LOG_FUNC_EXIT("%d", res);

    return res;
}


#ifdef YAGL_PLATFORM_WAYLAND
struct wl_display;
struct wl_resource;
#endif
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

#ifdef YAGL_PLATFORM_WAYLAND
    res = yagl_native_display_bind_wl_display(dpy->native_dpy, display);
#elif YAGL_PLATFORM_TIZEN
    res = yagl_native_display_bind_wl_display(dpy->display_id);
#endif

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

#ifdef YAGL_PLATFORM_WAYLAND
    res = yagl_native_display_unbind_wl_display(dpy->native_dpy);
#elif YAGL_PLATFORM_TIZEN
    res = yagl_native_display_unbind_wl_display(dpy->display_id);
#endif


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

#ifdef YAGL_PLATFORM_WAYLAND
    res = yagl_native_display_query_wl_buffer(dpy->native_dpy,
                                              buffer,
                                              attribute,
                                              value);
#elif YAGL_PLATFORM_TIZEN
    res = yagl_native_display_query_wl_buffer(dpy->display_id,
                                              buffer,
                                              attribute,
                                              value);
#endif


out:
    YAGL_LOG_FUNC_EXIT("%d", res);

    return res;
}

YAGL_API __eglMustCastToProperFunctionPointerType eglGetProcAddress(const char* procname)
{
    __eglMustCastToProperFunctionPointerType ret = NULL;

    YAGL_LOG_FUNC_ENTER(eglGetProcAddress, "procname = %s", procname);

    if (procname) {
        if (strncmp(procname, "gl", 2) == 0) {

            struct yagl_context *ctx = yagl_get_context();

            if (ctx) {
                switch (ctx->client_ctx->client_api) {
                case yagl_client_api_gles1:
                    ret = yagl_get_gles1_sym(procname);
                    break;
                case yagl_client_api_gles2:
                case yagl_client_api_gles3:
                    ret = yagl_get_gles2_sym(procname);
                    break;
                default:
                    break;
                }
            } else {
                /*
                 * Workaround for evas, which foolishly calls this without
                 * context being set.
                 */
                ret = yagl_get_gles2_sym(procname);
            }
        } else if (strncmp(procname, "egl", 3) == 0) {
            ret = dlsym(NULL, procname);
        }
    }

    YAGL_LOG_FUNC_EXIT("%p", ret);

    return ret;
}
