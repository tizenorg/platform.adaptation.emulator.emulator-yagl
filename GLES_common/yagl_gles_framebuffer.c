#include "GL/gl.h"
#include "yagl_gles_framebuffer.h"
#include "yagl_gles_renderbuffer.h"
#include "yagl_gles_texture.h"
#include "yagl_gles_validate.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"

static void yagl_gles_framebuffer_destroy(struct yagl_ref *ref)
{
    struct yagl_gles_framebuffer *fb = (struct yagl_gles_framebuffer*)ref;

    yagl_host_glDeleteObjects(&fb->global_name, 1);

    yagl_object_cleanup(&fb->base);

    yagl_free(fb);
}

struct yagl_gles_framebuffer *yagl_gles_framebuffer_create(void)
{
    struct yagl_gles_framebuffer *fb;
    int i;

    fb = yagl_malloc0(sizeof(*fb));

    yagl_object_init(&fb->base, &yagl_gles_framebuffer_destroy);

    fb->global_name = yagl_get_global_name();

    for (i = 0; i < YAGL_NUM_GLES_FRAMEBUFFER_ATTACHMENTS; ++i) {
        fb->attachment_states[i].type = GL_NONE;
    }

    yagl_host_glGenFramebuffers(&fb->global_name, 1);

    return fb;
}

void yagl_gles_framebuffer_acquire(struct yagl_gles_framebuffer *fb)
{
    if (fb) {
        yagl_object_acquire(&fb->base);
    }
}

void yagl_gles_framebuffer_release(struct yagl_gles_framebuffer *fb)
{
    if (fb) {
        yagl_object_release(&fb->base);
    }
}

int yagl_gles_framebuffer_renderbuffer(struct yagl_gles_framebuffer *fb,
                                       GLenum target,
                                       GLenum attachment,
                                       GLenum renderbuffer_target,
                                       struct yagl_gles_renderbuffer *rb)
{
    yagl_gles_framebuffer_attachment framebuffer_attachment;

    if (!yagl_gles_validate_framebuffer_attachment(attachment,
                                                   &framebuffer_attachment)) {
        return 0;
    }

    if (rb && (renderbuffer_target != GL_RENDERBUFFER)) {
        return 0;
    }

    if (rb) {
        fb->attachment_states[framebuffer_attachment].type = GL_RENDERBUFFER;
        fb->attachment_states[framebuffer_attachment].local_name = rb->base.local_name;
    } else {
        fb->attachment_states[framebuffer_attachment].type = GL_NONE;
        fb->attachment_states[framebuffer_attachment].local_name = 0;
    }

    yagl_host_glFramebufferRenderbuffer(target,
                                        attachment,
                                        renderbuffer_target,
                                        (rb ? rb->global_name : 0));

    return 1;
}

int yagl_gles_framebuffer_texture2d(struct yagl_gles_framebuffer *fb,
                                    GLenum target,
                                    GLenum attachment,
                                    GLenum textarget,
                                    GLint level,
                                    struct yagl_gles_texture *texture)
{
    yagl_gles_framebuffer_attachment framebuffer_attachment;
    GLenum squashed_textarget;

    if (!yagl_gles_validate_framebuffer_attachment(attachment,
                                                   &framebuffer_attachment)) {
        return 0;
    }

    if (texture && (level != 0)) {
        return 0;
    }

    if (!yagl_gles_validate_texture_target_squash(textarget,
                                                  &squashed_textarget)) {
        return 0;
    }

    if (texture && (yagl_gles_texture_get_target(texture) != squashed_textarget)) {
        return 0;
    }

    if (texture) {
        fb->attachment_states[framebuffer_attachment].type = GL_TEXTURE;
        fb->attachment_states[framebuffer_attachment].local_name = texture->base.local_name;
        fb->attachment_states[framebuffer_attachment].textarget = textarget;
    } else {
        fb->attachment_states[framebuffer_attachment].type = GL_NONE;
        fb->attachment_states[framebuffer_attachment].local_name = 0;
    }

    yagl_host_glFramebufferTexture2D(target,
                                     attachment,
                                     textarget,
                                     (texture ? texture->global_name : 0),
                                     level);

    return 1;
}

int yagl_gles_framebuffer_get_attachment_parameter(struct yagl_gles_framebuffer *fb,
                                                   GLenum attachment,
                                                   GLenum pname,
                                                   GLint *value)
{
    yagl_gles_framebuffer_attachment framebuffer_attachment;

    if (!yagl_gles_validate_framebuffer_attachment(attachment,
                                                   &framebuffer_attachment)) {
        return 0;
    }

    switch (pname) {
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
        *value = fb->attachment_states[framebuffer_attachment].type;
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
        *value = fb->attachment_states[framebuffer_attachment].local_name;
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
        if (fb->attachment_states[framebuffer_attachment].type == GL_TEXTURE) {
            *value = 0;
        } else {
            return 0;
        }
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
        if (fb->attachment_states[framebuffer_attachment].type == GL_TEXTURE) {
            *value =
                (fb->attachment_states[framebuffer_attachment].textarget == GL_TEXTURE_2D)
                ? 0 : fb->attachment_states[framebuffer_attachment].textarget;
        } else {
            return 0;
        }
        break;
    default:
        return 0;
    }

    return 1;
}

void yagl_gles_framebuffer_bind(struct yagl_gles_framebuffer *fb,
                                GLenum target)
{
    if (!fb) {
        yagl_host_glBindFramebuffer(target, 0);
        return;
    }

    yagl_host_glBindFramebuffer(target, fb->global_name);

    fb->was_bound = 1;
}

int yagl_gles_framebuffer_was_bound(struct yagl_gles_framebuffer *fb)
{
    return fb->was_bound;
}
