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

#ifndef _YAGL_PIXEL_FORMAT_H_
#define _YAGL_PIXEL_FORMAT_H_

#include "yagl_gles_types.h"

#define YAGL_PIXEL_FORMAT_DECL(prefix, internalformat, format, type) \
extern struct yagl_pixel_format yagl_##prefix##_pixel_format_##internalformat##_##format##_##type

#define YAGL_PIXEL_FORMAT_IMPL_NOCONV(prefix, internalformat, format, type, dstinternalformat, dsttype, bpp) \
struct yagl_pixel_format yagl_##prefix##_pixel_format_##internalformat##_##format##_##type = \
{ \
    .need_convert = 0, \
    .src_internalformat = internalformat, \
    .src_format = format, \
    .src_type = type, \
    .src_bpp = bpp, \
    .dst_internalformat = dstinternalformat, \
    .dst_format = format, \
    .dst_type = dsttype, \
    .dst_bpp = bpp, \
    .unpack = NULL, \
    .pack = NULL \
}

#define YAGL_PIXEL_FORMAT_IMPL_BEGIN(prefix, internalformat, format, type) \
struct yagl_pixel_format yagl_##prefix##_pixel_format_##internalformat##_##format##_##type = \
{ \
    .need_convert = 1, \
    .src_internalformat = internalformat, \
    .src_format = format, \
    .src_type = type,

#define YAGL_PIXEL_FORMAT_IMPL_END() };

#define YAGL_PIXEL_FORMAT_CASE(prefix, internalformat, format, type) \
case type: \
    return &yagl_##prefix##_pixel_format_##internalformat##_##format##_##type

struct yagl_pixel_format;

typedef void (*yagl_pixel_format_converter)(const GLvoid */*src*/,
                                            GLsizei /*src_stride*/,
                                            GLvoid */*dst*/,
                                            GLsizei /*dst_stride*/,
                                            GLsizei /*width*/,
                                            GLsizei /*height*/);

struct yagl_pixel_format
{
    int need_convert;

    GLenum src_internalformat;
    GLenum src_format;
    GLenum src_type;
    GLint src_bpp;

    GLenum dst_internalformat;
    GLenum dst_format;
    GLenum dst_type;
    GLint dst_bpp;

    /*
     * NULL if no conversion is required.
     */
    yagl_pixel_format_converter unpack;
    yagl_pixel_format_converter pack;
};

/*
 * Returns offset that needs to be applied to 'pixels' in
 * order to get correct data location. It can be non-0 when
 * GL_UNPACK_SKIP_XXX or GL_PACK_SKIP_XXX are non-0.
 * 'size' is unpacked size of the image.
 */
GLsizei yagl_pixel_format_get_info(struct yagl_pixel_format *pf,
                                   const struct yagl_gles_pixelstore *ps,
                                   GLsizei width,
                                   GLsizei height,
                                   GLsizei depth,
                                   GLsizei *size);

const GLvoid *yagl_pixel_format_unpack(struct yagl_pixel_format *pf,
                                       const struct yagl_gles_pixelstore *ps,
                                       GLsizei width,
                                       GLsizei height,
                                       GLsizei depth,
                                       const GLvoid *pixels);

GLvoid *yagl_pixel_format_pack_alloc(struct yagl_pixel_format *pf,
                                     const struct yagl_gles_pixelstore *ps,
                                     GLsizei width,
                                     GLsizei height,
                                     GLvoid *pixels);

void yagl_pixel_format_pack(struct yagl_pixel_format *pf,
                            const struct yagl_gles_pixelstore *ps,
                            GLsizei width,
                            GLsizei height,
                            const GLvoid *pixels_src,
                            GLvoid *pixels_dst);

#endif
