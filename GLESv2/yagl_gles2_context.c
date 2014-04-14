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
#include "yagl_gles_utils.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_egl_fence.h"
#include "yagl_texcompress.h"
#include "yagl_host_gles_calls.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define YAGL_SET_ERR(err) \
    yagl_gles_context_set_error(&ctx->base, err); \
    YAGL_LOG_ERROR("error = 0x%X", err)

/*
 * We can't include GL/glext.h here
 */
#define GL_POINT_SPRITE                    0x8861
#define GL_VERTEX_PROGRAM_POINT_SIZE       0x8642
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS   0x8B4A
#define GL_MAX_VARYING_FLOATS              0x8B4B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER     0x88FD

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
static const GLchar *texture_3d_ext = "GL_OES_texture_3D";
static const GLchar *blend_minmax_ext = "GL_EXT_blend_minmax";
static const GLchar *texture_storage_ext = "GL_EXT_texture_storage";
static const GLchar *pbo_ext = "GL_NV_pixel_buffer_object";
static const GLchar *read_buffer_ext = "GL_NV_read_buffer";
static const GLchar *compressed_etc1_rgb8_texture_ext = "GL_OES_compressed_ETC1_RGB8_texture";
static const GLchar *pack_subimage_ext = "GL_NV_pack_subimage";
static const GLchar *unpack_subimage_ext = "GL_EXT_unpack_subimage";
static const GLchar *egl_sync_ext = "GL_OES_EGL_sync";
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

