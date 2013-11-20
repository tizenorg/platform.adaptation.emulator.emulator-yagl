#ifndef _YAGL_GLES_UTILS_H_
#define _YAGL_GLES_UTILS_H_

#include "yagl_types.h"

GLenum yagl_gles_get_actual_type(GLenum type);

GLint yagl_gles_get_actual_internalformat(GLint internalformat);

GLint yagl_gles_get_actual_format(GLint format);

const GLvoid *yagl_gles_convert_to_host(GLsizei alignment,
                                        GLsizei width,
                                        GLsizei height,
                                        GLsizei depth,
                                        GLenum format,
                                        GLenum type,
                                        const GLvoid *pixels);

GLvoid *yagl_gles_convert_from_host_start(GLsizei alignment,
                                          GLsizei width,
                                          GLsizei height,
                                          GLenum format,
                                          GLenum type,
                                          GLvoid *pixels);

void yagl_gles_convert_from_host_end(GLsizei alignment,
                                     GLsizei width,
                                     GLsizei height,
                                     GLenum format,
                                     GLenum type,
                                     GLsizei stride,
                                     const GLvoid *pixels_from,
                                     GLvoid *pixels_to);

#endif
