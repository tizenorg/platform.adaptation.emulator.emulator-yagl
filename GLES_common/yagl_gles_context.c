#include "GL/gl.h"
#include "yagl_gles_context.h"
#include "yagl_gles_vertex_array.h"
#include "yagl_gles_array.h"
#include "yagl_gles_buffer.h"
#include "yagl_gles_texture.h"
#include "yagl_gles_texture_unit.h"
#include "yagl_gles_framebuffer.h"
#include "yagl_gles_renderbuffer.h"
#include "yagl_gles_validate.h"
#include "yagl_tex_image_binding.h"
#include "yagl_sharegroup.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_utils.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"
#include <string.h>
#include <assert.h>

#define YAGL_SET_ERR(err) \
    yagl_gles_context_set_error(ctx, err); \
    YAGL_LOG_ERROR("error = 0x%X", err)

static int yagl_gles_context_bind_tex_image(struct yagl_client_context *ctx,
                                            struct yagl_client_image *image,
                                            struct yagl_tex_image_binding *binding)
{
    struct yagl_gles_context *gles_ctx = (struct yagl_gles_context*)ctx;
    struct yagl_gles_texture_target_state *texture_target_state =
        yagl_gles_context_get_active_texture_target_state(gles_ctx,
                                                          yagl_gles_texture_target_2d);

    if (!texture_target_state->texture) {
        return 0;
    }

    yagl_gles_texture_bind_tex_image(texture_target_state->texture,
                                     (struct yagl_gles_image*)image,
                                     binding);

    binding->cookie = texture_target_state->texture;

    return 1;
}

void yagl_gles_context_init(struct yagl_gles_context *ctx,
                            yagl_client_api client_api,
                            struct yagl_sharegroup *sg)
{
    yagl_sharegroup_acquire(sg);

    yagl_namespace_init(&ctx->framebuffers);
    yagl_namespace_init(&ctx->vertex_arrays);

    ctx->base.bind_tex_image = &yagl_gles_context_bind_tex_image;

    ctx->base.client_api = client_api;
    ctx->base.sg = sg;

    ctx->error = GL_NO_ERROR;

    ctx->blend_equation_rgb = GL_FUNC_ADD;
    ctx->blend_equation_alpha = GL_FUNC_ADD;

    ctx->blend_src_rgb = GL_ONE;
    ctx->blend_dst_rgb = GL_ZERO;

    ctx->blend_src_alpha = GL_ONE;
    ctx->blend_dst_alpha = GL_ZERO;

    ctx->clear_depth = 1.0f;

    ctx->color_writemask[0] = GL_TRUE;
    ctx->color_writemask[1] = GL_TRUE;
    ctx->color_writemask[2] = GL_TRUE;
    ctx->color_writemask[3] = GL_TRUE;

    ctx->cull_face_mode = GL_BACK;

    ctx->depth_func = GL_LESS;

    ctx->depth_writemask = GL_TRUE;

    ctx->depth_range[0] = 0.0f;
    ctx->depth_range[1] = 1.0f;

    ctx->front_face_mode = GL_CCW;

    ctx->pack_alignment = 4;
    ctx->unpack_alignment = 4;

    ctx->dither_enabled = GL_TRUE;
}

