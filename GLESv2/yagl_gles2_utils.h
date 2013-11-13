#ifndef _YAGL_GLES2_UTILS_H_
#define _YAGL_GLES2_UTILS_H_

#include "yagl_types.h"

char *yagl_gles2_shader_patch(const char *source,
                              int length,
                              int *patched_len);

int yagl_gles2_shader_has_version(const char *source,
                                  int *is_es3);

void yagl_gles2_set_name(const GLchar *from, GLint from_size,
                         GLint bufsize,
                         GLint *length,
                         GLchar *name);

#endif
