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

/*
 * GLSL ES shader can contain something like this:
 *
 * #ifdef GL_ARB_draw_instanced
 * #extension GL_ARB_draw_instanced : require
 * #endif
 *
 * On real hardware GL_ARB_draw_instanced is not defined, so the code
 * inside won't compile. In emulated environment when host GPU
 * is used GL_ARB_draw_instanced will be defined and code inside
 * will be compiled when it shouldn't.
 *
 * A workaround for this is to find all "GL_" tokens and unless
 * this is some supported OpenGL ES extension, replace it with
 * something else that's not defined in host preprocessor for sure.
 *
 * Returns NULL if no "GL_" tokens were detected.
 */
char *yagl_gles2_shader_fix_extensions(const char *source,
                                       int length,
                                       const char **extensions,
                                       int num_extensions,
                                       int *patched_len);

#endif