static void yagl_gles2_array_apply(struct yagl_gles_array *array,
                                   uint32_t first,
                                   uint32_t count,
                                   const GLvoid *ptr,
                                   void *user_data)
{
    if (array->integer) {
        if (array->vbo) {
            yagl_host_glVertexAttribIPointerOffset(array->index,
                                                   array->size,
                                                   array->actual_type,
                                                   array->actual_stride,
                                                   array->actual_offset);
        } else {
            yagl_host_glVertexAttribIPointerData(array->index,
                                                 array->size,
                                                 array->actual_type,
                                                 array->actual_stride,
                                                 first,
                                                 ptr + (first * array->actual_stride),
                                                 count * array->actual_stride);
        }

        return;
    }

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

static int yagl_gles2_context_pre_use_program(struct yagl_gles2_context *ctx,
                                              struct yagl_gles2_program *program)
{
    return 1;
}

static int yagl_gles2_context_pre_link_program(struct yagl_gles2_context *ctx,
                                               struct yagl_gles2_program *program)
{
    return 1;
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

    yagl_gles_buffer_release(ctx->vertex_attrib0.vbo);

    yagl_gles_context_cleanup(&ctx->base);
}

void yagl_gles2_context_prepare(struct yagl_gles2_context *ctx)
{
    GLint num_texture_units = 0;
    int32_t size = 0;
    char *extensions, *nonconformant;
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

    ctx->vertex_attrib0.value.f[0] = 0.0f;
    ctx->vertex_attrib0.value.f[1] = 0.0f;
    ctx->vertex_attrib0.value.f[2] = 0.0f;
    ctx->vertex_attrib0.value.f[3] = 1.0f;
    ctx->vertex_attrib0.type = GL_FLOAT;
    ctx->vertex_attrib0.vbo = yagl_gles_buffer_create();

    nonconformant = getenv("YAGL_NONCONFORMANT");

    /*
     * Generating variable locations on target is faster, but
     * it's not conformant (will not pass some khronos tests)
     * since we can't know if variable with a given name exists or not,
     * so we just assume it exists.
     *
     * We used to set 'gen_locations' to 1 by default, but that's bad,
     * since app may do something like this:
     * if (glGetUniformLocation(p, "my_uniform") == -1) {
     *     // do logic when 'my_uniform' is not in the shader
     * } else {
     *     // do logic when 'my_uniform' is in the shader
     * }
     * i.e. 'gen_locations == 1' will break this logic, thus, we
     * now set gen_locations to 0 by default.
     */

    if (nonconformant && atoi(nonconformant)) {
        ctx->gen_locations = 1;
    } else {
        ctx->gen_locations = 0;
    }

    yagl_host_glGetString(GL_EXTENSIONS, NULL, 0, &size);
    extensions = yagl_malloc0(size);
    yagl_host_glGetString(GL_EXTENSIONS, extensions, size, NULL);

    if (yagl_get_host_gl_version() > yagl_gl_2) {
        ctx->texture_half_float = 1;
        ctx->vertex_half_float = 1;
    } else {
        ctx->texture_half_float = (strstr(extensions, "GL_ARB_half_float_pixel ") != NULL) ||
                                  (strstr(extensions, "GL_NV_half_float ") != NULL);

        ctx->vertex_half_float = (strstr(extensions, "GL_ARB_half_float_vertex ") != NULL);
    }

    ctx->standard_derivatives = 1;

    yagl_free(extensions);

    ctx->instanced_arrays = (yagl_get_host_gl_version() > yagl_gl_2);

    YAGL_LOG_FUNC_EXIT(NULL);
}

const GLchar **yagl_gles2_context_get_extensions(struct yagl_gles2_context *ctx,
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
    extensions[i++] = texture_3d_ext;
    extensions[i++] = blend_minmax_ext;
    extensions[i++] = texture_storage_ext;
    extensions[i++] = pbo_ext;
    extensions[i++] = read_buffer_ext;
    extensions[i++] = compressed_etc1_rgb8_texture_ext;
    extensions[i++] = pack_subimage_ext;
    extensions[i++] = unpack_subimage_ext;

    if (yagl_egl_fence_supported()) {
        extensions[i++] = egl_sync_ext;
    }

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

void yagl_gles2_context_pre_draw(struct yagl_gles2_context *ctx,
                                 GLenum mode,
                                 GLint count)
{
    YAGL_LOG_FUNC_SET(yagl_gles2_context_pre_draw);

    /*
     * 'count' can be <= 0 in case of integer overflows, this is
     * typically user problem, just don't simulate vertex attribute array 0
     * in this case.
     */

    if (!ctx->base.vao->arrays[0].enabled && (count > 0)) {
        /*
         * vertex attribute array 0 not enabled, simulate
         * vertex attribute array 0.
         */

        GLint size = count * sizeof(ctx->vertex_attrib0.value);
        void *tmp, *it;

        if (!ctx->vertex_attrib0.warned) {
            YAGL_LOG_WARN("vertex attribute array 0 not enabled, simulating");

            ctx->vertex_attrib0.warned = 1;
        }

        if (ctx->vertex_attrib0.vbo->size < size) {
            /*
             * Required buffer size is greater than allocated so far,
             * we're forced to reallocate. This is slow path, we basically
             * need to update everything.
             */

            tmp = it = yagl_get_tmp_buffer2(size);

            while (it < (tmp + size)) {
                memcpy(it, &ctx->vertex_attrib0.value, sizeof(ctx->vertex_attrib0.value));
                it += sizeof(ctx->vertex_attrib0.value);
            }

            yagl_gles_buffer_set_data(ctx->vertex_attrib0.vbo,
                                      size,
                                      tmp,
                                      GL_STREAM_DRAW);

            ctx->vertex_attrib0.count = count;
        } else {
            /*
             * We can fit it in existing buffer.
             */

            GLint offset = 0;

            tmp = ctx->vertex_attrib0.vbo->data;

            if (memcmp(tmp,
                       &ctx->vertex_attrib0.value,
                       sizeof(ctx->vertex_attrib0.value)) == 0) {
                if (ctx->vertex_attrib0.count < count) {
                    /*
                     * vertex attribute 0 didn't change, but we need more than
                     * 'vertex_attrib0.count' elements, thus, update only part
                     * of the buffer. This is semi-fast path. We also update
                     * 'vertex_attrib0.count' here to specify that we now
                     * have more valid elements in the buffer.
                     */
                    offset = sizeof(ctx->vertex_attrib0.value) * ctx->vertex_attrib0.count;
                    size = sizeof(ctx->vertex_attrib0.value) * (count - ctx->vertex_attrib0.count);
                    ctx->vertex_attrib0.count = count;
                } else {
                    /*
                     * vertex attribute 0 didn't change and we need less than
                     * 'vertex_attrib0.count' elements, this is fast-path,
                     * don't transfer anything. Also, don't update
                     * 'vertex_attrib0.count' here since elements pass 'count'
                     * are still equal to vertex attribute 0, thus, they can
                     * be reused later.
                     */
                    size = 0;
                }
            } else {
                /*
                 * vertex attribute 0 changed, we'll update the buffer
                 * starting from element 0 and up to 'count' elements.
                 * Also, we set 'vertex_attrib0.count' to specify that we have
                 * 'vertex_attrib0.count' elements valid. Note that the buffer
                 * may still contain elements with outdated vertex attribute 0
                 * values.
                 */

                ctx->vertex_attrib0.count = count;
            }

            tmp = it = yagl_get_tmp_buffer2(size);

            while (it < (tmp + size)) {
                memcpy(it, &ctx->vertex_attrib0.value, sizeof(ctx->vertex_attrib0.value));
                it += sizeof(ctx->vertex_attrib0.value);
            }

            yagl_gles_buffer_update_data(ctx->vertex_attrib0.vbo,
                                         offset,
                                         size,
                                         tmp);
        }

        yagl_host_glEnableVertexAttribArray(0);

        if (ctx->base.vao->arrays[0].divisor != 0) {
            yagl_host_glVertexAttribDivisor(0, 0);
        }

        yagl_gles_buffer_bind(ctx->vertex_attrib0.vbo,
                              0,
                              0,
                              GL_ARRAY_BUFFER);
        yagl_gles_buffer_transfer(ctx->vertex_attrib0.vbo,
                                  0,
                                  GL_ARRAY_BUFFER,
                                  0);
        switch (ctx->vertex_attrib0.type) {
        case GL_INT:
        case GL_UNSIGNED_INT:
            yagl_host_glVertexAttribIPointerOffset(0,
                                                   4,
                                                   ctx->vertex_attrib0.type,
                                                   0,
                                                   0);
            break;
        default:
            assert(0);
        case GL_FLOAT:
            yagl_host_glVertexAttribPointerOffset(0,
                                                  4,
                                                  ctx->vertex_attrib0.type,
                                                  GL_FALSE,
                                                  0,
                                                  0);
            break;
        }
        yagl_host_glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

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

void yagl_gles2_context_post_draw(struct yagl_gles2_context *ctx,
                                  GLenum mode,
                                  GLint count)
{
    if (mode == GL_POINTS) {
        yagl_host_glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
        if (yagl_get_host_gl_version() <= yagl_gl_2) {
            yagl_host_glDisable(GL_POINT_SPRITE);
        }
    }

    if (!ctx->base.vao->arrays[0].enabled && (count > 0)) {
        /*
         * Restore vertex attribute array 0 pointer.
         */
        yagl_gles_array_apply(&ctx->base.vao->arrays[0]);

        if (ctx->base.vao->arrays[0].divisor != 0) {
            /*
             * Restore vertex attribute array 0 divisor.
             */
            yagl_host_glVertexAttribDivisor(0, ctx->base.vao->arrays[0].divisor);
        }

        /*
         * Restore vertex attribute array 0 state.
         */
        yagl_host_glDisableVertexAttribArray(0);
    }
}

void yagl_gles2_context_compressed_tex_image_2d(struct yagl_gles_context *gles_ctx,
                                                GLenum target,
                                                struct yagl_gles_texture *texture,
                                                GLint level,
                                                GLenum internalformat,
                                                GLsizei width,
                                                GLsizei height,
                                                GLint border,
                                                GLsizei imageSize,
                                                const GLvoid *data)
{
    struct yagl_gles2_context *ctx = (struct yagl_gles2_context*)gles_ctx;
    struct yagl_texcompress_format *tc_format;
    GLsizei src_stride;
    GLsizei dst_stride;
    GLsizei dst_size;
    uint8_t *buff;

    YAGL_LOG_FUNC_SET(glCompressedTexImage2D);

    tc_format = yagl_texcompress_get_format(internalformat);

    if (!tc_format) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        return;
    }

    if (!yagl_texcompress_get_info(tc_format,
                                   width,
                                   height,
                                   imageSize,
                                   &src_stride,
                                   &dst_stride,
                                   &dst_size)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        return;
    }

    if (!data) {
        yagl_host_glTexImage2DData(target,
                                   level,
                                   tc_format->dst_internalformat,
                                   width,
                                   height,
                                   border,
                                   tc_format->dst_format,
                                   tc_format->dst_type,
                                   NULL,
                                   dst_size);

        yagl_gles_texture_set_internalformat(texture,
                                             tc_format->dst_internalformat,
                                             tc_format->dst_type,
                                             yagl_gles_context_convert_textures(gles_ctx));

        return;
    }

    buff = yagl_get_tmp_buffer(dst_size);

    tc_format->unpack(tc_format,
                      data,
                      width,
                      height,
                      src_stride,
                      buff,
                      dst_stride);

    yagl_gles_reset_unpack(&gles_ctx->unpack);

    yagl_host_glTexImage2DData(target,
                               level,
                               tc_format->dst_internalformat,
                               width,
                               height,
                               border,
                               tc_format->dst_format,
                               tc_format->dst_type,
                               buff,
                               dst_size);

    yagl_gles_set_unpack(&gles_ctx->unpack);

    yagl_gles_texture_set_internalformat(texture,
                                         tc_format->dst_internalformat,
                                         tc_format->dst_type,
                                         yagl_gles_context_convert_textures(gles_ctx));
}

void yagl_gles2_context_compressed_tex_sub_image_2d(struct yagl_gles_context *gles_ctx,
                                                    GLenum target,
                                                    GLint level,
                                                    GLint xoffset,
                                                    GLint yoffset,
                                                    GLsizei width,
                                                    GLsizei height,
                                                    GLenum format,
                                                    GLsizei imageSize,
                                                    const GLvoid *data)
{
    struct yagl_gles2_context *ctx = (struct yagl_gles2_context*)gles_ctx;
    struct yagl_texcompress_format *tc_format;
    GLsizei src_stride;
    GLsizei dst_stride;
    GLsizei dst_size;
    uint8_t *buff;

    YAGL_LOG_FUNC_SET(glCompressedTexSubImage2D);

    if (format == GL_ETC1_RGB8_OES) {
        /*
         * "INVALID_OPERATION is generated by CompressedTexSubImage2D,
         * TexSubImage2D, or CopyTexSubImage2D if the texture image <level>
         * bound to <target> has internal format ETC1_RGB8_OES."
         */
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        return;
    }

    tc_format = yagl_texcompress_get_format(format);

    if (!tc_format) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        return;
    }

    if (!yagl_texcompress_get_info(tc_format,
                                   width,
                                   height,
                                   imageSize,
                                   &src_stride,
                                   &dst_stride,
                                   &dst_size)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        return;
    }

    if (!data) {
        yagl_host_glTexSubImage2DData(target,
                                      level,
                                      xoffset,
                                      yoffset,
                                      width,
                                      height,
                                      tc_format->dst_format,
                                      tc_format->dst_type,
                                      NULL,
                                      dst_size);

        return;
    }

    buff = yagl_get_tmp_buffer(dst_size);

    tc_format->unpack(tc_format,
                      data,
                      width,
                      height,
                      src_stride,
                      buff,
                      dst_stride);

    yagl_gles_reset_unpack(&gles_ctx->unpack);

    yagl_host_glTexSubImage2DData(target,
                                  level,
                                  xoffset,
                                  yoffset,
                                  width,
                                  height,
                                  tc_format->dst_format,
                                  tc_format->dst_type,
                                  buff,
                                  dst_size);

    yagl_gles_set_unpack(&gles_ctx->unpack);
}

void yagl_gles2_context_compressed_tex_image_3d(struct yagl_gles2_context *ctx,
                                                GLenum target,
                                                struct yagl_gles_texture *texture,
                                                GLint level,
                                                GLenum internalformat,
                                                GLsizei width,
                                                GLsizei height,
                                                GLsizei depth,
                                                GLint border,
                                                GLsizei imageSize,
                                                const GLvoid *data)
{
    GLsizei singleImageSize = 0;
    struct yagl_texcompress_format *tc_format;
    GLsizei src_stride;
    GLsizei dst_stride;
    GLsizei dst_size;
    uint8_t *buff, *tmp;
    GLsizei i;

    YAGL_LOG_FUNC_SET(glCompressedTexImage3D);

    tc_format = yagl_texcompress_get_format(internalformat);

    if (!tc_format) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        return;
    }

    if (depth > 0) {
        singleImageSize = imageSize / depth;
    }

    if (!yagl_texcompress_get_info(tc_format,
                                   width,
                                   height,
                                   singleImageSize,
                                   &src_stride,
                                   &dst_stride,
                                   &dst_size)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        return;
    }

    if (!data) {
        yagl_host_glTexImage3DData(target,
                                   level,
                                   tc_format->dst_internalformat,
                                   width,
                                   height,
                                   depth,
                                   border,
                                   tc_format->dst_format,
                                   tc_format->dst_type,
                                   NULL,
                                   dst_size * depth);

        yagl_gles_texture_set_internalformat(texture,
                                             tc_format->dst_internalformat,
                                             tc_format->dst_type,
                                             yagl_gles_context_convert_textures(&ctx->base));

        return;
    }

    buff = tmp = yagl_get_tmp_buffer(dst_size * depth);

    for (i = 0; i < depth; ++i) {
        tc_format->unpack(tc_format,
                          data,
                          width,
                          height,
                          src_stride,
                          tmp,
                          dst_stride);
        data += singleImageSize;
        tmp += dst_size;
    }

    yagl_gles_reset_unpack(&ctx->base.unpack);

    yagl_host_glTexImage3DData(target,
                               level,
                               tc_format->dst_internalformat,
                               width,
                               height,
                               depth,
                               border,
                               tc_format->dst_format,
                               tc_format->dst_type,
                               buff,
                               dst_size * depth);

    yagl_gles_set_unpack(&ctx->base.unpack);

    yagl_gles_texture_set_internalformat(texture,
                                         tc_format->dst_internalformat,
                                         tc_format->dst_type,
                                         yagl_gles_context_convert_textures(&ctx->base));
}

void yagl_gles2_context_compressed_tex_sub_image_3d(struct yagl_gles2_context *ctx,
                                                    GLenum target,
                                                    GLint level,
                                                    GLint xoffset,
                                                    GLint yoffset,
                                                    GLint zoffset,
                                                    GLsizei width,
                                                    GLsizei height,
                                                    GLsizei depth,
                                                    GLenum format,
                                                    GLsizei imageSize,
                                                    const GLvoid *data)
{
    GLsizei singleImageSize = 0;
    struct yagl_texcompress_format *tc_format;
    GLsizei src_stride;
    GLsizei dst_stride;
    GLsizei dst_size;
    uint8_t *buff, *tmp;
    GLsizei i;

    YAGL_LOG_FUNC_SET(glCompressedTexSubImage3D);

    if (format == GL_ETC1_RGB8_OES) {
        /*
         * "INVALID_OPERATION is generated by CompressedTexSubImage2D,
         * TexSubImage2D, or CopyTexSubImage2D if the texture image <level>
         * bound to <target> has internal format ETC1_RGB8_OES."
         */
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        return;
    }

    tc_format = yagl_texcompress_get_format(format);

    if (!tc_format) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        return;
    }

    if (depth > 0) {
        singleImageSize = imageSize / depth;
    }

    if (!yagl_texcompress_get_info(tc_format,
                                   width,
                                   height,
                                   singleImageSize,
                                   &src_stride,
                                   &dst_stride,
                                   &dst_size)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        return;
    }

    if (!data) {
        yagl_host_glTexSubImage3DData(target,
                                      level,
                                      xoffset,
                                      yoffset,
                                      zoffset,
                                      width,
                                      height,
                                      depth,
                                      tc_format->dst_format,
                                      tc_format->dst_type,
                                      NULL,
                                      dst_size * depth);

        return;
    }

    buff = tmp = yagl_get_tmp_buffer(dst_size * depth);

    for (i = 0; i < depth; ++i) {
        tc_format->unpack(tc_format,
                          data,
                          width,
                          height,
                          src_stride,
                          tmp,
                          dst_stride);
        data += singleImageSize;
        tmp += dst_size;
    }

    yagl_gles_reset_unpack(&ctx->base.unpack);

    yagl_host_glTexSubImage3DData(target,
                                  level,
                                  xoffset,
                                  yoffset,
                                  zoffset,
                                  width,
                                  height,
                                  depth,
                                  tc_format->dst_format,
                                  tc_format->dst_type,
                                  buff,
                                  dst_size * depth);

    yagl_gles_set_unpack(&ctx->base.unpack);
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
        *params = 0;
        *num_params = 1;
        break;
    case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
        *params = yagl_texcompress_get_format_names(NULL);
        *num_params = 1;
        break;
    case GL_TEXTURE_BINDING_CUBE_MAP:
        tts = yagl_gles_context_get_active_texture_target_state(ctx,
            yagl_gles_texture_target_cubemap);
        *params = tts->texture->base.local_name;
        *num_params = 1;
        break;
    case GL_TEXTURE_BINDING_3D_OES:
        tts = yagl_gles_context_get_active_texture_target_state(ctx,
            yagl_gles_texture_target_3d);
        *params = tts->texture->base.local_name;
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
    case GL_COMPRESSED_TEXTURE_FORMATS:
        *num_params = yagl_texcompress_get_format_names((GLenum*)params);
        break;
    case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
        *num_params = 1;
        if (gles2_ctx->have_max_fragment_uniform_components) {
            *params = gles2_ctx->max_fragment_uniform_components;
        } else {
            yagl_host_glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, params, *num_params, NULL);
            gles2_ctx->max_fragment_uniform_components = *params;
            gles2_ctx->have_max_fragment_uniform_components = 1;
        }
        *params /= 4;
        break;
    case GL_MAX_VARYING_VECTORS:
        *num_params = 1;
        if (yagl_get_host_gl_version() >= yagl_gl_3_2) {
            /*
             * GL_MAX_VARYING_COMPONENTS is an alias for GL_MAX_VARYING_FLOATS
             * and it should be used in OpenGL 3.1, but it's deprecated in
             * OpenGL 3.2, thus, we use a constant.
             */
            *params = 64;
        } else if (gles2_ctx->have_max_varying_floats) {
            *params = gles2_ctx->max_varying_floats;
        } else {
            yagl_host_glGetIntegerv(GL_MAX_VARYING_FLOATS, params, *num_params, NULL);
            gles2_ctx->max_varying_floats = *params;
            gles2_ctx->have_max_varying_floats = 1;
        }
        *params /= 4;
        break;
    case GL_MAX_VERTEX_UNIFORM_VECTORS:
        *num_params = 1;
        if (gles2_ctx->have_max_vertex_uniform_components) {
            *params = gles2_ctx->max_vertex_uniform_components;
        } else {
            yagl_host_glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, params, *num_params, NULL);
            gles2_ctx->max_vertex_uniform_components = *params;
            gles2_ctx->have_max_vertex_uniform_components = 1;
        }
        *params /= 4;
        break;
    case GL_SHADER_COMPILER:
        *params = GL_TRUE;
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_FAIL:
        *params = ctx->stencil_back.fail;
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_FUNC:
        *params = ctx->stencil_back.func;
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
        *params = ctx->stencil_back.zfail;
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_PASS_DEPTH_PASS:
        *params = ctx->stencil_back.zpass;
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_REF:
        *params = ctx->stencil_back.ref;
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_VALUE_MASK:
        *params = ctx->stencil_back.mask;
        *num_params = 1;
        break;
    case GL_STENCIL_BACK_WRITEMASK:
        *params = ctx->stencil_back.writemask;
        *num_params = 1;
        break;
    case GL_DITHER:
        *params = ctx->dither_enabled;
        *num_params = 1;
        break;
    case GL_MAX_TEXTURE_SIZE:
        if (gles2_ctx->have_max_texture_size) {
            *params = gles2_ctx->max_texture_size;
            *num_params = 1;
        } else {
            processed = 0;
        }
        break;
    case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
        if (gles2_ctx->have_max_cubemap_texture_size) {
            *params = gles2_ctx->max_cubemap_texture_size;
            *num_params = 1;
        } else {
            processed = 0;
        }
        break;
    case GL_MAX_SAMPLES_IMG:
        if (gles2_ctx->have_max_samples_img) {
            *params = gles2_ctx->max_samples_img;
            *num_params = 1;
        } else {
            processed = 0;
        }
        break;
    case GL_MAX_TEXTURE_IMAGE_UNITS:
        if (gles2_ctx->have_max_texture_image_units) {
            *params = gles2_ctx->max_texture_image_units;
            *num_params = 1;
        } else {
            processed = 0;
        }
        break;
    case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
        if (gles2_ctx->have_max_texture_max_anisotropy) {
            *params = gles2_ctx->max_texture_max_anisotropy;
            *num_params = 1;
        } else {
            processed = 0;
        }
        break;
    case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
        if (gles2_ctx->have_max_vertex_texture_image_units) {
            *params = gles2_ctx->max_vertex_texture_image_units;
            *num_params = 1;
        } else {
            processed = 0;
        }
        break;
    case GL_MAX_3D_TEXTURE_SIZE_OES:
        if (gles2_ctx->have_max_3d_texture_size) {
            *params = gles2_ctx->max_3d_texture_size;
            *num_params = 1;
        } else {
            processed = 0;
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
    case GL_MAX_TEXTURE_SIZE:
        *num_params = 1;
        yagl_host_glGetIntegerv(pname, params, *num_params, NULL);
        gles2_ctx->max_texture_size = *params;
        gles2_ctx->have_max_texture_size = 1;
        break;
    case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
        *num_params = 1;
        yagl_host_glGetIntegerv(pname, params, *num_params, NULL);
        gles2_ctx->max_cubemap_texture_size = *params;
        gles2_ctx->have_max_cubemap_texture_size = 1;
        break;
    case GL_MAX_SAMPLES_IMG:
        *num_params = 1;
        yagl_host_glGetIntegerv(pname, params, *num_params, NULL);
        gles2_ctx->max_samples_img = *params;
        gles2_ctx->have_max_samples_img = 1;
        break;
    case GL_MAX_TEXTURE_IMAGE_UNITS:
        *num_params = 1;
        yagl_host_glGetIntegerv(pname, params, *num_params, NULL);
        gles2_ctx->max_texture_image_units = *params;
        gles2_ctx->have_max_texture_image_units = 1;
        break;
    case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
        *num_params = 1;
        yagl_host_glGetIntegerv(pname, params, *num_params, NULL);
        gles2_ctx->max_texture_max_anisotropy = *params;
        gles2_ctx->have_max_texture_max_anisotropy = 1;
        break;
    case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
        *num_params = 1;
        yagl_host_glGetIntegerv(pname, params, *num_params, NULL);
        gles2_ctx->max_vertex_texture_image_units = *params;
        gles2_ctx->have_max_vertex_texture_image_units = 1;
        break;
    case GL_SHADER_BINARY_FORMATS:
        *num_params = 0;
        break;
    case GL_MAX_3D_TEXTURE_SIZE_OES:
        *num_params = 1;
        yagl_host_glGetIntegerv(pname, params, *num_params, NULL);
        gles2_ctx->max_3d_texture_size = *params;
        gles2_ctx->have_max_3d_texture_size = 1;
        break;
    default:
        return 0;
    }

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
    struct yagl_gles2_context *gles2_ctx = (struct yagl_gles2_context*)ctx;

    yagl_gles2_context_pre_draw(gles2_ctx, mode, first + count);

    if (primcount < 0) {
        yagl_host_glDrawArrays(mode, first, count);
    } else {
        yagl_host_glDrawArraysInstanced(mode, first, count, primcount);
    }

    yagl_gles2_context_post_draw(gles2_ctx, mode, first + count);
}

