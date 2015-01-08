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

#ifndef _YAGL_ONSCREEN_SURFACE_H_
#define _YAGL_ONSCREEN_SURFACE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_surface.h"

struct vigs_drm_surface;

struct yagl_onscreen_surface
{
    struct yagl_surface base;

    /*
     * Backing pixmap for PBuffer surfaces. NULL otherwise.
     */
    struct yagl_native_drawable *tmp_pixmap;

    /*
     * For widow surfaces this is yagl_native_attachment_back.
     * For pixmap surfaces this is yagl_native_attachment_front.
     * For pbuffer surfaces this is yagl_native_attachment_front of 'tmp_pixmap'.
     *
     * TODO: For window surfaces we also need to support
     * yagl_native_attachment_front.
     */
    struct vigs_drm_surface *drm_sfc;

    /*
     * Last value of 'base.native_drawable->stamp'.
     */
    uint32_t last_stamp;
};

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_window(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         struct yagl_native_drawable *native_window,
                                         const EGLint *attrib_list);

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pixmap(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         struct yagl_native_drawable *native_pixmap,
                                         const EGLint *attrib_list);

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pbuffer(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          const EGLint *attrib_list);

#endif
