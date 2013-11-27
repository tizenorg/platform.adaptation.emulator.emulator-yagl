#ifndef _YAGL_GLES_VALIDATE_H_
#define _YAGL_GLES_VALIDATE_H_

#include "yagl_gles_types.h"

int yagl_gles_is_buffer_target_valid(GLenum target);

int yagl_gles_is_framebuffer_target_valid(GLenum target);

int yagl_gles_is_buffer_usage_valid(GLenum usage);

int yagl_gles_is_blend_equation_valid(GLenum mode);

int yagl_gles_is_blend_func_valid(GLenum func);

int yagl_gles_is_cull_face_mode_valid(GLenum mode);

int yagl_gles_is_depth_func_valid(GLenum func);

int yagl_gles_is_front_face_mode_valid(GLenum mode);

int yagl_gles_is_alignment_valid(GLint alignment);

int yagl_gles_get_index_size(GLenum type, int *index_size);

int yagl_gles_buffer_target_to_binding(GLenum target, GLenum *binding);

int yagl_gles_validate_texture_target(GLenum target,
    yagl_gles_texture_target *texture_target);

int yagl_gles_validate_framebuffer_attachment(GLenum attachment,
    yagl_gles_framebuffer_attachment *framebuffer_attachment);

int yagl_gles_validate_texture_target_squash(GLenum target,
    GLenum *squashed_target);

#endif