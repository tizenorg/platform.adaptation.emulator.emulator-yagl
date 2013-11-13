#include "GLES3/gl3.h"
#include "yagl_gles3_validate.h"

int yagl_gles3_is_uniform_param_valid(GLenum pname)
{
    switch (pname) {
    case GL_UNIFORM_TYPE:
    case GL_UNIFORM_SIZE:
    case GL_UNIFORM_NAME_LENGTH:
    case GL_UNIFORM_BLOCK_INDEX:
    case GL_UNIFORM_OFFSET:
    case GL_UNIFORM_ARRAY_STRIDE:
    case GL_UNIFORM_MATRIX_STRIDE:
    case GL_UNIFORM_IS_ROW_MAJOR:
        return 1;
    default:
        return 0;
    }
}

int yagl_gles3_is_transform_feedback_buffer_mode_valid(GLenum buffer_mode)
{
    switch (buffer_mode) {
    case GL_INTERLEAVED_ATTRIBS:
    case GL_SEPARATE_ATTRIBS:
        return 1;
    default:
        return 0;
    }
}

int yagl_gles3_is_primitive_mode_valid(GLenum primitive_mode)
{
    switch (primitive_mode) {
    case GL_POINTS:
    case GL_LINES:
    case GL_TRIANGLES:
        return 1;
    default:
        return 0;
    }
}
