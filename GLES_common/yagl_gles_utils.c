#include "GLES3/gl3.h"
#include "GLES2/gl2ext.h"
#include "yagl_gles_utils.h"
#include "yagl_host_gles_calls.h"
#include <assert.h>

/*
 * We can't include GL/glext.h here
 */
#define GL_DEPTH_COMPONENT32 0x81A7
#define GL_BGRA 0x80E1

#define YAGL_INTERNALFORMAT_R(arg_internalformat, \
                              arg_flags, \
                              arg_red_size) \
static const struct yagl_gles_format_info internalformat_##arg_internalformat = \
{ \
    .flags = arg_flags, \
    .num_components = 1, \
    .red_size = arg_red_size, \
    .green_size = 0, \
    .blue_size = 0, \
    .alpha_size = 0, \
    .depth_size = 0, \
    .stencil_size = 0 \
}

#define YAGL_INTERNALFORMAT_RG(arg_internalformat, \
                               arg_flags, \
                               arg_red_size, \
                               arg_green_size) \
static const struct yagl_gles_format_info internalformat_##arg_internalformat = \
{ \
    .flags = arg_flags, \
    .num_components = 2, \
    .red_size = arg_red_size, \
    .green_size = arg_green_size, \
    .blue_size = 0, \
    .alpha_size = 0, \
    .depth_size = 0, \
    .stencil_size = 0 \
}

#define YAGL_INTERNALFORMAT_RGB(arg_internalformat, \
                                arg_flags, \
                                arg_red_size, \
                                arg_green_size, \
                                arg_blue_size) \
static const struct yagl_gles_format_info internalformat_##arg_internalformat = \
{ \
    .flags = arg_flags, \
    .num_components = 3, \
    .red_size = arg_red_size, \
    .green_size = arg_green_size, \
    .blue_size = arg_blue_size, \
    .alpha_size = 0, \
    .depth_size = 0, \
    .stencil_size = 0 \
}

#define YAGL_INTERNALFORMAT_RGBA(arg_internalformat, \
                                 arg_flags, \
                                 arg_red_size, \
                                 arg_green_size, \
                                 arg_blue_size, \
                                 arg_alpha_size) \
static const struct yagl_gles_format_info internalformat_##arg_internalformat = \
{ \
    .flags = arg_flags, \
    .num_components = 4, \
    .red_size = arg_red_size, \
    .green_size = arg_green_size, \
    .blue_size = arg_blue_size, \
    .alpha_size = arg_alpha_size, \
    .depth_size = 0, \
    .stencil_size = 0 \
}

#define YAGL_INTERNALFORMAT_DEPTH(arg_internalformat, \
                                  arg_flags, \
                                  arg_depth_size) \
static const struct yagl_gles_format_info internalformat_##arg_internalformat = \
{ \
    .flags = arg_flags, \
    .num_components = 1, \
    .red_size = 0, \
    .green_size = 0, \
    .blue_size = 0, \
    .alpha_size = 0, \
    .depth_size = arg_depth_size, \
    .stencil_size = 0 \
}

#define YAGL_INTERNALFORMAT_STENCIL(arg_internalformat, \
                                    arg_flags, \
                                    arg_stencil_size) \
static const struct yagl_gles_format_info internalformat_##arg_internalformat = \
{ \
    .flags = arg_flags, \
    .num_components = 1, \
    .red_size = 0, \
    .green_size = 0, \
    .blue_size = 0, \
    .alpha_size = 0, \
    .depth_size = 0, \
    .stencil_size = arg_stencil_size \
}

#define YAGL_INTERNALFORMAT_DEPTH_STENCIL(arg_internalformat, \
                                          arg_flags, \
                                          arg_depth_size, \
                                          arg_stencil_size) \
static const struct yagl_gles_format_info internalformat_##arg_internalformat = \
{ \
    .flags = arg_flags, \
    .num_components = 2, \
    .red_size = 0, \
    .green_size = 0, \
    .blue_size = 0, \
    .alpha_size = 0, \
    .depth_size = arg_depth_size, \
    .stencil_size = arg_stencil_size \
}

