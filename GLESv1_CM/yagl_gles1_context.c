#include "GLES/gl.h"
#include "GLES/glext.h"
#include "yagl_gles1_context.h"
#include "yagl_gles_vertex_array.h"
#include "yagl_gles_array.h"
#include "yagl_gles_buffer.h"
#include "yagl_gles_texture.h"
#include "yagl_gles_texture_unit.h"
#include "yagl_gles_utils.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_utils.h"
#include "yagl_state.h"
#include "yagl_egl_fence.h"
#include "yagl_host_gles_calls.h"
#include "yagl_texcompress_etc1.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*
 * We can't include GL/gl.h here
 */
#define GL_RGBA8 0x8058

#define YAGL_SET_ERR(err) \
    yagl_gles_context_set_error(ctx, err); \
    YAGL_LOG_ERROR("error = 0x%X", err)

#define YAGL_GLES1_NUM_COMP_TEX_FORMATS 11

static const GLchar *blend_subtract_ext = "GL_OES_blend_subtract";
static const GLchar *blend_equation_separate_ext = "GL_OES_blend_equation_separate";
static const GLchar *blend_func_separate_ext = "GL_OES_blend_func_separate";
static const GLchar *blend_minmax_ext = "GL_EXT_blend_minmax";
static const GLchar *element_index_uint_ext = "GL_OES_element_index_uint";
static const GLchar *texture_mirrored_repeat_ext = "GL_OES_texture_mirrored_repeat";
static const GLchar *texture_format_bgra8888_ext = "GL_EXT_texture_format_BGRA8888";
static const GLchar *point_sprite_ext = "GL_OES_point_sprite";
static const GLchar *point_size_array_ext = "GL_OES_point_size_array";
static const GLchar *stencil_wrap_ext = "GL_OES_stencil_wrap";
static const GLchar *compressed_paletted_texture_ext = "GL_OES_compressed_paletted_texture";
static const GLchar *depth_texture_ext = "GL_OES_depth_texture";
static const GLchar *framebuffer_object_ext = "GL_OES_framebuffer_object";
static const GLchar *depth24_ext = "GL_OES_depth24";
static const GLchar *depth32_ext = "GL_OES_depth32";
static const GLchar *rgb8_rgba8_ext = "GL_OES_rgb8_rgba8";
static const GLchar *stencil1_ext = "GL_OES_stencil1";
static const GLchar *stencil4_ext = "GL_OES_stencil4";
static const GLchar *stencil8_ext = "GL_OES_stencil8";
static const GLchar *egl_image_ext = "GL_OES_EGL_image";
static const GLchar *framebuffer_blit_ext = "GL_ANGLE_framebuffer_blit";
static const GLchar *draw_buffers_ext = "GL_EXT_draw_buffers";
static const GLchar *mapbuffer_ext = "GL_OES_mapbuffer";
static const GLchar *map_buffer_range_ext = "GL_EXT_map_buffer_range";
static const GLchar *texture_storage_ext = "GL_EXT_texture_storage";
static const GLchar *pbo_ext = "GL_NV_pixel_buffer_object";
static const GLchar *read_buffer_ext = "GL_NV_read_buffer";
static const GLchar *compressed_etc1_rgb8_texture_ext = "GL_OES_compressed_ETC1_RGB8_texture";
static const GLchar *pack_subimage_ext = "GL_NV_pack_subimage";
static const GLchar *unpack_subimage_ext = "GL_EXT_unpack_subimage";
static const GLchar *egl_sync_ext = "GL_OES_EGL_sync";
static const GLchar *packed_depth_stencil_ext = "GL_OES_packed_depth_stencil";
static const GLchar *texture_npot_ext = "GL_OES_texture_npot";
static const GLchar *texture_filter_anisotropic_ext = "GL_EXT_texture_filter_anisotropic";
static const GLchar *vertex_array_object_ext = "GL_OES_vertex_array_object";
static const GLchar *matrix_palette_ext = "GL_OES_matrix_palette";

static const GLchar **yagl_gles1_context_get_extensions(struct yagl_gles1_context *ctx,
                                                        int *num_extensions)
{
    const GLchar **extensions;
    int i = 0;

    extensions = yagl_malloc(100 * sizeof(*extensions));

    extensions[i++] = blend_subtract_ext;
    extensions[i++] = blend_equation_separate_ext;
    extensions[i++] = blend_func_separate_ext;
    extensions[i++] = blend_minmax_ext;
    extensions[i++] = element_index_uint_ext;
    extensions[i++] = texture_mirrored_repeat_ext;
    extensions[i++] = texture_format_bgra8888_ext;
    extensions[i++] = point_sprite_ext;
    extensions[i++] = point_size_array_ext;
    extensions[i++] = stencil_wrap_ext;
    extensions[i++] = compressed_paletted_texture_ext;
    extensions[i++] = depth_texture_ext;
    extensions[i++] = framebuffer_object_ext;
    extensions[i++] = depth24_ext;
    extensions[i++] = depth32_ext;
    extensions[i++] = rgb8_rgba8_ext;
    extensions[i++] = stencil1_ext;
    extensions[i++] = stencil4_ext;
    extensions[i++] = stencil8_ext;
    extensions[i++] = egl_image_ext;
    extensions[i++] = framebuffer_blit_ext;
    extensions[i++] = draw_buffers_ext;
    extensions[i++] = mapbuffer_ext;
    extensions[i++] = map_buffer_range_ext;
    extensions[i++] = texture_storage_ext;
    extensions[i++] = pbo_ext;
    extensions[i++] = read_buffer_ext;
    extensions[i++] = compressed_etc1_rgb8_texture_ext;
    extensions[i++] = pack_subimage_ext;
    extensions[i++] = unpack_subimage_ext;

    if (yagl_egl_fence_supported()) {
        extensions[i++] = egl_sync_ext;
    }

    if (ctx->base.texture_npot) {
        extensions[i++] = texture_npot_ext;
    }

    if (ctx->base.texture_filter_anisotropic) {
        extensions[i++] = texture_filter_anisotropic_ext;
    }

    if (ctx->base.packed_depth_stencil) {
        extensions[i++] = packed_depth_stencil_ext;
    }

    if (ctx->base.vertex_arrays_supported) {
        extensions[i++] = vertex_array_object_ext;
    }

    if (ctx->matrix_palette) {
        extensions[i++] = matrix_palette_ext;
    }

    *num_extensions = i;

    return extensions;
}

