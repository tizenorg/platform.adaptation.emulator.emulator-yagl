#ifndef _YAGL_GLES3_CONTEXT_H_
#define _YAGL_GLES3_CONTEXT_H_

#include "yagl_gles2_context.h"

struct yagl_gles_buffer;
struct yagl_gles3_buffer_binding;

struct yagl_gles3_context
{
    struct yagl_gles2_context base;

    int num_program_binary_formats;

    struct yagl_gles_buffer *ubo;

    struct yagl_gles3_buffer_binding *uniform_buffer_bindings;
    int num_uniform_buffer_bindings;

    GLint uniform_buffer_offset_alignment;
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

#endif
