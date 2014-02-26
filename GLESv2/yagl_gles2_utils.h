#ifndef _YAGL_GLES2_UTILS_H_
#define _YAGL_GLES2_UTILS_H_

#include "yagl_types.h"

void yagl_gles2_set_name(const GLchar *from, GLint from_size,
                         GLint bufsize,
                         GLint *length,
                         GLchar *name);

#endif
