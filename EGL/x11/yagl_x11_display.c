#include "yagl_x11_display.h"
#include "yagl_x11_drawable.h"
#include "yagl_x11_image.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_dri2.h"
#include "vigs.h"
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>

static int yagl_x11_display_dri2_authenticate(Display *x_dpy, uint32_t id)
{
    if (!yagl_DRI2Authenticate(x_dpy,
                               RootWindow(x_dpy, DefaultScreen(x_dpy)),
                               id)) {
        fprintf(stderr, "Critical error! Failed to DRI2Authenticate on YaGL display, DRI2 not enabled ?\n");
        return 0;
    }

    return 1;
}

static int yagl_x11_display_dri2_init(Display *x_dpy, char **dri_device)
{
    int ret;
    int event_base, error_base;
    int dri_major, dri_minor;
    char *dri_driver = NULL;
    int drm_fd = -1;
    drm_magic_t magic;

    YAGL_LOG_FUNC_SET(eglGetDisplay);

    *dri_device = NULL;

    if (!yagl_DRI2QueryExtension(x_dpy, &event_base, &error_base)) {
        fprintf(stderr, "Critical error! Failed to DRI2QueryExtension on YaGL display, DRI2 not enabled ?\n");
        goto fail;
    }

    YAGL_LOG_TRACE("DRI2QueryExtension returned %d %d",
                   event_base, error_base);

    if (!yagl_DRI2QueryVersion(x_dpy, &dri_major, &dri_minor)) {
        fprintf(stderr, "Critical error! Failed to DRI2QueryVersion on YaGL display, DRI2 not enabled ?\n");
        goto fail;
    }

    YAGL_LOG_TRACE("DRI2QueryVersion returned %d %d",
                   dri_major, dri_minor);

    if (!yagl_DRI2Connect(x_dpy,
                          RootWindow(x_dpy, DefaultScreen(x_dpy)),
                          &dri_driver,
                          dri_device)) {
        fprintf(stderr, "Critical error! Failed to DRI2Connect on YaGL display, DRI2 not enabled ?\n");
        goto fail;
    }

    YAGL_LOG_TRACE("DRI2Connect returned %s %s",
                   dri_driver, *dri_device);

    drm_fd = open(*dri_device, O_RDWR);

    if (drm_fd < 0) {
        fprintf(stderr, "Critical error! Failed to open(\"%s\"): %s\n", *dri_device, strerror(errno));
        goto fail;
    }

    memset(&magic, 0, sizeof(magic));

    ret = drmGetMagic(drm_fd, &magic);

    if (ret != 0) {
        fprintf(stderr, "Critical error! drmGetMagic failed: %s\n", strerror(-ret));
        goto fail;
    }

    if (!yagl_x11_display_dri2_authenticate(x_dpy, magic)) {
        goto fail;
    }

    goto out;

fail:
    if (drm_fd >= 0) {
        close(drm_fd);
        drm_fd = -1;
    }
    if (*dri_device) {
        Xfree(*dri_device);
    }
out:
    if (dri_driver) {
        Xfree(dri_driver);
    }

    return drm_fd;
}

static int yagl_x11_display_authenticate(struct yagl_native_display *dpy,
                                         uint32_t id)
{
    Display *x_dpy = YAGL_X11_DPY(dpy->os_dpy);

    return yagl_x11_display_dri2_authenticate(x_dpy, id);
}

static struct yagl_native_drawable
    *yagl_x11_display_wrap_window(struct yagl_native_display *dpy,
                                  yagl_os_window os_window)
{
    return yagl_x11_drawable_create(dpy, os_window, 0, 0);
}

static struct yagl_native_drawable
    *yagl_x11_display_wrap_pixmap(struct yagl_native_display *dpy,
                                  yagl_os_pixmap os_pixmap)
{
    return yagl_x11_drawable_create(dpy, os_pixmap, 0, 1);
}

static struct yagl_native_drawable
    *yagl_x11_display_create_pixmap(struct yagl_native_display *dpy,
                                    uint32_t width,
                                    uint32_t height,
                                    uint32_t depth)
{
    struct yagl_native_drawable *drawable;
    Display *x_dpy = YAGL_X11_DPY(dpy->os_dpy);
    Pixmap x_pixmap = XCreatePixmap(x_dpy,
                                    RootWindow(x_dpy, DefaultScreen(x_dpy)),
                                    width, height, depth);

    if (!x_pixmap) {
        return NULL;
    }

    drawable = yagl_x11_drawable_create(dpy, (yagl_os_pixmap)x_pixmap, 1, 1);

    if (!drawable) {
        XFreePixmap(x_dpy, x_pixmap);
        return NULL;
    }

    return drawable;
}

