#ifndef _YAGL_GLES_FRAMEBUFFER_H_
#define _YAGL_GLES_FRAMEBUFFER_H_

#include "yagl_gles_types.h"
#include "yagl_object.h"

struct yagl_gles_texture;
struct yagl_gles_renderbuffer;

struct yagl_gles_framebuffer_attachment_state
{
    GLenum type;

    yagl_object_name local_name;

    GLenum textarget;
};

struct yagl_gles_framebuffer
{
    struct yagl_object base;

    yagl_object_name global_name;

    struct yagl_gles_framebuffer_attachment_state attachment_states[YAGL_NUM_GLES_FRAMEBUFFER_ATTACHMENTS];

    int was_bound;
};

struct yagl_gles_framebuffer *yagl_gles_framebuffer_create(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_framebuffer_acquire(struct yagl_gles_framebuffer *fb);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_framebuffer_release(struct yagl_gles_framebuffer *fb);

int yagl_gles_framebuffer_renderbuffer(struct yagl_gles_framebuffer *fb,
                                       GLenum target,
                                       GLenum attachment,
                                       GLenum renderbuffer_target,
                                       struct yagl_gles_renderbuffer *rb);

int yagl_gles_framebuffer_texture2d(struct yagl_gles_framebuffer *fb,
                                    GLenum target,
                                    GLenum attachment,
                                    GLenum textarget,
                                    GLint level,
                                    struct yagl_gles_texture *texture);

int yagl_gles_framebuffer_get_attachment_parameter(struct yagl_gles_framebuffer *fb,
                                                   GLenum attachment,
                                                   GLenum pname,
                                                   GLint *value);

/*
 * Assumes that 'target' is valid.
 */
void yagl_gles_framebuffer_bind(struct yagl_gles_framebuffer *fb,
                                GLenum target);

int yagl_gles_framebuffer_was_bound(struct yagl_gles_framebuffer *fb);

#endif