static void yagl_gles1_compressed_texture_formats_fill(GLint *params)
{
    params[0] = GL_PALETTE4_RGB8_OES;
    params[1] = GL_PALETTE4_RGBA8_OES;
    params[2] = GL_PALETTE4_R5_G6_B5_OES;
    params[3] = GL_PALETTE4_RGBA4_OES;
    params[4] = GL_PALETTE4_RGB5_A1_OES;
    params[5] = GL_PALETTE8_RGB8_OES;
    params[6] = GL_PALETTE8_RGBA8_OES;
    params[7] = GL_PALETTE8_R5_G6_B5_OES;
    params[8] = GL_PALETTE8_RGBA4_OES;
    params[9] = GL_PALETTE8_RGB5_A1_OES;
    params[10] = GL_ETC1_RGB8_OES;
}

static void yagl_gles1_vertex_array_apply(struct yagl_gles_array *array,
                                          uint32_t first,
                                          uint32_t count,
                                          const GLvoid *ptr,
                                          void *user_data)
{
    if (array->vbo) {
        yagl_host_glVertexPointerOffset(array->size,
                                        array->actual_type,
                                        array->actual_stride,
                                        array->actual_offset);
    } else {
        yagl_host_glVertexPointerData(array->size,
                                      array->actual_type,
                                      array->actual_stride,
                                      first,
                                      ptr + (first * array->actual_stride),
                                      count * array->actual_stride);
    }
}

static void yagl_gles1_normal_array_apply(struct yagl_gles_array *array,
                                          uint32_t first,
                                          uint32_t count,
                                          const GLvoid *ptr,
                                          void *user_data)
{
    if (array->vbo) {
        yagl_host_glNormalPointerOffset(array->actual_type,
                                        array->actual_stride,
                                        array->actual_offset);
    } else {
        yagl_host_glNormalPointerData(array->actual_type,
                                      array->actual_stride,
                                      first,
                                      ptr + (first * array->actual_stride),
                                      count * array->actual_stride);
    }
}

static void yagl_gles1_color_array_apply(struct yagl_gles_array *array,
                                         uint32_t first,
                                         uint32_t count,
                                         const GLvoid *ptr,
                                         void *user_data)
{
    if (array->vbo) {
        yagl_host_glColorPointerOffset(array->size,
                                       array->actual_type,
                                       array->actual_stride,
                                       array->actual_offset);
    } else {
        yagl_host_glColorPointerData(array->size,
                                     array->actual_type,
                                     array->actual_stride,
                                     first,
                                     ptr + (first * array->actual_stride),
                                     count * array->actual_stride);
    }
}

static void yagl_gles1_texcoord_array_apply(struct yagl_gles_array *array,
                                            uint32_t first,
                                            uint32_t count,
                                            const GLvoid *ptr,
                                            void *user_data)
{
    struct yagl_gles1_context *gles1_ctx = user_data;
    int tex_id = array->index - yagl_gles1_array_texcoord;

    if (tex_id != gles1_ctx->client_active_texture) {
        yagl_host_glClientActiveTexture(tex_id + GL_TEXTURE0);
    }

    if (array->vbo) {
        yagl_host_glTexCoordPointerOffset(array->size,
                                          array->actual_type,
                                          array->actual_stride,
                                          array->actual_offset);
    } else {
        yagl_host_glTexCoordPointerData(tex_id,
                                        array->size,
                                        array->actual_type,
                                        array->actual_stride,
                                        first,
                                        ptr + (first * array->actual_stride),
                                        count * array->actual_stride);
    }

    if (tex_id != gles1_ctx->client_active_texture) {
        yagl_host_glClientActiveTexture(gles1_ctx->client_active_texture +
                                        GL_TEXTURE0);
    }
}

static void yagl_gles1_pointsize_array_apply(struct yagl_gles_array *array,
                                             uint32_t first,
                                             uint32_t count,
                                             const GLvoid *ptr,
                                             void *user_data)
{
}

static unsigned yagl_gles1_array_idx_from_pname(struct yagl_gles1_context *ctx,
                                                GLenum pname)
{
    switch (pname) {
    case GL_VERTEX_ARRAY:
    case GL_VERTEX_ARRAY_BUFFER_BINDING:
    case GL_VERTEX_ARRAY_SIZE:
    case GL_VERTEX_ARRAY_STRIDE:
    case GL_VERTEX_ARRAY_TYPE:
        return yagl_gles1_array_vertex;
    case GL_COLOR_ARRAY:
    case GL_COLOR_ARRAY_BUFFER_BINDING:
    case GL_COLOR_ARRAY_SIZE:
    case GL_COLOR_ARRAY_STRIDE:
    case GL_COLOR_ARRAY_TYPE:
        return yagl_gles1_array_color;
    case GL_NORMAL_ARRAY:
    case GL_NORMAL_ARRAY_BUFFER_BINDING:
    case GL_NORMAL_ARRAY_STRIDE:
    case GL_NORMAL_ARRAY_TYPE:
        return yagl_gles1_array_normal;
    case GL_TEXTURE_COORD_ARRAY:
    case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
    case GL_TEXTURE_COORD_ARRAY_SIZE:
    case GL_TEXTURE_COORD_ARRAY_STRIDE:
    case GL_TEXTURE_COORD_ARRAY_TYPE:
        return yagl_gles1_array_texcoord + ctx->client_active_texture;
    case GL_POINT_SIZE_ARRAY_TYPE_OES:
    case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
    case GL_POINT_SIZE_ARRAY_OES:
    case GL_POINT_SIZE_ARRAY_STRIDE_OES:
        return yagl_gles1_array_pointsize;
    default:
        fprintf(stderr, "Critical error! Bad array name!\n");
        exit(1);
    }
}

