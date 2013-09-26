#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include <assert.h>
#include "yagl_gles_utils.h"
#include "yagl_mem_gl.h"
#include "yagl_gles_context.h"

int yagl_host_glGetIntegerv(GLenum pname, GLint* params);

int yagl_gles_get_stride(GLsizei alignment,
                         GLsizei width,
                         GLenum format,
                         GLenum type,
                         GLsizei *stride)
{
    int num_components = 0;
    GLsizei bpp = 0;

    switch (format) {
    case GL_ALPHA:
        num_components = 1;
        break;
    case GL_RGB:
        num_components = 3;
        break;
    case GL_RGBA:
        num_components = 4;
        break;
    case GL_BGRA_EXT:
        num_components = 4;
        break;
    case GL_LUMINANCE:
        num_components = 1;
        break;
    case GL_LUMINANCE_ALPHA:
        num_components = 2;
        break;
    case GL_DEPTH_STENCIL_OES:
        if ((type == GL_FLOAT) || (type == GL_HALF_FLOAT_OES)) {
            return 0;
        }
        num_components = 1;
        break;
    case GL_DEPTH_COMPONENT:
        if ((type != GL_UNSIGNED_SHORT) && (type != GL_UNSIGNED_INT)) {
            return 0;
        }
        num_components = 1;
        break;
    default:
        return 0;
    }

    switch (type) {
    case GL_UNSIGNED_BYTE:
        bpp = num_components;
        break;
    case GL_UNSIGNED_SHORT_5_6_5:
        if (format != GL_RGB) {
            return 0;
        }
        bpp = 2;
        break;
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
        if (format != GL_RGBA) {
            return 0;
        }
        bpp = 2;
        break;
    case GL_UNSIGNED_INT_24_8_OES:
        bpp = num_components * 4;
        break;
    case GL_UNSIGNED_SHORT:
        if (format != GL_DEPTH_COMPONENT) {
            return 0;
        }
        bpp = num_components * 2;
        break;
    case GL_UNSIGNED_INT:
        if (format != GL_DEPTH_COMPONENT) {
            return 0;
        }
        bpp = num_components * 4;
        break;
    case GL_FLOAT:
        bpp = num_components * 4;
        break;
    case GL_HALF_FLOAT_OES:
        bpp = num_components * 2;
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

void yagl_update_vbo(void)
{
    struct yagl_gles_context *ctx = yagl_gles_context_get();

    if (!ctx || ctx->vbo_valid) {
        return;
    }

    ctx->vbo = yagl_get_integer(GL_ARRAY_BUFFER_BINDING);
    ctx->vbo_valid = 1;
}
