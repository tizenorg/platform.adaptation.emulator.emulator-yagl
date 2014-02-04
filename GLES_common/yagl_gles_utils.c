#include "GL/gl.h"
#include "GL/glext.h"
#include "yagl_gles_utils.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"
#include <assert.h>

/*
 * We can't include GLES2/gl2ext.h here
 */
#define GL_HALF_FLOAT_OES 0x8D61

/*
 * 1.0f in half-float.
 */
#define YAGL_HALF_FLOAT_1_0 0x3C00

static GLsizei yagl_gles_get_stride(const struct yagl_gles_pixelstore* ps,
                                    GLsizei width,
                                    GLsizei height,
                                    GLsizei bpp,
                                    GLsizei *image_stride)
{
    GLsizei num_columns = (ps->row_length > 0) ? ps->row_length : width;
    GLsizei row_stride = ((num_columns * bpp) + ps->alignment - 1) & ~(ps->alignment - 1);

    if (image_stride) {
        GLsizei num_rows = (ps->image_height > 0) ? ps->image_height : height;

        *image_stride = row_stride * num_rows;
    }

    return row_stride;
}

GLsizei yagl_gles_get_offset(const struct yagl_gles_pixelstore* ps,
                             GLsizei width,
                             GLsizei height,
                             GLsizei depth,
                             GLsizei bpp,
                             GLsizei *size)
{
    GLsizei row_stride, image_stride;

    row_stride = yagl_gles_get_stride(ps, width, height,
                                      bpp, &image_stride);

    *size = (width * bpp) +
            ((height - 1) * row_stride) +
            ((depth - 1) * image_stride);

    return (ps->skip_images * image_stride) +
           (ps->skip_rows * row_stride) +
           (ps->skip_pixels * bpp);
}

void yagl_gles_reset_unpack(const struct yagl_gles_pixelstore* ps)
{
    if (ps->alignment != 1) {
        yagl_host_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    if (ps->row_length > 0) {
        yagl_host_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }

    if (ps->image_height > 0) {
        yagl_host_glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    }
}

void yagl_gles_set_unpack(const struct yagl_gles_pixelstore* ps)
{
    if (ps->alignment != 1) {
        yagl_host_glPixelStorei(GL_UNPACK_ALIGNMENT, ps->alignment);
    }

    if (ps->row_length > 0) {
        yagl_host_glPixelStorei(GL_UNPACK_ROW_LENGTH, ps->row_length);
    }

    if (ps->image_height > 0) {
        yagl_host_glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, ps->image_height);
    }
}

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

