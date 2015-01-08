/*
 * YaGL
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Stanislav Vorobiov <s.vorobiov@samsung.com>
 * Jinhyung Jo <jinhyung.jo@samsung.com>
 * YeongKyoon Lee <yeongkyoon.lee@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * - S-Core Co., Ltd
 *
 */

#include "GL/gl.h"
#include "GL/glext.h"
#include "yagl_gles_pixel_formats.h"

/*
 * We can't include GLES2/gl2ext.h here
 */
#define GL_HALF_FLOAT_OES 0x8D61

/*
 * 1.0f in half-float.
 */
#define YAGL_HALF_FLOAT_1_0 0x3C00

static void yagl_convert_alpha_ub_bgra_ub(const GLvoid *src,
                                          GLsizei src_stride,
                                          GLvoid *dst,
                                          GLsizei dst_stride,
                                          GLsizei width,
                                          GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            *(uint32_t*)(dst + j * 4) = ((uint32_t)(*(uint8_t*)(src + j)) << 24);
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_bgra_ub_alpha_ub(const GLvoid *src,
                                          GLsizei src_stride,
                                          GLvoid *dst,
                                          GLsizei dst_stride,
                                          GLsizei width,
                                          GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            *(uint8_t*)(dst + j) = *(uint32_t*)(src + j * 4) >> 24;
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_alpha_f_bgra_f(const GLvoid *src,
                                        GLsizei src_stride,
                                        GLvoid *dst,
                                        GLsizei dst_stride,
                                        GLsizei width,
                                        GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            *(GLfloat*)(dst + j * 16 + 0) = 0.0f;
            *(GLfloat*)(dst + j * 16 + 4) = 0.0f;
            *(GLfloat*)(dst + j * 16 + 8) = 0.0f;
            *(uint32_t*)(dst + j * 16 + 12) = *(uint32_t*)(src + j * 4);
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_bgra_f_alpha_f(const GLvoid *src,
                                        GLsizei src_stride,
                                        GLvoid *dst,
                                        GLsizei dst_stride,
                                        GLsizei width,
                                        GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            *(uint32_t*)(dst + j * 4) = *(uint32_t*)(src + j * 16 + 12);
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_alpha_hf_bgra_hf(const GLvoid *src,
                                          GLsizei src_stride,
                                          GLvoid *dst,
                                          GLsizei dst_stride,
                                          GLsizei width,
                                          GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            *(uint16_t*)(dst + j * 8 + 0) = 0;
            *(uint16_t*)(dst + j * 8 + 2) = 0;
            *(uint16_t*)(dst + j * 8 + 4) = 0;
            *(uint16_t*)(dst + j * 8 + 6) = *(uint16_t*)(src + j * 2);
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_bgra_hf_alpha_hf(const GLvoid *src,
                                          GLsizei src_stride,
                                          GLvoid *dst,
                                          GLsizei dst_stride,
                                          GLsizei width,
                                          GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            *(uint16_t*)(dst + j * 2) = *(uint16_t*)(src + j * 8 + 6);
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_luminance_ub_bgra_ub(const GLvoid *src,
                                              GLsizei src_stride,
                                              GLvoid *dst,
                                              GLsizei dst_stride,
                                              GLsizei width,
                                              GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            uint32_t l = *(uint8_t*)(src + j);
            *(uint32_t*)(dst + j * 4) = (l << 0) |
                                        (l << 8) |
                                        (l << 16) |
                                        (255U << 24);
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_bgra_ub_luminance_ub(const GLvoid *src,
                                              GLsizei src_stride,
                                              GLvoid *dst,
                                              GLsizei dst_stride,
                                              GLsizei width,
                                              GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            *(uint8_t*)(dst + j) = *(uint32_t*)(src + j * 4) & 0xFF;
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_luminance_f_bgra_f(const GLvoid *src,
                                            GLsizei src_stride,
                                            GLvoid *dst,
                                            GLsizei dst_stride,
                                            GLsizei width,
                                            GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            uint32_t l = *(uint32_t*)(src + j * 4);
            *(uint32_t*)(dst + j * 16 + 0) = l;
            *(uint32_t*)(dst + j * 16 + 4) = l;
            *(uint32_t*)(dst + j * 16 + 8) = l;
            *(GLfloat*)(dst + j * 16 + 12) = 1.0f;
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_bgra_f_luminance_f(const GLvoid *src,
                                            GLsizei src_stride,
                                            GLvoid *dst,
                                            GLsizei dst_stride,
                                            GLsizei width,
                                            GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            *(uint32_t*)(dst + j * 4) = *(uint32_t*)(src + j * 16);
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_luminance_hf_bgra_hf(const GLvoid *src,
                                              GLsizei src_stride,
                                              GLvoid *dst,
                                              GLsizei dst_stride,
                                              GLsizei width,
                                              GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            uint16_t l = *(uint16_t*)(src + j * 2);
            *(uint16_t*)(dst + j * 8 + 0) = l;
            *(uint16_t*)(dst + j * 8 + 2) = l;
            *(uint16_t*)(dst + j * 8 + 4) = l;
            *(uint16_t*)(dst + j * 8 + 6) = YAGL_HALF_FLOAT_1_0;
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_bgra_hf_luminance_hf(const GLvoid *src,
                                              GLsizei src_stride,
                                              GLvoid *dst,
                                              GLsizei dst_stride,
                                              GLsizei width,
                                              GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            *(uint16_t*)(dst + j * 2) = *(uint16_t*)(src + j * 8);
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_luminance_alpha_ub_bgra_ub(const GLvoid *src,
                                                    GLsizei src_stride,
                                                    GLvoid *dst,
                                                    GLsizei dst_stride,
                                                    GLsizei width,
                                                    GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            uint32_t l = *(uint8_t*)(src + j * 2 + 0);
            uint32_t a = *(uint8_t*)(src + j * 2 + 1);
            *(uint32_t*)(dst + j * 4) = (l << 0) |
                                        (l << 8) |
                                        (l << 16) |
                                        (a << 24);
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_bgra_ub_luminance_alpha_ub(const GLvoid *src,
                                                    GLsizei src_stride,
                                                    GLvoid *dst,
                                                    GLsizei dst_stride,
                                                    GLsizei width,
                                                    GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            *(uint8_t*)(dst + j * 2 + 0) = *(uint32_t*)(src + j * 4) & 0xFF;
            *(uint8_t*)(dst + j * 2 + 1) = *(uint32_t*)(src + j * 4) >> 24;
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_luminance_alpha_f_bgra_f(const GLvoid *src,
                                                  GLsizei src_stride,
                                                  GLvoid *dst,
                                                  GLsizei dst_stride,
                                                  GLsizei width,
                                                  GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            uint32_t l = *(uint32_t*)(src + j * 8 + 0);
            uint32_t a = *(uint32_t*)(src + j * 8 + 4);
            *(uint32_t*)(dst + j * 16 + 0) = l;
            *(uint32_t*)(dst + j * 16 + 4) = l;
            *(uint32_t*)(dst + j * 16 + 8) = l;
            *(uint32_t*)(dst + j * 16 + 12) = a;
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_bgra_f_luminance_alpha_f(const GLvoid *src,
                                                  GLsizei src_stride,
                                                  GLvoid *dst,
                                                  GLsizei dst_stride,
                                                  GLsizei width,
                                                  GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            *(uint32_t*)(dst + j * 8 + 0) = *(uint32_t*)(src + j * 16);
            *(uint32_t*)(dst + j * 8 + 4) = *(uint32_t*)(src + j * 16 + 12);
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_luminance_alpha_hf_bgra_hf(const GLvoid *src,
                                                    GLsizei src_stride,
                                                    GLvoid *dst,
                                                    GLsizei dst_stride,
                                                    GLsizei width,
                                                    GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            uint16_t l = *(uint16_t*)(src + j * 4 + 0);
            uint16_t a = *(uint16_t*)(src + j * 4 + 2);
            *(uint16_t*)(dst + j * 8 + 0) = l;
            *(uint16_t*)(dst + j * 8 + 2) = l;
            *(uint16_t*)(dst + j * 8 + 4) = l;
            *(uint16_t*)(dst + j * 8 + 6) = a;
        }
        src += src_stride;
        dst += dst_stride;
    }
}

static void yagl_convert_bgra_hf_luminance_alpha_hf(const GLvoid *src,
                                                    GLsizei src_stride,
                                                    GLvoid *dst,
                                                    GLsizei dst_stride,
                                                    GLsizei width,
                                                    GLsizei height)
{
    GLsizei i, j;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            *(uint16_t*)(dst + j * 4 + 0) = *(uint16_t*)(src + j * 8);
            *(uint16_t*)(dst + j * 4 + 2) = *(uint16_t*)(src + j * 8 + 6);
        }
        src += src_stride;
        dst += dst_stride;
    }
}

YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles_gl2, GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, GL_ALPHA, GL_UNSIGNED_BYTE, 1);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles_gl2, GL_ALPHA, GL_ALPHA, GL_FLOAT, GL_ALPHA, GL_FLOAT, 4);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles_gl2, GL_ALPHA, GL_ALPHA, GL_HALF_FLOAT, GL_ALPHA, GL_HALF_FLOAT, 2);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles_gl2, GL_ALPHA, GL_ALPHA, GL_HALF_FLOAT_OES, GL_ALPHA, GL_HALF_FLOAT, 2);

YAGL_PIXEL_FORMAT_IMPL_BEGIN(gles_gl3, GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE)
    .src_bpp = 1,
    .dst_internalformat = GL_RGBA,
    .dst_format = GL_BGRA,
    .dst_type = GL_UNSIGNED_BYTE,
    .dst_bpp = 4,
    .unpack = &yagl_convert_alpha_ub_bgra_ub,
    .pack = &yagl_convert_bgra_ub_alpha_ub
YAGL_PIXEL_FORMAT_IMPL_END()

YAGL_PIXEL_FORMAT_IMPL_BEGIN(gles_gl3, GL_ALPHA, GL_ALPHA, GL_FLOAT)
    .src_bpp = 4,
    .dst_internalformat = GL_RGBA,
    .dst_format = GL_BGRA,
    .dst_type = GL_FLOAT,
    .dst_bpp = 16,
    .unpack = &yagl_convert_alpha_f_bgra_f,
    .pack = &yagl_convert_bgra_f_alpha_f
YAGL_PIXEL_FORMAT_IMPL_END()

YAGL_PIXEL_FORMAT_IMPL_BEGIN(gles_gl3, GL_ALPHA, GL_ALPHA, GL_HALF_FLOAT)
    .src_bpp = 2,
    .dst_internalformat = GL_RGBA,
    .dst_format = GL_BGRA,
    .dst_type = GL_HALF_FLOAT,
    .dst_bpp = 8,
    .unpack = &yagl_convert_alpha_hf_bgra_hf,
    .pack = &yagl_convert_bgra_hf_alpha_hf
YAGL_PIXEL_FORMAT_IMPL_END()

YAGL_PIXEL_FORMAT_IMPL_BEGIN(gles_gl3, GL_ALPHA, GL_ALPHA, GL_HALF_FLOAT_OES)
    .src_bpp = 2,
    .dst_internalformat = GL_RGBA,
    .dst_format = GL_BGRA,
    .dst_type = GL_HALF_FLOAT,
    .dst_bpp = 8,
    .unpack = &yagl_convert_alpha_hf_bgra_hf,
    .pack = &yagl_convert_bgra_hf_alpha_hf
YAGL_PIXEL_FORMAT_IMPL_END()

YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, GL_RGB, GL_UNSIGNED_BYTE, 3);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 2);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_RGB, GL_RGB, GL_FLOAT, GL_RGB, GL_FLOAT, 12);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_RGB, GL_RGB, GL_HALF_FLOAT, GL_RGB, GL_HALF_FLOAT, 6);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_RGB, GL_RGB, GL_HALF_FLOAT_OES, GL_RGB, GL_HALF_FLOAT, 6);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_RGBA, GL_UNSIGNED_BYTE, 4);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 2);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 2);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_RGBA, GL_RGBA, GL_FLOAT, GL_RGBA, GL_FLOAT, 16);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_RGBA, GL_RGBA, GL_HALF_FLOAT, GL_RGBA, GL_HALF_FLOAT, 8);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_RGBA, GL_RGBA, GL_HALF_FLOAT_OES, GL_RGBA, GL_HALF_FLOAT, 8);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_BGRA, GL_BGRA, GL_UNSIGNED_BYTE, GL_RGBA, GL_UNSIGNED_BYTE, 4);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_BGRA, GL_BGRA, GL_FLOAT, GL_RGBA, GL_FLOAT, 16);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_BGRA, GL_BGRA, GL_HALF_FLOAT, GL_RGBA, GL_HALF_FLOAT, 8);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_BGRA, GL_BGRA, GL_HALF_FLOAT_OES, GL_RGBA, GL_HALF_FLOAT, 8);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles_gl2, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, GL_LUMINANCE, GL_UNSIGNED_BYTE, 1);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles_gl2, GL_LUMINANCE, GL_LUMINANCE, GL_FLOAT, GL_LUMINANCE, GL_FLOAT, 4);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles_gl2, GL_LUMINANCE, GL_LUMINANCE, GL_HALF_FLOAT, GL_LUMINANCE, GL_HALF_FLOAT, 2);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles_gl2, GL_LUMINANCE, GL_LUMINANCE, GL_HALF_FLOAT_OES, GL_LUMINANCE, GL_HALF_FLOAT, 2);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles_gl2, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 2);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles_gl2, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_FLOAT, GL_LUMINANCE_ALPHA, GL_FLOAT, 8);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles_gl2, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT, 4);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles_gl2, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT_OES, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT, 4);

