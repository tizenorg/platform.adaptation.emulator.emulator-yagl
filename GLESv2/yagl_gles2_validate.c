#include "GLES2/gl2.h"
#include "yagl_gles2_validate.h"

int yagl_gles2_is_array_param_valid(GLenum pname)
{
    switch (pname) {
    case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
    case GL_CURRENT_VERTEX_ATTRIB:
        return 1;
    default:
        return 0;
    }
}

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
