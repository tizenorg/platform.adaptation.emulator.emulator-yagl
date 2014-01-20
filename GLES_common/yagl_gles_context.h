#ifndef _YAGL_GLES_CONTEXT_H_
#define _YAGL_GLES_CONTEXT_H_

#include "yagl_gles_types.h"
#include "yagl_client_context.h"
#include "yagl_namespace.h"

struct yagl_gles_vertex_array;
struct yagl_gles_array;
struct yagl_gles_buffer;
struct yagl_gles_texture;
struct yagl_gles_texture_unit;
struct yagl_gles_framebuffer;
struct yagl_gles_renderbuffer;

struct yagl_gles_context
{
    struct yagl_client_context base;

    struct yagl_gles_array *(*create_arrays)(struct yagl_gles_context */*ctx*/);

    const GLchar *(*get_string)(struct yagl_gles_context */*ctx*/,
                                GLenum /*name*/);

    void (*compressed_tex_image)(struct yagl_gles_context */*ctx*/,
                                 GLenum /*target*/,
                                 GLint /*level*/,
                                 GLenum /*internalformat*/,
                                 GLsizei /*width*/,
                                 GLsizei /*height*/,
                                 GLint /*border*/,
                                 GLsizei /*imageSize*/,
                                 const GLvoid */*data*/);

    void (*compressed_tex_sub_image)(struct yagl_gles_context */*ctx*/,
                                     GLenum /*target*/,
                                     GLint /*level*/,
                                     GLint /*xoffset*/,
                                     GLint /*yoffset*/,
                                     GLsizei /*width*/,
                                     GLsizei /*height*/,
                                     GLenum /*format*/,
                                     GLsizei /*imageSize*/,
                                     const GLvoid */*data*/);

    int (*enable)(struct yagl_gles_context */*ctx*/,
                  GLenum /*cap*/,
                  GLboolean /*enable*/);

    int (*is_enabled)(struct yagl_gles_context */*ctx*/,
                      GLenum /*cap*/,
                      GLboolean */*enabled*/);

    int (*get_integerv)(struct yagl_gles_context */*ctx*/,
                        GLenum /*pname*/,
                        GLint */*params*/,
                        uint32_t */*num_params*/);

    int (*get_floatv)(struct yagl_gles_context */*ctx*/,
                      GLenum /*pname*/,
                      GLfloat */*params*/,
                      uint32_t */*num_params*/,
                      int */*needs_map*/);

    /*
     * 'primcount' < 0 - usual draw.
     * 'primcount' >= 0 - instanced draw.
     */
    void (*draw_arrays)(struct yagl_gles_context */*ctx*/,
                        GLenum /*mode*/,
                        GLint /*first*/,
                        GLsizei /*count*/,
                        GLsizei /*primcount*/);

    /*
     * 'primcount' < 0 - usual draw.
     * 'primcount' >= 0 - instanced draw.
     */
    void (*draw_elements)(struct yagl_gles_context */*ctx*/,
                          GLenum /*mode*/,
                          GLsizei /*count*/,
                          GLenum /*type*/,
                          const GLvoid */*indices*/,
                          int32_t /*indices_count*/,
                          GLsizei /*primcount*/);

    int (*bind_buffer)(struct yagl_gles_context */*ctx*/,
                       GLenum /*target*/,
                       struct yagl_gles_buffer */*buffer*/);

    void (*unbind_buffer)(struct yagl_gles_context */*ctx*/,
                          yagl_object_name /*buffer_local_name*/);

    int (*acquire_binded_buffer)(struct yagl_gles_context */*ctx*/,
                                 GLenum /*target*/,
                                 struct yagl_gles_buffer **/*buffer*/);

    int (*validate_texture_target)(struct yagl_gles_context */*ctx*/,
                                   GLenum /*target*/,
                                   yagl_gles_texture_target */*texture_target*/);

    int (*validate_texture_internalformat)(struct yagl_gles_context */*ctx*/,
                                           GLenum */*internalformat*/,
                                           GLenum */*any_format*/,
                                           GLenum */*any_type*/);

    int (*get_stride)(struct yagl_gles_context */*ctx*/,
                      GLsizei /*alignment*/,
                      GLsizei /*width*/,
                      GLenum /*format*/,
                      GLenum /*type*/,
                      GLsizei */*stride*/);

