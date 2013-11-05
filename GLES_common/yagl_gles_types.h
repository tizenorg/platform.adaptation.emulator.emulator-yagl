#ifndef _YAGL_GLES_TYPES_H_
#define _YAGL_GLES_TYPES_H_

#include "yagl_types.h"

typedef enum
{
    yagl_gles_texture_target_2d = 0,
    yagl_gles_texture_target_cubemap = 1
} yagl_gles_texture_target;

#define YAGL_NUM_GLES_TEXTURE_TARGETS 2

typedef enum
{
    yagl_gles_framebuffer_attachment_depth = 0,
    yagl_gles_framebuffer_attachment_stencil = 1,
    yagl_gles_framebuffer_attachment_color0 = 2
} yagl_gles_framebuffer_attachment;

#define YAGL_MAX_GLES_FRAMEBUFFER_COLOR_ATTACHMENTS 16

#define YAGL_MAX_GLES_DRAW_BUFFERS 16

#endif