static void yagl_gles1_context_prepare(struct yagl_client_context *ctx)
{
    struct yagl_gles1_context *gles1_ctx = (struct yagl_gles1_context*)ctx;
    GLint num_texture_units = 0;
    int32_t size = 0;
    char *host_extensions;
    const GLchar **extensions;
    int num_extensions;

    YAGL_LOG_FUNC_ENTER(yagl_gles1_context_prepare, "%p", ctx);

    yagl_host_glGetIntegerv(GL_MAX_TEXTURE_UNITS,
                            &num_texture_units, 1, NULL);

    /*
     * We limit this by 32 for conformance.
     */
    if (num_texture_units > 32) {
        num_texture_units = 32;
    }

    yagl_gles_context_prepare(&gles1_ctx->base,
                              num_texture_units,
                              /* Each texture unit has its own client-side array state */
                              yagl_gles1_array_texcoord + num_texture_units);

    yagl_host_glGetIntegerv(GL_MAX_CLIP_PLANES,
                            &gles1_ctx->max_clip_planes,
                            1, NULL);

    if (gles1_ctx->max_clip_planes < 6) {
        YAGL_LOG_WARN("host GL_MAX_CLIP_PLANES=%d is less then required 6",
                      gles1_ctx->max_clip_planes);
    } else {
        /* According to OpenGLES 1.1 docs on khronos website we only need
         * to support 6 planes. This will protect us from bogus
         * GL_MAX_CLIP_PLANES value reported by some drivers */
        gles1_ctx->max_clip_planes = 6;
    }

    yagl_host_glGetIntegerv(GL_MAX_LIGHTS, &gles1_ctx->max_lights, 1, NULL);

    yagl_host_glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gles1_ctx->max_tex_size, 1, NULL);

    yagl_host_glGetString(GL_EXTENSIONS, NULL, 0, &size);
    host_extensions = yagl_malloc0(size);
    yagl_host_glGetString(GL_EXTENSIONS, host_extensions, size, NULL);

    gles1_ctx->matrix_palette =
        (strstr(host_extensions, "GL_ARB_vertex_blend ") != NULL) &&
        (strstr(host_extensions, "GL_ARB_matrix_palette ") != NULL);

    yagl_free(host_extensions);

    extensions = yagl_gles1_context_get_extensions(gles1_ctx, &num_extensions);

    yagl_gles_context_prepare_end(&gles1_ctx->base, extensions, num_extensions);

    YAGL_LOG_FUNC_EXIT(NULL);
}

static void yagl_gles1_context_destroy(struct yagl_client_context *ctx)
{
    struct yagl_gles1_context *gles1_ctx = (struct yagl_gles1_context*)ctx;

    YAGL_LOG_FUNC_ENTER(yagl_gles1_context_destroy, "%p", ctx);

    yagl_gles_context_cleanup(&gles1_ctx->base);

    yagl_free(gles1_ctx);

    YAGL_LOG_FUNC_EXIT(NULL);
}

static struct yagl_gles_array
    *yagl_gles1_context_create_arrays(struct yagl_gles_context *ctx)
{
    GLint i;
    struct yagl_gles_array *arrays;

    arrays = yagl_malloc(ctx->num_arrays * sizeof(*arrays));

    yagl_gles_array_init(&arrays[yagl_gles1_array_vertex],
                         yagl_gles1_array_vertex,
                         &yagl_gles1_vertex_array_apply,
                         ctx);

    yagl_gles_array_init(&arrays[yagl_gles1_array_color],
                         yagl_gles1_array_color,
                         &yagl_gles1_color_array_apply,
                         ctx);

    yagl_gles_array_init(&arrays[yagl_gles1_array_normal],
                         yagl_gles1_array_normal,
                         &yagl_gles1_normal_array_apply,
                         ctx);

    yagl_gles_array_init(&arrays[yagl_gles1_array_pointsize],
                         yagl_gles1_array_pointsize,
                         &yagl_gles1_pointsize_array_apply,
                         ctx);

    for (i = yagl_gles1_array_texcoord; i < ctx->num_arrays; ++i) {
        yagl_gles_array_init(&arrays[i],
                             i,
                             &yagl_gles1_texcoord_array_apply,
                             ctx);
    }

    return arrays;
}

static const GLchar
    *yagl_gles1_context_get_string(struct yagl_gles_context *ctx,
                                   GLenum name)
{
    const char *str = NULL;

    switch (name) {
    case GL_VERSION:
        str = "OpenGL ES-CM 1.1";
        break;
    case GL_RENDERER:
        str = "YaGL GLESv1_CM";
        break;
    default:
        str = "";
    }

    return str;
}

typedef struct YaglGles1PalFmtDesc {
    GLenum uncomp_format;
    GLenum pixel_type;
    unsigned pixel_size;
    unsigned bits_per_index;
} YaglGles1PalFmtDesc;

static inline int yagl_log2(int val)
{
   int ret = 0;

   if (val > 0) {
       while (val >>= 1) {
           ret++;
       }
   }

   return ret;
}

static inline int yagl_gles1_tex_dims_valid(GLsizei width,
                                            GLsizei height,
                                            int max_size)
{
    if (width < 0 || height < 0 || width > max_size || height > max_size ||
        (width & (width - 1)) || (height & (height - 1))) {
        return 0;
    }

    return 1;
}

static void yagl_gles1_cpal_format_get_descr(GLenum format,
                                             YaglGles1PalFmtDesc *desc)
{
    assert(format >= GL_PALETTE4_RGB8_OES && format <= GL_PALETTE8_RGB5_A1_OES);

    switch (format) {
    case GL_PALETTE4_RGB8_OES:
        desc->uncomp_format = GL_RGB;
        desc->bits_per_index = 4;
        desc->pixel_type = GL_UNSIGNED_BYTE;
        desc->pixel_size = 3;
        break;
    case GL_PALETTE4_RGBA8_OES:
        desc->uncomp_format = GL_RGBA;
        desc->bits_per_index = 4;
        desc->pixel_type = GL_UNSIGNED_BYTE;
        desc->pixel_size = 4;
        break;
    case GL_PALETTE4_R5_G6_B5_OES:
        desc->uncomp_format = GL_RGB;
        desc->bits_per_index = 4;
        desc->pixel_type = GL_UNSIGNED_SHORT_5_6_5;
        desc->pixel_size = 2;
        break;
    case GL_PALETTE4_RGBA4_OES:
        desc->uncomp_format = GL_RGBA;
        desc->bits_per_index = 4;
        desc->pixel_type = GL_UNSIGNED_SHORT_4_4_4_4;
        desc->pixel_size = 2;
        break;
    case GL_PALETTE4_RGB5_A1_OES:
        desc->uncomp_format = GL_RGBA;
        desc->bits_per_index = 4;
        desc->pixel_type = GL_UNSIGNED_SHORT_5_5_5_1;
        desc->pixel_size = 2;
        break;
    case GL_PALETTE8_RGB8_OES:
        desc->uncomp_format = GL_RGB;
        desc->bits_per_index = 8;
        desc->pixel_type = GL_UNSIGNED_BYTE;
        desc->pixel_size = 3;
        break;
    case GL_PALETTE8_RGBA8_OES:
        desc->uncomp_format = GL_RGBA;
        desc->bits_per_index = 8;
        desc->pixel_type = GL_UNSIGNED_BYTE;
        desc->pixel_size = 4;
        break;
    case GL_PALETTE8_R5_G6_B5_OES:
        desc->uncomp_format = GL_RGB;
        desc->bits_per_index = 8;
        desc->pixel_type = GL_UNSIGNED_SHORT_5_6_5;
        desc->pixel_size = 2;
        break;
    case GL_PALETTE8_RGBA4_OES:
        desc->uncomp_format = GL_RGBA;
        desc->bits_per_index = 8;
        desc->pixel_type = GL_UNSIGNED_SHORT_4_4_4_4;
        desc->pixel_size = 2;
        break;
    case GL_PALETTE8_RGB5_A1_OES:
        desc->uncomp_format = GL_RGBA;
        desc->bits_per_index = 8;
        desc->pixel_type = GL_UNSIGNED_SHORT_5_5_5_1;
        desc->pixel_size = 2;
        break;
    }
}

