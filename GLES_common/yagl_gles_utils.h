#ifndef _YAGL_GLES_UTILS_H_
#define _YAGL_GLES_UTILS_H_

#include "yagl_gles_types.h"

GLsizei yagl_gles_get_stride(const struct yagl_gles_pixelstore* ps,
                             GLsizei width,
                             GLsizei height,
                             GLsizei bpp,
                             GLsizei *image_stride);

GLsizei yagl_gles_get_offset(const struct yagl_gles_pixelstore* ps,
                             GLsizei width,
                             GLsizei height,
                             GLsizei depth,
                             GLsizei bpp,
                             GLsizei *size);

void yagl_gles_reset_unpack(const struct yagl_gles_pixelstore* ps);

void yagl_gles_set_unpack(const struct yagl_gles_pixelstore* ps);

GLenum yagl_gles_get_actual_type(GLenum type);

GLint yagl_gles_get_actual_internalformat(GLint internalformat);

GLint yagl_gles_get_actual_format(GLint format);

const GLvoid *yagl_gles_convert_to_host(const struct yagl_gles_pixelstore* ps,
                                        GLsizei width,
                                        GLsizei height,
                                        GLsizei depth,
                                        GLenum format,
                                        GLenum type,
                                        const GLvoid *pixels);

GLvoid *yagl_gles_convert_from_host_start(const struct yagl_gles_pixelstore* ps,
                                          GLsizei width,
                                          GLsizei height,
                                          GLenum format,
                                          GLenum type,
                                          GLvoid *pixels);

void yagl_gles_convert_from_host_end(const struct yagl_gles_pixelstore* ps,
                                     GLsizei width,
                                     GLsizei height,
                                     GLenum format,
                                     GLenum type,
                                     GLsizei stride,
                                     const GLvoid *pixels_from,
                                     GLvoid *pixels_to);

#endif
