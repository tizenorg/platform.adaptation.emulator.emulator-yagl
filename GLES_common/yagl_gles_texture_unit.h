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

#ifndef _YAGL_GLES_TEXTURE_UNIT_H_
#define _YAGL_GLES_TEXTURE_UNIT_H_

#include "yagl_gles_types.h"
#include "yagl_sharegroup.h"

struct yagl_gles_texture;
struct yagl_gles_sampler;

struct yagl_gles_texture_target_state
{
    struct yagl_gles_texture *texture;

    /*
     * For GLESv1 only. In GLESv2 2D texture and cubemap textures cannot be
     * enabled/disabled. Currently not used.
     */
    int enabled;

    struct yagl_gles_texture *texture_zero;
};

struct yagl_gles_texture_unit
{
    struct yagl_gles_texture_target_state target_states[YAGL_NUM_TEXTURE_TARGETS];

    struct yagl_gles_sampler *sampler;
};

void yagl_gles_texture_unit_init(struct yagl_gles_texture_unit *texture_unit,
                                 struct yagl_sharegroup *sg);

void yagl_gles_texture_unit_cleanup(struct yagl_gles_texture_unit *texture_unit);

#endif
