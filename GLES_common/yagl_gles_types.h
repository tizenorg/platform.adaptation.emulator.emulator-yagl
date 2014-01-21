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

#define YAGL_NUM_GLES_TEXTURE_TARGETS 4

typedef enum
{
    yagl_gles_framebuffer_attachment_depth = 0,
    yagl_gles_framebuffer_attachment_stencil = 1,
    yagl_gles_framebuffer_attachment_color0 = 2
} yagl_gles_framebuffer_attachment;

#define YAGL_MAX_GLES_FRAMEBUFFER_COLOR_ATTACHMENTS 16

#define YAGL_MAX_GLES_DRAW_BUFFERS 16

struct yagl_gles_pixelstore
{
    GLint alignment;
    struct yagl_gles_buffer *pbo;
};

#endif
