#include "GLES2/gl2.h"
#include "yagl_gles2_validate.h"

int yagl_gles2_is_shader_type_valid(GLenum type)
{
    switch (type) {
    case GL_VERTEX_SHADER:
    case GL_FRAGMENT_SHADER:
        return 1;
    default:
        return 0;
    }
}
