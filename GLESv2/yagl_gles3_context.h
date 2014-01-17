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

    int num_program_binary_formats;

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

#endif
