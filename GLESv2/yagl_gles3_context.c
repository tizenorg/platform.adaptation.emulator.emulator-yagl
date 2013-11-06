#include "GLES3/gl3.h"
#include "yagl_gles3_context.h"
#include "yagl_gles2_utils.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"
#include <string.h>
#include <stdlib.h>

static void yagl_gles3_context_destroy(struct yagl_client_context *ctx)
{
    struct yagl_gles2_context *gles3_ctx = (struct yagl_gles2_context*)ctx;

    YAGL_LOG_FUNC_ENTER(yagl_gles3_context_destroy, "%p", ctx);

    yagl_gles2_context_cleanup(gles3_ctx);

    yagl_free(gles3_ctx);

    YAGL_LOG_FUNC_EXIT(NULL);
}

static const GLchar
    *yagl_gles3_context_get_string(struct yagl_gles_context *ctx,
                                   GLenum name)
{
    const char *str = NULL;

    switch (name) {
    case GL_VERSION:
        str = "OpenGL ES 3.0";
        break;
    case GL_RENDERER:
        str = "YaGL GLESv3";
        break;
    case GL_SHADING_LANGUAGE_VERSION:
        str = "OpenGL ES GLSL ES 3.0";
        break;
    default:
        str = "";
    }

    return str;
}

static GLchar *yagl_gles3_context_get_extensions(struct yagl_gles_context *ctx)
{
    struct yagl_gles2_context *gles3_ctx = (struct yagl_gles2_context*)ctx;

    const GLchar *mandatory_extensions =
        "GL_OES_EGL_image GL_OES_depth24 GL_OES_depth32 "
        "GL_OES_texture_float GL_OES_texture_float_linear "
        "GL_EXT_texture_format_BGRA8888 GL_OES_depth_texture ";
    const GLchar *packed_depth_stencil = "GL_OES_packed_depth_stencil ";
    const GLchar *texture_npot = "GL_OES_texture_npot ";
    const GLchar *texture_rectangle = "GL_ARB_texture_rectangle ";
    const GLchar *texture_filter_anisotropic = "GL_EXT_texture_filter_anisotropic ";
    const GLchar *texture_half_float = "GL_OES_texture_half_float GL_OES_texture_half_float_linear ";
    const GLchar *vertex_half_float = "GL_OES_vertex_half_float ";
    const GLchar *standard_derivatives = "GL_OES_standard_derivatives ";

    GLuint len = strlen(mandatory_extensions);
    GLchar *str;

    if (gles3_ctx->base.packed_depth_stencil) {
        len += strlen(packed_depth_stencil);
    }

    if (gles3_ctx->base.texture_npot) {
        len += strlen(texture_npot);
    }

    if (gles3_ctx->base.texture_rectangle) {
        len += strlen(texture_rectangle);
    }

    if (gles3_ctx->base.texture_filter_anisotropic) {
        len += strlen(texture_filter_anisotropic);
    }

    if (gles3_ctx->texture_half_float) {
        len += strlen(texture_half_float);
    }

    if (gles3_ctx->vertex_half_float) {
        len += strlen(vertex_half_float);
    }

    if (gles3_ctx->standard_derivatives) {
        len += strlen(standard_derivatives);
    }

    str = yagl_malloc0(len + 1);

    strcpy(str, mandatory_extensions);

    if (gles3_ctx->base.packed_depth_stencil) {
        strcat(str, packed_depth_stencil);
    }

    if (gles3_ctx->base.texture_npot) {
        strcat(str, texture_npot);
    }

    if (gles3_ctx->base.texture_rectangle) {
        strcat(str, texture_rectangle);
    }

    if (gles3_ctx->base.texture_filter_anisotropic) {
        strcat(str, texture_filter_anisotropic);
    }

    if (gles3_ctx->texture_half_float) {
        strcat(str, texture_half_float);
    }

    if (gles3_ctx->vertex_half_float) {
        strcat(str, vertex_half_float);
    }

    if (gles3_ctx->standard_derivatives) {
        strcat(str, standard_derivatives);
    }

    return str;
}

static int yagl_gles3_context_enable(struct yagl_gles_context *ctx,
                                     GLenum cap,
                                     GLboolean enable)
{
    return 0;
}

static int yagl_gles3_context_is_enabled(struct yagl_gles_context *ctx,
                                         GLenum cap,
                                         GLboolean *enabled)
{
    return 0;
}

static char *yagl_gles3_context_shader_patch(struct yagl_gles2_context *ctx,
                                             const char *source,
                                             int len,
                                             int *patched_len)
{
    int is_es3 = 0;
    yagl_gl_version gl_version;

    if (!yagl_gles2_shader_has_version(source, &is_es3) || !is_es3) {
        /*
         * It's not a ES3 shader, process as GLESv2 shader.
         */
        return yagl_gles2_context_shader_patch(ctx,
                                               source,
                                               len,
                                               patched_len);
    }

    gl_version = yagl_get_host_gl_version();

    switch (gl_version) {
    case yagl_gl_3_1_es3:
        /*
         * GL_ARB_ES3_compatibility includes full ES 3.00 shader
         * support, no patching is required.
         */
        return NULL;
    case yagl_gl_3_2:
    default:
        /*
         * TODO: Patch shader to run with GLSL 1.50
         */
        return NULL;
    }
}

struct yagl_client_context *yagl_gles3_context_create(struct yagl_sharegroup *sg)
{
    struct yagl_gles2_context *gles3_ctx;

    YAGL_LOG_FUNC_ENTER(yagl_gles3_context_create, NULL);

    gles3_ctx = yagl_malloc0(sizeof(*gles3_ctx));

    yagl_gles2_context_init(gles3_ctx, yagl_client_api_gles3, sg);

    gles3_ctx->base.base.prepare = &yagl_gles2_context_prepare;
    gles3_ctx->base.base.destroy = &yagl_gles3_context_destroy;
    gles3_ctx->base.create_arrays = &yagl_gles2_context_create_arrays;
    gles3_ctx->base.get_string = &yagl_gles3_context_get_string;
    gles3_ctx->base.get_extensions = &yagl_gles3_context_get_extensions;
    gles3_ctx->base.compressed_tex_image = &yagl_gles2_context_compressed_tex_image;
    gles3_ctx->base.enable = &yagl_gles3_context_enable;
    gles3_ctx->base.is_enabled = &yagl_gles3_context_is_enabled;
    gles3_ctx->base.get_integerv = &yagl_gles2_context_get_integerv;
    gles3_ctx->base.get_floatv = &yagl_gles2_context_get_floatv;
    gles3_ctx->base.draw_arrays = &yagl_gles2_context_draw_arrays;
    gles3_ctx->base.draw_elements = &yagl_gles2_context_draw_elements;
    gles3_ctx->shader_patch = &yagl_gles3_context_shader_patch;

    YAGL_LOG_FUNC_EXIT("%p", gles3_ctx);

    return &gles3_ctx->base.base;
}