    struct yagl_namespace framebuffers;

    struct yagl_namespace vertex_arrays;

    const GLchar **extensions;
    int num_extensions;

    GLchar *extension_string;

    GLenum error;

    struct yagl_gles_vertex_array *va_zero;

    /*
     * GLES texture units, the number of texture units is determined
     * at runtime.
     */
    struct yagl_gles_texture_unit *texture_units;
    int num_texture_units;

    int num_arrays;

    int vertex_arrays_supported;

    int packed_depth_stencil;

    int texture_npot;

    int texture_rectangle;

    int texture_filter_anisotropic;

    int max_color_attachments;

    int max_draw_buffers;

    int active_texture_unit;

    struct yagl_gles_vertex_array *vao;

    struct yagl_gles_buffer *vbo;

    struct yagl_gles_framebuffer *fbo_draw;
    struct yagl_gles_framebuffer *fbo_read;

    struct yagl_gles_renderbuffer *rbo;

    GLenum blend_equation_rgb;
    GLenum blend_equation_alpha;

    GLenum blend_src_rgb;
    GLenum blend_dst_rgb;

    GLenum blend_src_alpha;
    GLenum blend_dst_alpha;

    GLclampf clear_color[4];

    GLclampf clear_depth;

    GLboolean color_writemask[4];

    GLenum cull_face_mode;

    GLenum depth_func;

    GLboolean depth_writemask;

    GLclampf depth_range[2];

    GLenum front_face_mode;

    GLint pack_alignment;
    GLint unpack_alignment;

    int have_viewport;
    GLint viewport[4];

    GLenum draw_buffers[YAGL_MAX_GLES_DRAW_BUFFERS];

    GLboolean blend_enabled;
    GLboolean cull_face_enabled;
    GLboolean depth_test_enabled;
    GLboolean dither_enabled;
    GLboolean polygon_offset_fill_enabled;
    GLboolean sample_alpha_to_coverage_enabled;
    GLboolean sample_coverage_enabled;
    GLboolean scissor_test_enabled;
    GLboolean stencil_test_enabled;
};

void yagl_gles_context_init(struct yagl_gles_context *ctx,
                            yagl_client_api client_api,
                            struct yagl_sharegroup *sg);

void yagl_gles_context_prepare(struct yagl_gles_context *ctx,
                               int num_texture_units,
                               int num_arrays);

/*
 * Takes ownership of 'extensions'.
 */
void yagl_gles_context_prepare_end(struct yagl_gles_context *ctx,
                                   const GLchar **extensions,
                                   int num_extensions);

void yagl_gles_context_cleanup(struct yagl_gles_context *ctx);

void yagl_gles_context_set_error(struct yagl_gles_context *ctx, GLenum error);

GLenum yagl_gles_context_get_error(struct yagl_gles_context *ctx);

int yagl_gles_context_get_stride(struct yagl_gles_context *ctx,
                                 GLsizei alignment,
                                 GLsizei width,
                                 GLenum format,
                                 GLenum type,
                                 GLsizei *stride);

void yagl_gles_context_bind_vertex_array(struct yagl_gles_context *ctx,
                                         struct yagl_gles_vertex_array *va);

void yagl_gles_context_unbind_vertex_array(struct yagl_gles_context *ctx,
                                           yagl_object_name va_local_name);

int yagl_gles_context_validate_texture_target(struct yagl_gles_context *ctx,
                                              GLenum target,
                                              yagl_gles_texture_target *texture_target);

int yagl_gles_context_validate_texture_internalformat(struct yagl_gles_context *ctx,
                                                      GLenum *internalformat,
                                                      GLenum *any_format,
                                                      GLenum *any_type);

void yagl_gles_context_set_active_texture(struct yagl_gles_context *ctx,
                                          GLenum texture);

struct yagl_gles_texture_unit
    *yagl_gles_context_get_active_texture_unit(struct yagl_gles_context *ctx);

struct yagl_gles_texture_target_state
    *yagl_gles_context_get_active_texture_target_state(struct yagl_gles_context *ctx,
                                                       yagl_gles_texture_target texture_target);

