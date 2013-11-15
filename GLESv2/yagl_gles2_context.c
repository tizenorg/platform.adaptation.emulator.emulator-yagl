#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "yagl_gles2_context.h"
#include "yagl_gles2_program.h"
#include "yagl_gles2_utils.h"
#include "yagl_gles_array.h"
#include "yagl_gles_buffer.h"
#include "yagl_gles_texture.h"
#include "yagl_gles_texture_unit.h"
#include "yagl_gles_vertex_array.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"
#include <string.h>
#include <stdlib.h>

#define YAGL_SET_ERR(err) \
    yagl_gles_context_set_error(&ctx->base, err); \
    YAGL_LOG_ERROR("error = 0x%X", err)

/*
 * We can't include GL/glext.h here
 */
#define GL_POINT_SPRITE                   0x8861
#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642

static const GLchar *egl_image_ext = "GL_OES_EGL_image";
static const GLchar *depth24_ext = "GL_OES_depth24";
static const GLchar *depth32_ext = "GL_OES_depth32";
static const GLchar *texture_float_ext = "GL_OES_texture_float";
static const GLchar *texture_float_linear_ext = "GL_OES_texture_float_linear";
static const GLchar *texture_format_bgra8888_ext = "GL_EXT_texture_format_BGRA8888";
static const GLchar *depth_texture_ext = "GL_OES_depth_texture";
static const GLchar *framebuffer_blit_ext = "GL_ANGLE_framebuffer_blit";
static const GLchar *draw_buffers_ext = "GL_EXT_draw_buffers";
static const GLchar *mapbuffer_ext = "GL_OES_mapbuffer";
static const GLchar *map_buffer_range_ext = "GL_EXT_map_buffer_range";
static const GLchar *element_index_uint_ext = "GL_OES_element_index_uint";
static const GLchar *packed_depth_stencil_ext = "GL_OES_packed_depth_stencil";
static const GLchar *texture_npot_ext = "GL_OES_texture_npot";
static const GLchar *texture_rectangle_ext = "GL_ARB_texture_rectangle";
static const GLchar *texture_filter_anisotropic_ext = "GL_EXT_texture_filter_anisotropic";
static const GLchar *vertex_array_object_ext = "GL_OES_vertex_array_object";
static const GLchar *texture_half_float_ext = "GL_OES_texture_half_float";
static const GLchar *texture_half_float_linear_ext = "GL_OES_texture_half_float_linear";
static const GLchar *vertex_half_float_ext = "GL_OES_vertex_half_float";
static const GLchar *standard_derivatives_ext = "GL_OES_standard_derivatives";
static const GLchar *instanced_arrays_ext = "GL_EXT_instanced_arrays";

static const GLchar **yagl_gles2_context_get_extensions(struct yagl_gles2_context *ctx,
                                                        int *num_extensions)
{
    const GLchar **extensions;
    int i = 0;

    extensions = yagl_malloc(100 * sizeof(*extensions));

    extensions[i++] = egl_image_ext;
    extensions[i++] = depth24_ext;
    extensions[i++] = depth32_ext;
    extensions[i++] = texture_float_ext;
    extensions[i++] = texture_float_linear_ext;
    extensions[i++] = texture_format_bgra8888_ext;
    extensions[i++] = depth_texture_ext;
    extensions[i++] = framebuffer_blit_ext;
    extensions[i++] = draw_buffers_ext;
    extensions[i++] = mapbuffer_ext;
    extensions[i++] = map_buffer_range_ext;
    extensions[i++] = element_index_uint_ext;

    if (ctx->base.packed_depth_stencil) {
        extensions[i++] = packed_depth_stencil_ext;
    }

    if (ctx->base.texture_npot) {
        extensions[i++] = texture_npot_ext;
    }

    if (ctx->base.texture_rectangle) {
        extensions[i++] = texture_rectangle_ext;
    }

    if (ctx->base.texture_filter_anisotropic) {
        extensions[i++] = texture_filter_anisotropic_ext;
    }

    if (ctx->base.vertex_arrays_supported) {
        extensions[i++] = vertex_array_object_ext;
    }

    if (ctx->texture_half_float) {
        extensions[i++] = texture_half_float_ext;
        extensions[i++] = texture_half_float_linear_ext;
    }

    if (ctx->vertex_half_float) {
        extensions[i++] = vertex_half_float_ext;
    }

    if (ctx->standard_derivatives) {
        extensions[i++] = standard_derivatives_ext;
    }

    if (ctx->instanced_arrays) {
        extensions[i++] = instanced_arrays_ext;
    }

    *num_extensions = i;

    return extensions;
}