static struct yagl_native_image
    *yagl_x11_display_create_image(struct yagl_native_display *dpy,
                                   uint32_t width,
                                   uint32_t height,
                                   uint32_t depth)
{
    return yagl_x11_image_create(dpy, width, height, depth);
}

static int yagl_x11_display_get_visual(struct yagl_native_display *dpy,
                                       int *visual_id,
                                       int *visual_type)
{
    Display *x_dpy = YAGL_X11_DPY(dpy->os_dpy);
    int screen;
    XVisualInfo vi;

    screen = XScreenNumberOfScreen(XDefaultScreenOfDisplay(x_dpy));

    /*
     * 24-bit is the highest supported by soft framebuffer.
     */
    if (!XMatchVisualInfo(x_dpy, screen, 24, TrueColor, &vi)) {
        return 0;
    }

    *visual_id = XVisualIDFromVisual(vi.visual);
    *visual_type = TrueColor;

    return 1;
}

static void yagl_x11_display_update_wl_server(struct yagl_native_display *dpy)
{
        return ;
}

static void yagl_x11_display_destroy(struct yagl_native_display *dpy)
{
    struct yagl_x11_display *x11_dpy = (struct yagl_x11_display*)dpy;
    Display *x_dpy = YAGL_X11_DPY(dpy->os_dpy);

    if (dpy->drm_dev) {
        int fd = dpy->drm_dev->fd;
        vigs_drm_device_destroy(dpy->drm_dev);
        close(fd);
    }

    yagl_native_display_cleanup(dpy);

    if (x11_dpy->own_dpy) {
        XCloseDisplay(x_dpy);
    }

    yagl_free(x11_dpy);
}

struct yagl_native_display *yagl_x11_display_create(struct yagl_native_platform *platform,
                                                    yagl_os_display os_dpy,
                                                    int own_dpy,
                                                    int enable_drm)
{
    Display *x_dpy = YAGL_X11_DPY(os_dpy);
    struct yagl_x11_display *dpy;
    int xmajor;
    int xminor;
    Bool pixmaps;
    struct vigs_drm_device *drm_dev = NULL;
    char *dri_device = NULL;
    int ret;

    YAGL_LOG_FUNC_SET(eglGetDisplay);

    dpy = yagl_malloc0(sizeof(*dpy));

    dpy->own_dpy = own_dpy;

    dpy->xshm_images_supported = XShmQueryVersion(x_dpy,
                                                  &xmajor,
                                                  &xminor,
                                                  &pixmaps);
    dpy->xshm_pixmaps_supported = pixmaps;

    YAGL_LOG_DEBUG("XShm images are%s supported, version %d, %d (pixmaps = %d)",
                   (dpy->xshm_images_supported ? "" : " NOT"),
                   xmajor,
                   xminor,
                   pixmaps);

    if (enable_drm) {
        int drm_fd = yagl_x11_display_dri2_init(x_dpy, &dri_device);

        if (drm_fd < 0) {
            yagl_free(dpy);
            return NULL;
        }

        ret = vigs_drm_device_create(drm_fd, &drm_dev);

        if (ret != 0) {
            fprintf(stderr,
                    "Critical error! vigs_drm_device_create failed: %s\n",
                    strerror(-ret));
            close(drm_fd);
            Xfree(dri_device);
            yagl_free(dpy);
            return NULL;
        }
    }

    yagl_native_display_init(&dpy->base,
                             platform,
                             os_dpy,
                             drm_dev,
                             dri_device);

    dpy->base.authenticate = &yagl_x11_display_authenticate;
    dpy->base.wrap_window = &yagl_x11_display_wrap_window;
    dpy->base.wrap_pixmap = &yagl_x11_display_wrap_pixmap;
    dpy->base.create_pixmap = &yagl_x11_display_create_pixmap;
    dpy->base.create_image = &yagl_x11_display_create_image;
    dpy->base.get_visual = &yagl_x11_display_get_visual;
    dpy->base.update_wl_server = &yagl_x11_display_update_wl_server;
    dpy->base.destroy = &yagl_x11_display_destroy;

    if (dri_device) {
        Xfree(dri_device);
    }

    return &dpy->base;
}
