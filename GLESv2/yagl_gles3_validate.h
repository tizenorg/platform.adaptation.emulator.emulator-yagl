#ifndef _YAGL_GLES3_VALIDATE_H_
#define _YAGL_GLES3_VALIDATE_H_

#include "yagl_types.h"

int yagl_gles3_is_uniform_param_valid(GLenum pname);

int yagl_gles3_is_transform_feedback_buffer_mode_valid(GLenum buffer_mode);

int yagl_gles3_is_primitive_mode_valid(GLenum primitive_mode);

int yagl_gles3_is_buffer_valid(GLenum buffer);

#endif