static inline void yagl_gles2_context_pre_draw(struct yagl_gles_context *ctx, GLenum mode)
{
    /*
     * Enable texture generation for GL_POINTS and gl_PointSize shader variable.
     * GLESv2 assumes this is enabled by default, we need to set this
     * state for GL.
     */

    if (mode == GL_POINTS) {
        if (yagl_get_host_gl_version() <= yagl_gl_2) {
            yagl_host_glEnable(GL_POINT_SPRITE);
        }
        yagl_host_glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    }
}

static inline void yagl_gles2_context_post_draw(struct yagl_gles_context *ctx, GLenum mode)
{
    if (mode == GL_POINTS) {
        yagl_host_glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
        if (yagl_get_host_gl_version() <= yagl_gl_2) {
            yagl_host_glDisable(GL_POINT_SPRITE);
        }
    }
}

static void yagl_gles2_array_apply(struct yagl_gles_array *array,
                                   uint32_t first,
                                   uint32_t count,
                                   const GLvoid *ptr,
                                   void *user_data)
{
    if (array->vbo) {
        yagl_host_glVertexAttribPointerOffset(array->index,
                                              array->size,
                                              array->actual_type,
                                              array->normalized,
                                              array->actual_stride,
                                              array->actual_offset);
    } else {
        yagl_host_glVertexAttribPointerData(array->index,
                                            array->size,
                                            array->actual_type,
                                            array->normalized,
                                            array->actual_stride,
                                            first,
                                            ptr + (first * array->actual_stride),
                                            count * array->actual_stride);
    }
}

static void yagl_gles2_context_prepare_internal(struct yagl_client_context *ctx)
{
    struct yagl_gles2_context *gles2_ctx = (struct yagl_gles2_context*)ctx;
    const GLchar **extensions;
    int num_extensions;

    yagl_gles2_context_prepare(gles2_ctx);

    extensions = yagl_gles2_context_get_extensions(gles2_ctx, &num_extensions);

    yagl_gles_context_prepare_end(&gles2_ctx->base, extensions, num_extensions);
}

static void yagl_gles2_context_destroy(struct yagl_client_context *ctx)
{
    struct yagl_gles2_context *gles2_ctx = (struct yagl_gles2_context*)ctx;

    YAGL_LOG_FUNC_ENTER(yagl_gles2_context_destroy, "%p", ctx);

    yagl_gles2_context_cleanup(gles2_ctx);

    yagl_free(gles2_ctx);

    YAGL_LOG_FUNC_EXIT(NULL);
}

static const GLchar
    *yagl_gles2_context_get_string(struct yagl_gles_context *ctx,
                                   GLenum name)
{
    const char *str = NULL;

    switch (name) {
    case GL_VERSION:
        str = "OpenGL ES 2.0";
        break;
    case GL_RENDERER:
        str = "YaGL GLESv2";
        break;
    case GL_SHADING_LANGUAGE_VERSION:
        str = "OpenGL ES GLSL ES 1.4";
        break;
    default:
        str = "";
    }

    return str;
}

static int yagl_gles2_context_enable(struct yagl_gles_context *ctx,
                                     GLenum cap,
                                     GLboolean enable)
{
    return 0;
}

static int yagl_gles2_context_is_enabled(struct yagl_gles_context *ctx,
                                         GLenum cap,
                                         GLboolean *enabled)
{
    return 0;
}

static int yagl_gles2_context_bind_buffer(struct yagl_gles_context *ctx,
                                          GLenum target,
                                          struct yagl_gles_buffer *buffer)
{
    return 0;
}

static void yagl_gles2_context_unbind_buffer(struct yagl_gles_context *ctx,
                                             yagl_object_name buffer_local_name)
{
}

static int yagl_gles2_context_acquire_binded_buffer(struct yagl_gles_context *ctx,
                                                    GLenum target,
                                                    struct yagl_gles_buffer **buffer)
{
    return 0;
}

void yagl_gles2_context_init(struct yagl_gles2_context *ctx,
                             yagl_client_api client_api,
                             struct yagl_sharegroup *sg)
{
    yagl_gles_context_init(&ctx->base, client_api, sg);

