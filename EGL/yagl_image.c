#include "yagl_image.h"
#include "yagl_native_drawable.h"

static void yagl_gles_image_update(struct yagl_gles_image *image)
{
    struct yagl_image *egl_image =
        yagl_containerof(image, struct yagl_image, gles_image);

    egl_image->update(egl_image);
}

void yagl_image_init(struct yagl_image *image,
                     yagl_ref_destroy_func destroy_func,
                     yagl_host_handle host_handle,
                     struct yagl_display *dpy,
                     EGLImageKHR client_handle)
{
    yagl_resource_init(&image->res, destroy_func, host_handle);

    image->dpy = dpy;
    image->client_handle = client_handle;
    image->gles_image.host_image = host_handle;
    image->gles_image.update = &yagl_gles_image_update;
}

void yagl_image_cleanup(struct yagl_image *image)
{
    yagl_resource_cleanup(&image->res);
}

void yagl_image_acquire(struct yagl_image *image)
{
    if (image) {
        yagl_resource_acquire(&image->res);
    }
}

void yagl_image_release(struct yagl_image *image)
{
    if (image) {
        yagl_resource_release(&image->res);
    }
}
