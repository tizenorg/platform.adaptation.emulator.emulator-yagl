#ifndef _YAGL_GLES_UTILS_H_
#define _YAGL_GLES_UTILS_H_

#include "yagl_gles_types.h"

void yagl_gles_reset_unpack(const struct yagl_gles_pixelstore* ps);

void yagl_gles_set_unpack(const struct yagl_gles_pixelstore* ps);

uint32_t yagl_gles_internalformat_flags(GLenum internalformat);

uint32_t yagl_gles_internalformat_num_components(GLenum internalformat);

#endif
