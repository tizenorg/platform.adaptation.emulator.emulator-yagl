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
    return 0;
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
    dpy->destroy = &yagl_gbm_display_destroy;

    free(device_name);

    return dpy;
}
