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

#ifndef _YAGL_NATIVE_DRAWABLE_H_
#define _YAGL_NATIVE_DRAWABLE_H_

#include "yagl_export.h"
#include "yagl_native_types.h"

struct yagl_native_image;
struct vigs_drm_surface;

struct yagl_native_drawable
{
    struct yagl_native_display *dpy;

    yagl_os_drawable os_drawable;

    /*
     * This gets incremented in drawable invalidate handler.
     */
    uint32_t stamp;

    int (*get_buffer)(struct yagl_native_drawable */*drawable*/,
                      yagl_native_attachment /*attachment*/,
                      uint32_t */*buffer_name*/,
                      struct vigs_drm_surface **/*buffer_sfc*/);

#ifdef YAGL_PLATFORM_TIZEN
    int (*validate)(struct yagl_native_drawable */*drawable*/);
#endif

    int (*get_buffer_age)(struct yagl_native_drawable */*drawable*/);

    void (*swap_buffers)(struct yagl_native_drawable */*drawable*/);

    /*
     * 'width' and 'height' are here only because of DRI2.
     * DRI2 requires width and height to be passed to DRI2CopyRegion.
     */
    void (*wait)(struct yagl_native_drawable */*drawable*/,
                 uint32_t /*width*/,
                 uint32_t /*height*/);

    void (*copy_to_pixmap)(struct yagl_native_drawable */*drawable*/,
                           yagl_os_pixmap /*os_pixmap*/,
                           uint32_t /*from_x*/,
                           uint32_t /*from_y*/,
                           uint32_t /*to_x*/,
                           uint32_t /*to_y*/,
                           uint32_t /*width*/,
                           uint32_t /*height*/);

    void (*set_swap_interval)(struct yagl_native_drawable */*drawable*/,
                              int /*interval*/);

    void (*get_geometry)(struct yagl_native_drawable */*drawable*/,
                         uint32_t */*width*/,
                         uint32_t */*height*/,
                         uint32_t */*depth*/);

    struct yagl_native_image *(*get_image)(struct yagl_native_drawable */*drawable*/,
                                           uint32_t /*width*/,
                                           uint32_t /*height*/);

    void (*destroy)(struct yagl_native_drawable */*drawable*/);
};

void yagl_native_drawable_init(struct yagl_native_drawable *drawable,
                               struct yagl_native_display *dpy,
                               yagl_os_drawable os_drawable);

void yagl_native_drawable_cleanup(struct yagl_native_drawable *drawable);

#endif
