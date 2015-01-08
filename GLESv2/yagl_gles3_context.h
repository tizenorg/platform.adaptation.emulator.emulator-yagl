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

#ifndef _YAGL_GLES3_CONTEXT_H_
#define _YAGL_GLES3_CONTEXT_H_

#include "yagl_gles2_context.h"
#include "yagl_namespace.h"
#include "yagl_list.h"

struct yagl_gles_buffer;
struct yagl_gles_sampler;
struct yagl_gles3_buffer_binding;
struct yagl_gles3_transform_feedback;
struct yagl_gles3_query;

struct yagl_gles3_context
{
    struct yagl_gles2_context base;

    struct yagl_namespace transform_feedbacks;
    struct yagl_namespace queries;

    GLboolean primitive_restart_fixed_index;
    GLboolean rasterizer_discard;

    GLenum fragment_shader_derivative_hint;

    int have_max_array_texture_layers;
    GLint max_array_texture_layers;

    int have_max_combined_fragment_uniform_components;
    GLint max_combined_fragment_uniform_components;

    int have_max_combined_uniform_blocks;
    GLint max_combined_uniform_blocks;

    int have_max_combined_vertex_uniform_components;
    GLint max_combined_vertex_uniform_components;

    int have_max_element_index;
    GLint max_element_index;

    int have_max_elements_indices;
    GLint max_elements_indices;

    int have_max_elements_vertices;
    GLint max_elements_vertices;

    int have_max_fragment_input_components;
    GLint max_fragment_input_components;

    int have_max_fragment_uniform_blocks;
    GLint max_fragment_uniform_blocks;

    int have_max_fragment_uniform_components;
    GLint max_fragment_uniform_components;

    int have_max_program_texel_offset;
    GLint max_program_texel_offset;

    int have_max_samples;
    GLint max_samples;

    int have_max_texture_lod_bias;
    GLint max_texture_lod_bias;

    int have_max_transform_feedback_interleaved_components;
    GLint max_transform_feedback_interleaved_components;

    int have_max_transform_feedback_separate_components;
    GLint max_transform_feedback_separate_components;

    int have_max_uniform_block_size;
    GLint max_uniform_block_size;

    int have_max_vertex_output_components;
    GLint max_vertex_output_components;

    int have_max_vertex_uniform_blocks;
    GLint max_vertex_uniform_blocks;

    int have_max_vertex_uniform_components;
    GLint max_vertex_uniform_components;

    int have_min_program_texel_offset;
    GLint min_program_texel_offset;

    /*
     * Uniform buffer objects.
     * @{
     */

    struct yagl_gles_buffer *ubo;

    struct yagl_gles3_buffer_binding *uniform_buffer_bindings;
    GLuint num_uniform_buffer_bindings;

    struct yagl_list active_uniform_buffer_bindings;

    GLint uniform_buffer_offset_alignment;

    /*
     * @}
     */

    /*
     * Transform feedbacks.
     * @{
     */

    struct yagl_gles_buffer *tfbo;
    struct yagl_gles3_transform_feedback *tf_zero;
    struct yagl_gles3_transform_feedback *tfo;
    GLenum tf_primitive_mode;

    GLint max_transform_feedback_separate_attribs;

    /*
     * @}
     */

    /*
     * Queries.
     * @{
     */

    struct yagl_gles3_query *tf_primitives_written_query;
    struct yagl_gles3_query *occlusion_query;

    /*
     * @}
     */

    /*
     * Copy buffer objects.
     * @{
     */

    struct yagl_gles_buffer *crbo;
    struct yagl_gles_buffer *cwbo;

    /*
     * @}
     */
};

struct yagl_client_context *yagl_gles3_context_create(struct yagl_sharegroup *sg);

void yagl_gles3_context_bind_buffer_base(struct yagl_gles3_context *ctx,
                                         GLenum target,
                                         GLuint index,
                                         struct yagl_gles_buffer *buffer);

void yagl_gles3_context_bind_buffer_range(struct yagl_gles3_context *ctx,
                                          GLenum target,
                                          GLuint index,
                                          GLintptr offset,
                                          GLsizeiptr size,
                                          struct yagl_gles_buffer *buffer);

void yagl_gles3_context_bind_transform_feedback(struct yagl_gles3_context *ctx,
                                                GLenum target,
                                                struct yagl_gles3_transform_feedback *tfo);

void yagl_gles3_context_begin_query(struct yagl_gles3_context *ctx,
                                    GLenum target,
                                    struct yagl_gles3_query *query);

void yagl_gles3_context_end_query(struct yagl_gles3_context *ctx,
                                  GLenum target);

int yagl_gles3_context_acquire_active_query(struct yagl_gles3_context *ctx,
                                            GLenum target,
                                            struct yagl_gles3_query **query);

int yagl_gles3_context_bind_sampler(struct yagl_gles3_context *ctx,
                                    GLuint unit,
                                    struct yagl_gles_sampler *sampler);

void yagl_gles3_context_unbind_sampler(struct yagl_gles3_context *ctx,
                                       yagl_object_name sampler_local_name);

int yagl_gles3_context_get_integerv_indexed(struct yagl_gles3_context *ctx,
                                            GLenum target,
                                            GLuint index,
                                            GLint *params,
                                            uint32_t *num_params);

void yagl_gles3_context_draw_range_elements(struct yagl_gles3_context *ctx,
                                            GLenum mode,
                                            GLuint start,
                                            GLuint end,
                                            GLsizei count,
                                            GLenum type,
                                            const GLvoid *indices,
                                            int32_t indices_count);

#endif
