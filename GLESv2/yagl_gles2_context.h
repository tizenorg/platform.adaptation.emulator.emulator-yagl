#ifndef _YAGL_GLES2_CONTEXT_H_
#define _YAGL_GLES2_CONTEXT_H_

#include "yagl_gles_context.h"

struct yagl_gles2_program;

struct yagl_gles2_context
{
    struct yagl_gles_context base;

    /*
     * From 'base.base.sg'.
     */
    struct yagl_sharegroup *sg;

    /*
     * Generate program uniform locations ourselves or vmexit
     * and ask host.
     */
    int gen_locations;

    int num_compressed_texture_formats;

    int num_shader_binary_formats;

    int texture_half_float;

    int vertex_half_float;

    int standard_derivatives;

    struct yagl_gles2_program *program;

    GLclampf blend_color[4];
};

struct yagl_client_context *yagl_gles2_context_create(struct yagl_sharegroup *sg);

void yagl_gles2_context_use_program(struct yagl_gles2_context *ctx,
                                    struct yagl_gles2_program *program);

void yagl_gles2_context_unuse_program(struct yagl_gles2_context *ctx,
                                      struct yagl_gles2_program *program);

#endif
