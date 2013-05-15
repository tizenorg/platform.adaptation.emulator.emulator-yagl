#include "yagl_onscreen_display.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include <sys/fcntl.h>

struct yagl_onscreen_display
    *yagl_onscreen_display_create(EGLNativeDisplayType display_id,
                                  Display *x_dpy,
                                  yagl_host_handle host_dpy)
{
    struct yagl_onscreen_display *tmp_dpy;
    struct yagl_onscreen_display *dpy = NULL;
    int ret;
    int event_base, error_base;
    int dri_major, dri_minor;
    char *dri_driver = NULL;
    char *dri_device = NULL;
    int drm_fd = -1;
    drm_magic_t magic;
    struct vigs_drm_device *drm_dev = NULL;

    YAGL_LOG_FUNC_SET(eglGetDisplay);

    tmp_dpy = yagl_malloc0(sizeof(*dpy));

    yagl_display_init(&tmp_dpy->base, display_id, x_dpy, host_dpy);

    if (!yagl_DRI2QueryExtension(x_dpy, &event_base, &error_base)) {
        YAGL_LOG_ERROR("DRI2QueryExtension failed");
        goto out;
    }

    YAGL_LOG_TRACE("DRI2QueryExtension returned %d %d",
                   event_base, error_base);

    if (!yagl_DRI2QueryVersion(x_dpy, &dri_major, &dri_minor)) {
        YAGL_LOG_ERROR("DRI2QueryVersion failed");
        goto out;
    }

    YAGL_LOG_TRACE("DRI2QueryVersion returned %d %d",
                   dri_major, dri_minor);

    if (!yagl_DRI2Connect(x_dpy,
                          RootWindow(x_dpy, DefaultScreen(x_dpy)),
                          &dri_driver,
                          &dri_device)) {
        YAGL_LOG_ERROR("DRI2Connect failed");
        goto out;
    }

    YAGL_LOG_TRACE("DRI2Connect returned %s %s",
                   dri_driver, dri_device);

    drm_fd = open(dri_device, O_RDWR);

    if (drm_fd < 0) {
        YAGL_LOG_ERROR("open(\"%s\") failed", dri_device);
        goto fail;
    }

    memset(&magic, 0, sizeof(magic));

    ret = drmGetMagic(drm_fd, &magic);

    if (ret != 0) {
        YAGL_LOG_ERROR("drmGetMagic failed: %s", strerror(-ret));
        goto fail;
    }

    if (!yagl_DRI2Authenticate(x_dpy,
                               RootWindow(x_dpy, DefaultScreen(x_dpy)),
                               magic)) {
        YAGL_LOG_ERROR("DRI2Authenticate failed");
        goto fail;
    }

    ret = vigs_drm_device_create(drm_fd, &drm_dev);

    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_device_create failed: %s", strerror(-ret));
        goto fail;
    }

    tmp_dpy->drm_fd = drm_fd;
    tmp_dpy->drm_dev = drm_dev;

    dpy = tmp_dpy;
    tmp_dpy = NULL;

    goto out;

fail:
    if (drm_dev) {
        vigs_drm_device_destroy(drm_dev);
    }
    if (drm_fd >= 0) {
        close(drm_fd);
    }
out:
    if (dri_driver) {
        Xfree(dri_driver);
    }
    if (dri_device) {
        Xfree(dri_device);
    }
    if (tmp_dpy) {
        yagl_display_cleanup(&tmp_dpy->base);
        yagl_free(tmp_dpy);
    }

    return dpy;
}

int yagl_onscreen_display_create_buffer(struct yagl_onscreen_display* dpy,
                                        Drawable d,
                                        unsigned int attachment,
                                        yagl_DRI2Buffer **buffer,
                                        yagl_winsys_id *id,
                                        uint32_t *width,
                                        uint32_t *height)
{
    int ret;
    unsigned int attachments[1] =
    {
        attachment
    };
    yagl_DRI2Buffer *tmp_buffer;
    int tmp_width, tmp_height, num_buffers;
    struct vigs_drm_surface *drm_sfc;

    YAGL_LOG_FUNC_ENTER(yagl_onscreen_display_create_buffer,
                        "dpy = %p, d = 0x%X, attachment = %u",
                        dpy,
                        d,
                        attachment);

    tmp_buffer = yagl_DRI2GetBuffers(dpy->base.x_dpy, d,
                                     &tmp_width, &tmp_height,
                                     &attachments[0],
                                     sizeof(attachments)/sizeof(attachments[0]),
                                     &num_buffers);
    if (!tmp_buffer) {
        YAGL_LOG_ERROR("DRI2GetBuffers failed for drawable 0x%X", d);
        YAGL_LOG_FUNC_EXIT(NULL);
        return 0;
    }

    ret = vigs_drm_surface_open(dpy->drm_dev, tmp_buffer->name, &drm_sfc);

    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_surface_open failed for drawable 0x%X: %s",
                       d,
                       strerror(-ret));
        Xfree(tmp_buffer);
        YAGL_LOG_FUNC_EXIT(NULL);
        return 0;
    }

    *buffer = tmp_buffer;
    *id = drm_sfc->id;
    *width = tmp_width;
    *height = tmp_height;

    vigs_drm_gem_unref(&drm_sfc->gem);

    YAGL_LOG_FUNC_EXIT("1");

    return 1;
}

void yagl_onscreen_display_destroy_buffer(yagl_DRI2Buffer *buffer)
{
    Xfree(buffer);
}
