#include "GLES/gl.h"
#include "GLES/glext.h"
#include "yagl_gles1_context.h"
#include "yagl_gles_array.h"
#include "yagl_gles_buffer.h"
#include "yagl_gles_texture.h"
#include "yagl_gles_texture_unit.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_utils.h"
#include "yagl_host_gles_calls.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define YAGL_GLES1_NUM_COMP_TEX_FORMATS 10

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
    GLint i, num_texture_units = 0;
    struct yagl_gles_array *arrays;
    int32_t size = 0;
    char *extensions;
    int num_arrays;

    YAGL_LOG_FUNC_ENTER(yagl_gles1_context_prepare, "%p", ctx);

    yagl_host_glGetIntegerv(GL_MAX_TEXTURE_UNITS,
                            &num_texture_units, 1, NULL);

    /*
     * We limit this by 32 for conformance.
     */
    if (num_texture_units > 32) {
        num_texture_units = 32;
    }

    /* Each texture unit has its own client-side array state */
    num_arrays = yagl_gles1_array_texcoord + num_texture_units;

    arrays = yagl_malloc(num_arrays * sizeof(*arrays));

    yagl_gles_array_init(&arrays[yagl_gles1_array_vertex],
                         yagl_gles1_array_vertex,
                         &yagl_gles1_vertex_array_apply,
                         gles1_ctx);

    yagl_gles_array_init(&arrays[yagl_gles1_array_color],
                         yagl_gles1_array_color,
                         &yagl_gles1_color_array_apply,
                         gles1_ctx);

    yagl_gles_array_init(&arrays[yagl_gles1_array_normal],
                         yagl_gles1_array_normal,
                         &yagl_gles1_normal_array_apply,
                         gles1_ctx);

    yagl_gles_array_init(&arrays[yagl_gles1_array_pointsize],
                         yagl_gles1_array_pointsize,
                         &yagl_gles1_pointsize_array_apply,
                         gles1_ctx);

    for (i = yagl_gles1_array_texcoord; i < num_arrays; ++i) {
        yagl_gles_array_init(&arrays[i],
                             i,
                             &yagl_gles1_texcoord_array_apply,
                             gles1_ctx);
    }

    yagl_gles_context_prepare(&gles1_ctx->base, arrays, num_arrays,
                              num_texture_units);

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
    extensions = yagl_malloc0(size);
    yagl_host_glGetString(GL_EXTENSIONS, extensions, size, NULL);

    gles1_ctx->matrix_palette =
        (strstr(extensions, "GL_ARB_vertex_blend ") != NULL) &&
        (strstr(extensions, "GL_ARB_matrix_palette ") != NULL);

    yagl_free(extensions);

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

