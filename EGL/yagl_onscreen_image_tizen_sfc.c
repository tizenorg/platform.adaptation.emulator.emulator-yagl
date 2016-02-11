/*
 * YaGL
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Vasily Ulyanov <v.ulyanov@samsung.com>
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

#include "yagl_onscreen_image_tizen_sfc.h"
#include "yagl_display.h"
#include "yagl_native_display.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_host_egl_calls.h"
#include "yagl_egl_state.h"
#include "yagl_state.h"
#include "yagl_client_interface.h"
#include "yagl_client_image.h"
#include "vigs.h"
#include <tbm_bufmgr.h>
#include <tbm_bufmgr_backend.h>
#include <tbm_surface.h>
#include <tbm_surface_internal.h>
#include <string.h>

static inline uint32_t yuv2argb(float y, float u, float v)
{
    int32_t r, g, b;

    r = y + 1.402 * (v - 128);
    g = y - 0.344 * (u - 128) - 0.714 * (v - 128);
    b = y + 1.772 * (u - 128);

    r = r < 0 ? 0 : (r > 255 ? 255 : r);
    g = g < 0 ? 0 : (g > 255 ? 255 : g);
    b = b < 0 ? 0 : (b > 255 ? 255 : b);

    return (0xff000000 | (r << 16) | (g << 8) | b);
}

/* TODO need to think about HW convertion */
static bool yagl_onscreen_image_tizen_sfc_convert(struct yagl_onscreen_image_tizen_sfc *image)
{
    uint32_t *dst;
    float y, u, v;
    int i, j;
    tbm_surface_info_s info;
    int ret;

    YAGL_LOG_FUNC_SET(yagl_onscreen_image_tizen_sfc_convert);

    if (!image->planar) {
        return true;
    }

    ret = tbm_surface_map(image->sfc, TBM_SURF_OPTION_READ, &info);

    if (ret != TBM_SURFACE_ERROR_NONE) {
        YAGL_LOG_ERROR("tbm_surface_map failed: %d", ret);
        return false;
    }

    ret = vigs_drm_surface_start_access(image->drm_sfc, VIGS_DRM_SAF_WRITE);

    if (ret) {
        YAGL_LOG_ERROR("vigs_drm_surface_start_access failed: %s", strerror(-ret));
        tbm_surface_unmap(image->sfc);
        return false;
    }

    dst = image->drm_sfc->gem.vaddr;
    dst += info.width * info.height - info.width;

    switch (info.format) {
    case TBM_FORMAT_NV21:
        for (i = 0; i < info.height; i++) {
            for (j = 0; j < info.width; j++) {
                y = info.planes[0].ptr[i * info.width + j];
                v = info.planes[1].ptr[i * info.width / 2 + (j & ~1) + 0];
                u = info.planes[1].ptr[i * info.width / 2 + (j & ~1) + 1];

                *(dst - i * info.width + j) = yuv2argb(y, u, v);
            }
        }
        break;
    case TBM_FORMAT_YUV420:
        for (i = 0; i < info.height; i++) {
            for (j = 0; j < info.width; j++) {
                y = info.planes[0].ptr[i * info.width + j];
                u = info.planes[1].ptr[(i / 2) * (info.width / 2) + (j / 2)];
                v = info.planes[2].ptr[(i / 2) * (info.width / 2) + (j / 2)];

                *(dst - i * info.width + j) = yuv2argb(y, u, v);
            }
        }
        break;
    }

    ret = vigs_drm_surface_end_access(image->drm_sfc, 1);

    if (ret) {
        YAGL_LOG_ERROR("vigs_drm_surface_end_access failed: %s", strerror(-ret));
    }

    tbm_surface_unmap(image->sfc);

    return true;
}

static void yagl_onscreen_image_tizen_sfc_update(struct yagl_image *image)
{
    struct yagl_onscreen_image_tizen_sfc *tizen_sfc_image =
        (struct yagl_onscreen_image_tizen_sfc *)image;

    yagl_onscreen_image_tizen_sfc_convert(tizen_sfc_image);
}

static void yagl_onscreen_image_tizen_sfc_destroy(struct yagl_ref *ref)
{
    struct yagl_onscreen_image_tizen_sfc *image = (struct yagl_onscreen_image_tizen_sfc *)ref;

    vigs_drm_gem_unref(&image->drm_sfc->gem);

    tbm_surface_internal_unref(image->sfc);

    yagl_image_cleanup(&image->base);

    yagl_free(image);
}

