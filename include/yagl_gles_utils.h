#ifndef _YAGL_GLES_UTILS_H_
#define _YAGL_GLES_UTILS_H_

#include "yagl_export.h"
#include "yagl_types.h"

YAGL_API int yagl_gles_get_stride(GLsizei alignment,
                                  GLsizei width,
                                  GLenum format,
                                  GLenum type,
                                  GLsizei *stride);

#endif