static GLsizei yagl_gles1_cpal_tex_size(YaglGles1PalFmtDesc *fmt_desc,
                                        unsigned width,
                                        unsigned height,
                                        unsigned max_level)
{
    GLsizei size;

    /* Palette table size */
    size = (1 << fmt_desc->bits_per_index) * fmt_desc->pixel_size;

    /* Texture palette indices array size for each miplevel */
    do {
        if (fmt_desc->bits_per_index == 4) {
            size += (width * height + 1) / 2;
        } else {
            size += width * height;
        }

        width >>= 1;
        if (width == 0) {
            width = 1;
        }

        height >>= 1;
        if (height == 0) {
            height = 1;
        }
    } while (max_level--);

    return size;
}

static void yagl_gles1_cpal_tex_uncomp_and_apply(struct yagl_gles_context *ctx,
                                                 struct yagl_gles_texture *texture,
                                                 YaglGles1PalFmtDesc *fmt_desc,
                                                 unsigned max_level,
                                                 unsigned width,
                                                 unsigned height,
                                                 const GLvoid *data)
{
    uint8_t *tex_img_data = NULL;
    uint8_t *img;
    const uint8_t *indices;
    unsigned cur_level, i;
    unsigned num_of_texels = width * height;

    if (!data) {
        for (cur_level = 0; cur_level <= max_level; ++cur_level) {
            yagl_host_glTexImage2DData(GL_TEXTURE_2D,
                                       cur_level,
                                       fmt_desc->uncomp_format,
                                       width, height,
                                       0,
                                       fmt_desc->uncomp_format,
                                       fmt_desc->pixel_type,
                                       NULL, 0);
            width >>= 1;
            height >>= 1;

            if (width == 0) {
                width = 1;
            }

            if (height == 0) {
                height = 1;
            }
        }

        yagl_gles_texture_set_internalformat(texture,
                                             fmt_desc->uncomp_format,
                                             fmt_desc->pixel_type,
                                             yagl_gles_context_convert_textures(ctx));

        return;
    }

    /* Jump over palette data to first image data */
    indices = data + (1 << fmt_desc->bits_per_index) * fmt_desc->pixel_size;

    /* 0 level image is the largest */
    tex_img_data = yagl_malloc(num_of_texels * fmt_desc->pixel_size);

    /* We will pass tightly packed data to glTexImage2D */
    yagl_gles_reset_unpack(&ctx->unpack);

    for (cur_level = 0; cur_level <= max_level; ++cur_level) {
        img = tex_img_data;

        if (fmt_desc->bits_per_index == 4) {
            unsigned cur_idx;

            for (i = 0; i < num_of_texels; ++i) {
                if ((i % 2) == 0) {
                    cur_idx = indices[i / 2] >> 4;
                } else {
                    cur_idx = indices[i / 2] & 0xf;
                }

                memcpy(img,
                       data + cur_idx * fmt_desc->pixel_size,
                       fmt_desc->pixel_size);

                img += fmt_desc->pixel_size;
            }

            indices += (num_of_texels + 1) / 2;
        } else {
            for (i = 0; i < num_of_texels; ++i) {
                memcpy(img,
                       data + indices[i] * fmt_desc->pixel_size,
                       fmt_desc->pixel_size);
                img += fmt_desc->pixel_size;
            }

            indices += num_of_texels;
        }

        yagl_host_glTexImage2DData(GL_TEXTURE_2D,
                                   cur_level,
                                   fmt_desc->uncomp_format,
                                   width, height,
                                   0,
                                   fmt_desc->uncomp_format,
                                   fmt_desc->pixel_type,
                                   tex_img_data,
                                   num_of_texels * fmt_desc->pixel_size);

        width >>= 1;
        if (width == 0) {
            width = 1;
        }

        height >>= 1;
        if (height == 0) {
            height = 1;
        }

        num_of_texels = width * height;
    }

    yagl_free(tex_img_data);

    yagl_gles_set_unpack(&ctx->unpack);

    yagl_gles_texture_set_internalformat(texture,
                                         fmt_desc->uncomp_format,
                                         fmt_desc->pixel_type,
                                         yagl_gles_context_convert_textures(ctx));
}

static void yagl_gles1_etc1_rgb8_uncomp_and_apply(struct yagl_gles_context *ctx,
                                                  struct yagl_gles_texture *texture,
                                                  GLint level,
                                                  GLsizei width,
                                                  GLsizei height,
                                                  GLint border,
                                                  GLsizei imageSize,
                                                  const GLvoid *data)
{
    GLsizei wblocks = (width + 3) / 4;
    GLsizei hblocks = (height + 3) / 4;
    uint8_t *buff;

    YAGL_LOG_FUNC_SET(glCompressedTexImage2D);

    if (imageSize != (wblocks * hblocks * 8)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        return;
    }

    if (!data) {
        yagl_host_glTexImage2DData(GL_TEXTURE_2D,
                                   level,
                                   GL_RGBA8,
                                   width,
                                   height,
                                   border,
                                   GL_RGBA,
                                   GL_UNSIGNED_BYTE,
                                   NULL,
                                   width * height * 4);

        yagl_gles_texture_set_internalformat(texture,
                                             GL_RGBA8,
                                             GL_UNSIGNED_BYTE,
                                             yagl_gles_context_convert_textures(ctx));

        return;
    }

    buff = yagl_get_tmp_buffer(width * height * 4);

    yagl_texcompress_etc1_unpack_rgba8888(buff,
                                          (width * 4),
                                          data,
                                          (wblocks * 8),
                                          width,
                                          height);

    yagl_gles_reset_unpack(&ctx->unpack);

    yagl_host_glTexImage2DData(GL_TEXTURE_2D,
                               level,
                               GL_RGBA8,
                               width,
                               height,
                               border,
                               GL_RGBA,
                               GL_UNSIGNED_BYTE,
                               buff,
                               width * height * 4);

    yagl_gles_set_unpack(&ctx->unpack);

    yagl_gles_texture_set_internalformat(texture,
                                         GL_RGBA8,
                                         GL_UNSIGNED_BYTE,
                                         yagl_gles_context_convert_textures(ctx));
}

