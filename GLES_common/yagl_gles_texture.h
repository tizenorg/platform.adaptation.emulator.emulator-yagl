#ifndef _YAGL_GLES_TEXTURE_H_
#define _YAGL_GLES_TEXTURE_H_

#include "yagl_types.h"
#include "yagl_object.h"

#define YAGL_NS_TEXTURE 1

struct yagl_gles_image;
struct yagl_tex_image_binding;

struct yagl_gles_texture
{
    struct yagl_object base;

    yagl_object_name global_name;

    GLenum target;

    GLenum internalformat;

    GLboolean immutable;

    /*
     * Non-NULL if it's an EGLImage/eglBindTexImage target.
     */
    struct yagl_gles_image *image;

    /*
     * Non-NULL if it's an eglBindTexImage target.
     */
    struct yagl_tex_image_binding *binding;
};

struct yagl_gles_texture *yagl_gles_texture_create(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_texture_acquire(struct yagl_gles_texture *texture);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_texture_release(struct yagl_gles_texture *texture);

/*
 * Assumes that 'target' is valid.
 */
int yagl_gles_texture_bind(struct yagl_gles_texture *texture,
                           GLenum target);

void yagl_gles_texture_set_internalformat(struct yagl_gles_texture *texture,
                                          GLenum internalformat);

void yagl_gles_texture_set_immutable(struct yagl_gles_texture *texture,
                                     GLenum internalformat);

void yagl_gles_texture_set_image(struct yagl_gles_texture *texture,
                                 struct yagl_gles_image *image);

void yagl_gles_texture_unset_image(struct yagl_gles_texture *texture);

void yagl_gles_texture_bind_tex_image(struct yagl_gles_texture *texture,
                                      struct yagl_gles_image *image,
                                      struct yagl_tex_image_binding *binding);

/*
 * Can be called with an arbitrary context being set, careful.
 */
void yagl_gles_texture_release_tex_image(struct yagl_gles_texture *texture);

#endif