static const struct yagl_gles_format_info internalformat_0 =
{
    .flags = 0,
    .num_components = 4,
    .red_size = 8,
    .green_size = 8,
    .blue_size = 8,
    .alpha_size = 8,
    .depth_size = 0,
    .stencil_size = 0
};

static const struct yagl_gles_format_info internalformat_GL_ALPHA =
{
    .flags = 0,
    .num_components = 1,
    .red_size = 0,
    .green_size = 0,
    .blue_size = 0,
    .alpha_size = 8,
    .depth_size = 0,
    .stencil_size = 0
};

static const struct yagl_gles_format_info internalformat_GL_LUMINANCE =
{
    .flags = 0,
    .num_components = 1,
    .red_size = 0,
    .green_size = 0,
    .blue_size = 0,
    .alpha_size = 0,
    .depth_size = 0,
    .stencil_size = 0
};

static const struct yagl_gles_format_info internalformat_GL_LUMINANCE_ALPHA =
{
    .flags = 0,
    .num_components = 2,
    .red_size = 0,
    .green_size = 0,
    .blue_size = 0,
    .alpha_size = 8,
    .depth_size = 0,
    .stencil_size = 0
};

YAGL_INTERNALFORMAT_R(GL_RED, yagl_gles_format_color_renderable, 8);
YAGL_INTERNALFORMAT_RG(GL_RG, yagl_gles_format_color_renderable, 8, 8);
YAGL_INTERNALFORMAT_RGB(GL_RGB, yagl_gles_format_color_renderable, 8, 8, 8);
YAGL_INTERNALFORMAT_RGBA(GL_RGBA, yagl_gles_format_color_renderable, 8, 8, 8, 8);
YAGL_INTERNALFORMAT_RGBA(GL_BGRA, yagl_gles_format_color_renderable, 8, 8, 8, 8);
YAGL_INTERNALFORMAT_R(GL_R8_SNORM, yagl_gles_format_sized, 8);
YAGL_INTERNALFORMAT_RG(GL_RG8_SNORM, yagl_gles_format_sized, 8, 8);
YAGL_INTERNALFORMAT_RGB(GL_RGB8_SNORM, yagl_gles_format_sized, 8, 8, 8);
YAGL_INTERNALFORMAT_RGBA(GL_RGBA8_SNORM, yagl_gles_format_sized, 8, 8, 8, 8);
YAGL_INTERNALFORMAT_RGB(GL_RGB9_E5, yagl_gles_format_sized, 9, 9, 9);
YAGL_INTERNALFORMAT_RGB(GL_SRGB8, yagl_gles_format_sized | yagl_gles_format_srgb, 8, 8, 8);
YAGL_INTERNALFORMAT_RGB(GL_RGB8UI, yagl_gles_format_sized | yagl_gles_format_unsigned_integer, 8, 8, 8);
YAGL_INTERNALFORMAT_RGB(GL_RGB16UI, yagl_gles_format_sized | yagl_gles_format_unsigned_integer, 8, 8, 8);
YAGL_INTERNALFORMAT_RGB(GL_RGB32UI, yagl_gles_format_sized | yagl_gles_format_unsigned_integer, 8, 8, 8);
YAGL_INTERNALFORMAT_RGB(GL_RGB8I, yagl_gles_format_sized | yagl_gles_format_signed_integer, 8, 8, 8);
YAGL_INTERNALFORMAT_RGB(GL_RGB16I, yagl_gles_format_sized | yagl_gles_format_signed_integer, 8, 8, 8);
YAGL_INTERNALFORMAT_RGB(GL_RGB32I, yagl_gles_format_sized | yagl_gles_format_signed_integer, 8, 8, 8);
YAGL_INTERNALFORMAT_R(GL_R8, yagl_gles_format_sized | yagl_gles_format_color_renderable, 8);
YAGL_INTERNALFORMAT_RG(GL_RG8, yagl_gles_format_sized | yagl_gles_format_color_renderable, 8, 8);
YAGL_INTERNALFORMAT_RGB(GL_RGB8, yagl_gles_format_sized | yagl_gles_format_color_renderable, 8, 8, 8);
YAGL_INTERNALFORMAT_RGB(GL_RGB565, yagl_gles_format_sized | yagl_gles_format_color_renderable, 5, 6, 5);
YAGL_INTERNALFORMAT_RGBA(GL_RGBA8, yagl_gles_format_sized | yagl_gles_format_color_renderable, 8, 8, 8, 8);
YAGL_INTERNALFORMAT_RGBA(GL_RGB5_A1, yagl_gles_format_sized | yagl_gles_format_color_renderable, 5, 5, 5, 1);
YAGL_INTERNALFORMAT_RGBA(GL_RGBA4, yagl_gles_format_sized | yagl_gles_format_color_renderable, 4, 4, 4, 4);
YAGL_INTERNALFORMAT_RGBA(GL_RGB10_A2, yagl_gles_format_sized | yagl_gles_format_color_renderable, 10, 10, 10, 2);
YAGL_INTERNALFORMAT_RGBA(GL_SRGB8_ALPHA8, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_srgb, 8, 8, 8, 8);
YAGL_INTERNALFORMAT_R(GL_R8UI, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_unsigned_integer, 8);
YAGL_INTERNALFORMAT_R(GL_R16UI, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_unsigned_integer, 16);
YAGL_INTERNALFORMAT_R(GL_R32UI, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_unsigned_integer, 32);
YAGL_INTERNALFORMAT_RG(GL_RG8UI, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_unsigned_integer, 8, 8);
YAGL_INTERNALFORMAT_RG(GL_RG16UI, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_unsigned_integer, 16, 16);
YAGL_INTERNALFORMAT_RG(GL_RG32UI, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_unsigned_integer, 32, 32);
YAGL_INTERNALFORMAT_RGBA(GL_RGBA8UI, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_unsigned_integer, 8, 8, 8, 8);
YAGL_INTERNALFORMAT_RGBA(GL_RGB10_A2UI, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_unsigned_integer, 10, 10, 10, 2);
YAGL_INTERNALFORMAT_RGBA(GL_RGBA16UI, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_unsigned_integer, 16, 16, 16, 16);
YAGL_INTERNALFORMAT_RGBA(GL_RGBA32UI, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_unsigned_integer, 32, 32, 32, 32);
YAGL_INTERNALFORMAT_R(GL_R8I, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_signed_integer, 8);
YAGL_INTERNALFORMAT_R(GL_R16I, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_signed_integer, 16);
YAGL_INTERNALFORMAT_R(GL_R32I, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_signed_integer, 32);
YAGL_INTERNALFORMAT_RG(GL_RG8I, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_signed_integer, 8, 8);
YAGL_INTERNALFORMAT_RG(GL_RG16I, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_signed_integer, 16, 16);
YAGL_INTERNALFORMAT_RG(GL_RG32I, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_signed_integer, 32, 32);
YAGL_INTERNALFORMAT_RGBA(GL_RGBA8I, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_signed_integer, 8, 8, 8, 8);
YAGL_INTERNALFORMAT_RGBA(GL_RGBA16I, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_signed_integer, 16, 16, 16, 16);
YAGL_INTERNALFORMAT_RGBA(GL_RGBA32I, yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_signed_integer, 32, 32, 32, 32);
YAGL_INTERNALFORMAT_R(GL_R16F, yagl_gles_format_sized | yagl_gles_format_float, 16);
YAGL_INTERNALFORMAT_R(GL_R32F, yagl_gles_format_sized | yagl_gles_format_float, 32);
YAGL_INTERNALFORMAT_RG(GL_RG16F, yagl_gles_format_sized | yagl_gles_format_float, 16, 16);
YAGL_INTERNALFORMAT_RG(GL_RG32F, yagl_gles_format_sized | yagl_gles_format_float, 32, 32);
YAGL_INTERNALFORMAT_RGB(GL_R11F_G11F_B10F, yagl_gles_format_sized | yagl_gles_format_float, 11, 11, 10);
YAGL_INTERNALFORMAT_RGB(GL_RGB16F, yagl_gles_format_sized | yagl_gles_format_float, 16, 16, 16);
YAGL_INTERNALFORMAT_RGB(GL_RGB32F, yagl_gles_format_sized | yagl_gles_format_float, 32, 32, 32);
YAGL_INTERNALFORMAT_RGBA(GL_RGBA16F, yagl_gles_format_sized | yagl_gles_format_float, 16, 16, 16, 16);
YAGL_INTERNALFORMAT_RGBA(GL_RGBA32F, yagl_gles_format_sized | yagl_gles_format_float, 32, 32, 32, 32);
YAGL_INTERNALFORMAT_DEPTH(GL_DEPTH_COMPONENT, yagl_gles_format_depth_renderable, 8);
YAGL_INTERNALFORMAT_DEPTH(GL_DEPTH_COMPONENT16, yagl_gles_format_depth_renderable, 16);
YAGL_INTERNALFORMAT_DEPTH(GL_DEPTH_COMPONENT24, yagl_gles_format_depth_renderable, 24);
YAGL_INTERNALFORMAT_DEPTH(GL_DEPTH_COMPONENT32, yagl_gles_format_depth_renderable, 32);
YAGL_INTERNALFORMAT_DEPTH(GL_DEPTH_COMPONENT32F, yagl_gles_format_depth_renderable | yagl_gles_format_float, 32);
YAGL_INTERNALFORMAT_STENCIL(GL_STENCIL_INDEX8, yagl_gles_format_stencil_renderable, 8);
YAGL_INTERNALFORMAT_DEPTH_STENCIL(GL_DEPTH_STENCIL, yagl_gles_format_depth_renderable | yagl_gles_format_stencil_renderable, 24, 8);
YAGL_INTERNALFORMAT_DEPTH_STENCIL(GL_DEPTH24_STENCIL8, yagl_gles_format_depth_renderable | yagl_gles_format_stencil_renderable, 24, 8);
YAGL_INTERNALFORMAT_DEPTH_STENCIL(GL_DEPTH32F_STENCIL8, yagl_gles_format_depth_renderable | yagl_gles_format_stencil_renderable, 32, 8);

