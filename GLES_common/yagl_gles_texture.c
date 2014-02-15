#include "GL/gl.h"
#include "yagl_gles_texture.h"
#include "yagl_gles_image.h"
#include "yagl_tex_image_binding.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"
#include <assert.h>

/*
 * We can't include GLES2/gl2ext.h here
 */
#define GL_HALF_FLOAT_OES 0x8D61

static void yagl_gles_texture_destroy(struct yagl_ref *ref)
{
    struct yagl_gles_texture *texture = (struct yagl_gles_texture*)ref;

    if (texture->binding) {
        texture->binding->unbind(texture->binding);
        texture->binding = NULL;
    }

    if (texture->image) {
        yagl_gles_image_release(texture->image);
        texture->image = NULL;
    } else {
        yagl_host_glDeleteObjects(&texture->global_name, 1);
    }

    yagl_object_cleanup(&texture->base);

    yagl_free(texture);
}

struct yagl_gles_texture *yagl_gles_texture_create(void)
{
    struct yagl_gles_texture *texture;

    texture = yagl_malloc0(sizeof(*texture));

    yagl_object_init(&texture->base, &yagl_gles_texture_destroy);

    texture->global_name = yagl_get_global_name();
    texture->min_filter = GL_NEAREST_MIPMAP_LINEAR;
    texture->mag_filter = GL_LINEAR;

    yagl_host_glGenTextures(&texture->global_name, 1);

    return texture;
}

void yagl_gles_texture_acquire(struct yagl_gles_texture *texture)
{
    if (texture) {
        yagl_object_acquire(&texture->base);
    }
}

void yagl_gles_texture_release(struct yagl_gles_texture *texture)
{
    if (texture) {
        yagl_object_release(&texture->base);
    }
}

int yagl_gles_texture_bind(struct yagl_gles_texture *texture,
                           GLenum target)
{
    if (texture->target && (texture->target != target)) {
        return 0;
    }

    yagl_host_glBindTexture(target, texture->global_name);

    texture->target = target;

    return 1;
}

void yagl_gles_texture_set_internalformat(struct yagl_gles_texture *texture,
                                          GLenum internalformat,
                                          GLenum type)
{
    texture->internalformat = internalformat;

    switch (type) {
    case GL_FLOAT:
    case GL_HALF_FLOAT:
    case GL_HALF_FLOAT_OES:
        texture->is_float = 1;
        break;
    default:
        texture->is_float = 0;
        break;
    }
}

void yagl_gles_texture_set_immutable(struct yagl_gles_texture *texture,
                                     GLenum internalformat,
                                     GLenum type)
{
    texture->immutable = GL_TRUE;
    texture->internalformat = internalformat;

    switch (type) {
    case GL_FLOAT:
    case GL_HALF_FLOAT:
    case GL_HALF_FLOAT_OES:
        texture->is_float = 1;
        break;
    default:
        texture->is_float = 0;
        break;
    }
}

int yagl_gles_texture_color_renderable(struct yagl_gles_texture *texture)
{
    if (!texture->is_float) {
        return 1;
    }

    switch (texture->min_filter) {
    case GL_NEAREST:
    case GL_NEAREST_MIPMAP_NEAREST:
        break;
    default:
        return 0;
    }

    switch (texture->mag_filter) {
    case GL_NEAREST:
        break;
    default:
        return 0;
    }

    return 1;
}

void yagl_gles_texture_set_image(struct yagl_gles_texture *texture,
                                 struct yagl_gles_image *image)
{
    assert(texture->target);
    assert(image);

    if (texture->binding) {
        texture->binding->unbind(texture->binding);
        texture->binding = NULL;
    }

    if (texture->image == image) {
        return;
    }

    yagl_gles_image_acquire(image);

    if (texture->image) {
        yagl_gles_image_release(texture->image);
    } else {
        yagl_host_glDeleteObjects(&texture->global_name, 1);
    }

    texture->global_name = image->tex_global_name;
    texture->image = image;

    yagl_host_glBindTexture(texture->target, texture->global_name);
}

void yagl_gles_texture_unset_image(struct yagl_gles_texture *texture)
{
    if (texture->binding) {
        texture->binding->unbind(texture->binding);
        texture->binding = NULL;
    }

    if (texture->image) {
        yagl_gles_image_release(texture->image);
        texture->image = NULL;

        texture->global_name = yagl_get_global_name();

        yagl_host_glGenTextures(&texture->global_name, 1);
        yagl_host_glBindTexture(texture->target, texture->global_name);
    }
}

void yagl_gles_texture_bind_tex_image(struct yagl_gles_texture *texture,
                                      struct yagl_gles_image *image,
                                      struct yagl_tex_image_binding *binding)
{
    assert(texture->target);
    assert(image);

    if (texture->binding) {
        texture->binding->unbind(texture->binding);
        texture->binding = NULL;
    }

    yagl_gles_image_acquire(image);

    if (texture->image) {
        yagl_gles_image_release(texture->image);
    } else {
        yagl_host_glDeleteObjects(&texture->global_name, 1);
    }

    texture->global_name = image->tex_global_name;
    texture->image = image;
    texture->binding = binding;

    yagl_host_glBindTexture(texture->target, texture->global_name);
}

/*
 * Can be called with an arbitrary context being set, careful.
 */
void yagl_gles_texture_release_tex_image(struct yagl_gles_texture *texture)
{
    assert(texture->image);

    texture->binding = NULL;

    yagl_gles_image_release(texture->image);
    texture->image = NULL;

    texture->global_name = yagl_get_global_name();

    /*
     * Must ensure context on host for this call!
     */
    yagl_host_glGenTextures(&texture->global_name, 1);
}
