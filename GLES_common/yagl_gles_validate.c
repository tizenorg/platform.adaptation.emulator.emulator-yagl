#include "GL/gl.h"
#include "yagl_gles_validate.h"

int yagl_gles_is_buffer_usage_valid(GLenum usage)
{
    switch (usage) {
    case GL_STREAM_DRAW:
    case GL_STATIC_DRAW:
    case GL_DYNAMIC_DRAW:
        return 1;
    default:
        return 0;
    }
}

int yagl_gles_is_blend_equation_valid(GLenum mode)
{
    switch (mode) {
    case GL_FUNC_ADD:
    case GL_FUNC_SUBTRACT:
    case GL_FUNC_REVERSE_SUBTRACT:
        return 1;
    default:
        return 0;
    }
}

int yagl_gles_is_blend_func_valid(GLenum func)
{
    switch (func) {
    case GL_ZERO:
    case GL_ONE:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_CONSTANT_COLOR:
    case GL_ONE_MINUS_CONSTANT_COLOR:
    case GL_CONSTANT_ALPHA:
    case GL_ONE_MINUS_CONSTANT_ALPHA:
    case GL_SRC_ALPHA_SATURATE:
        return 1;
    default:
        return 0;
    }
}

int yagl_gles_is_cull_face_mode_valid(GLenum mode)
{
    switch (mode) {
    case GL_FRONT:
    case GL_BACK:
    case GL_FRONT_AND_BACK:
        return 1;
    default:
        return 0;
    }
}

int yagl_gles_is_depth_func_valid(GLenum func)
{
    switch (func) {
    case GL_NEVER:
    case GL_LESS:
    case GL_EQUAL:
    case GL_LEQUAL:
    case GL_GREATER:
    case GL_NOTEQUAL:
    case GL_GEQUAL:
    case GL_ALWAYS:
        return 1;
    default:
        return 0;
    }
}

int yagl_gles_is_front_face_mode_valid(GLenum mode)
{
    switch (mode) {
    case GL_CW:
    case GL_CCW:
        return 1;
    default:
        return 0;
    }
}

int yagl_gles_is_alignment_valid(GLint alignment)
{
    switch (alignment) {
    case 1:
    case 2:
    case 4:
    case 8:
        return 1;
    default:
        return 0;
    }
}

int yagl_gles_get_index_size(GLenum type, int *index_size)
{
    switch (type) {
    case GL_UNSIGNED_BYTE:
        *index_size = 1;
        break;
    case GL_UNSIGNED_SHORT:
        *index_size = 2;
        break;
    case GL_UNSIGNED_INT:
        *index_size = 4;
        break;
    default:
        return 0;
    }
    return 1;
}

int yagl_gles_validate_texture_target(GLenum target,
    yagl_gles_texture_target *texture_target)
{
    switch (target) {
    case GL_TEXTURE_2D:
        *texture_target = yagl_gles_texture_target_2d;
        break;
    case GL_TEXTURE_CUBE_MAP:
        *texture_target = yagl_gles_texture_target_cubemap;
        break;
    default:
        return 0;
    }

    return 1;
}

int yagl_gles_validate_framebuffer_attachment(GLenum attachment,
    int num_color_attachments,
    yagl_gles_framebuffer_attachment *framebuffer_attachment)
{
    switch (attachment) {
    case GL_DEPTH_ATTACHMENT:
        *framebuffer_attachment = yagl_gles_framebuffer_attachment_depth;
        break;
    case GL_STENCIL_ATTACHMENT:
        *framebuffer_attachment = yagl_gles_framebuffer_attachment_stencil;
        break;
    default:
        if ((attachment >= GL_COLOR_ATTACHMENT0) &&
            (attachment <= (GL_COLOR_ATTACHMENT0 + num_color_attachments - 1))) {
            *framebuffer_attachment = yagl_gles_framebuffer_attachment_color0 +
                                      (attachment - GL_COLOR_ATTACHMENT0);
            break;
        }
        return 0;
    }

    return 1;
}

int yagl_gles_validate_texture_target_squash(GLenum target,
    GLenum *squashed_target)
{
    switch (target) {
    case GL_TEXTURE_2D:
        *squashed_target = GL_TEXTURE_2D;
        break;
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        *squashed_target = GL_TEXTURE_CUBE_MAP;
        break;
    default:
        return 0;
    }

    return 1;
}
