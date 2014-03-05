#ifndef _YAGL_GLES_UTILS_H_
#define _YAGL_GLES_UTILS_H_

#include "yagl_gles_types.h"

void yagl_gles_reset_unpack(const struct yagl_gles_pixelstore* ps);

void yagl_gles_set_unpack(const struct yagl_gles_pixelstore* ps);

const struct yagl_gles_format_info
    *yagl_gles_internalformat_info(GLenum internalformat);

#endif
