#ifndef _YAGL_GLES2_UTILS_H_
#define _YAGL_GLES2_UTILS_H_

#include "yagl_types.h"

char *yagl_gles2_shader_patch(const char *source,
                              int length,
                              int *patched_len);

int yagl_gles2_shader_has_version(const char *source,
                                  int *is_es3);

#endif