struct yagl_onscreen_image_tizen_sfc
    *yagl_onscreen_image_tizen_sfc_create(struct yagl_display *dpy,
                                          EGLClientBuffer buffer,
                                          struct yagl_client_interface *iface)
{
    EGLint error = 0;
    yagl_object_name tex_global_name = yagl_get_global_name();
    struct yagl_client_image *client_image;
    struct yagl_onscreen_image_tizen_sfc *image = NULL;
    struct vigs_drm_surface *drm_sfc = NULL;
    tbm_surface_h sfc;
    tbm_bo bo;
    tbm_surface_info_s info;
    bool planar;
    int ret;

    YAGL_LOG_FUNC_SET(yagl_onscreen_image_tizen_sfc_create);

    sfc = (tbm_surface_h)buffer;

    tbm_surface_internal_ref(sfc);

    ret = tbm_surface_get_info(sfc, &info);

    if (ret != TBM_SURFACE_ERROR_NONE) {
        YAGL_LOG_ERROR("tbm_surface_get_info failed: %d", ret);
        yagl_set_error(EGL_BAD_PARAMETER);
        goto fail;
    }

    switch (info.format) {
        case TBM_FORMAT_RGB888:
        case TBM_FORMAT_ARGB8888:
        case TBM_FORMAT_XRGB8888:
            planar = false;
            break;
        case TBM_FORMAT_NV21:
        case TBM_FORMAT_YUV420:
            planar = true;
            break;
        default:
            YAGL_LOG_ERROR("bad format: 0x%X", info.format);
            yagl_set_error(EGL_BAD_PARAMETER);
            goto fail;
    }

    if (planar) {
        ret = vigs_drm_surface_create(dpy->native_dpy->drm_dev,
                                      info.width,
                                      info.height,
                                      info.width * 4,
                                      vigs_drm_surface_bgra8888,
                                      0,
                                      &drm_sfc);

        if (ret) {
            YAGL_LOG_ERROR("vigs_drm_surface_create failed: %s", strerror(-ret));
            yagl_set_error(EGL_BAD_ALLOC);
            goto fail;
        }

        ret = vigs_drm_gem_get_name(&drm_sfc->gem);

        if (ret) {
            YAGL_LOG_ERROR("vigs_drm_gem_get_name failed: %s", strerror(-ret));
            yagl_set_error(EGL_BAD_ALLOC);
            goto fail;
        }

        ret = vigs_drm_gem_map(&drm_sfc->gem, 1);

        if (ret) {
            YAGL_LOG_ERROR("vigs_drm_gem_map failed: %s", strerror(-ret));
            yagl_set_error(EGL_BAD_ALLOC);
            goto fail;
        }
    } else {
        if (tbm_surface_internal_get_num_bos(sfc) < 1) {
            YAGL_LOG_ERROR("tbm_surface_internal_get_num_bos < 1");
            yagl_set_error(EGL_BAD_PARAMETER);
            goto fail;
        }

        bo = tbm_surface_internal_get_bo(sfc, 0);
        drm_sfc = bo ? tbm_backend_get_bo_priv(bo) : NULL;

        if (!drm_sfc) {
            YAGL_LOG_ERROR("drm_sfc is NULL");
            yagl_set_error(EGL_BAD_PARAMETER);
            goto fail;
        }

        vigs_drm_gem_ref(&drm_sfc->gem);
    }

    if (!yagl_host_eglCreateImageYAGL(tex_global_name,
                                      dpy->host_dpy,
                                      drm_sfc->id,
                                      &error)) {
        YAGL_LOG_ERROR("yagl_host_eglCreateImageYAGL failed: %d", error);
        yagl_set_error(error);
        goto fail;
    }

    client_image = iface->create_image(iface, tex_global_name);

    image = yagl_malloc0(sizeof(*image));

    yagl_image_init(&image->base,
                    &yagl_onscreen_image_tizen_sfc_destroy,
                    dpy,
                    (EGLImageKHR)INT2VOIDP(drm_sfc->gem.name),
                    client_image);

    yagl_client_image_release(client_image);

    image->base.update = &yagl_onscreen_image_tizen_sfc_update;
    image->sfc = sfc;
    image->planar = planar;
    image->drm_sfc = drm_sfc;

    YAGL_LOG_DEBUG("%ux%u/%u, sfc_id = %u, planar = %d (0x%X), num_planes = %u, size = %u",
                   info.width,
                   info.height,
                   info.bpp,
                   drm_sfc->id,
                   planar,
                   info.format,
                   info.num_planes,
                   info.size);

    return image;

fail:
    if (drm_sfc) {
        vigs_drm_gem_unref(&drm_sfc->gem);
    }

    yagl_free(image);

    tbm_surface_internal_unref(sfc);

    return NULL;
}