YAGL_PIXEL_FORMAT_IMPL_BEGIN(gles_gl3, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE)
    .src_bpp = 1,
    .dst_internalformat = GL_RGBA,
    .dst_format = GL_BGRA,
    .dst_type = GL_UNSIGNED_BYTE,
    .dst_bpp = 4,
    .unpack = &yagl_convert_luminance_ub_bgra_ub,
    .pack = &yagl_convert_bgra_ub_luminance_ub
YAGL_PIXEL_FORMAT_IMPL_END()

YAGL_PIXEL_FORMAT_IMPL_BEGIN(gles_gl3, GL_LUMINANCE, GL_LUMINANCE, GL_FLOAT)
    .src_bpp = 4,
    .dst_internalformat = GL_RGBA,
    .dst_format = GL_BGRA,
    .dst_type = GL_FLOAT,
    .dst_bpp = 16,
    .unpack = &yagl_convert_luminance_f_bgra_f,
    .pack = &yagl_convert_bgra_f_luminance_f
YAGL_PIXEL_FORMAT_IMPL_END()

YAGL_PIXEL_FORMAT_IMPL_BEGIN(gles_gl3, GL_LUMINANCE, GL_LUMINANCE, GL_HALF_FLOAT)
    .src_bpp = 2,
    .dst_internalformat = GL_RGBA,
    .dst_format = GL_BGRA,
    .dst_type = GL_HALF_FLOAT,
    .dst_bpp = 8,
    .unpack = &yagl_convert_luminance_hf_bgra_hf,
    .pack = &yagl_convert_bgra_hf_luminance_hf