static void yagl_gles1_context_compressed_tex_image_2d(struct yagl_gles_context *ctx,
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
    const int max_tex_size = ((struct yagl_gles1_context*)ctx)->max_tex_size;
    YaglGles1PalFmtDesc fmt_desc;

    YAGL_LOG_FUNC_SET(glCompressedTexImage2D);

    if (target != GL_TEXTURE_2D) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        return;
    }

    switch (internalformat) {
    case GL_PALETTE4_RGB8_OES ... GL_PALETTE8_RGB5_A1_OES:
        yagl_gles1_cpal_format_get_descr(internalformat, &fmt_desc);

        if ((level > 0) || (-level > yagl_log2(max_tex_size)) ||
            !yagl_gles1_tex_dims_valid(width, height, max_tex_size) ||
            border != 0 || (imageSize !=
                yagl_gles1_cpal_tex_size(&fmt_desc, width, height, -level))) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            return;
        }

        yagl_gles1_cpal_tex_uncomp_and_apply(ctx,
                                             texture,
                                             &fmt_desc,
                                             -level,
                                             width,
                                             height,
                                             data);
        break;
    case GL_ETC1_RGB8_OES:
        yagl_gles1_etc1_rgb8_uncomp_and_apply(ctx, texture, level, width, height,
                                              border, imageSize, data);
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        return;
    }
}

static void yagl_gles1_context_compressed_tex_sub_image_2d(struct yagl_gles_context *ctx,
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
    YAGL_LOG_FUNC_SET(glCompressedTexSubImage2D);

    /* No formats are supported by this call in GLES1 API */
    YAGL_SET_ERR(GL_INVALID_OPERATION);
}

static int yagl_gles1_context_enable(struct yagl_gles_context *ctx,
                                     GLenum cap,
                                     GLboolean enable)
{
    struct yagl_gles1_context *gles1_ctx = (struct yagl_gles1_context*)ctx;

    switch (cap) {
    case GL_TEXTURE_2D:
        yagl_gles_context_active_texture_set_enabled(ctx,
            yagl_gles_texture_target_2d, enable);
        break;
    case GL_ALPHA_TEST:
    case GL_COLOR_LOGIC_OP:
    case GL_COLOR_MATERIAL:
    case GL_FOG:
    case GL_LIGHTING:
    case GL_LINE_SMOOTH:
    case GL_MULTISAMPLE:
    case GL_NORMALIZE:
    case GL_POINT_SMOOTH:
    case GL_POINT_SPRITE_OES:
    case GL_RESCALE_NORMAL:
    case GL_SAMPLE_ALPHA_TO_ONE:
        break;
    default:
        if ((cap >= GL_CLIP_PLANE0 &&
             cap <= (GL_CLIP_PLANE0 + gles1_ctx->max_clip_planes - 1)) ||
            (cap >= GL_LIGHT0 &&
             cap <= (GL_LIGHT0 + gles1_ctx->max_lights - 1))) {
            break;
        }
        return 0;
    }

    if (enable) {
        yagl_host_glEnable(cap);
    } else {
        yagl_host_glDisable(cap);
    }

    return 1;
}

static int yagl_gles1_context_is_enabled(struct yagl_gles_context *ctx,
                                         GLenum cap,
                                         GLboolean *enabled)
{
    struct yagl_gles1_context *gles1_ctx = (struct yagl_gles1_context*)ctx;
    struct yagl_gles_texture_target_state *tts;

    switch (cap) {
    case GL_TEXTURE_2D:
        tts = yagl_gles_context_get_active_texture_target_state(ctx,
                  yagl_gles_texture_target_2d);
        *enabled = tts->enabled;
        return 1;
    case GL_VERTEX_ARRAY:
    case GL_NORMAL_ARRAY:
    case GL_COLOR_ARRAY:
    case GL_TEXTURE_COORD_ARRAY:
    case GL_POINT_SIZE_ARRAY_OES:
        *enabled = ctx->vao->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, cap)].enabled;
        return 1;
    case GL_ALPHA_TEST:
    case GL_COLOR_LOGIC_OP:
    case GL_COLOR_MATERIAL:
    case GL_FOG:
    case GL_LIGHTING:
    case GL_LINE_SMOOTH:
    case GL_MULTISAMPLE:
    case GL_NORMALIZE:
    case GL_POINT_SMOOTH:
    case GL_RESCALE_NORMAL:
    case GL_SAMPLE_ALPHA_TO_ONE:
    case GL_POINT_SPRITE_OES:
        break;
    default:
        if ((cap >= GL_CLIP_PLANE0 &&
             cap <= (GL_CLIP_PLANE0 + gles1_ctx->max_clip_planes - 1)) ||
            (cap >= GL_LIGHT0 &&
             cap <= (GL_LIGHT0 + gles1_ctx->max_lights - 1))) {
            break;
        }
        return 0;
    }

    *enabled = yagl_host_glIsEnabled(cap);

    return 1;
}

