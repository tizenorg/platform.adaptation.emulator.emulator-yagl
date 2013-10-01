#include "GL/gl.h"
#include "yagl_gles_image.h"
#include "yagl_malloc.h"
#include "yagl_host_gles_calls.h"

static void yagl_gles_image_update(struct yagl_client_image *image,
                                   uint32_t width,
                                   uint32_t height,
                                   uint32_t bpp,
                                   const void *pixels)
{
    struct yagl_gles_image *gles_image = (struct yagl_gles_image*)image;

    yagl_host_glUpdateOffscreenImageYAGL(gles_image->tex_global_name,
                                         width,
                                         height,
                                         bpp,
                                         pixels,
                                         width * height * bpp);
}

static void yagl_gles_image_destroy(struct yagl_ref *ref)
{
    struct yagl_gles_image *image = (struct yagl_gles_image*)ref;

    yagl_host_glDeleteObjects(&image->tex_global_name, 1);

    yagl_client_image_cleanup(&image->base);

    yagl_free(image);
}

struct yagl_gles_image *yagl_gles_image_create(yagl_object_name tex_global_name)
{
    struct yagl_gles_image *image;

    image = yagl_malloc0(sizeof(*image));

    yagl_client_image_init(&image->base, &yagl_gles_image_destroy);

    image->base.update = &yagl_gles_image_update;

    image->tex_global_name = tex_global_name;

    return image;
}

void yagl_gles_image_acquire(struct yagl_gles_image *image)
{
    if (image) {
        yagl_client_image_acquire(&image->base);
    }
}

void yagl_gles_image_release(struct yagl_gles_image *image)
{
    if (image) {
        yagl_client_image_release(&image->base);
    }
}