void yagl_gles2_context_draw_elements(struct yagl_gles_context *ctx,
                                      GLenum mode,
                                      GLsizei count,
                                      GLenum type,
                                      const GLvoid *indices,
                                      int32_t indices_count,
                                      GLsizei primcount,
                                      uint32_t max_idx)
{
    struct yagl_gles2_context *gles2_ctx = (struct yagl_gles2_context*)ctx;

    yagl_gles2_context_pre_draw(gles2_ctx, mode, max_idx + 1);

    if (primcount < 0) {
        yagl_host_glDrawElements(mode, count, type, indices, indices_count);
    } else {
        yagl_host_glDrawElementsInstanced(mode, count, type, indices, indices_count, primcount);
    }

    yagl_gles2_context_post_draw(gles2_ctx, mode, max_idx + 1);
}

int yagl_gles2_context_validate_texture_target(struct yagl_gles_context *ctx,
                                               GLenum target,
                                               yagl_gles_texture_target *texture_target)
{
    switch (target) {
    case GL_TEXTURE_3D_OES:
        *texture_target = yagl_gles_texture_target_3d;
        break;
    case GL_TEXTURE_CUBE_MAP:
        *texture_target = yagl_gles_texture_target_cubemap;
        break;
    default:
        return 0;
    }

    return 1;
}