    ctx->sg = sg;
}

void yagl_gles2_context_cleanup(struct yagl_gles2_context *ctx)
{
    yagl_gles2_program_release(ctx->program);

    yagl_gles_context_cleanup(&ctx->base);
}

void yagl_gles2_context_prepare(struct yagl_gles2_context *ctx)
{
    GLint num_texture_units = 0;
    int32_t size = 0;
    char *extensions, *conformant;
    int num_arrays = 1;

    YAGL_LOG_FUNC_ENTER(yagl_gles2_context_prepare, "%p", ctx);

    yagl_host_glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &num_arrays, 1, NULL);

    yagl_host_glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
                            &num_texture_units, 1, NULL);

    /*
     * We limit this by 32 for conformance.
     */
    if (num_texture_units > 32) {
        num_texture_units = 32;
    }

    yagl_gles_context_prepare(&ctx->base,
                              num_texture_units,
                              num_arrays);

    conformant = getenv("YAGL_CONFORMANT");

    if (conformant && atoi(conformant)) {
        /*
         * Generating variable locations on target is faster, but
         * it's not conformant (will not pass some khronos tests)
         * since we can't know if variable with a given name exists or not,
         * so we just assume it exists.
         */
        ctx->gen_locations = 0;
    } else {
        ctx->gen_locations = 1;
    }

    yagl_host_glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS,
                            &ctx->num_compressed_texture_formats,
                            1, NULL);

    /*
     * We don't support it for now...
     */
    ctx->num_shader_binary_formats = 0;

    yagl_host_glGetString(GL_EXTENSIONS, NULL, 0, &size);
    extensions = yagl_malloc0(size);
    yagl_host_glGetString(GL_EXTENSIONS, extensions, size, NULL);

    ctx->texture_half_float = (strstr(extensions, "GL_ARB_half_float_pixel ") != NULL) ||
                              (strstr(extensions, "GL_NV_half_float ") != NULL);

    ctx->vertex_half_float = (strstr(extensions, "GL_ARB_half_float_vertex ") != NULL);

    ctx->standard_derivatives = (strstr(extensions, "GL_OES_standard_derivatives ") != NULL);

    yagl_free(extensions);

    ctx->instanced_arrays = (yagl_get_host_gl_version() > yagl_gl_2);

    YAGL_LOG_FUNC_EXIT(NULL);
}

struct yagl_gles_array
    *yagl_gles2_context_create_arrays(struct yagl_gles_context *ctx)
{
    GLint i;
    struct yagl_gles_array *arrays;

    arrays = yagl_malloc(ctx->num_arrays * sizeof(*arrays));

    for (i = 0; i < ctx->num_arrays; ++i) {
        yagl_gles_array_init(&arrays[i],
                             i,
                             &yagl_gles2_array_apply,
                             ctx);
    }

    return arrays;
}

GLenum yagl_gles2_context_compressed_tex_image(struct yagl_gles_context *ctx,
                                               GLenum target,
                                               GLint level,
                                               GLenum internalformat,
                                               GLsizei width,
                                               GLsizei height,
                                               GLint border,
                                               GLsizei imageSize,
                                               const GLvoid *data)
{
    yagl_host_glCompressedTexImage2D(target,
                                     level,
                                     internalformat,
                                     width,
                                     height,
                                     border,
                                     data,
                                     imageSize);

    return GL_NO_ERROR;
}

