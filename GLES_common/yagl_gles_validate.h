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

#ifndef _YAGL_GLES_VALIDATE_H_
#define _YAGL_GLES_VALIDATE_H_

#include "yagl_gles_types.h"

int yagl_gles_is_stencil_op_valid(GLenum op);

int yagl_gles_is_stencil_func_valid(GLenum func);

int yagl_gles_is_hint_mode_valid(GLenum mode);

int yagl_gles_is_draw_mode_valid(GLenum mode);

int yagl_gles_is_buffer_usage_valid(GLenum usage);

int yagl_gles_is_blend_equation_valid(GLenum mode);

int yagl_gles_is_blend_func_valid(GLenum func);

int yagl_gles_is_cull_face_mode_valid(GLenum mode);

int yagl_gles_is_depth_func_valid(GLenum func);

int yagl_gles_is_front_face_mode_valid(GLenum mode);

int yagl_gles_is_alignment_valid(GLint alignment);

int yagl_gles_get_index_size(GLenum type, int *index_size);

int yagl_gles_validate_framebuffer_attachment(GLenum attachment,
    int num_color_attachments,
    yagl_gles_framebuffer_attachment *framebuffer_attachment);

int yagl_gles_validate_texture_target_squash(GLenum target,
    GLenum *squashed_target);

#endif