struct yagl_pixel_format
    *yagl_gles2_context_validate_teximage_format(struct yagl_gles_context *ctx,
                                                 GLenum internalformat,
                                                 GLenum format,
                                                 GLenum type)
{
    return NULL;
}

struct yagl_pixel_format
    *yagl_gles2_context_validate_getteximage_format(struct yagl_gles_context *ctx,
                                                    GLenum readbuffer_internalformat,
                                                    GLenum format,
                                                    GLenum type)
{
    return NULL;
}

int yagl_gles2_context_validate_copyteximage_format(struct yagl_gles_context *ctx,
                                                    GLenum readbuffer_internalformat,
                                                    GLenum *internalformat)
{
    return 0;
}

int yagl_gles2_context_validate_texstorage_format(struct yagl_gles_context *ctx,
                                                  GLenum *internalformat,
                                                  GLenum *base_internalformat,
                                                  GLenum *any_format,
                                                  GLenum *any_type)
{
    struct yagl_texcompress_format *tc_format;

    if (*internalformat == GL_ETC1_RGB8_OES) {
        /*
         * This is not allowed for TexStorage.
         */
        return 0;
    }

    tc_format = yagl_texcompress_get_format(*internalformat);

    if (!tc_format) {
        return 0;
    }

    *internalformat = tc_format->dst_internalformat;
    *base_internalformat = tc_format->dst_internalformat;
    *any_format = tc_format->dst_format;
    *any_type = tc_format->dst_type;

    return 1;
}