int yagl_gles2_context_get_integerv(struct yagl_gles_context *ctx,
                                    GLenum pname,
                                    GLint *params,
                                    uint32_t *num_params)
{
    int processed = 1;
    struct yagl_gles2_context *gles2_ctx = (struct yagl_gles2_context*)ctx;
    struct yagl_gles_texture_target_state *tts;

    switch (pname) {
    case GL_NUM_SHADER_BINARY_FORMATS:
        *params = gles2_ctx->num_shader_binary_formats;
        *num_params = 1;
        break;
    case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
        *params = gles2_ctx->num_compressed_texture_formats;
        *num_params = 1;
        break;
    case GL_TEXTURE_BINDING_CUBE_MAP:
        tts = yagl_gles_context_get_active_texture_target_state(ctx,
            yagl_gles_texture_target_cubemap);
        *params = tts->texture ? tts->texture->base.local_name : 0;
        *num_params = 1;
        break;
    case GL_CURRENT_PROGRAM:
        *params = gles2_ctx->program ? gles2_ctx->program->base.local_name : 0;
        *num_params = 1;
        break;
    case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
        *params = ctx->num_texture_units;
        *num_params = 1;
        break;
    case GL_MAX_VERTEX_ATTRIBS:
        *params = ctx->num_arrays;
        *num_params = 1;
        break;
    default:
        processed = 0;
        break;
    }

    if (processed) {
        return 1;
    }

    switch (pname) {
    case GL_MAX_TEXTURE_SIZE:
        *num_params = 1;
        break;
    case GL_COMPRESSED_TEXTURE_FORMATS:
        *num_params = gles2_ctx->num_compressed_texture_formats;
        break;
    case GL_DITHER:
        *num_params = 1;
        break;
    case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
        *num_params = 1;
        break;
    case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
        *num_params = 1;
        break;
    case GL_MAX_SAMPLES_IMG:
        *num_params = 1;
        break;
    case GL_MAX_TEXTURE_IMAGE_UNITS:
        *num_params = 1;
        break;
    case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
        *num_params = 1;
        break;
    case GL_MAX_VARYING_VECTORS:
        *num_params = 1;
        break;
    case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
        *num_params = 1;
        break;
    case GL_MAX_VERTEX_UNIFORM_VECTORS:
        *num_params = 1;
        break;
    case GL_SHADER_BINARY_FORMATS:
        *num_params = gles2_ctx->num_shader_binary_formats;
        break;
    case GL_SHADER_COMPILER:
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_FAIL:
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_FUNC:
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_PASS_DEPTH_PASS:
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_REF:
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_VALUE_MASK:
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_WRITEMASK:
        *num_params = 1;
        break;
    default:
        return 0;
    }

    yagl_host_glGetIntegerv(pname, params, *num_params, NULL);

    return 1;
}

int yagl_gles2_context_get_floatv(struct yagl_gles_context *ctx,
                                  GLenum pname,
                                  GLfloat *params,
                                  uint32_t *num_params,
                                  int *needs_map)
{
    struct yagl_gles2_context *gles2_ctx = (struct yagl_gles2_context*)ctx;

    switch (pname) {
    case GL_BLEND_COLOR:
        params[0] = gles2_ctx->blend_color[0];
        params[1] = gles2_ctx->blend_color[1];
        params[2] = gles2_ctx->blend_color[2];
        params[3] = gles2_ctx->blend_color[3];
        *num_params = 4;
        *needs_map = 1;
        break;
    default:
        return 0;
    }

    return 1;
}

void yagl_gles2_context_draw_arrays(struct yagl_gles_context *ctx,
                                    GLenum mode,
                                    GLint first,
                                    GLsizei count,
                                    GLsizei primcount)
{
    yagl_gles2_context_pre_draw(ctx, mode);

    if (primcount < 0) {
        yagl_host_glDrawArrays(mode, first, count);
    } else {
        yagl_host_glDrawArraysInstanced(mode, first, count, primcount);
    }

    yagl_gles2_context_post_draw(ctx, mode);
}

void yagl_gles2_context_draw_elements(struct yagl_gles_context *ctx,
                                      GLenum mode,
                                      GLsizei count,
                                      GLenum type,
                                      const GLvoid *indices,
                                      int32_t indices_count,
                                      GLsizei primcount)
{
    yagl_gles2_context_pre_draw(ctx, mode);

    if (primcount < 0) {
        yagl_host_glDrawElements(mode, count, type, indices, indices_count);
    } else {
        yagl_host_glDrawElementsInstanced(mode, count, type, indices, indices_count, primcount);
    }

    yagl_gles2_context_post_draw(ctx, mode);
}

