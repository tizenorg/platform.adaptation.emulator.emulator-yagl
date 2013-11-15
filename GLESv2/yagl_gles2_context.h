#ifndef _YAGL_GLES2_CONTEXT_H_
#define _YAGL_GLES2_CONTEXT_H_

#include "yagl_gles_context.h"

struct yagl_gles2_program;

struct yagl_gles2_context
{
    struct yagl_gles_context base;

    char *(*shader_patch)(struct yagl_gles2_context */*ctx*/,
                          const char */*source*/,
                          int /*len*/,
                          int */*patched_len*/);

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

    int instanced_arrays;

    struct yagl_gles2_program *program;

    GLclampf blend_color[4];
};

/*
 * GLESv2/v3 common.
 * @{
 */

void yagl_gles2_context_init(struct yagl_gles2_context *ctx,
                             yagl_client_api client_api,
                             struct yagl_sharegroup *sg);

void yagl_gles2_context_cleanup(struct yagl_gles2_context *ctx);

void yagl_gles2_context_prepare(struct yagl_gles2_context *ctx);

struct yagl_gles_array
    *yagl_gles2_context_create_arrays(struct yagl_gles_context *ctx);

GLenum yagl_gles2_context_compressed_tex_image(struct yagl_gles_context *ctx,
                                               GLenum target,
                                               GLint level,
                                               GLenum internalformat,
                                               GLsizei width,
                                               GLsizei height,
                                               GLint border,
                                               GLsizei imageSize,
                                               const GLvoid *data);

int yagl_gles2_context_get_integerv(struct yagl_gles_context *ctx,
                                    GLenum pname,
                                    GLint *params,
                                    uint32_t *num_params);

int yagl_gles2_context_get_floatv(struct yagl_gles_context *ctx,
                                  GLenum pname,
                                  GLfloat *params,
                                  uint32_t *num_params,
                                  int *needs_map);

void yagl_gles2_context_draw_arrays(struct yagl_gles_context *ctx,
                                    GLenum mode,
                                    GLint first,
                                    GLsizei count,
                                    GLsizei primcount);

void yagl_gles2_context_draw_elements(struct yagl_gles_context *ctx,
                                      GLenum mode,
                                      GLsizei count,
                                      GLenum type,
                                      const GLvoid *indices,
                                      int32_t indices_count,
                                      GLsizei primcount);

char *yagl_gles2_context_shader_patch(struct yagl_gles2_context *ctx,
                                      const char *source,
                                      int len,
                                      int *patched_len);

/*
 * @}
 */

struct yagl_client_context *yagl_gles2_context_create(struct yagl_sharegroup *sg);

void yagl_gles2_context_use_program(struct yagl_gles2_context *ctx,
                                    struct yagl_gles2_program *program);

void yagl_gles2_context_unuse_program(struct yagl_gles2_context *ctx,
                                      struct yagl_gles2_program *program);

int yagl_gles2_context_get_array_param(struct yagl_gles2_context *ctx,
                                       GLuint index,
                                       GLenum pname,
                                       GLint *param);

#endif