static GLchar *yagl_gles1_context_get_extensions(struct yagl_gles_context *ctx)
{
    struct yagl_gles1_context *gles1_ctx = (struct yagl_gles1_context*)ctx;

    const GLchar *default_ext =
        "GL_OES_blend_subtract GL_OES_blend_equation_separate "
        "GL_OES_blend_func_separate GL_OES_element_index_uint "
        "GL_OES_texture_mirrored_repeat "
        "GL_EXT_texture_format_BGRA8888 GL_OES_point_sprite "
        "GL_OES_point_size_array GL_OES_stencil_wrap "
        "GL_OES_compressed_paletted_texture "
        "GL_OES_depth_texture "
        "GL_OES_framebuffer_object GL_OES_depth24 GL_OES_depth32 "
        "GL_OES_rgb8_rgba8 GL_OES_stencil1 GL_OES_stencil4 "
        "GL_OES_stencil8 GL_OES_EGL_image ";
    const GLchar *packed_depth_stencil = "GL_OES_packed_depth_stencil ";
    const GLchar *texture_npot = "GL_OES_texture_npot ";
    const GLchar *texture_filter_anisotropic = "GL_EXT_texture_filter_anisotropic ";
    const GLchar *matrix_palette = "GL_OES_matrix_palette ";

    size_t len = strlen(default_ext);
    GLchar *str;

    if (gles1_ctx->base.texture_npot) {
        len += strlen(texture_npot);
    }

    if (gles1_ctx->base.texture_filter_anisotropic) {
        len += strlen(texture_filter_anisotropic);
    }

    if (gles1_ctx->base.packed_depth_stencil) {
        len += strlen(packed_depth_stencil);
    }

    if (gles1_ctx->matrix_palette) {
        len += strlen(matrix_palette);
    }

    str = yagl_malloc0(len + 1);

    strcpy(str, default_ext);

    if (gles1_ctx->base.texture_npot) {
        strcat(str, texture_npot);
    }

    if (gles1_ctx->base.texture_filter_anisotropic) {
        strcat(str, texture_filter_anisotropic);
    }

    if (gles1_ctx->base.packed_depth_stencil) {
        strcat(str, packed_depth_stencil);
    }

    if (gles1_ctx->matrix_palette) {
        strcat(str, matrix_palette);
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
    GLint saved_alignment;

    if (!data) {
        for (cur_level = 0; cur_level <= max_level; ++cur_level) {
            yagl_host_glTexImage2D(GL_TEXTURE_2D,
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

        return;
    }

    /* Jump over palette data to first image data */
    indices = data + (1 << fmt_desc->bits_per_index) * fmt_desc->pixel_size;

    /* 0 level image is the largest */
    tex_img_data = yagl_malloc(num_of_texels * fmt_desc->pixel_size);

    /* We will pass tightly packed data to glTexImage2D */
    saved_alignment = ctx->unpack_alignment;

    if (saved_alignment != 1) {
        yagl_host_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

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

        yagl_host_glTexImage2D(GL_TEXTURE_2D,
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

    if (saved_alignment != 1) {
        yagl_host_glPixelStorei(GL_UNPACK_ALIGNMENT, saved_alignment);
    }
}

static GLenum yagl_gles1_context_compressed_tex_image(struct yagl_gles_context *ctx,
                                                      GLenum target,
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

    if (target != GL_TEXTURE_2D) {
        return GL_INVALID_ENUM;
    }

    switch (internalformat) {
    case GL_PALETTE4_RGB8_OES ... GL_PALETTE8_RGB5_A1_OES:
        yagl_gles1_cpal_format_get_descr(internalformat, &fmt_desc);

        if ((level > 0) || (-level > yagl_log2(max_tex_size)) ||
            !yagl_gles1_tex_dims_valid(width, height, max_tex_size) ||
            border != 0 || (imageSize !=
                yagl_gles1_cpal_tex_size(&fmt_desc, width, height, -level))) {
            return GL_INVALID_VALUE;
        }

        yagl_gles1_cpal_tex_uncomp_and_apply(ctx,
                                             &fmt_desc,
                                             -level,
                                             width,
                                             height,
                                             data);
        break;
    default:
        return GL_INVALID_ENUM;
    }

    return GL_NO_ERROR;
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
        *enabled = ctx->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, cap)].enabled;
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
        *params = ctx->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, pname)].enabled;
        *num_params = 1;
        break;
    case GL_VERTEX_ARRAY_BUFFER_BINDING:
    case GL_COLOR_ARRAY_BUFFER_BINDING:
    case GL_NORMAL_ARRAY_BUFFER_BINDING:
    case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
    case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
        *params = ctx->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, pname)].vbo ?
                  ctx->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, pname)].vbo->base.local_name
                  : 0;
        *num_params = 1;
        break;
    case GL_VERTEX_ARRAY_STRIDE:
    case GL_COLOR_ARRAY_STRIDE:
    case GL_NORMAL_ARRAY_STRIDE:
    case GL_TEXTURE_COORD_ARRAY_STRIDE:
    case GL_POINT_SIZE_ARRAY_STRIDE_OES:
        *params = ctx->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, pname)].stride;
        *num_params = 1;
        break;
    case GL_VERTEX_ARRAY_TYPE:
    case GL_COLOR_ARRAY_TYPE:
    case GL_NORMAL_ARRAY_TYPE:
    case GL_TEXTURE_COORD_ARRAY_TYPE:
    case GL_POINT_SIZE_ARRAY_TYPE_OES:
        *params = ctx->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, pname)].type;
        *num_params = 1;
        break;
    case GL_VERTEX_ARRAY_SIZE:
    case GL_COLOR_ARRAY_SIZE:
    case GL_TEXTURE_COORD_ARRAY_SIZE:
        *params = ctx->arrays[yagl_gles1_array_idx_from_pname(gles1_ctx, pname)].size;
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
    struct yagl_gles_array *parray = &ctx->arrays[yagl_gles1_array_pointsize];
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
    struct yagl_gles_array *parray = &ctx->arrays[yagl_gles1_array_pointsize];
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

    next_psize_p = yagl_get_next_psize_p(ctx->ebo, parray, type, i, indices, indices_count);

    while (i < count) {
        points_cnt = 0;
        arr_offset = i;
        cur_psize = yagl_gles1_pointsize_to_float(parray, next_psize_p);

        do {
            ++points_cnt;
            ++i;
            next_psize_p = yagl_get_next_psize_p(ctx->ebo,
                                                 parray,
                                                 type,
                                                 i,
                                                 indices,
                                                 indices_count);
        } while ((i < count) && (cur_psize == yagl_gles1_pointsize_to_float(parray, next_psize_p)));

        yagl_host_glPointSize(cur_psize);

        if (ctx->ebo) {
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
                                           GLsizei count)
{
    if (!ctx->arrays[yagl_gles1_array_vertex].enabled) {
        return;
    }

    if ((mode == GL_POINTS) && ctx->arrays[yagl_gles1_array_pointsize].enabled) {
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
                                             int32_t indices_count)
{
    if (!ctx->arrays[yagl_gles1_array_vertex].enabled) {
        return;
    }

    if ((mode == GL_POINTS) && ctx->arrays[yagl_gles1_array_pointsize].enabled) {
        yagl_gles1_draw_elem_psize(ctx, count, type, indices, indices_count);
    } else {
        yagl_host_glDrawElements(mode, count, type, indices, indices_count);
    }
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
    gles1_ctx->base.get_string = &yagl_gles1_context_get_string;
    gles1_ctx->base.get_extensions = &yagl_gles1_context_get_extensions;
    gles1_ctx->base.compressed_tex_image = &yagl_gles1_context_compressed_tex_image;
    gles1_ctx->base.enable = &yagl_gles1_context_enable;
    gles1_ctx->base.is_enabled = &yagl_gles1_context_is_enabled;
    gles1_ctx->base.get_integerv = &yagl_gles1_context_get_integerv;
    gles1_ctx->base.get_floatv = &yagl_gles1_context_get_floatv;
    gles1_ctx->base.draw_arrays = &yagl_gles1_context_draw_arrays;
    gles1_ctx->base.draw_elements = &yagl_gles1_context_draw_elements;

    YAGL_LOG_FUNC_EXIT("%p", gles1_ctx);

    return &gles1_ctx->base.base;
}