char *yagl_gles2_context_shader_patch(struct yagl_gles2_context *ctx,
                                      const char *source,
                                      int len,
                                      int *patched_len)
{
    char *tmp;
    int tmp_len;

    char *patched_string = yagl_gles2_shader_patch(source,
                                                   len,
                                                   patched_len);

    tmp = yagl_gles2_shader_fix_extensions(patched_string,
                                           *patched_len,
                                           ctx->base.extensions,
                                           ctx->base.num_extensions,
                                           &tmp_len);

    if (tmp) {
        yagl_free(patched_string);
        patched_string = tmp;
        *patched_len = tmp_len;
    }

    /*
     * On some GPUs (like Ivybridge Desktop) it's necessary to add
     * "#version" directive as the first line of the shader, otherwise
     * some of the features might not be available to the shader.
     *
     * For example, on Ivybridge Desktop, if we don't add the "#version"
     * line to the fragment shader then "gl_PointCoord"
     * won't be available.
     */

    if (!yagl_gles2_shader_has_version(patched_string, NULL)) {
        *patched_len += sizeof("#version 120\n\n") - 1;
        char *tmp = yagl_malloc(*patched_len + 1);
        strcpy(tmp, "#version 120\n\n");
        strcat(tmp, patched_string);
        yagl_free(patched_string);
        patched_string = tmp;
    }

    return patched_string;
}

struct yagl_client_context *yagl_gles2_context_create(struct yagl_sharegroup *sg)
{
    struct yagl_gles2_context *gles2_ctx;

    YAGL_LOG_FUNC_ENTER(yagl_gles2_context_create, NULL);

    gles2_ctx = yagl_malloc0(sizeof(*gles2_ctx));

    yagl_gles2_context_init(gles2_ctx, yagl_client_api_gles2, sg);

    gles2_ctx->base.base.prepare = &yagl_gles2_context_prepare_internal;
    gles2_ctx->base.base.destroy = &yagl_gles2_context_destroy;
    gles2_ctx->base.create_arrays = &yagl_gles2_context_create_arrays;
    gles2_ctx->base.get_string = &yagl_gles2_context_get_string;
    gles2_ctx->base.compressed_tex_image = &yagl_gles2_context_compressed_tex_image;
    gles2_ctx->base.enable = &yagl_gles2_context_enable;
    gles2_ctx->base.is_enabled = &yagl_gles2_context_is_enabled;
    gles2_ctx->base.get_integerv = &yagl_gles2_context_get_integerv;
    gles2_ctx->base.get_floatv = &yagl_gles2_context_get_floatv;
    gles2_ctx->base.draw_arrays = &yagl_gles2_context_draw_arrays;
    gles2_ctx->base.draw_elements = &yagl_gles2_context_draw_elements;
    gles2_ctx->base.bind_buffer = &yagl_gles2_context_bind_buffer;
    gles2_ctx->base.unbind_buffer = &yagl_gles2_context_unbind_buffer;
    gles2_ctx->base.acquire_binded_buffer = &yagl_gles2_context_acquire_binded_buffer;
    gles2_ctx->shader_patch = &yagl_gles2_context_shader_patch;

    YAGL_LOG_FUNC_EXIT("%p", gles2_ctx);

    return &gles2_ctx->base.base;
}

void yagl_gles2_context_use_program(struct yagl_gles2_context *ctx,
                                    struct yagl_gles2_program *program)
{
    yagl_gles2_program_acquire(program);
    yagl_gles2_program_release(ctx->program);
    ctx->program = program;

    yagl_host_glUseProgram((program ? program->global_name : 0));
}

void yagl_gles2_context_unuse_program(struct yagl_gles2_context *ctx,
                                      struct yagl_gles2_program *program)
{
    if (ctx->program == program) {
        yagl_gles2_program_release(ctx->program);
        ctx->program = NULL;
    }
}

int yagl_gles2_context_get_array_param(struct yagl_gles2_context *ctx,
                                       GLuint index,
                                       GLenum pname,
                                       GLint *param)
{
    struct yagl_gles_array *array;

    YAGL_LOG_FUNC_SET(yagl_gles2_context_get_array_param);

    if (index >= ctx->base.num_arrays) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        return 1;
    }

    array = &ctx->base.vao->arrays[index];

    switch (pname) {
    case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
        *param = array->vbo ? array->vbo->base.local_name : 0;
        break;
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
        *param = array->enabled;
        break;
    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
        *param = array->size;
        break;
    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
        *param = array->stride;
        break;
    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
        *param = array->type;
        break;
    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
        *param = array->normalized;
        break;
    case GL_CURRENT_VERTEX_ATTRIB:
        return 0;
    case GL_VERTEX_ATTRIB_ARRAY_DIVISOR_EXT:
        if (ctx->instanced_arrays) {
            *param = array->divisor;
        } else {
            YAGL_SET_ERR(GL_INVALID_ENUM);
        }
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        break;
    }

    return 1;
}
