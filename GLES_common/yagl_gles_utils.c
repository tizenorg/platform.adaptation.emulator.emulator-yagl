#include "GL/gl.h"
#include "GL/glext.h"
#include "yagl_gles_utils.h"
#include "yagl_state.h"
#include <assert.h>

/*
 * We can't include GLES2/gl2ext.h here
 */
#define GL_HALF_FLOAT_OES 0x8D61

/*
 * 1.0f in half-float.
 */
#define YAGL_HALF_FLOAT_1_0 0x3C00

GLenum yagl_gles_get_actual_type(GLenum type)
{
    switch (type) {
    case GL_HALF_FLOAT_OES:
        return GL_HALF_FLOAT;
    default:
        return type;
    }
}

GLint yagl_gles_get_actual_internalformat(GLint internalformat)
{
    switch (internalformat) {
    case GL_BGRA:
    case GL_ALPHA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
        return GL_RGBA;
    default:
        return internalformat;
    }
}

GLint yagl_gles_get_actual_format(GLint format)
{
    switch (format) {
    case GL_ALPHA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
        return GL_BGRA;
    default:
        return format;
    }
}

const GLvoid *yagl_gles_convert_to_host(GLsizei alignment,
                                        GLsizei width,
                                        GLsizei height,
                                        GLsizei depth,
                                        GLenum format,
                                        GLenum type,
                                        const GLvoid *pixels)
{
    GLsizei bpc, d, i, j, converted_stride;
    uint8_t *tmp, *iter;

    if (!pixels) {
        return pixels;
    }

    type = yagl_gles_get_actual_type(type);

    switch (type) {
    case GL_FLOAT:
        bpc = 4;
        break;
    case GL_HALF_FLOAT:
        bpc = 2;
        break;
    case GL_UNSIGNED_BYTE:
    default:
        bpc = 1;
        break;
    }

    converted_stride = ((width * bpc * 4) + alignment - 1) & ~(alignment - 1);

    switch (format) {
    case GL_ALPHA:
        tmp = iter = yagl_get_tmp_buffer(converted_stride * height * depth);
        switch (type) {
        case GL_UNSIGNED_BYTE:
            for (d = 0; d < depth; ++d) {
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        *(uint32_t*)(iter + j * 4) = (uint32_t)(*(uint8_t*)(pixels + j)) << 24;
                    }
                    pixels += (width + alignment - 1) & ~(alignment - 1);
                    iter += converted_stride;
                }
            }
            break;
        case GL_FLOAT:
            for (d = 0; d < depth; ++d) {
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        *(uint32_t*)(iter + j * 16 + 0) = 0;
                        *(uint32_t*)(iter + j * 16 + 4) = 0;
                        *(uint32_t*)(iter + j * 16 + 8) = 0;
                        *(uint32_t*)(iter + j * 16 + 12) = *(uint32_t*)(pixels + j * 4);
                    }
                    pixels += (width * 4 + alignment - 1) & ~(alignment - 1);
                    iter += converted_stride;
                }
            }
            break;
        case GL_HALF_FLOAT:
            for (d = 0; d < depth; ++d) {
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        *(uint16_t*)(iter + j * 8 + 0) = 0;
                        *(uint16_t*)(iter + j * 8 + 2) = 0;
                        *(uint16_t*)(iter + j * 8 + 4) = 0;
                        *(uint16_t*)(iter + j * 8 + 6) = *(uint16_t*)(pixels + j * 2);
                    }
                    pixels += (width * 2 + alignment - 1) & ~(alignment - 1);
                    iter += converted_stride;
                }
            }
            break;
        default:
            assert(0);
            break;
        }
        break;
    case GL_LUMINANCE:
        tmp = iter = yagl_get_tmp_buffer(converted_stride * height * depth);
        switch (type) {
        case GL_UNSIGNED_BYTE:
            for (d = 0; d < depth; ++d) {
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        uint32_t l = *(uint8_t*)(pixels + j);
                        *(uint32_t*)(iter + j * 4) = (l << 0) |
                                                     (l << 8) |
                                                     (l << 16) |
                                                     (255U << 24);
                    }
                    pixels += (width + alignment - 1) & ~(alignment - 1);
                    iter += converted_stride;
                }
            }
            break;
        case GL_FLOAT:
            for (d = 0; d < depth; ++d) {
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        uint32_t l = *(uint32_t*)(pixels + j * 4);
                        *(uint32_t*)(iter + j * 16 + 0) = l;
                        *(uint32_t*)(iter + j * 16 + 4) = l;
                        *(uint32_t*)(iter + j * 16 + 8) = l;
                        *(GLfloat*)(iter + j * 16 + 12) = 1.0f;
                    }
                    pixels += (width * 4 + alignment - 1) & ~(alignment - 1);
                    iter += converted_stride;
                }
            }
            break;
        case GL_HALF_FLOAT:
            for (d = 0; d < depth; ++d) {
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        uint16_t l = *(uint16_t*)(pixels + j * 2);
                        *(uint16_t*)(iter + j * 8 + 0) = l;
                        *(uint16_t*)(iter + j * 8 + 2) = l;
                        *(uint16_t*)(iter + j * 8 + 4) = l;
                        *(uint16_t*)(iter + j * 8 + 6) = YAGL_HALF_FLOAT_1_0;
                    }
                    pixels += (width * 2 + alignment - 1) & ~(alignment - 1);
                    iter += converted_stride;
                }
            }
            break;
        default:
            assert(0);
            break;
        }
        break;
    case GL_LUMINANCE_ALPHA:
        tmp = iter = yagl_get_tmp_buffer(converted_stride * height * depth);
        switch (type) {
        case GL_UNSIGNED_BYTE:
            for (d = 0; d < depth; ++d) {
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        uint32_t l = *(uint8_t*)(pixels + j * 2 + 0);
                        uint32_t a = *(uint8_t*)(pixels + j * 2 + 1);
                        *(uint32_t*)(iter + j * 4) = (l << 0) |
                                                     (l << 8) |
                                                     (l << 16) |
                                                     (a << 24);
                    }
                    pixels += (width * 2 + alignment - 1) & ~(alignment - 1);
                    iter += converted_stride;
                }
            }
            break;
        case GL_FLOAT:
            for (d = 0; d < depth; ++d) {
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        uint32_t l = *(uint32_t*)(pixels + j * 8 + 0);
                        uint32_t a = *(uint32_t*)(pixels + j * 8 + 4);
                        *(uint32_t*)(iter + j * 16 + 0) = l;
                        *(uint32_t*)(iter + j * 16 + 4) = l;
                        *(uint32_t*)(iter + j * 16 + 8) = l;
                        *(uint32_t*)(iter + j * 16 + 12) = a;
                    }
                    pixels += (width * 8 + alignment - 1) & ~(alignment - 1);
                    iter += converted_stride;
                }
            }
            break;
        case GL_HALF_FLOAT:
            for (d = 0; d < depth; ++d) {
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        uint16_t l = *(uint16_t*)(pixels + j * 4 + 0);
                        uint16_t a = *(uint16_t*)(pixels + j * 4 + 2);
                        *(uint16_t*)(iter + j * 8 + 0) = l;
                        *(uint16_t*)(iter + j * 8 + 2) = l;
                        *(uint16_t*)(iter + j * 8 + 4) = l;
                        *(uint16_t*)(iter + j * 8 + 6) = a;
                    }
                    pixels += (width * 4 + alignment - 1) & ~(alignment - 1);
                    iter += converted_stride;
                }
            }
            break;
        default:
            assert(0);
            break;
        }
        break;
    default:
        return pixels;
    }

    return tmp;
}