static int yagl_gles1_context_get_integerv(struct yagl_gles_context *ctx,
                                           GLenum pname,
                                           GLint *params,
                                           uint32_t *num_params)
{
    int processed = 1;
    struct yagl_gles1_context *gles1_ctx = (struct yagl_gles1_context*)ctx;

    switch (pname) {
    case GL_COMPRESSED_TEXTURE_FORMATS:
        yagl_gles1_compressed_texture_formats_fill(params);
        *num_params = YAGL_GLES1_NUM_COMP_TEX_FORMATS;
        break;
    case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
        *params = YAGL_GLES1_NUM_COMP_TEX_FORMATS;
        *num_params = 1;
        break;
    case GL_MAX_CLIP_PLANES:
        *params = gles1_ctx->max_clip_planes;
        *num_params = 1;
        break;
    case GL_MAX_LIGHTS:
        *params = gles1_ctx->max_lights;
        *num_params = 1;
        break;
    case GL_MAX_TEXTURE_SIZE:
        *params = gles1_ctx->max_tex_size;
        *num_params = 1;
        break;
    case GL_VERTEX_ARRAY:
    case GL_NORMAL_ARRAY:
    case GL_COLOR_ARRAY:
    case GL_TEXTURE_COORD_ARRAY:
    case GL_POINT_SIZE_ARRAY_OES:
        *params = ctx->vao->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, pname)].enabled;
        *num_params = 1;
        break;
    case GL_VERTEX_ARRAY_BUFFER_BINDING:
    case GL_COLOR_ARRAY_BUFFER_BINDING:
    case GL_NORMAL_ARRAY_BUFFER_BINDING:
    case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
    case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
        *params = ctx->vao->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, pname)].vbo ?
                  ctx->vao->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, pname)].vbo->base.local_name
                  : 0;
        *num_params = 1;
        break;
    case GL_VERTEX_ARRAY_STRIDE:
    case GL_COLOR_ARRAY_STRIDE:
    case GL_NORMAL_ARRAY_STRIDE:
    case GL_TEXTURE_COORD_ARRAY_STRIDE:
    case GL_POINT_SIZE_ARRAY_STRIDE_OES:
        *params = ctx->vao->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, pname)].stride;
        *num_params = 1;
        break;
    case GL_VERTEX_ARRAY_TYPE:
    case GL_COLOR_ARRAY_TYPE:
    case GL_NORMAL_ARRAY_TYPE:
    case GL_TEXTURE_COORD_ARRAY_TYPE:
    case GL_POINT_SIZE_ARRAY_TYPE_OES:
        *params = ctx->vao->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, pname)].type;
        *num_params = 1;
        break;
    case GL_VERTEX_ARRAY_SIZE:
    case GL_COLOR_ARRAY_SIZE:
    case GL_TEXTURE_COORD_ARRAY_SIZE:
        *params = ctx->vao->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, pname)].size;
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
    case GL_ALPHA_TEST:
        *num_params = 1;
        break;
    case GL_ALPHA_TEST_FUNC:
        *num_params = 1;
        break;
    case GL_BLEND_DST:
        *num_params = 1;
        break;
    case GL_BLEND_SRC:
        *num_params = 1;
        break;
    case GL_CLIENT_ACTIVE_TEXTURE:
        *num_params = 1;
        break;
    case GL_COLOR_LOGIC_OP:
        *num_params = 1;
        break;
    case GL_COLOR_MATERIAL:
        *num_params = 1;
        break;
    case GL_FOG:
        *num_params = 1;
        break;
    case GL_FOG_HINT:
        *num_params = 1;
        break;
    case GL_FOG_MODE:
        *num_params = 1;
        break;
    case GL_LIGHTING:
        *num_params = 1;
        break;
    case GL_LIGHT_MODEL_TWO_SIDE:
        *num_params = 1;
        break;
    case GL_LINE_SMOOTH:
        *num_params = 1;
        break;
    case GL_LINE_SMOOTH_HINT:
        *num_params = 1;
        break;
    case GL_LOGIC_OP_MODE:
        *num_params = 1;
        break;
    case GL_MATRIX_MODE:
        *num_params = 1;
        break;
    case GL_MAX_MODELVIEW_STACK_DEPTH:
        *num_params = 1;
        break;
    case GL_MAX_PROJECTION_STACK_DEPTH:
        *num_params = 1;
        break;
    case GL_MAX_TEXTURE_STACK_DEPTH:
        *num_params = 1;
        break;
    case GL_MAX_TEXTURE_UNITS:
        *num_params = 1;
        break;
    case GL_MODELVIEW_STACK_DEPTH:
        *num_params = 1;
        break;
    case GL_MULTISAMPLE:
        *num_params = 1;
        break;
    case GL_NORMALIZE:
        *num_params = 1;
        break;
    case GL_PERSPECTIVE_CORRECTION_HINT:
        *num_params = 1;
        break;
    case GL_POINT_SMOOTH:
        *num_params = 1;
        break;
    case GL_POINT_SMOOTH_HINT:
        *num_params = 1;
        break;
    case GL_POINT_SPRITE_OES:
        *num_params = 1;
        break;
    case GL_PROJECTION_STACK_DEPTH:
        *num_params = 1;
        break;
    case GL_RESCALE_NORMAL:
        *num_params = 1;
        break;
    case GL_SAMPLE_ALPHA_TO_ONE:
        *num_params = 1;
        break;
    case GL_SHADE_MODEL:
        *num_params = 1;
        break;
    case GL_TEXTURE_2D:
        *num_params = 1;
        break;
    case GL_TEXTURE_STACK_DEPTH:
        *num_params = 1;
        break;
    /* GL_OES_matrix_palette */
    case GL_MAX_PALETTE_MATRICES_OES:
    case GL_MAX_VERTEX_UNITS_OES:
    case GL_CURRENT_PALETTE_MATRIX_OES:
    case GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES:
    case GL_MATRIX_INDEX_ARRAY_SIZE_OES:
    case GL_MATRIX_INDEX_ARRAY_STRIDE_OES:
    case GL_MATRIX_INDEX_ARRAY_TYPE_OES:
    case GL_WEIGHT_ARRAY_BUFFER_BINDING_OES:
    case GL_WEIGHT_ARRAY_SIZE_OES:
    case GL_WEIGHT_ARRAY_STRIDE_OES:
    case GL_WEIGHT_ARRAY_TYPE_OES:
        if (!gles1_ctx->matrix_palette) {
            return 0;
        }
        *num_params = 1;
        break;
    default:
        if ((pname >= GL_CLIP_PLANE0 &&
             pname <= (GL_CLIP_PLANE0 + gles1_ctx->max_clip_planes - 1)) ||
            (pname >= GL_LIGHT0 &&
             pname <= (GL_LIGHT0 + gles1_ctx->max_lights - 1))) {
            *num_params = 1;
            break;
        }
        return 0;
    }

    yagl_host_glGetIntegerv(pname, params, *num_params, NULL);

    return 1;
}

