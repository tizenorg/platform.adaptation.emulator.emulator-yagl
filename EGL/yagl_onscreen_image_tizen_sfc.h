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

#ifndef _YAGL_ONSCREEN_IMAGE_TIZEN_SFC_H_
#define _YAGL_ONSCREEN_IMAGE_TIZEN_SFC_H_

#include "yagl_image.h"
#include "EGL/egl.h"
#include <tbm_surface.h>

struct yagl_client_interface;
struct vigs_drm_surface;

struct yagl_onscreen_image_tizen_sfc
{
    struct yagl_image base;

    tbm_surface_h sfc;

    bool planar;

    struct vigs_drm_surface *drm_sfc;
};

struct yagl_onscreen_image_tizen_sfc
    *yagl_onscreen_image_tizen_sfc_create(struct yagl_display *dpy,
                                          EGLClientBuffer buffer,
                                          struct yagl_client_interface *iface);

#endif