void yagl_gles_context_active_texture_set_enabled(struct yagl_gles_context *ctx,
    yagl_gles_texture_target texture_target, int enabled);

void yagl_gles_context_bind_texture(struct yagl_gles_context *ctx,
                                    GLenum target,
                                    struct yagl_gles_texture *texture);

void yagl_gles_context_unbind_texture(struct yagl_gles_context *ctx,
                                      yagl_object_name texture_local_name);

void yagl_gles_context_bind_buffer(struct yagl_gles_context *ctx,
                                   GLenum target,
                                   struct yagl_gles_buffer *buffer);

void yagl_gles_context_unbind_buffer(struct yagl_gles_context *ctx,
                                     yagl_object_name buffer_local_name);

void yagl_gles_context_bind_framebuffer(struct yagl_gles_context *ctx,
                                        GLenum target,
                                        struct yagl_gles_framebuffer *fbo);

void yagl_gles_context_unbind_framebuffer(struct yagl_gles_context *ctx,
                                          yagl_object_name fbo_local_name);

void yagl_gles_context_bind_renderbuffer(struct yagl_gles_context *ctx,
                                         GLenum target,
                                         struct yagl_gles_renderbuffer *rbo);

void yagl_gles_context_unbind_renderbuffer(struct yagl_gles_context *ctx,
                                           yagl_object_name rbo_local_name);

int yagl_gles_context_acquire_binded_buffer(struct yagl_gles_context *ctx,
                                             GLenum target,
                                             struct yagl_gles_buffer **buffer);

int yagl_gles_context_acquire_binded_framebuffer(struct yagl_gles_context *ctx,
                                                 GLenum target,
                                                 struct yagl_gles_framebuffer **fb);

void yagl_gles_context_enable(struct yagl_gles_context *ctx,
                              GLenum cap,
                              GLboolean enable);

GLboolean yagl_gles_context_is_enabled(struct yagl_gles_context *ctx,
                                       GLenum cap);

int yagl_gles_context_get_integerv(struct yagl_gles_context *ctx,
                                   GLenum pname,
                                   GLint *params,
                                   uint32_t *num_params);

int yagl_gles_context_get_floatv(struct yagl_gles_context *ctx,
                                 GLenum pname,
                                 GLfloat *params,
                                 uint32_t *num_params,
                                 int *needs_map);

void yagl_gles_context_draw_arrays(struct yagl_gles_context *ctx,
                                   GLenum mode, GLint first, GLsizei count,
                                   GLsizei primcount);

void yagl_gles_context_draw_elements(struct yagl_gles_context *ctx,
                                     GLenum mode, GLsizei count,
                                     GLenum type, const GLvoid *indices,
                                     GLsizei primcount);

void yagl_gles_context_line_width(struct yagl_gles_context *ctx,
                                  GLfloat width);

void yagl_gles_context_tex_parameterf(struct yagl_gles_context *ctx,
                                      GLenum target,
                                      GLenum pname,
                                      GLfloat param);

void yagl_gles_context_tex_parameterfv(struct yagl_gles_context *ctx,
                                       GLenum target,
                                       GLenum pname,
                                       const GLfloat *params);

int yagl_gles_context_get_tex_parameterfv(struct yagl_gles_context *ctx,
                                          GLenum target,
                                          GLenum pname,
                                          GLfloat *params);

int yagl_gles_context_get_tex_parameteriv(struct yagl_gles_context *ctx,
                                          GLenum target,
                                          GLenum pname,
                                          GLint *params);

void yagl_gles_context_clear_color(struct yagl_gles_context *ctx,
                                   GLclampf red,
                                   GLclampf green,
                                   GLclampf blue,
                                   GLclampf alpha);

void yagl_gles_context_clear_depthf(struct yagl_gles_context *ctx,
                                    GLclampf depth);

void yagl_gles_context_sample_coverage(struct yagl_gles_context *ctx,
                                       GLclampf value,
                                       GLboolean invert);

void yagl_gles_context_depth_rangef(struct yagl_gles_context *ctx,
                                    GLclampf zNear,
                                    GLclampf zFar);

void yagl_gles_context_polygon_offset(struct yagl_gles_context *ctx,
                                      GLfloat factor,
                                      GLfloat units);

#endif