int yagl_gles2_context_validate_renderbuffer_format(struct yagl_gles_context *ctx,
                                                    GLenum *internalformat)
{
    return 0;
}

void yagl_gles2_context_hint(struct yagl_gles_context *ctx,
                             GLenum target,
                             GLenum mode)
{
}

int yagl_gles2_context_get_programiv(struct yagl_gles2_context *ctx,
                                     struct yagl_gles2_program *program,
                                     GLenum pname,
                                     GLint *params)
{
    switch (pname) {
    case GL_ATTACHED_SHADERS:
        *params = (program->fragment_shader != NULL) ? 1 : 0;
        *params += (program->vertex_shader != NULL) ? 1 : 0;
        break;
    case GL_INFO_LOG_LENGTH:
        *params = program->info_log_length;
        break;
    case GL_ACTIVE_ATTRIBUTES:
        *params = program->num_active_attribs;
        break;
    case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
        *params = program->max_active_attrib_bufsize;
        break;
    case GL_ACTIVE_UNIFORMS:
        *params = program->num_active_uniforms;
        break;
    case GL_ACTIVE_UNIFORM_MAX_LENGTH:
        *params = program->max_active_uniform_bufsize;
        break;
    case GL_DELETE_STATUS:
        *params = GL_FALSE;
        break;
    case GL_LINK_STATUS:
        *params = program->link_status;
        break;
    default:
        return 0;
    }

    return 1;
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
    gles2_ctx->base.compressed_tex_image_2d = &yagl_gles2_context_compressed_tex_image_2d;
    gles2_ctx->base.compressed_tex_sub_image_2d = &yagl_gles2_context_compressed_tex_sub_image_2d;
    gles2_ctx->base.enable = &yagl_gles2_context_enable;
    gles2_ctx->base.is_enabled = &yagl_gles2_context_is_enabled;
    gles2_ctx->base.get_integerv = &yagl_gles2_context_get_integerv;
    gles2_ctx->base.get_floatv = &yagl_gles2_context_get_floatv;
    gles2_ctx->base.draw_arrays = &yagl_gles2_context_draw_arrays;
    gles2_ctx->base.draw_elements = &yagl_gles2_context_draw_elements;
    gles2_ctx->base.bind_buffer = &yagl_gles2_context_bind_buffer;
    gles2_ctx->base.unbind_buffer = &yagl_gles2_context_unbind_buffer;
    gles2_ctx->base.acquire_binded_buffer = &yagl_gles2_context_acquire_binded_buffer;
    gles2_ctx->base.validate_texture_target = &yagl_gles2_context_validate_texture_target;
    gles2_ctx->base.validate_teximage_format = &yagl_gles2_context_validate_teximage_format;
    gles2_ctx->base.validate_getteximage_format = &yagl_gles2_context_validate_getteximage_format;
    gles2_ctx->base.validate_copyteximage_format = &yagl_gles2_context_validate_copyteximage_format;
    gles2_ctx->base.validate_texstorage_format = &yagl_gles2_context_validate_texstorage_format;
    gles2_ctx->base.validate_renderbuffer_format = &yagl_gles2_context_validate_renderbuffer_format;
    gles2_ctx->base.hint = &yagl_gles2_context_hint;
    gles2_ctx->get_programiv = &yagl_gles2_context_get_programiv;
    gles2_ctx->pre_use_program = &yagl_gles2_context_pre_use_program;
    gles2_ctx->pre_link_program = &yagl_gles2_context_pre_link_program;

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
    case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
        if (ctx->base.base.client_api == yagl_client_api_gles3) {
            *param = array->integer;
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
