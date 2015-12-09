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

#include "yagl_gbm_display.h"
#include "yagl_gbm_window.h"
#include "yagl_gbm_pixmap.h"
#include "yagl_native_display.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_gbm.h"
#include "vigs.h"
#include <xf86drm.h>
#include <libudev.h>
#include <string.h>
#include <stdlib.h>

static struct udev_device *udev_device_new_from_fd(struct udev *udev, int fd)
{
    struct udev_device *device;
    struct stat buf;

    if (fstat(fd, &buf) < 0) {
        return NULL;
    }

    device = udev_device_new_from_devnum(udev, 'c', buf.st_rdev);

    if (!device) {
        return NULL;
    }

    return device;
}

static char *get_device_name_for_fd(int fd)
{
    struct udev *udev;
    struct udev_device *device;
    const char *const_device_name;
    char *device_name = NULL;

    udev = udev_new();

    device = udev_device_new_from_fd(udev, fd);

    if (!device) {
        return NULL;
    }

    const_device_name = udev_device_get_devnode(device);

    if (!const_device_name) {
        goto out;
    }

    device_name = strdup(const_device_name);

out:
    udev_device_unref(device);
    udev_unref(udev);

    return device_name;
}

static int yagl_gbm_display_authenticate(struct yagl_native_display *dpy,
                                         uint32_t id)
{
    struct gbm_device *gbm_dpy = YAGL_GBM_DPY(dpy->os_dpy);

    return drmAuthMagic(gbm_dpy->drm_dev->fd, id);
}

static struct yagl_native_drawable
    *yagl_gbm_display_wrap_window(struct yagl_native_display *dpy,
                                  yagl_os_window os_window)
{
    return yagl_gbm_window_create(dpy, os_window);
}

static struct yagl_native_drawable
    *yagl_gbm_display_wrap_pixmap(struct yagl_native_display *dpy,
                                  yagl_os_pixmap os_pixmap)
{
    return yagl_gbm_pixmap_create(dpy, os_pixmap);
}

static struct yagl_native_drawable
    *yagl_gbm_display_create_pixmap(struct yagl_native_display *dpy,
                                    uint32_t width,
                                    uint32_t height,
                                    uint32_t depth)
{
    return NULL;
}

static struct yagl_native_image
    *yagl_gbm_display_create_image(struct yagl_native_display *dpy,
                                   uint32_t width,
                                   uint32_t height,
                                   uint32_t depth)
{
    return NULL;
}

static int yagl_gbm_display_get_visual(struct yagl_native_display *dpy,
                                       int *visual_id,
                                       int *visual_type)
{
    /* XXX: W/A for evas
       efl GIT: platform/upstream/efl
       evas src: /src/modules/evas/engines/gl_drm/evas_outbuf.c
       evas func.: _evas_outbuf_egl_setup()
       reference: https://www.khronos.org/registry/egl/
                          extensions/KHR/EGL_KHR_platform_gbm.txt
                  read 'New Behavior' section
    */
    *visual_id = GBM_FORMAT_ARGB8888;
    *visual_type = 0;

    return 1;
}

static void yagl_gbm_display_update_wl_server(struct yagl_native_display *dpy)
{
#ifdef YAGL_PLATFORM_WAYLAND
    struct gbm_device *gbm_dev = YAGL_GBM_DPY(dpy->os_dpy);
    gbm_dev->wl_drm = dpy->wl_server_drm;
#endif
}

static void yagl_gbm_display_destroy(struct yagl_native_display *dpy)
{
    yagl_native_display_cleanup(dpy);

    yagl_free(dpy);
}

struct yagl_native_display
    *yagl_gbm_display_create(struct yagl_native_platform *platform,
                             yagl_os_display os_dpy)
{
    struct gbm_device *gbm_dpy = YAGL_GBM_DPY(os_dpy);
    struct yagl_native_display *dpy;
    char *device_name = get_device_name_for_fd(gbm_dpy->drm_dev->fd);

    if (!device_name) {
        return NULL;
    }

    dpy = yagl_malloc0(sizeof(*dpy));

    yagl_native_display_init(dpy,
                             platform,
                             os_dpy,
                             gbm_dpy->drm_dev,
                             device_name);

    dpy->authenticate = &yagl_gbm_display_authenticate;
    dpy->wrap_window = &yagl_gbm_display_wrap_window;
    dpy->wrap_pixmap = &yagl_gbm_display_wrap_pixmap;
    dpy->create_pixmap = &yagl_gbm_display_create_pixmap;
    dpy->create_image = &yagl_gbm_display_create_image;
    dpy->get_visual = &yagl_gbm_display_get_visual;
    dpy->update_wl_server = &yagl_gbm_display_update_wl_server;
    dpy->destroy = &yagl_gbm_display_destroy;

    free(device_name);

    return dpy;
}
