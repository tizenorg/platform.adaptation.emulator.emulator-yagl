#include "GLES2/gl2.h"
#include "yagl_export.h"
#include "yagl_client_interface.h"
#include "yagl_gles2_context.h"
#include "yagl_gles3_context.h"
#include "yagl_gles_image.h"
#include "yagl_gles_texture.h"

static struct yagl_client_context *yagl_gles2_create_ctx(struct yagl_client_interface *iface,
                                                         yagl_client_api client_api,
                                                         struct yagl_sharegroup *sg)
{
    switch (client_api) {
    case yagl_client_api_gles2:
        return yagl_gles2_context_create(sg);
    case yagl_client_api_gles3:
        return yagl_gles3_context_create(sg);
    default:
        return NULL;
    }
}

static struct yagl_client_image
    *yagl_gles2_create_image(struct yagl_client_interface *iface,
                             yagl_object_name tex_global_name)
{
    return &yagl_gles_image_create(tex_global_name)->base;
}

static void yagl_gles2_release_tex_image(struct yagl_client_interface *iface,
                                         void *cookie)
{
    struct yagl_gles_texture *texture = cookie;

    yagl_gles_texture_release_tex_image(texture);
}

YAGL_API struct yagl_client_interface yagl_gles2_interface =
{
    .create_ctx = &yagl_gles2_create_ctx,
    .create_image = &yagl_gles2_create_image,
    .release_tex_image = &yagl_gles2_release_tex_image
};