const GLvoid *yagl_gles_convert_to_host(const struct yagl_gles_pixelstore* ps,
                                        GLsizei width,
                                        GLsizei height,
                                        GLsizei depth,
                                        GLenum format,
                                        GLenum type,
                                        const GLvoid *pixels)
{
    GLsizei bpc, d, i, j;
    GLsizei row_stride, image_stride;
    GLsizei converted_row_stride, converted_image_stride;
    uint8_t *tmp, *iter;
    const GLvoid *from;

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

    converted_row_stride = yagl_gles_get_stride(ps, width, height,
                                                bpc * 4,
                                                &converted_image_stride);

    switch (format) {
    case GL_ALPHA:
        tmp = yagl_get_tmp_buffer(converted_image_stride * depth);
        switch (type) {
        case GL_UNSIGNED_BYTE:
            row_stride = yagl_gles_get_stride(ps, width, height,
                                              1, &image_stride);
            for (d = 0; d < depth; ++d) {
                from = pixels;
                iter = tmp + (converted_image_stride * d);
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        *(uint32_t*)(iter + j * 4) = (uint32_t)(*(uint8_t*)(from + j)) << 24;
                    }
                    from += row_stride;
                    iter += converted_row_stride;
                }
                pixels += image_stride;
            }
            break;
        case GL_FLOAT:
            row_stride = yagl_gles_get_stride(ps, width, height,
                                              4, &image_stride);
            for (d = 0; d < depth; ++d) {
                from = pixels;
                iter = tmp + (converted_image_stride * d);
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        *(uint32_t*)(iter + j * 16 + 0) = 0;
                        *(uint32_t*)(iter + j * 16 + 4) = 0;
                        *(uint32_t*)(iter + j * 16 + 8) = 0;
                        *(uint32_t*)(iter + j * 16 + 12) = *(uint32_t*)(from + j * 4);
                    }
                    from += row_stride;
                    iter += converted_row_stride;
                }
                pixels += image_stride;
            }
            break;
        case GL_HALF_FLOAT:
            row_stride = yagl_gles_get_stride(ps, width, height,
                                              2, &image_stride);
            for (d = 0; d < depth; ++d) {
                from = pixels;
                iter = tmp + (converted_image_stride * d);
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        *(uint16_t*)(iter + j * 8 + 0) = 0;
                        *(uint16_t*)(iter + j * 8 + 2) = 0;
                        *(uint16_t*)(iter + j * 8 + 4) = 0;
                        *(uint16_t*)(iter + j * 8 + 6) = *(uint16_t*)(from + j * 2);
                    }
                    from += row_stride;
                    iter += converted_row_stride;
                }
                pixels += image_stride;
            }
            break;
        default:
            assert(0);
            break;
        }
        break;
    case GL_LUMINANCE:
        tmp = yagl_get_tmp_buffer(converted_image_stride * depth);
        switch (type) {
        case GL_UNSIGNED_BYTE:
            row_stride = yagl_gles_get_stride(ps, width, height,
                                              1, &image_stride);
            for (d = 0; d < depth; ++d) {
                from = pixels;
                iter = tmp + (converted_image_stride * d);
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        uint32_t l = *(uint8_t*)(from + j);
                        *(uint32_t*)(iter + j * 4) = (l << 0) |
                                                     (l << 8) |
                                                     (l << 16) |
                                                     (255U << 24);
                    }
                    from += row_stride;
                    iter += converted_row_stride;
                }
                pixels += image_stride;
            }
            break;
        case GL_FLOAT:
            row_stride = yagl_gles_get_stride(ps, width, height,
                                              4, &image_stride);
            for (d = 0; d < depth; ++d) {
                from = pixels;
                iter = tmp + (converted_image_stride * d);
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        uint32_t l = *(uint32_t*)(from + j * 4);
                        *(uint32_t*)(iter + j * 16 + 0) = l;
                        *(uint32_t*)(iter + j * 16 + 4) = l;
                        *(uint32_t*)(iter + j * 16 + 8) = l;
                        *(GLfloat*)(iter + j * 16 + 12) = 1.0f;
                    }
                    from += row_stride;
                    iter += converted_row_stride;
                }
                pixels += image_stride;
            }
            break;
        case GL_HALF_FLOAT:
            row_stride = yagl_gles_get_stride(ps, width, height,
                                              2, &image_stride);
            for (d = 0; d < depth; ++d) {
                from = pixels;
                iter = tmp + (converted_image_stride * d);
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        uint16_t l = *(uint16_t*)(from + j * 2);
                        *(uint16_t*)(iter + j * 8 + 0) = l;
                        *(uint16_t*)(iter + j * 8 + 2) = l;
                        *(uint16_t*)(iter + j * 8 + 4) = l;
                        *(uint16_t*)(iter + j * 8 + 6) = YAGL_HALF_FLOAT_1_0;
                    }
                    from += row_stride;
                    iter += converted_row_stride;
                }
                pixels += image_stride;
            }
            break;
        default:
            assert(0);
            break;
        }
        break;
    case GL_LUMINANCE_ALPHA:
        tmp = yagl_get_tmp_buffer(converted_image_stride * depth);
        switch (type) {
        case GL_UNSIGNED_BYTE:
            row_stride = yagl_gles_get_stride(ps, width, height,
                                              2, &image_stride);
            for (d = 0; d < depth; ++d) {
                from = pixels;
                iter = tmp + (converted_image_stride * d);
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        uint32_t l = *(uint8_t*)(from + j * 2 + 0);
                        uint32_t a = *(uint8_t*)(from + j * 2 + 1);
                        *(uint32_t*)(iter + j * 4) = (l << 0) |
                                                     (l << 8) |
                                                     (l << 16) |
                                                     (a << 24);
                    }
                    from += row_stride;
                    iter += converted_row_stride;
                }
                pixels += image_stride;
            }
            break;
        case GL_FLOAT:
            row_stride = yagl_gles_get_stride(ps, width, height,
                                              8, &image_stride);
            for (d = 0; d < depth; ++d) {
                from = pixels;
                iter = tmp + (converted_image_stride * d);
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        uint32_t l = *(uint32_t*)(from + j * 8 + 0);
                        uint32_t a = *(uint32_t*)(from + j * 8 + 4);
                        *(uint32_t*)(iter + j * 16 + 0) = l;
                        *(uint32_t*)(iter + j * 16 + 4) = l;
                        *(uint32_t*)(iter + j * 16 + 8) = l;
                        *(uint32_t*)(iter + j * 16 + 12) = a;
                    }
                    from += row_stride;
                    iter += converted_row_stride;
                }
                pixels += image_stride;
            }
            break;
        case GL_HALF_FLOAT:
            row_stride = yagl_gles_get_stride(ps, width, height,
                                              4, &image_stride);
            for (d = 0; d < depth; ++d) {
                from = pixels;
                iter = tmp + (converted_image_stride * d);
                for (i = 0; i < height; ++i) {
                    for (j = 0; j < width; ++j) {
                        uint16_t l = *(uint16_t*)(from + j * 4 + 0);
                        uint16_t a = *(uint16_t*)(from + j * 4 + 2);
                        *(uint16_t*)(iter + j * 8 + 0) = l;
                        *(uint16_t*)(iter + j * 8 + 2) = l;
                        *(uint16_t*)(iter + j * 8 + 4) = l;
                        *(uint16_t*)(iter + j * 8 + 6) = a;
                    }
                    from += row_stride;
                    iter += converted_row_stride;
                }
                pixels += image_stride;
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

GLvoid *yagl_gles_convert_from_host_start(const struct yagl_gles_pixelstore* ps,
                                          GLsizei width,
                                          GLsizei height,
                                          GLsizei bpp,
                                          int need_convert,
                                          GLvoid *pixels,
                                          GLsizei *stride)
{
    if (need_convert) {
        *stride = yagl_gles_get_stride(ps, width, height, bpp, NULL);

        return yagl_get_tmp_buffer(*stride * height);
    } else {
        return pixels;
    }
}

void yagl_gles_convert_from_host_end(const struct yagl_gles_pixelstore* ps,
                                     GLsizei width,
                                     GLsizei height,
                                     GLenum format,
                                     GLenum type,
                                     GLsizei stride,
                                     const GLvoid *pixels_from,
                                     GLvoid *pixels_to)
{
    GLsizei i, j, to_stride;

    type = yagl_gles_get_actual_type(type);

    switch (format) {
    case GL_ALPHA:
        switch (type) {
        case GL_UNSIGNED_BYTE:
            to_stride = yagl_gles_get_stride(ps, width, height, 1, NULL);
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint8_t*)(pixels_to + j) = *(uint32_t*)(pixels_from + j * 4) >> 24;
                }
                pixels_from += stride;
                pixels_to += to_stride;
            }
            break;
        case GL_FLOAT:
            to_stride = yagl_gles_get_stride(ps, width, height, 4, NULL);
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint32_t*)(pixels_to + j * 4) = *(uint32_t*)(pixels_from + j * 16 + 12);
                }
                pixels_from += stride;
                pixels_to += to_stride;
            }
            break;
        case GL_HALF_FLOAT:
            to_stride = yagl_gles_get_stride(ps, width, height, 2, NULL);
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint16_t*)(pixels_to + j * 2) = *(uint16_t*)(pixels_from + j * 8 + 6);
                }
                pixels_from += stride;
                pixels_to += to_stride;
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
            to_stride = yagl_gles_get_stride(ps, width, height, 1, NULL);
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint8_t*)(pixels_to + j) = *(uint32_t*)(pixels_from + j * 4) & 0xFF;
                }
                pixels_from += stride;
                pixels_to += to_stride;
            }
            break;
        case GL_FLOAT:
            to_stride = yagl_gles_get_stride(ps, width, height, 4, NULL);
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint32_t*)(pixels_to + j * 4) = *(uint32_t*)(pixels_from + j * 16);
                }
                pixels_from += stride;
                pixels_to += to_stride;
            }
            break;
        case GL_HALF_FLOAT:
            to_stride = yagl_gles_get_stride(ps, width, height, 2, NULL);
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint16_t*)(pixels_to + j * 2) = *(uint16_t*)(pixels_from + j * 8);
                }
                pixels_from += stride;
                pixels_to += to_stride;
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
            to_stride = yagl_gles_get_stride(ps, width, height, 2, NULL);
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint8_t*)(pixels_to + j * 2 + 0) = *(uint32_t*)(pixels_from + j * 4) & 0xFF;
                    *(uint8_t*)(pixels_to + j * 2 + 1) = *(uint32_t*)(pixels_from + j * 4) >> 24;
                }
                pixels_from += stride;
                pixels_to += to_stride;
            }
            break;
        case GL_FLOAT:
            to_stride = yagl_gles_get_stride(ps, width, height, 8, NULL);
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint32_t*)(pixels_to + j * 8 + 0) = *(uint32_t*)(pixels_from + j * 16);
                    *(uint32_t*)(pixels_to + j * 8 + 4) = *(uint32_t*)(pixels_from + j * 16 + 12);
                }
                pixels_from += stride;
                pixels_to += to_stride;
            }
            break;
        case GL_HALF_FLOAT:
            to_stride = yagl_gles_get_stride(ps, width, height, 4, NULL);
            for (i = 0; i < height; ++i) {
                for (j = 0; j < width; ++j) {
                    *(uint16_t*)(pixels_to + j * 4 + 0) = *(uint16_t*)(pixels_from + j * 8);
                    *(uint16_t*)(pixels_to + j * 4 + 2) = *(uint16_t*)(pixels_from + j * 8 + 6);
                }
                pixels_from += stride;
                pixels_to += to_stride;
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
