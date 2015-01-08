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

#ifndef _YAGL_GLES1_CONTEXT_H_
#define _YAGL_GLES1_CONTEXT_H_

#include "yagl_gles_context.h"

/*
 * GLES1 has arrays of vertices, normals, colors, texture coordinates and
 * point sizes. Every texture unit has its own texture coordinates array.
 */
typedef enum
{
    yagl_gles1_array_vertex = 0,
    yagl_gles1_array_color,
    yagl_gles1_array_normal,
    yagl_gles1_array_pointsize,
    yagl_gles1_array_texcoord,
} yagl_gles1_array_type;

struct yagl_gles1_context
{
    struct yagl_gles_context base;

    /*
     * From 'base.base.sg'.
     */
    struct yagl_sharegroup *sg;

    /* GL_OES_matrix_palette */
    int matrix_palette;

    int client_active_texture;

    int max_clip_planes;

    int max_lights;

    int max_tex_size;
};

struct yagl_client_context *yagl_gles1_context_create(struct yagl_sharegroup *sg);

#endif
