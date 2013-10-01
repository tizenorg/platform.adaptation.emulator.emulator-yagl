#include "yagl_image.h"
#include "yagl_native_drawable.h"
#include "yagl_client_image.h"

void yagl_image_init(struct yagl_image *image,
                     yagl_ref_destroy_func destroy_func,
                     struct yagl_display *dpy,
                     EGLImageKHR client_handle,
                     struct yagl_client_image *client_image)
{
    yagl_resource_init(&image->res, destroy_func, 0);

    image->dpy = dpy;
    image->client_handle = client_handle;
    image->client_image = client_image;

    yagl_client_image_acquire(client_image);
}

void yagl_image_cleanup(struct yagl_image *image)
{
    yagl_client_image_release(image->client_image);

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
