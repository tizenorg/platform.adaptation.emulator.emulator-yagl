#ifndef _YAGL_GLES_TYPES_H_
#define _YAGL_GLES_TYPES_H_

#include "yagl_types.h"

struct yagl_gles_buffer;

typedef enum
{
    yagl_gles_texture_target_2d = 0,
    yagl_gles_texture_target_2d_array = 1,
    yagl_gles_texture_target_3d = 2,
    yagl_gles_texture_target_cubemap = 3
} yagl_gles_texture_target;

typedef enum
{
    yagl_gles_framebuffer_attachment_depth = 0,
    yagl_gles_framebuffer_attachment_stencil = 1,
    yagl_gles_framebuffer_attachment_color0 = 2
} yagl_gles_framebuffer_attachment;

#define YAGL_MAX_GLES_FRAMEBUFFER_COLOR_ATTACHMENTS 16

typedef enum
{
    yagl_gles_format_color_renderable = (1 << 0),
    yagl_gles_format_depth_renderable = (1 << 1),
    yagl_gles_format_stencil_renderable = (1 << 2),
    yagl_gles_format_sized = (1 << 3),
    yagl_gles_format_signed_integer = (1 << 4),
    yagl_gles_format_unsigned_integer = (1 << 5),
    yagl_gles_format_float = (1 << 6),
    yagl_gles_format_srgb = (1 << 7)
} yagl_gles_format_flag;

struct yagl_gles_pixelstore
{
    GLint alignment;
    GLint row_length;
    GLint image_height;
    GLint skip_pixels;
    GLint skip_rows;
    GLint skip_images;
    struct yagl_gles_buffer *pbo;
};

struct yagl_gles_format_info
{
    uint32_t flags;
    uint32_t num_components;
    uint32_t red_size;
    uint32_t green_size;
    uint32_t blue_size;
    uint32_t alpha_size;
    uint32_t depth_size;
    uint32_t stencil_size;
};

#endif
