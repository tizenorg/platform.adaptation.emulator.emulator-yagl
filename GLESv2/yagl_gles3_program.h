#ifndef _YAGL_GLES3_PROGRAM_H_
#define _YAGL_GLES3_PROGRAM_H_

#include "yagl_gles2_program.h"

int yagl_gles3_program_get_active_uniformsiv(struct yagl_gles2_program *program,
                                             const GLuint *indices,
                                             int num_indices,
                                             GLenum pname,
                                             GLint *params);

#endif
