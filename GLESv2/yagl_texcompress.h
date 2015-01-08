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

#ifndef _YAGL_TEXCOMPRESS_H_
#define _YAGL_TEXCOMPRESS_H_

#include "yagl_types.h"

struct yagl_texcompress_format
{
    void (*unpack)(struct yagl_texcompress_format */*format*/,
                   const GLvoid */*src*/,
                   GLsizei /*width*/,
                   GLsizei /*height*/,
                   GLsizei /*src_stride*/,
                   GLvoid */*dst*/,
                   GLsizei /*dst_stride*/);

    GLenum src_format;
    GLint block_width;
    GLint block_height;
    GLint block_bytes;

    GLenum dst_format;
    GLenum dst_internalformat;
    GLenum dst_type;
};

int yagl_texcompress_get_format_names(GLenum *formats);

struct yagl_texcompress_format *yagl_texcompress_get_format(GLenum format);

int yagl_texcompress_get_info(struct yagl_texcompress_format *format,
                              GLsizei width,
                              GLsizei height,
                              GLsizei src_size,
                              GLsizei *src_stride,
                              GLsizei *dst_stride,
                              GLsizei *dst_size);

#endif
