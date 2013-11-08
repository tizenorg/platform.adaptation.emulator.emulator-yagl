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