YAGL_PIXEL_FORMAT_IMPL_END()

YAGL_PIXEL_FORMAT_IMPL_BEGIN(gles_gl3, GL_LUMINANCE, GL_LUMINANCE, GL_HALF_FLOAT_OES)
    .src_bpp = 2,
    .dst_internalformat = GL_RGBA,
    .dst_format = GL_BGRA,
    .dst_type = GL_HALF_FLOAT,
    .dst_bpp = 8,
    .unpack = &yagl_convert_luminance_hf_bgra_hf,
    .pack = &yagl_convert_bgra_hf_luminance_hf
YAGL_PIXEL_FORMAT_IMPL_END()

YAGL_PIXEL_FORMAT_IMPL_BEGIN(gles_gl3, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE)
    .src_bpp = 2,
    .dst_internalformat = GL_RGBA,
    .dst_format = GL_BGRA,
    .dst_type = GL_UNSIGNED_BYTE,
    .dst_bpp = 4,
    .unpack = &yagl_convert_luminance_alpha_ub_bgra_ub,
    .pack = &yagl_convert_bgra_ub_luminance_alpha_ub
YAGL_PIXEL_FORMAT_IMPL_END()

YAGL_PIXEL_FORMAT_IMPL_BEGIN(gles_gl3, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_FLOAT)
    .src_bpp = 8,
    .dst_internalformat = GL_RGBA,
    .dst_format = GL_BGRA,
    .dst_type = GL_FLOAT,
    .dst_bpp = 16,
    .unpack = &yagl_convert_luminance_alpha_f_bgra_f,
    .pack = &yagl_convert_bgra_f_luminance_alpha_f
YAGL_PIXEL_FORMAT_IMPL_END()

YAGL_PIXEL_FORMAT_IMPL_BEGIN(gles_gl3, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT)
    .src_bpp = 4,
    .dst_internalformat = GL_RGBA,
    .dst_format = GL_BGRA,
    .dst_type = GL_HALF_FLOAT,
    .dst_bpp = 8,
    .unpack = &yagl_convert_luminance_alpha_hf_bgra_hf,
    .pack = &yagl_convert_bgra_hf_luminance_alpha_hf
YAGL_PIXEL_FORMAT_IMPL_END()

YAGL_PIXEL_FORMAT_IMPL_BEGIN(gles_gl3, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT_OES)
    .src_bpp = 4,
    .dst_internalformat = GL_RGBA,
    .dst_format = GL_BGRA,
    .dst_type = GL_HALF_FLOAT,
    .dst_bpp = 8,
    .unpack = &yagl_convert_luminance_alpha_hf_bgra_hf,
    .pack = &yagl_convert_bgra_hf_luminance_alpha_hf
YAGL_PIXEL_FORMAT_IMPL_END()

YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 2);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 4);
YAGL_PIXEL_FORMAT_IMPL_NOCONV(gles, GL_DEPTH_STENCIL, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 4);
