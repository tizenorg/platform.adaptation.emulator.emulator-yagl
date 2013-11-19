#include "GLES3/gl3.h"
#include "yagl_texcompress.h"
#include "yagl_texcompress_etc2.h"
#include <assert.h>

/*
 * We can't include GL/gl.h here
 */
#define GL_BGRA 0x80E1

#define YAGL_TEXCOMPRESS_ETC2_UNPACK_IMPL(name) \
    static void yagl_texcompress_unpack_##name(struct yagl_texcompress_format *format, \
                                               const GLvoid *src, \
                                               GLsizei width, \
                                               GLsizei height, \
                                               GLsizei src_stride, \
                                               GLvoid *dst, \
                                               GLsizei dst_stride) \
    { \
        yagl_texcompress_etc2_unpack_##name(dst, \
                                            dst_stride, \
                                            src, \
                                            src_stride, \
                                            width, \
                                            height); \
    }

YAGL_TEXCOMPRESS_ETC2_UNPACK_IMPL(r11)
YAGL_TEXCOMPRESS_ETC2_UNPACK_IMPL(signed_r11)
YAGL_TEXCOMPRESS_ETC2_UNPACK_IMPL(rg11)
YAGL_TEXCOMPRESS_ETC2_UNPACK_IMPL(signed_rg11)
YAGL_TEXCOMPRESS_ETC2_UNPACK_IMPL(rgb8)
YAGL_TEXCOMPRESS_ETC2_UNPACK_IMPL(srgb8)
YAGL_TEXCOMPRESS_ETC2_UNPACK_IMPL(rgb8_punchthrough_alpha1)
YAGL_TEXCOMPRESS_ETC2_UNPACK_IMPL(srgb8_punchthrough_alpha1)
YAGL_TEXCOMPRESS_ETC2_UNPACK_IMPL(rgba8)
YAGL_TEXCOMPRESS_ETC2_UNPACK_IMPL(srgb8_alpha8)

static struct yagl_texcompress_format texcompress_formats[] =
{
    {
        .unpack = &yagl_texcompress_unpack_r11,
        .src_format = GL_COMPRESSED_R11_EAC,
        .block_width = 4,
        .block_height = 4,
        .block_bytes = 8,
        .dst_format = GL_RED,
        .dst_internalformat = GL_RED,
        .dst_type = GL_UNSIGNED_SHORT
    },
    {
        .unpack = &yagl_texcompress_unpack_signed_r11,
        .src_format = GL_COMPRESSED_SIGNED_R11_EAC,
        .block_width = 4,
        .block_height = 4,
        .block_bytes = 8,
        .dst_format = GL_RED,
        .dst_internalformat = GL_RED,
        .dst_type = GL_SHORT
    },
    {
        .unpack = &yagl_texcompress_unpack_rg11,
        .src_format = GL_COMPRESSED_RG11_EAC,
        .block_width = 4,
        .block_height = 4,
        .block_bytes = 16,
        .dst_format = GL_RGB,
        .dst_internalformat = GL_RGB,
        .dst_type = GL_UNSIGNED_SHORT
    },
    {
        .unpack = &yagl_texcompress_unpack_signed_rg11,
        .src_format = GL_COMPRESSED_SIGNED_RG11_EAC,
        .block_width = 4,
        .block_height = 4,
        .block_bytes = 16,
        .dst_format = GL_RGB,
        .dst_internalformat = GL_RGB,
        .dst_type = GL_SHORT
    },
    {
        .unpack = &yagl_texcompress_unpack_rgb8,
        .src_format = GL_COMPRESSED_RGB8_ETC2,
        .block_width = 4,
        .block_height = 4,
        .block_bytes = 8,
        .dst_format = GL_RGBA,
        .dst_internalformat = GL_RGBA8,
        .dst_type = GL_UNSIGNED_BYTE
    },
    {
        .unpack = &yagl_texcompress_unpack_srgb8,
        .src_format = GL_COMPRESSED_SRGB8_ETC2,
        .block_width = 4,
        .block_height = 4,
        .block_bytes = 8,
        .dst_format = GL_BGRA,
        .dst_internalformat = GL_SRGB8_ALPHA8,
        .dst_type = GL_UNSIGNED_BYTE
    },
    {
        .unpack = &yagl_texcompress_unpack_rgb8_punchthrough_alpha1,
        .src_format = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,
        .block_width = 4,
        .block_height = 4,
        .block_bytes = 8,
        .dst_format = GL_RGBA,
        .dst_internalformat = GL_RGBA8,
        .dst_type = GL_UNSIGNED_BYTE
    },
    {
        .unpack = &yagl_texcompress_unpack_srgb8_punchthrough_alpha1,
        .src_format = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,
        .block_width = 4,
        .block_height = 4,
        .block_bytes = 8,
        .dst_format = GL_BGRA,
        .dst_internalformat = GL_SRGB8_ALPHA8,
        .dst_type = GL_UNSIGNED_BYTE
    },
    {
        .unpack = &yagl_texcompress_unpack_rgba8,
        .src_format = GL_COMPRESSED_RGBA8_ETC2_EAC,
        .block_width = 4,
        .block_height = 4,
        .block_bytes = 16,
        .dst_format = GL_RGBA,
        .dst_internalformat = GL_RGBA8,
        .dst_type = GL_UNSIGNED_BYTE
    },
    {
        .unpack = &yagl_texcompress_unpack_srgb8_alpha8,
        .src_format = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,
        .block_width = 4,
        .block_height = 4,
        .block_bytes = 16,
        .dst_format = GL_BGRA,
        .dst_internalformat = GL_SRGB8_ALPHA8,
        .dst_type = GL_UNSIGNED_BYTE
    }
};

int yagl_texcompress_get_format_names(GLenum *formats)
{
    int num_formats = sizeof(texcompress_formats)/sizeof(texcompress_formats[0]);
    int i;

    if (formats) {
        for (i = 0; i < num_formats; ++i) {
            formats[i] = texcompress_formats[i].src_format;
        }
    }

    return num_formats;
}

struct yagl_texcompress_format *yagl_texcompress_get_format(GLenum format)
{
    int i;

    for (i = 0;
         i < sizeof(texcompress_formats)/sizeof(texcompress_formats[0]);
         ++i) {
        if (texcompress_formats[i].src_format == format) {
            return &texcompress_formats[i];
        }
    }

    return NULL;
}

int yagl_texcompress_get_info(struct yagl_texcompress_format *format,
                              GLsizei width,
                              GLsizei height,
                              GLsizei src_size,
                              GLsizei *src_stride,
                              GLsizei *dst_stride,
                              GLsizei *dst_size)
{
    GLsizei wblocks = (width + format->block_width - 1) / format->block_width;
    GLsizei hblocks = (height + format->block_height - 1) / format->block_height;
    GLsizei num_components = 0, bpp = 0;

    if ((width < 0) || (height < 0)) {
        return 0;
    }

    if (src_size != (wblocks * hblocks * format->block_bytes)) {
        return 0;
    }

    switch (format->dst_format) {
    case GL_RED:
        num_components = 1;
        break;
    case GL_RGB:
        num_components = 3;
        break;
    case GL_RGBA:
    case GL_BGRA:
        num_components = 4;
        break;
    default:
        assert(0);
        return 0;
    }

    switch (format->dst_type) {
    case GL_UNSIGNED_BYTE:
        bpp = num_components;
        break;
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
        bpp = num_components * 2;
        break;
    default:
        assert(0);
        return 0;
    }

    *src_stride = wblocks * format->block_bytes;
    *dst_stride = width * bpp;
    *dst_size = *dst_stride * height;

    return 1;
}
