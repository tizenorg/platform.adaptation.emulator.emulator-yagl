#include <GLES/gl.h>
#include <GLES/glext.h>
#include <assert.h>
#include "yagl_gles_utils.h"
#include "yagl_host_gles1_calls.h"
#include "yagl_mem_gl.h"

int yagl_gles_get_stride(GLsizei alignment,
                         GLsizei width,
                         GLenum format,
                         GLenum type,
                         GLsizei *stride)
{
    int per_byte;
    GLsizei bpp;

    switch (type) {
    case GL_UNSIGNED_BYTE:
        per_byte = 1;
        break;
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
        per_byte = 0;
        break;
    default:
        return 0;
    }

    switch (format) {
    case GL_ALPHA:
        bpp = 1;
        break;
    case GL_RGB:
        bpp = (per_byte ? 3 : 2);
        break;
    case GL_RGBA:
        bpp = (per_byte ? 4 : 2);
        break;
    case GL_LUMINANCE:
        bpp = 1;
        break;
    case GL_LUMINANCE_ALPHA:
        bpp = 2;
        break;
    default:
        return 0;
    }

    assert(alignment > 0);

    *stride = ((width * bpp) + alignment - 1) & ~(alignment - 1);

    return 1;
}

int yagl_get_el_size(GLenum type, int *el_size)
{
    switch (type) {
    case GL_BYTE:
        *el_size = 1;
        break;
    case GL_UNSIGNED_BYTE:
        *el_size = 1;
        break;
    case GL_SHORT:
        *el_size = 2;
        break;
    case GL_UNSIGNED_SHORT:
        *el_size = 2;
        break;
    case GL_FLOAT:
        *el_size = 4;
        break;
    case GL_FIXED:
        *el_size = 4;
        break;
    default:
        return 0;
    }
    return 1;
}

GLint yagl_get_integer(GLenum pname)
{
    GLint param = 0;

    do {
        yagl_mem_probe_write_GLint(&param);
    } while (!yagl_host_glGetIntegerv(pname, &param));

    return param;
}