void yagl_gles_context_prepare(struct yagl_gles_context *ctx,
                               int num_texture_units,
                               int num_arrays)
{
    int i;
    int32_t size = 0;
    char *extensions;
    struct yagl_gles_array *arrays;

    if (num_texture_units < 1) {
        num_texture_units = 1;
    }

    YAGL_LOG_FUNC_ENTER(yagl_gles_context_prepare,
                        "num_texture_units = %d",
                        num_texture_units);

    ctx->num_texture_units = num_texture_units;
    ctx->texture_units =
        yagl_malloc(ctx->num_texture_units * sizeof(*ctx->texture_units));

    for (i = 0; i < ctx->num_texture_units; ++i) {
        yagl_gles_texture_unit_init(&ctx->texture_units[i]);
    }

    ctx->num_arrays = num_arrays;

    yagl_host_glGetString(GL_EXTENSIONS, NULL, 0, &size);
    extensions = yagl_malloc0(size);
    yagl_host_glGetString(GL_EXTENSIONS, extensions, size, NULL);

    ctx->packed_depth_stencil = (strstr(extensions, "GL_EXT_packed_depth_stencil ") != NULL);

    ctx->texture_npot = (strstr(extensions, "GL_OES_texture_npot ") != NULL) ||
                        (strstr(extensions, "GL_ARB_texture_non_power_of_two ") != NULL);

    ctx->texture_rectangle = (strstr(extensions, "GL_NV_texture_rectangle ") != NULL) ||
                             (strstr(extensions, "GL_EXT_texture_rectangle ") != NULL) ||
                             (strstr(extensions, "GL_ARB_texture_rectangle ") != NULL);

    ctx->texture_filter_anisotropic = (strstr(extensions, "GL_EXT_texture_filter_anisotropic ") != NULL);

    yagl_free(extensions);

    arrays = ctx->create_arrays(ctx);

    ctx->vertex_arrays_supported = (yagl_get_host_gl_version() > yagl_gl_2);

    if (ctx->vertex_arrays_supported) {
        ctx->va_zero = yagl_gles_vertex_array_create(0, arrays, num_arrays);

        yagl_gles_context_bind_vertex_array(ctx, NULL);
    } else {
        ctx->va_zero = yagl_gles_vertex_array_create(1, arrays, num_arrays);

        /*
         * Don't bind, VAOs are not supported, just reference.
         */
        yagl_gles_vertex_array_acquire(ctx->va_zero);
        ctx->vao = ctx->va_zero;
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

void yagl_gles_context_cleanup(struct yagl_gles_context *ctx)
{
    int i;

    yagl_gles_renderbuffer_release(ctx->rbo);
    yagl_gles_framebuffer_release(ctx->fbo);
    yagl_gles_buffer_release(ctx->vbo);
    yagl_gles_vertex_array_release(ctx->vao);

    for (i = 0; i < ctx->num_texture_units; ++i) {
        yagl_gles_texture_unit_cleanup(&ctx->texture_units[i]);
    }

    yagl_free(ctx->texture_units);

    yagl_gles_vertex_array_release(ctx->va_zero);

    yagl_free(ctx->extensions);

    yagl_namespace_cleanup(&ctx->vertex_arrays);
    yagl_namespace_cleanup(&ctx->framebuffers);

    yagl_sharegroup_release(ctx->base.sg);
}

void yagl_gles_context_set_error(struct yagl_gles_context *ctx, GLenum error)
{
    if (ctx->error == GL_NO_ERROR) {
        ctx->error = error;
    }
}

GLenum yagl_gles_context_get_error(struct yagl_gles_context *ctx)
{
    GLenum error = ctx->error;

    ctx->error = GL_NO_ERROR;

    return error;
}

void yagl_gles_context_bind_vertex_array(struct yagl_gles_context *ctx,
                                         struct yagl_gles_vertex_array *va)
{
    if (!va) {
        va = ctx->va_zero;
    }

    yagl_gles_vertex_array_acquire(va);
    yagl_gles_vertex_array_release(ctx->vao);
    ctx->vao = va;

    yagl_gles_vertex_array_bind(va);
}

void yagl_gles_context_unbind_vertex_array(struct yagl_gles_context *ctx,
                                           yagl_object_name va_local_name)
{
    if ((ctx->vao != ctx->va_zero) &&
        (ctx->vao->base.local_name == va_local_name)) {
        yagl_gles_vertex_array_acquire(ctx->va_zero);
        yagl_gles_vertex_array_release(ctx->vao);
        ctx->vao = ctx->va_zero;
    }
}

void yagl_gles_context_set_active_texture(struct yagl_gles_context *ctx,
                                          GLenum texture)
{
    YAGL_LOG_FUNC_SET(glActiveTexture);

    if ((texture < GL_TEXTURE0) ||
        (texture >= (GL_TEXTURE0 + ctx->num_texture_units))) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        return;
    }

    ctx->active_texture_unit = texture - GL_TEXTURE0;

    yagl_host_glActiveTexture(texture);
}

struct yagl_gles_texture_unit
    *yagl_gles_context_get_active_texture_unit(struct yagl_gles_context *ctx)
{
    return &ctx->texture_units[ctx->active_texture_unit];
}

struct yagl_gles_texture_target_state
    *yagl_gles_context_get_active_texture_target_state(struct yagl_gles_context *ctx,
                                                       yagl_gles_texture_target texture_target)
{
    return &yagl_gles_context_get_active_texture_unit(ctx)->target_states[texture_target];
}

void yagl_gles_context_active_texture_set_enabled(struct yagl_gles_context *ctx,
    yagl_gles_texture_target texture_target, int enabled)
{
    struct yagl_gles_texture_target_state *texture_target_state;

    texture_target_state =
            yagl_gles_context_get_active_texture_target_state(ctx,
                                                              texture_target);
    texture_target_state->enabled = enabled;
}

void yagl_gles_context_bind_texture(struct yagl_gles_context *ctx,
                                    GLenum target,
                                    struct yagl_gles_texture *texture)
{
    yagl_gles_texture_target texture_target;
    struct yagl_gles_texture_target_state *texture_target_state;

    YAGL_LOG_FUNC_SET(glBindTexture);

    if (!yagl_gles_validate_texture_target(target, &texture_target)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        return;
    }

    if (!yagl_gles_texture_bind(texture, target)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        return;
    }

    texture_target_state =
        yagl_gles_context_get_active_texture_target_state(ctx, texture_target);

    yagl_gles_texture_acquire(texture);
    yagl_gles_texture_release(texture_target_state->texture);
    texture_target_state->texture = texture;
}

void yagl_gles_context_unbind_texture(struct yagl_gles_context *ctx,
                                      yagl_object_name texture_local_name)
{
    int i;
    struct yagl_gles_texture_unit *texture_unit =
        yagl_gles_context_get_active_texture_unit(ctx);

    for (i = 0; i < YAGL_NUM_GLES_TEXTURE_TARGETS; ++i) {
        if (texture_unit->target_states[i].texture &&
            (texture_unit->target_states[i].texture->base.local_name == texture_local_name)) {
            yagl_gles_texture_release(texture_unit->target_states[i].texture);
            texture_unit->target_states[i].texture = NULL;
        }
    }
}

void yagl_gles_context_bind_buffer(struct yagl_gles_context *ctx,
                                   GLenum target,
                                   struct yagl_gles_buffer *buffer)
{
    YAGL_LOG_FUNC_SET(glBindBuffer);

    switch (target) {
    case GL_ARRAY_BUFFER:
        yagl_gles_buffer_acquire(buffer);
        yagl_gles_buffer_release(ctx->vbo);
        ctx->vbo = buffer;
        break;
    case GL_ELEMENT_ARRAY_BUFFER:
        yagl_gles_buffer_acquire(buffer);
        yagl_gles_buffer_release(ctx->vao->ebo);
        ctx->vao->ebo = buffer;
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        return;
    }

    if (buffer) {
        yagl_gles_buffer_set_bound(buffer);
    }
}

void yagl_gles_context_unbind_buffer(struct yagl_gles_context *ctx,
                                     yagl_object_name buffer_local_name)
{
    if (ctx->vbo && (ctx->vbo->base.local_name == buffer_local_name)) {
        yagl_gles_buffer_release(ctx->vbo);
        ctx->vbo = NULL;
    } else if (ctx->vao->ebo && (ctx->vao->ebo->base.local_name == buffer_local_name)) {
        yagl_gles_buffer_release(ctx->vao->ebo);
        ctx->vao->ebo = NULL;
    }
}

void yagl_gles_context_bind_framebuffer(struct yagl_gles_context *ctx,
                                        GLenum target,
                                        struct yagl_gles_framebuffer *fbo)
{
    YAGL_LOG_FUNC_SET(glBindFramebuffer);

    switch (target) {
    case GL_FRAMEBUFFER:
        yagl_gles_framebuffer_acquire(fbo);
        yagl_gles_framebuffer_release(ctx->fbo);
        ctx->fbo = fbo;
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        return;
    }

    yagl_gles_framebuffer_bind(fbo, target);
}

void yagl_gles_context_unbind_framebuffer(struct yagl_gles_context *ctx,
                                          yagl_object_name fbo_local_name)
{
    if (ctx->fbo && (ctx->fbo->base.local_name == fbo_local_name)) {
        yagl_gles_framebuffer_release(ctx->fbo);
        ctx->fbo = NULL;
    }
}

void yagl_gles_context_bind_renderbuffer(struct yagl_gles_context *ctx,
                                         GLenum target,
                                         struct yagl_gles_renderbuffer *rbo)
{
    YAGL_LOG_FUNC_SET(glBindRenderbuffer);

    switch (target) {
    case GL_RENDERBUFFER:
        yagl_gles_renderbuffer_acquire(rbo);
        yagl_gles_renderbuffer_release(ctx->rbo);
        ctx->rbo = rbo;
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        return;
    }

    yagl_gles_renderbuffer_bind(rbo, target);
}

void yagl_gles_context_unbind_renderbuffer(struct yagl_gles_context *ctx,
                                           yagl_object_name rbo_local_name)
{
    if (ctx->rbo && (ctx->rbo->base.local_name == rbo_local_name)) {
        yagl_gles_renderbuffer_release(ctx->rbo);
        ctx->rbo = NULL;
    }
}

struct yagl_gles_buffer
    *yagl_gles_context_acquire_binded_buffer(struct yagl_gles_context *ctx,
                                             GLenum target)
{
    switch (target) {
    case GL_ARRAY_BUFFER:
        yagl_gles_buffer_acquire(ctx->vbo);
        return ctx->vbo;
    case GL_ELEMENT_ARRAY_BUFFER:
        yagl_gles_buffer_acquire(ctx->vao->ebo);
        return ctx->vao->ebo;
    default:
        return NULL;
    }
}

struct yagl_gles_framebuffer
    *yagl_gles_context_acquire_binded_framebuffer(struct yagl_gles_context *ctx,
                                                  GLenum target)
{
    switch (target) {
    case GL_FRAMEBUFFER:
        yagl_gles_framebuffer_acquire(ctx->fbo);
        return ctx->fbo;
    default:
        return NULL;
    }
}

void yagl_gles_context_enable(struct yagl_gles_context *ctx,
                              GLenum cap,
                              GLboolean enable)
{
    YAGL_LOG_FUNC_SET(yagl_gles_context_enable);

    switch (cap) {
    case GL_BLEND:
        ctx->blend_enabled = enable;
        break;
    case GL_CULL_FACE:
        ctx->cull_face_enabled = enable;
        break;
    case GL_DEPTH_TEST:
        ctx->depth_test_enabled = enable;
        break;
    case GL_DITHER:
        ctx->dither_enabled = enable;
        break;
    case GL_POLYGON_OFFSET_FILL:
        ctx->polygon_offset_fill_enabled = enable;
        break;
    case GL_SAMPLE_ALPHA_TO_COVERAGE:
        ctx->sample_alpha_to_coverage_enabled = enable;
        break;
    case GL_SAMPLE_COVERAGE:
        ctx->sample_coverage_enabled = enable;
        break;
    case GL_SCISSOR_TEST:
        ctx->scissor_test_enabled = enable;
        break;
    case GL_STENCIL_TEST:
        ctx->stencil_test_enabled = enable;
        break;
    default:
        if (!ctx->enable(ctx, cap, enable)) {
            YAGL_SET_ERR(GL_INVALID_ENUM);
            return;
        }
        break;
    }

    if (enable) {
        yagl_host_glEnable(cap);
    } else {
        yagl_host_glDisable(cap);
    }
}

GLboolean yagl_gles_context_is_enabled(struct yagl_gles_context *ctx,
                                       GLenum cap)
{
    GLboolean res = GL_FALSE;

    YAGL_LOG_FUNC_SET(glIsEnabled);

    switch (cap) {
    case GL_BLEND:
        res = ctx->blend_enabled;
        break;
    case GL_CULL_FACE:
        res = ctx->cull_face_enabled;
        break;
    case GL_DEPTH_TEST:
        res = ctx->depth_test_enabled;
        break;
    case GL_DITHER:
        res = ctx->dither_enabled;
        break;
    case GL_POLYGON_OFFSET_FILL:
        res = ctx->polygon_offset_fill_enabled;
        break;
    case GL_SAMPLE_ALPHA_TO_COVERAGE:
        res = ctx->sample_alpha_to_coverage_enabled;
        break;
    case GL_SAMPLE_COVERAGE:
        res = ctx->sample_coverage_enabled;
        break;
    case GL_SCISSOR_TEST:
        res = ctx->scissor_test_enabled;
        break;
    case GL_STENCIL_TEST:
        res = ctx->stencil_test_enabled;
        break;
    default:
        if (!ctx->is_enabled(ctx, cap, &res)) {
            YAGL_SET_ERR(GL_INVALID_ENUM);
            return GL_FALSE;
        }
        break;
    }

    return res;
}

int yagl_gles_context_get_integerv(struct yagl_gles_context *ctx,
                                   GLenum pname,
                                   GLint *params,
                                   uint32_t *num_params)
{
    int processed = 1;
    struct yagl_gles_texture_target_state *tts;

    switch (pname) {
    case GL_ACTIVE_TEXTURE:
        *params = GL_TEXTURE0 + ctx->active_texture_unit;
        *num_params = 1;
        break;
    case GL_TEXTURE_BINDING_2D:
        tts = yagl_gles_context_get_active_texture_target_state(ctx,
            yagl_gles_texture_target_2d);
        *params = tts->texture ? tts->texture->base.local_name : 0;
        *num_params = 1;
        break;
    case GL_ARRAY_BUFFER_BINDING:
        *params = ctx->vbo ? ctx->vbo->base.local_name : 0;
        *num_params = 1;
        break;
    case GL_ELEMENT_ARRAY_BUFFER_BINDING:
        *params = ctx->vao->ebo ? ctx->vao->ebo->base.local_name : 0;
        *num_params = 1;
        break;
    case GL_FRAMEBUFFER_BINDING:
        *params = ctx->fbo ? ctx->fbo->base.local_name : 0;
        *num_params = 1;
        break;
    case GL_RENDERBUFFER_BINDING:
        *params = ctx->rbo ? ctx->rbo->base.local_name : 0;
        *num_params = 1;
        break;
    case GL_BLEND_EQUATION_RGB:
        *params = ctx->blend_equation_rgb;
        *num_params = 1;
        break;
    case GL_BLEND_EQUATION_ALPHA:
        *params = ctx->blend_equation_alpha;
        *num_params = 1;
        break;
    case GL_BLEND_SRC_RGB:
        *params = ctx->blend_src_rgb;
        *num_params = 1;
        break;
    case GL_BLEND_DST_RGB:
        *params = ctx->blend_dst_rgb;
        *num_params = 1;
        break;
    case GL_BLEND_SRC_ALPHA:
        *params = ctx->blend_src_alpha;
        *num_params = 1;
        break;
    case GL_BLEND_DST_ALPHA:
        *params = ctx->blend_dst_alpha;
        *num_params = 1;
        break;
    case GL_COLOR_WRITEMASK:
        params[0] = ctx->color_writemask[0];
        params[1] = ctx->color_writemask[1];
        params[2] = ctx->color_writemask[2];
        params[3] = ctx->color_writemask[3];
        *num_params = 4;
        break;
    case GL_CULL_FACE_MODE:
        *params = ctx->cull_face_mode;
        *num_params = 1;
        break;
    case GL_DEPTH_FUNC:
        *params = ctx->depth_func;
        *num_params = 1;
        break;
    case GL_DEPTH_WRITEMASK:
        *params = ctx->depth_writemask;
        *num_params = 1;
        break;
    case GL_FRONT_FACE:
        *params = ctx->front_face_mode;
        *num_params = 1;
        break;
    case GL_PACK_ALIGNMENT:
        *params = ctx->pack_alignment;
        *num_params = 1;
        break;
    case GL_UNPACK_ALIGNMENT:
        *params = ctx->unpack_alignment;
        *num_params = 1;
        break;
    case GL_VIEWPORT:
        if (ctx->have_viewport) {
            params[0] = ctx->viewport[0];
            params[1] = ctx->viewport[1];
            params[2] = ctx->viewport[2];
            params[3] = ctx->viewport[3];
            *num_params = 4;
        } else {
            processed = 0;
        }
        break;
    case GL_VERTEX_ARRAY_BINDING:
        if (ctx->vertex_arrays_supported) {
            *params = (ctx->vao != ctx->va_zero) ? ctx->vao->base.local_name : 0;
            *num_params = 1;
        } else {
            return 0;
        }
        break;
    default:
        processed = 0;
        break;
    }

    if (processed) {
        return 1;
    }

    switch (pname) {
    case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
        *num_params = 1;
        break;
    case GL_IMPLEMENTATION_COLOR_READ_TYPE:
        *num_params = 1;
        break;
    case GL_MAX_RENDERBUFFER_SIZE:
        *num_params = 1;
        break;
    case GL_ALPHA_BITS:
        *num_params = 1;
        break;
    case GL_BLEND:
        *num_params = 1;
        break;
    case GL_BLUE_BITS:
        *num_params = 1;
        break;
    case GL_CULL_FACE:
        *num_params = 1;
        break;
    case GL_DEPTH_BITS:
        *num_params = 1;
        break;
    case GL_DEPTH_TEST:
        *num_params = 1;
        break;
    case GL_GENERATE_MIPMAP_HINT:
        *num_params = 1;
        break;
    case GL_GREEN_BITS:
        *num_params = 1;
        break;
    case GL_MAX_VIEWPORT_DIMS:
        *num_params = 2;
        break;
    case GL_POLYGON_OFFSET_FILL:
        *num_params = 1;
        break;
    case GL_RED_BITS:
        *num_params = 1;
        break;
    case GL_SAMPLE_ALPHA_TO_COVERAGE:
        *num_params = 1;
        break;
    case GL_SAMPLE_BUFFERS:
        *num_params = 1;
        break;
    case GL_SAMPLE_COVERAGE:
        *num_params = 1;
        break;
    case GL_SAMPLE_COVERAGE_INVERT:
        *num_params = 1;
        break;
    case GL_SAMPLES:
        *num_params = 1;
        break;
    case GL_SCISSOR_BOX:
        *num_params = 4;
        break;
    case GL_SCISSOR_TEST:
        *num_params = 1;
        break;
    case GL_STENCIL_BITS:
        *num_params = 1;
        break;
    case GL_STENCIL_CLEAR_VALUE:
        *num_params = 1;
        break;
    case GL_STENCIL_FAIL:
        *num_params = 1;
        break;
    case GL_STENCIL_FUNC:
        *num_params = 1;
        break;
    case GL_STENCIL_PASS_DEPTH_FAIL:
        *num_params = 1;
        break;
    case GL_STENCIL_PASS_DEPTH_PASS:
        *num_params = 1;
        break;
    case GL_STENCIL_REF:
        *num_params = 1;
        break;
    case GL_STENCIL_TEST:
        *num_params = 1;
        break;
    case GL_STENCIL_VALUE_MASK:
        *num_params = 1;
        break;
    case GL_STENCIL_WRITEMASK:
        *num_params = 1;
        break;
    case GL_SUBPIXEL_BITS:
        *num_params = 1;
        break;
    case GL_VIEWPORT:
        *num_params = 4;
        break;
    default:
        return ctx->get_integerv(ctx, pname, params, num_params);
    }

    yagl_host_glGetIntegerv(pname, params, *num_params, NULL);

    return 1;
}

int yagl_gles_context_get_floatv(struct yagl_gles_context *ctx,
                                 GLenum pname,
                                 GLfloat *params,
                                 uint32_t *num_params,
                                 int *needs_map)
{
    int processed = 1;

    switch (pname) {
    case GL_COLOR_CLEAR_VALUE:
        params[0] = ctx->clear_color[0];
        params[1] = ctx->clear_color[1];
        params[2] = ctx->clear_color[2];
        params[3] = ctx->clear_color[3];
        *num_params = 4;
        *needs_map = 1;
        break;
    case GL_DEPTH_CLEAR_VALUE:
        *params = ctx->clear_depth;
        *num_params = 1;
        *needs_map = 1;
        break;
    case GL_DEPTH_RANGE:
        params[0] = ctx->depth_range[0];
        params[1] = ctx->depth_range[1];
        *num_params = 2;
        *needs_map = 1;
        break;
    default:
        processed = 0;
        break;
    }

    if (processed) {
        return 1;
    }

    switch (pname) {
    case GL_ALIASED_LINE_WIDTH_RANGE:
        *num_params = 2;
        break;
    case GL_ALIASED_POINT_SIZE_RANGE:
        *num_params = 2;
        break;
    case GL_LINE_WIDTH:
        *num_params = 1;
        break;
    case GL_POLYGON_OFFSET_FACTOR:
        *num_params = 1;
        break;
    case GL_POLYGON_OFFSET_UNITS:
        *num_params = 1;
        break;
    case GL_SAMPLE_COVERAGE_VALUE:
        *num_params = 1;
        break;
    default:
        return ctx->get_floatv(ctx, pname, params, num_params, needs_map);
    }

    yagl_host_glGetFloatv(pname, params, *num_params, NULL);

    return 1;
}

const GLchar *yagl_gles_context_get_extensions(struct yagl_gles_context *ctx)
{
    if (!ctx->extensions) {
        ctx->extensions = ctx->get_extensions(ctx);
    }

    return ctx->extensions;
}

void yagl_gles_context_line_width(struct yagl_gles_context *ctx,
                                  GLfloat width)
{
    yagl_host_glLineWidth(width);
}

void yagl_gles_context_tex_parameterf(struct yagl_gles_context *ctx,
                                      GLenum target,
                                      GLenum pname,
                                      GLfloat param)
{
    yagl_host_glTexParameterf(target, pname, param);
}

void yagl_gles_context_tex_parameterfv(struct yagl_gles_context *ctx,
                                       GLenum target,
                                       GLenum pname,
                                       const GLfloat *params)
{
    yagl_host_glTexParameterfv(target, pname, params, 1);
}

void yagl_gles_context_get_tex_parameterfv(struct yagl_gles_context *ctx,
                                           GLenum target,
                                           GLenum pname,
                                           GLfloat *params)
{
    yagl_host_glGetTexParameterfv(target, pname, params);
}

void yagl_gles_context_clear_color(struct yagl_gles_context *ctx,
                                   GLclampf red,
                                   GLclampf green,
                                   GLclampf blue,
                                   GLclampf alpha)
{
    ctx->clear_color[0] = yagl_clampf(red);
    ctx->clear_color[1] = yagl_clampf(green);
    ctx->clear_color[2] = yagl_clampf(blue);
    ctx->clear_color[3] = yagl_clampf(alpha);

    yagl_host_glClearColor(red, green, blue, alpha);
}

void yagl_gles_context_clear_depthf(struct yagl_gles_context *ctx,
                                    GLclampf depth)
{
    ctx->clear_depth = yagl_clampf(depth);

    yagl_host_glClearDepthf(depth);
}

void yagl_gles_context_sample_coverage(struct yagl_gles_context *ctx,
                                       GLclampf value,
                                       GLboolean invert)
{
    yagl_host_glSampleCoverage(value, invert);
}

void yagl_gles_context_depth_rangef(struct yagl_gles_context *ctx,
                                    GLclampf zNear,
                                    GLclampf zFar)
{
    ctx->depth_range[0] = yagl_clampf(zNear);
    ctx->depth_range[1] = yagl_clampf(zFar);

    yagl_host_glDepthRangef(zNear, zFar);
}

void yagl_gles_context_polygon_offset(struct yagl_gles_context *ctx,
                                      GLfloat factor,
                                      GLfloat units)
{
    yagl_host_glPolygonOffset(factor, units);
}