#define YAGL_INTERNALFORMAT_CASE(arg_internalformat) \
case arg_internalformat: \
    return &internalformat_##arg_internalformat

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

const struct yagl_gles_format_info
    *yagl_gles_internalformat_info(GLenum internalformat)
{
    switch (internalformat) {
    case 0:
        /*
         * No internalformat, it's ok.
         */
        return &internalformat_0;
    YAGL_INTERNALFORMAT_CASE(GL_ALPHA);
    YAGL_INTERNALFORMAT_CASE(GL_LUMINANCE);
    YAGL_INTERNALFORMAT_CASE(GL_LUMINANCE_ALPHA);
    YAGL_INTERNALFORMAT_CASE(GL_RED);
    YAGL_INTERNALFORMAT_CASE(GL_RG);
    YAGL_INTERNALFORMAT_CASE(GL_RGB);
    YAGL_INTERNALFORMAT_CASE(GL_RGBA);
    YAGL_INTERNALFORMAT_CASE(GL_BGRA);
    YAGL_INTERNALFORMAT_CASE(GL_R8_SNORM);
    YAGL_INTERNALFORMAT_CASE(GL_RG8_SNORM);
    YAGL_INTERNALFORMAT_CASE(GL_RGB8_SNORM);
    YAGL_INTERNALFORMAT_CASE(GL_RGBA8_SNORM);
    YAGL_INTERNALFORMAT_CASE(GL_RGB9_E5);
    YAGL_INTERNALFORMAT_CASE(GL_SRGB8);
    YAGL_INTERNALFORMAT_CASE(GL_RGB8UI);
    YAGL_INTERNALFORMAT_CASE(GL_RGB16UI);
    YAGL_INTERNALFORMAT_CASE(GL_RGB32UI);
    YAGL_INTERNALFORMAT_CASE(GL_RGB8I);
    YAGL_INTERNALFORMAT_CASE(GL_RGB16I);
    YAGL_INTERNALFORMAT_CASE(GL_RGB32I);
    YAGL_INTERNALFORMAT_CASE(GL_R8);
    YAGL_INTERNALFORMAT_CASE(GL_RG8);
    YAGL_INTERNALFORMAT_CASE(GL_RGB8);
    YAGL_INTERNALFORMAT_CASE(GL_RGB565);
    YAGL_INTERNALFORMAT_CASE(GL_RGBA8);
    YAGL_INTERNALFORMAT_CASE(GL_RGB5_A1);
    YAGL_INTERNALFORMAT_CASE(GL_RGBA4);
    YAGL_INTERNALFORMAT_CASE(GL_RGB10_A2);
    YAGL_INTERNALFORMAT_CASE(GL_SRGB8_ALPHA8);
    YAGL_INTERNALFORMAT_CASE(GL_R8UI);
    YAGL_INTERNALFORMAT_CASE(GL_R16UI);
    YAGL_INTERNALFORMAT_CASE(GL_R32UI);
    YAGL_INTERNALFORMAT_CASE(GL_RG8UI);
    YAGL_INTERNALFORMAT_CASE(GL_RG16UI);
    YAGL_INTERNALFORMAT_CASE(GL_RG32UI);
    YAGL_INTERNALFORMAT_CASE(GL_RGBA8UI);
    YAGL_INTERNALFORMAT_CASE(GL_RGB10_A2UI);
    YAGL_INTERNALFORMAT_CASE(GL_RGBA16UI);
    YAGL_INTERNALFORMAT_CASE(GL_RGBA32UI);
    YAGL_INTERNALFORMAT_CASE(GL_R8I);
    YAGL_INTERNALFORMAT_CASE(GL_R16I);
    YAGL_INTERNALFORMAT_CASE(GL_R32I);
    YAGL_INTERNALFORMAT_CASE(GL_RG8I);
    YAGL_INTERNALFORMAT_CASE(GL_RG16I);
    YAGL_INTERNALFORMAT_CASE(GL_RG32I);
    YAGL_INTERNALFORMAT_CASE(GL_RGBA8I);
    YAGL_INTERNALFORMAT_CASE(GL_RGBA16I);
    YAGL_INTERNALFORMAT_CASE(GL_RGBA32I);
    YAGL_INTERNALFORMAT_CASE(GL_R16F);
    YAGL_INTERNALFORMAT_CASE(GL_R32F);
    YAGL_INTERNALFORMAT_CASE(GL_RG16F);
    YAGL_INTERNALFORMAT_CASE(GL_RG32F);
    YAGL_INTERNALFORMAT_CASE(GL_R11F_G11F_B10F);
    YAGL_INTERNALFORMAT_CASE(GL_RGB16F);
    YAGL_INTERNALFORMAT_CASE(GL_RGB32F);
    YAGL_INTERNALFORMAT_CASE(GL_RGBA16F);
    YAGL_INTERNALFORMAT_CASE(GL_RGBA32F);
    YAGL_INTERNALFORMAT_CASE(GL_DEPTH_COMPONENT);
    YAGL_INTERNALFORMAT_CASE(GL_DEPTH_COMPONENT16);
    YAGL_INTERNALFORMAT_CASE(GL_DEPTH_COMPONENT24);
    YAGL_INTERNALFORMAT_CASE(GL_DEPTH_COMPONENT32);
    YAGL_INTERNALFORMAT_CASE(GL_DEPTH_COMPONENT32F);
    YAGL_INTERNALFORMAT_CASE(GL_STENCIL_INDEX8);
    YAGL_INTERNALFORMAT_CASE(GL_DEPTH_STENCIL);
    YAGL_INTERNALFORMAT_CASE(GL_DEPTH24_STENCIL8);
    YAGL_INTERNALFORMAT_CASE(GL_DEPTH32F_STENCIL8);
    default:
        assert(0);
        return &internalformat_0;
    }
}
