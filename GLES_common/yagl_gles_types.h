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

#ifndef _YAGL_GLES_TYPES_H_
#define _YAGL_GLES_TYPES_H_

#include "yagl_types.h"

struct yagl_gles_buffer;

typedef enum
{
    yagl_gles_texture_target_2d = 0,
    yagl_gles_texture_target_2d_array = 1,
    yagl_gles_texture_target_3d = 2,
    yagl_gles_texture_target_cubemap = 3,
    yagl_gles_texture_target_external_oes = 4
} yagl_gles_texture_target;

typedef enum
{
    yagl_gles_framebuffer_attachment_depth = 0,
    yagl_gles_framebuffer_attachment_stencil = 1,
    yagl_gles_framebuffer_attachment_color0 = 2
} yagl_gles_framebuffer_attachment;

#define YAGL_MAX_GLES_FRAMEBUFFER_COLOR_ATTACHMENTS 16

typedef enum
{
    yagl_gles_format_color_renderable = (1 << 0),
    yagl_gles_format_depth_renderable = (1 << 1),
    yagl_gles_format_stencil_renderable = (1 << 2),
    yagl_gles_format_sized = (1 << 3),
    yagl_gles_format_signed_integer = (1 << 4),
    yagl_gles_format_unsigned_integer = (1 << 5),
    yagl_gles_format_float = (1 << 6),
    yagl_gles_format_srgb = (1 << 7)
} yagl_gles_format_flag;

struct yagl_gles_pixelstore
{
    GLint alignment;
    GLint row_length;
    GLint image_height;
    GLint skip_pixels;
    GLint skip_rows;
    GLint skip_images;
    struct yagl_gles_buffer *pbo;
};

struct yagl_gles_format_info
{
    uint32_t flags;
    uint32_t num_components;
    uint32_t red_size;
    uint32_t green_size;
    uint32_t blue_size;
    uint32_t alpha_size;
    uint32_t depth_size;
    uint32_t stencil_size;
};

#endif
