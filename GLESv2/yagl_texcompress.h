#ifndef _YAGL_TEXCOMPRESS_H_
#define _YAGL_TEXCOMPRESS_H_

#include "yagl_types.h"

struct yagl_texcompress_format
{
    void (*unpack)(struct yagl_texcompress_format */*format*/,
                   const GLvoid */*src*/,
                   GLsizei /*width*/,
                   GLsizei /*height*/,
                   GLsizei /*src_stride*/,
                   GLvoid */*dst*/,
                   GLsizei /*dst_stride*/);

    GLenum src_format;
    GLint block_width;
    GLint block_height;
    GLint block_bytes;

    GLenum dst_format;
    GLenum dst_internalformat;
    GLenum dst_type;
};

int yagl_texcompress_get_format_names(GLenum *formats);

struct yagl_texcompress_format *yagl_texcompress_get_format(GLenum format);

int yagl_texcompress_get_info(struct yagl_texcompress_format *format,
                              GLsizei width,
                              GLsizei height,
                              GLsizei src_size,
                              GLsizei *src_stride,
                              GLsizei *dst_stride,
                              GLsizei *dst_size);

#endif
