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

#include "GLES3/gl3.h"
#include "yagl_gles3_transform_feedback.h"
#include "yagl_gles3_buffer_binding.h"
#include "yagl_gles_buffer.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"

static void yagl_gles3_transform_feedback_destroy(struct yagl_ref *ref)
{
    struct yagl_gles3_transform_feedback *tf = (struct yagl_gles3_transform_feedback*)ref;
    GLuint i;

    for (i = 0; i < tf->num_buffer_bindings; ++i) {
        yagl_gles3_buffer_binding_reset(&tf->buffer_bindings[i]);
    }

    yagl_free(tf->buffer_bindings);

    if (tf->global_name) {
        yagl_host_glDeleteObjects(&tf->global_name, 1);
    }

    yagl_object_cleanup(&tf->base);

    yagl_free(tf);
}

struct yagl_gles3_transform_feedback
    *yagl_gles3_transform_feedback_create(int fake,
                                          GLuint num_buffer_bindings)
{
    struct yagl_gles3_transform_feedback *tf;
    GLuint i;

    tf = yagl_malloc0(sizeof(*tf));

    yagl_object_init(&tf->base, &yagl_gles3_transform_feedback_destroy);

    if (fake) {
        tf->was_bound = 1;
    } else {
        tf->global_name = yagl_get_global_name();

        yagl_host_glGenTransformFeedbacks(&tf->global_name, 1);
    }

    tf->num_buffer_bindings = num_buffer_bindings;

    tf->buffer_bindings = yagl_malloc0(sizeof(tf->buffer_bindings[0]) *
                                       tf->num_buffer_bindings);

    for (i = 0; i < tf->num_buffer_bindings; ++i) {
        yagl_gles3_buffer_binding_init(&tf->buffer_bindings[i],
                                       GL_TRANSFORM_FEEDBACK_BUFFER,
                                       i);
    }

    return tf;
}

void yagl_gles3_transform_feedback_acquire(struct yagl_gles3_transform_feedback *tf)
{
    if (tf) {
        yagl_object_acquire(&tf->base);
    }
}

void yagl_gles3_transform_feedback_release(struct yagl_gles3_transform_feedback *tf)
{
    if (tf) {
        yagl_object_release(&tf->base);
    }
}

int yagl_gles3_transform_feedback_bind_buffer_base(struct yagl_gles3_transform_feedback *tf,
                                                   GLuint index,
                                                   struct yagl_gles_buffer *buffer)
{
    if (index >= tf->num_buffer_bindings) {
        return 0;
    }

    yagl_gles3_buffer_binding_set_base(&tf->buffer_bindings[index], buffer);

    return 1;
}

int yagl_gles3_transform_feedback_bind_buffer_range(struct yagl_gles3_transform_feedback *tf,
                                                    GLuint index,
                                                    GLintptr offset,
                                                    GLsizeiptr size,
                                                    struct yagl_gles_buffer *buffer)
{
    if (index >= tf->num_buffer_bindings) {
        return 0;
    }

    yagl_gles3_buffer_binding_set_range(&tf->buffer_bindings[index],
                                        buffer, offset, size);

    return 1;
}

void yagl_gles3_transform_feedback_unbind_buffer(struct yagl_gles3_transform_feedback *tf,
                                                 yagl_object_name buffer_local_name)
{
    GLuint i;

    for (i = 0; i < tf->num_buffer_bindings; ++i) {
        struct yagl_gles_buffer *buffer = tf->buffer_bindings[i].buffer;

        if (buffer && (buffer->base.local_name == buffer_local_name)) {
            yagl_gles3_buffer_binding_reset(&tf->buffer_bindings[i]);
        }
    }
}

void yagl_gles3_transform_feedback_bind(struct yagl_gles3_transform_feedback *tf,
                                        GLenum target)
{
    yagl_host_glBindTransformFeedback(target, tf->global_name);

    tf->was_bound = 1;
}

int yagl_gles3_transform_feedback_was_bound(struct yagl_gles3_transform_feedback *tf)
{
    return tf->was_bound;
}

void yagl_gles3_transform_feedback_begin(struct yagl_gles3_transform_feedback *tf,
                                         GLenum primitive_mode,
                                         GLuint num_active_buffer_bindings)
{
    GLuint i;

    if (num_active_buffer_bindings > tf->num_buffer_bindings) {
        return;
    }

    tf->num_active_buffer_bindings = num_active_buffer_bindings;

    for (i = 0; i < num_active_buffer_bindings; ++i) {
        yagl_gles3_buffer_binding_transfer_begin(&tf->buffer_bindings[i]);
    }

    yagl_host_glBeginTransformFeedback(primitive_mode);

    tf->active = 1;
}

void yagl_gles3_transform_feedback_pause(struct yagl_gles3_transform_feedback *tf)
{
    tf->paused = 1;

    yagl_host_glPauseTransformFeedback();
}

void yagl_gles3_transform_feedback_resume(struct yagl_gles3_transform_feedback *tf)
{
    yagl_host_glResumeTransformFeedback();

    tf->paused = 0;
}

void yagl_gles3_transform_feedback_post_draw(struct yagl_gles3_transform_feedback *tf)
{
    GLuint i;

    for (i = 0; i < tf->num_active_buffer_bindings; ++i) {
        yagl_gles3_buffer_binding_set_gpu_dirty(&tf->buffer_bindings[i]);
    }
}

void yagl_gles3_transform_feedback_end(struct yagl_gles3_transform_feedback *tf)
{
    GLuint i;

    yagl_host_glEndTransformFeedback();

    for (i = 0; i < tf->num_active_buffer_bindings; ++i) {
        yagl_gles3_buffer_binding_transfer_end(&tf->buffer_bindings[i]);
    }

    tf->active = 0;
    tf->paused = 0;
    tf->num_active_buffer_bindings = 0;
}
