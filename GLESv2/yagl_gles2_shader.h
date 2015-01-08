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

#ifndef _YAGL_GLES2_SHADER_H_
#define _YAGL_GLES2_SHADER_H_

#include "yagl_types.h"
#include "yagl_object.h"

/*
 * Programs and shaders share the same namespace,
 * pretty clumsy!
 */
#define YAGL_NS_SHADER_PROGRAM 3

struct yagl_gles2_shader
{
    /*
     * These members must be exactly as in yagl_gles2_program
     * @{
     */
    struct yagl_object base;

    int is_shader;
    /*
     * @}
     */

    yagl_object_name global_name;

    GLenum type;

    GLchar *source;
};

struct yagl_gles2_shader *yagl_gles2_shader_create(GLenum type);

/*
 * Takes ownership of 'source'.
 */
void yagl_gles2_shader_source(struct yagl_gles2_shader *shader,
                              GLchar *source,
                              const GLchar *patched_source,
                              int patched_len);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles2_shader_acquire(struct yagl_gles2_shader *shader);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles2_shader_release(struct yagl_gles2_shader *shader);

#endif