GLvoid *yagl_gles_convert_from_host_start(GLsizei alignment,
                                          GLsizei width,
                                          GLsizei height,
                                          GLenum format,
                                          GLenum type,
                                          GLvoid *pixels)
{
    int num_components;
    GLsizei bpp;

    switch (format) {
    case GL_ALPHA:
        num_components = 1;
        break;
    case GL_LUMINANCE:
        num_components = 1;
        break;
    case GL_LUMINANCE_ALPHA:
        num_components = 2;
        break;
    default:
        return pixels;
    }

    type = yagl_gles_get_actual_type(type);

    switch (type) {
    case GL_FLOAT:
        bpp = num_components * 4;
        break;
    case GL_HALF_FLOAT:
        bpp = num_components * 2;
        break;
    case GL_UNSIGNED_BYTE:
    default:
        bpp = num_components;
        break;
    }

    return yagl_get_tmp_buffer(
        (((width * bpp) + alignment - 1) & ~(alignment - 1)) * height);
}

void yagl_gles_convert_from_host_end(GLsizei alignment,
                                     GLsizei width,
                                     GLsizei height,
                                     GLenum format,
                                     GLenum type,
                                     GLsizei stride,
                                     const GLvoid *pixels_from,
                                     GLvoid *pixels_to)
{
    GLsizei i, j;

    type = yagl_gles_get_actual_type(type);

    switch (format) {
    case GL_ALPHA:
        switch (type) {
        case GL_UNSIGNED_BYTE:
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint8_t*)(pixels_to + j) = *(uint32_t*)(pixels_from + j * 4) >> 24;
                }
                pixels_from += stride;
                pixels_to += (width + alignment - 1) & ~(alignment - 1);
            }
            break;
        case GL_FLOAT:
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint32_t*)(pixels_to + j * 4) = *(uint32_t*)(pixels_from + j * 16 + 12);
                }
                pixels_from += stride;
                pixels_to += (width * 4 + alignment - 1) & ~(alignment - 1);
            }
            break;
        case GL_HALF_FLOAT:
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint16_t*)(pixels_to + j * 2) = *(uint16_t*)(pixels_from + j * 8 + 6);
                }
                pixels_from += stride;
                pixels_to += (width * 2 + alignment - 1) & ~(alignment - 1);
            }
            break;
        default:
            assert(0);
            break;
        }
        break;
    case GL_LUMINANCE:
        switch (type) {
        case GL_UNSIGNED_BYTE:
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint8_t*)(pixels_to + j) = *(uint32_t*)(pixels_from + j * 4) & 0xFF;
                }
                pixels_from += stride;
                pixels_to += (width + alignment - 1) & ~(alignment - 1);
            }
            break;
        case GL_FLOAT:
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint32_t*)(pixels_to + j * 4) = *(uint32_t*)(pixels_from + j * 16);
                }
                pixels_from += stride;
                pixels_to += (width * 4 + alignment - 1) & ~(alignment - 1);
            }
            break;
        case GL_HALF_FLOAT:
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint16_t*)(pixels_to + j * 2) = *(uint16_t*)(pixels_from + j * 8);
                }
                pixels_from += stride;
                pixels_to += (width * 2 + alignment - 1) & ~(alignment - 1);
            }
            break;
        default:
            assert(0);
            break;
        }
        break;
    case GL_LUMINANCE_ALPHA:
        switch (type) {
        case GL_UNSIGNED_BYTE:
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint8_t*)(pixels_to + j * 2 + 0) = *(uint32_t*)(pixels_from + j * 4) & 0xFF;
                    *(uint8_t*)(pixels_to + j * 2 + 1) = *(uint32_t*)(pixels_from + j * 4) >> 24;
                }
                pixels_from += stride;
                pixels_to += (width * 2 + alignment - 1) & ~(alignment - 1);
            }
            break;
        case GL_FLOAT:
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint32_t*)(pixels_to + j * 8 + 0) = *(uint32_t*)(pixels_from + j * 16);
                    *(uint32_t*)(pixels_to + j * 8 + 4) = *(uint32_t*)(pixels_from + j * 16 + 12);
                }
                pixels_from += stride;
                pixels_to += (width * 8 + alignment - 1) & ~(alignment - 1);
            }
            break;
        case GL_HALF_FLOAT:
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint16_t*)(pixels_to + j * 4 + 0) = *(uint16_t*)(pixels_from + j * 8);
                    *(uint16_t*)(pixels_to + j * 4 + 2) = *(uint16_t*)(pixels_from + j * 8 + 6);
                }
                pixels_from += stride;
                pixels_to += (width * 4 + alignment - 1) & ~(alignment - 1);
            }
            break;
        default:
            assert(0);
            break;
        }
        break;
    default:
        break;
    }
}