static int yagl_gles1_context_get_floatv(struct yagl_gles_context *ctx,
                                         GLenum pname,
                                         GLfloat *params,
                                         uint32_t *num_params,
                                         int *needs_map)
{
    switch (pname) {
    case GL_ALPHA_TEST_REF:
        *num_params = 1;
        *needs_map = 1;
        break;
    case GL_CURRENT_COLOR:
        *num_params = 4;
        *needs_map = 1;
        break;
    case GL_CURRENT_NORMAL:
        *num_params = 3;
        *needs_map = 1;
        break;
    case GL_CURRENT_TEXTURE_COORDS:
        *num_params = 4;
        break;
    case GL_FOG_COLOR:
        *num_params = 4;
        *needs_map = 1;
        break;
    case GL_FOG_DENSITY:
        *num_params = 1;
        break;
    case GL_FOG_END:
        *num_params = 1;
        break;
    case GL_FOG_START:
        *num_params = 1;
        break;
    case GL_LIGHT_MODEL_AMBIENT:
        *num_params = 4;
        *needs_map = 1;
        break;
    case GL_MODELVIEW_MATRIX:
        *num_params = 16;
        break;
    case GL_POINT_DISTANCE_ATTENUATION:
        *num_params = 3;
        break;
    case GL_POINT_FADE_THRESHOLD_SIZE:
        *num_params = 1;
        break;
    case GL_POINT_SIZE:
        *num_params = 1;
        break;
    case GL_POINT_SIZE_MAX:
        *num_params = 1;
        break;
    case GL_POINT_SIZE_MIN:
        *num_params = 1;
        break;
    case GL_PROJECTION_MATRIX:
        *num_params = 16;
        break;
    case GL_SMOOTH_LINE_WIDTH_RANGE:
        *num_params = 2;
        break;
    case GL_SMOOTH_POINT_SIZE_RANGE:
        *num_params = 2;
        break;
    case GL_TEXTURE_MATRIX:
        *num_params = 16;
        break;
    default:
        return 0;
    }

    yagl_host_glGetFloatv(pname, params, *num_params, NULL);

    return 1;
}

static __inline GLfloat yagl_gles1_pointsize_to_float(struct yagl_gles_array *array,
                                                      const void *psize)
{
    switch (array->type) {
    case GL_FIXED:
        return yagl_fixed_to_float(*(GLfixed*)psize);
        break;
    case GL_FLOAT:
        return *(GLfloat*)psize;
        break;
    default:
        fprintf(stderr, "Critical error! Bad pointsize type!\n");
        exit(1);
    }
}

static void yagl_gles1_draw_arrays_psize(struct yagl_gles_context *ctx,
                                         GLint first,
                                         GLsizei count)
{
    struct yagl_gles_array *parray = &ctx->vao->arrays[yagl_gles1_array_pointsize];
    unsigned i = 0;
    unsigned stride = parray->stride;
    GLsizei points_cnt;
    GLint arr_offset;
    const void *next_psize_p;
    GLfloat cur_psize;

    if (parray->vbo) {
        next_psize_p = parray->vbo->data + parray->offset + first * stride;
    } else {
        next_psize_p = parray->ptr + first * stride;
    }

    while (i < count) {
        points_cnt = 0;
        arr_offset = i;
        cur_psize = yagl_gles1_pointsize_to_float(parray, next_psize_p);

        do {
            ++points_cnt;
            ++i;
            next_psize_p += stride;
        } while ((i < count) && (cur_psize == yagl_gles1_pointsize_to_float(parray, next_psize_p)));

        yagl_host_glPointSize(cur_psize);

        yagl_host_glDrawArrays(GL_POINTS, first + arr_offset, points_cnt);
    }
}

static inline const void *yagl_get_next_psize_p(struct yagl_gles_buffer *ebo,
                                                struct yagl_gles_array *parray,
                                                GLenum type,
                                                unsigned idx,
                                                const GLvoid *indices,
                                                int32_t indices_count)
{
    unsigned idx_val;

    if (ebo) {
        if (type == GL_UNSIGNED_SHORT) {
            idx_val = ((uint16_t *)(ebo->data + indices_count))[idx];
        } else {
            idx_val = ((uint8_t *)(ebo->data + indices_count))[idx];
        }
    } else {
        if (type == GL_UNSIGNED_SHORT) {
            idx_val = ((uint16_t *)indices)[idx];
        } else {
            idx_val = ((uint8_t *)indices)[idx];
        }
    }

    if (parray->vbo) {
        return parray->vbo->data + parray->offset + idx_val * parray->stride;
    } else {
        return parray->ptr + idx_val * parray->stride;
    }
}

static void yagl_gles1_draw_elem_psize(struct yagl_gles_context *ctx,
                                       GLsizei count,
                                       GLenum type,
                                       const GLvoid *indices,
                                       int32_t indices_count)
{
    struct yagl_gles_array *parray = &ctx->vao->arrays[yagl_gles1_array_pointsize];
    unsigned i = 0, el_size;
    GLsizei points_cnt;
    GLint arr_offset;
    GLfloat cur_psize;
    const void *next_psize_p;

    switch (type) {
    case GL_UNSIGNED_BYTE:
        el_size = 1;
        break;
    case GL_UNSIGNED_SHORT:
        el_size = 2;
        break;
    default:
        el_size = 0;
        break;
    }

    assert(el_size > 0);

    next_psize_p = yagl_get_next_psize_p(ctx->vao->ebo, parray, type, i, indices, indices_count);

    while (i < count) {
        points_cnt = 0;
        arr_offset = i;
        cur_psize = yagl_gles1_pointsize_to_float(parray, next_psize_p);

        do {
            ++points_cnt;
            ++i;
            next_psize_p = yagl_get_next_psize_p(ctx->vao->ebo,
                                                 parray,
                                                 type,
                                                 i,
                                                 indices,
                                                 indices_count);
        } while ((i < count) && (cur_psize == yagl_gles1_pointsize_to_float(parray, next_psize_p)));

        yagl_host_glPointSize(cur_psize);

        if (ctx->vao->ebo) {
            yagl_host_glDrawElements(GL_POINTS,
                                     points_cnt,
                                     type,
                                     NULL,
                                     indices_count + arr_offset * el_size);
        } else {
            yagl_host_glDrawElements(GL_POINTS,
                                     points_cnt,
                                     type,
                                     indices + arr_offset * el_size,
                                     points_cnt * el_size);
        }
    }
}

