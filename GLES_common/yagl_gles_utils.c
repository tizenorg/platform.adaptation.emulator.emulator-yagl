#include "GLES3/gl3.h"
#include "GLES2/gl2ext.h"
#include "yagl_gles_utils.h"
#include "yagl_host_gles_calls.h"
#include <assert.h>

/*
 * We can't include GL/glext.h here
 */
#define GL_DEPTH_COMPONENT32 0x81A7

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

uint32_t yagl_gles_internalformat_flags(GLenum internalformat)
{
    switch (internalformat) {
    case 0:
        /*
         * No internalformat, it's ok.
         */
    case GL_ALPHA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
        return 0;
    case GL_RED:
    case GL_RG:
    case GL_RGB:
    case GL_RGBA:
    case GL_BGRA_EXT:
        return yagl_gles_format_color_renderable;
    case GL_R8_SNORM:
    case GL_RG8_SNORM:
    case GL_RGB8_SNORM:
    case GL_RGBA8_SNORM:
    case GL_RGB9_E5:
        return yagl_gles_format_sized;
    case GL_SRGB8:
        return yagl_gles_format_sized | yagl_gles_format_srgb;
    case GL_RGB8UI:
    case GL_RGB16UI:
    case GL_RGB32UI:
        return yagl_gles_format_sized | yagl_gles_format_unsigned_integer;
    case GL_RGB8I:
    case GL_RGB16I:
    case GL_RGB32I:
        return yagl_gles_format_sized | yagl_gles_format_signed_integer;
    case GL_R8:
    case GL_RG8:
    case GL_RGB8:
    case GL_RGB565:
    case GL_RGBA8:
    case GL_RGB5_A1:
    case GL_RGBA4:
    case GL_RGB10_A2:
        return yagl_gles_format_sized | yagl_gles_format_color_renderable;
    case GL_SRGB8_ALPHA8:
        return yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_srgb;
    case GL_R8UI:
    case GL_R16UI:
    case GL_R32UI:
    case GL_RG8UI:
    case GL_RG16UI:
    case GL_RG32UI:
    case GL_RGBA8UI:
    case GL_RGB10_A2UI:
    case GL_RGBA16UI:
    case GL_RGBA32UI:
        return yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_unsigned_integer;
    case GL_R8I:
    case GL_R16I:
    case GL_R32I:
    case GL_RG8I:
    case GL_RG16I:
    case GL_RG32I:
    case GL_RGBA8I:
    case GL_RGBA16I:
    case GL_RGBA32I:
        return yagl_gles_format_sized | yagl_gles_format_color_renderable | yagl_gles_format_signed_integer;
    case GL_R16F:
    case GL_R32F:
    case GL_RG16F:
    case GL_RG32F:
    case GL_R11F_G11F_B10F:
    case GL_RGB16F:
    case GL_RGB32F:
    case GL_RGBA16F:
    case GL_RGBA32F:
        return yagl_gles_format_sized | yagl_gles_format_float;
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
        return yagl_gles_format_depth_renderable;
    case GL_DEPTH_COMPONENT32F:
        return yagl_gles_format_depth_renderable | yagl_gles_format_float;
    case GL_STENCIL_INDEX8:
        return yagl_gles_format_stencil_renderable;
    case GL_DEPTH_STENCIL:
    case GL_DEPTH24_STENCIL8:
    case GL_DEPTH32F_STENCIL8:
        return yagl_gles_format_depth_renderable | yagl_gles_format_stencil_renderable;
    default:
        assert(0);
        return 0;
    }
}

uint32_t yagl_gles_internalformat_num_components(GLenum internalformat)
{
    switch (internalformat) {
    case GL_ALPHA:
    case GL_LUMINANCE:
    case GL_RED:
    case GL_R8_SNORM:
    case GL_R8:
    case GL_R8UI:
    case GL_R16UI:
    case GL_R32UI:
    case GL_R8I:
    case GL_R16F:
    case GL_R32F:
    case GL_R16I:
    case GL_R32I:
    case GL_STENCIL_INDEX8:
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
    case GL_DEPTH_COMPONENT32F:
        return 1;
    case GL_LUMINANCE_ALPHA:
    case GL_RG:
    case GL_RG8_SNORM:
    case GL_RG8:
    case GL_RG8UI:
    case GL_RG16UI:
    case GL_RG32UI:
    case GL_RG8I:
    case GL_RG16I:
    case GL_RG32I:
    case GL_RG16F:
    case GL_RG32F:
    case GL_DEPTH_STENCIL:
    case GL_DEPTH24_STENCIL8:
    case GL_DEPTH32F_STENCIL8:
        return 2;
    case GL_RGB:
    case GL_RGB8_SNORM:
    case GL_SRGB8:
    case GL_RGB9_E5:
    case GL_RGB8UI:
    case GL_RGB16UI:
    case GL_RGB32UI:
    case GL_RGB8I:
    case GL_RGB16I:
    case GL_RGB32I:
    case GL_RGB8:
    case GL_RGB565:
    case GL_RGB16F:
    case GL_RGB32F:
    case GL_R11F_G11F_B10F:
        return 3;
    case GL_RGBA:
    case GL_BGRA_EXT:
    case GL_RGBA8_SNORM:
    case GL_RGBA8:
    case GL_RGBA4:
    case GL_RGBA8UI:
    case GL_RGBA16UI:
    case GL_RGBA32UI:
    case GL_RGBA8I:
    case GL_RGBA16I:
    case GL_RGBA32I:
    case GL_RGBA16F:
    case GL_RGBA32F:
    case GL_SRGB8_ALPHA8:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
    case GL_RGB10_A2UI:
    case 0:
        /*
         * No internalformat, assume maximum components.
         */
        return 4;
    default:
        assert(0);
        return 4;
    }
}
