#ifndef _YAGL_GLES1_VALIDATE_H_
#define _YAGL_GLES1_VALIDATE_H_

#include "yagl_types.h"

int yagl_gles1_get_texenv_param_count(GLenum type, int *count);

int yagl_gles1_get_point_param_count(GLenum pname, int *count);

int yagl_gles1_get_fog_param_count(GLenum pname, int *count);

int yagl_gles1_get_light_param_count(GLenum pname, int *count);

int yagl_gles1_get_light_model_param_count(GLenum pname, int *count);

int yagl_gles1_get_material_param_count(GLenum pname, int *count);

#endif