static void yagl_gles1_context_draw_arrays(struct yagl_gles_context *ctx,
                                           GLenum mode,
                                           GLint first,
                                           GLsizei count,
                                           GLsizei primcount)
{
    assert(primcount < 0);

    if (!ctx->vao->arrays[yagl_gles1_array_vertex].enabled) {
        return;
    }

    if ((mode == GL_POINTS) && ctx->vao->arrays[yagl_gles1_array_pointsize].enabled) {
        yagl_gles1_draw_arrays_psize(ctx, first, count);
    } else {
        yagl_host_glDrawArrays(mode, first, count);
    }
}

static void yagl_gles1_context_draw_elements(struct yagl_gles_context *ctx,
                                             GLenum mode,
                                             GLsizei count,
                                             GLenum type,
                                             const GLvoid *indices,
                                             int32_t indices_count,
                                             GLsizei primcount,
                                             uint32_t max_idx)
{
    assert(primcount < 0);

    if (!ctx->vao->arrays[yagl_gles1_array_vertex].enabled) {
        return;
    }

    if ((mode == GL_POINTS) && ctx->vao->arrays[yagl_gles1_array_pointsize].enabled) {
        yagl_gles1_draw_elem_psize(ctx, count, type, indices, indices_count);
    } else {
        yagl_host_glDrawElements(mode, count, type, indices, indices_count);
    }
}

static int yagl_gles1_context_bind_buffer(struct yagl_gles_context *ctx,
                                          GLenum target,
                                          struct yagl_gles_buffer *buffer)
{
    return 0;
}

static void yagl_gles1_context_unbind_buffer(struct yagl_gles_context *ctx,
                                             yagl_object_name buffer_local_name)
{
}

static int yagl_gles1_context_acquire_binded_buffer(struct yagl_gles_context *ctx,
                                                    GLenum target,
                                                    struct yagl_gles_buffer **buffer)
{
    return 0;
}

static int yagl_gles1_context_validate_texture_target(struct yagl_gles_context *ctx,
                                                      GLenum target,
                                                      yagl_gles_texture_target *texture_target)
{
    return 0;
}

static struct yagl_pixel_format
    *yagl_gles1_context_validate_teximage_format(struct yagl_gles_context *ctx,
                                                 GLenum internalformat,
                                                 GLenum format,
                                                 GLenum type)
{
    return NULL;
}

static struct yagl_pixel_format
    *yagl_gles1_context_validate_getteximage_format(struct yagl_gles_context *ctx,
                                                    GLenum readbuffer_internalformat,
                                                    GLenum format,
                                                    GLenum type)
{
    return NULL;
}

static int yagl_gles1_context_validate_copyteximage_format(struct yagl_gles_context *ctx,
                                                           GLenum readbuffer_internalformat,
                                                           GLenum *internalformat)
{
    return 0;
}

static int yagl_gles1_context_validate_texstorage_format(struct yagl_gles_context *ctx,
                                                         GLenum *internalformat,
                                                         GLenum *base_internalformat,
                                                         GLenum *any_format,
                                                         GLenum *any_type)
{
    YaglGles1PalFmtDesc fmt_desc;

    switch (*internalformat) {
    case GL_PALETTE4_RGB8_OES ... GL_PALETTE8_RGB5_A1_OES:
        yagl_gles1_cpal_format_get_descr(*internalformat, &fmt_desc);
        *internalformat = fmt_desc.uncomp_format;
        *base_internalformat = fmt_desc.uncomp_format;
        *any_format = fmt_desc.uncomp_format;
        *any_type = fmt_desc.pixel_type;
        break;
    default:
        return 0;
    }

    return 1;
}

static int yagl_gles1_context_validate_renderbuffer_format(struct yagl_gles_context *ctx,
                                                           GLenum *internalformat)
{
    return 0;
}

static void yagl_gles1_context_hint(struct yagl_gles_context *ctx,
                                    GLenum target,
                                    GLenum mode)
{
}

struct yagl_client_context *yagl_gles1_context_create(struct yagl_sharegroup *sg)
{
    struct yagl_gles1_context *gles1_ctx;

    YAGL_LOG_FUNC_ENTER(yagl_gles1_context_create, NULL);

    gles1_ctx = yagl_malloc0(sizeof(*gles1_ctx));

    yagl_gles_context_init(&gles1_ctx->base, yagl_client_api_gles1, sg);

    gles1_ctx->sg = sg;

    gles1_ctx->base.base.prepare = &yagl_gles1_context_prepare;
    gles1_ctx->base.base.destroy = &yagl_gles1_context_destroy;
    gles1_ctx->base.create_arrays = &yagl_gles1_context_create_arrays;
    gles1_ctx->base.get_string = &yagl_gles1_context_get_string;
    gles1_ctx->base.compressed_tex_image_2d = &yagl_gles1_context_compressed_tex_image_2d;
    gles1_ctx->base.compressed_tex_sub_image_2d = &yagl_gles1_context_compressed_tex_sub_image_2d;
    gles1_ctx->base.enable = &yagl_gles1_context_enable;
    gles1_ctx->base.is_enabled = &yagl_gles1_context_is_enabled;
    gles1_ctx->base.get_integerv = &yagl_gles1_context_get_integerv;
    gles1_ctx->base.get_floatv = &yagl_gles1_context_get_floatv;
    gles1_ctx->base.draw_arrays = &yagl_gles1_context_draw_arrays;
    gles1_ctx->base.draw_elements = &yagl_gles1_context_draw_elements;
    gles1_ctx->base.bind_buffer = &yagl_gles1_context_bind_buffer;
    gles1_ctx->base.unbind_buffer = &yagl_gles1_context_unbind_buffer;
    gles1_ctx->base.acquire_binded_buffer = &yagl_gles1_context_acquire_binded_buffer;
    gles1_ctx->base.validate_texture_target = &yagl_gles1_context_validate_texture_target;
    gles1_ctx->base.validate_teximage_format = &yagl_gles1_context_validate_teximage_format;
    gles1_ctx->base.validate_getteximage_format = &yagl_gles1_context_validate_getteximage_format;
    gles1_ctx->base.validate_copyteximage_format = &yagl_gles1_context_validate_copyteximage_format;
    gles1_ctx->base.validate_texstorage_format = &yagl_gles1_context_validate_texstorage_format;
    gles1_ctx->base.validate_renderbuffer_format = &yagl_gles1_context_validate_renderbuffer_format;
    gles1_ctx->base.hint = &yagl_gles1_context_hint;

    YAGL_LOG_FUNC_EXIT("%p", gles1_ctx);

    return &gles1_ctx->base.base;
}
