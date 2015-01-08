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

#include "GLES2/gl2.h"
#include "yagl_gles2_shader.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"

static void yagl_gles2_shader_destroy(struct yagl_ref *ref)
{
    struct yagl_gles2_shader *shader = (struct yagl_gles2_shader*)ref;

    yagl_free(shader->source);

    yagl_host_glDeleteObjects(&shader->global_name, 1);

    yagl_object_cleanup(&shader->base);

    yagl_free(shader);
}

struct yagl_gles2_shader *yagl_gles2_shader_create(GLenum type)
{
    struct yagl_gles2_shader *shader;

    shader = yagl_malloc0(sizeof(*shader));

    yagl_object_init(&shader->base, &yagl_gles2_shader_destroy);

    shader->is_shader = 1;
    shader->global_name = yagl_get_global_name();
    shader->type = type;

    yagl_host_glCreateShader(shader->global_name, type);

    return shader;
}

void yagl_gles2_shader_source(struct yagl_gles2_shader *shader,
                              GLchar *source,
                              const GLchar *patched_source,
                              int patched_len)
{
    yagl_host_glShaderSource(shader->global_name,
                             patched_source,
                             patched_len + 1);

    yagl_free(shader->source);
    shader->source = source;
}

void yagl_gles2_shader_acquire(struct yagl_gles2_shader *shader)
{
    if (shader) {
        yagl_object_acquire(&shader->base);
    }
}

void yagl_gles2_shader_release(struct yagl_gles2_shader *shader)
{
    if (shader) {
        yagl_object_release(&shader->base);
    }
}
