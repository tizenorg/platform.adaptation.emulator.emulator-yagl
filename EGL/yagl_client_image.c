#include "yagl_client_image.h"
#include "yagl_egl_state.h"
#include "yagl_display.h"
#include "yagl_context.h"
#include "yagl_image.h"

void yagl_client_image_init(struct yagl_client_image *image,
                            yagl_ref_destroy_func destroy_func)
{
    yagl_object_init(&image->base, destroy_func);
}

void yagl_client_image_cleanup(struct yagl_client_image *image)
{
    yagl_object_cleanup(&image->base);
}

void yagl_client_image_acquire(struct yagl_client_image *image)
{
    if (image) {
        yagl_object_acquire(&image->base);
    }
}

void yagl_client_image_release(struct yagl_client_image *image)
{
    if (image) {
        yagl_object_release(&image->base);
    }
}

struct yagl_client_image *yagl_acquire_client_image(yagl_host_handle handle)
{
    struct yagl_context *ctx = yagl_get_context();
    struct yagl_image *image;
    struct yagl_client_image *client_image;

    if (!ctx) {
        return NULL;
    }

    image = yagl_display_image_acquire(ctx->dpy, (EGLImageKHR)handle);

    if (!image) {
        return NULL;
    }

    image->update(image);

    client_image = image->client_image;

    yagl_client_image_acquire(client_image);
    yagl_image_release(image);

    return client_image;
}
