#ifndef _YAGL_PIXEL_FORMAT_H_
#define _YAGL_PIXEL_FORMAT_H_

#include "yagl_gles_types.h"

#define YAGL_PIXEL_FORMAT_DECL(prefix, internalformat, format, type) \
extern struct yagl_pixel_format yagl_##prefix##_pixel_format_##internalformat##_##format##_##type

#define YAGL_PIXEL_FORMAT_IMPL_NOCONV(prefix, internalformat, format, type, dstinternalformat, dsttype, bpp) \
struct yagl_pixel_format yagl_##prefix##_pixel_format_##internalformat##_##format##_##type = \
{ \
    .need_convert = 0, \
    .src_internalformat = internalformat, \
    .src_format = format, \
    .src_type = type, \
    .src_bpp = bpp, \
    .dst_internalformat = dstinternalformat, \
    .dst_format = format, \
    .dst_type = dsttype, \
    .dst_bpp = bpp, \
    .unpack = NULL, \
    .pack = NULL \
}

#define YAGL_PIXEL_FORMAT_IMPL_BEGIN(prefix, internalformat, format, type) \
struct yagl_pixel_format yagl_##prefix##_pixel_format_##internalformat##_##format##_##type = \
{ \
    .need_convert = 1, \
    .src_internalformat = internalformat, \
    .src_format = format, \
    .src_type = type,

#define YAGL_PIXEL_FORMAT_IMPL_END() };

#define YAGL_PIXEL_FORMAT_CASE(prefix, internalformat, format, type) \
case type: \
    return &yagl_##prefix##_pixel_format_##internalformat##_##format##_##type

struct yagl_pixel_format;

typedef void (*yagl_pixel_format_converter)(const GLvoid */*src*/,
                                            GLsizei /*src_stride*/,
                                            GLvoid */*dst*/,
                                            GLsizei /*dst_stride*/,
                                            GLsizei /*width*/,
                                            GLsizei /*height*/);

struct yagl_pixel_format
{
    int need_convert;

    GLenum src_internalformat;
    GLenum src_format;
    GLenum src_type;
    GLenum src_bpp;

    GLenum dst_internalformat;
    GLenum dst_format;
    GLenum dst_type;
    GLenum dst_bpp;

    /*
     * NULL if no conversion is required.
     */
    yagl_pixel_format_converter unpack;
    yagl_pixel_format_converter pack;
};

/*
 * Returns offset that needs to be applied to 'pixels' in
 * order to get correct data location. It can be non-0 when
 * GL_UNPACK_SKIP_XXX or GL_PACK_SKIP_XXX are non-0.
 * 'size' is unpacked size of the image.
 */
GLsizei yagl_pixel_format_get_info(struct yagl_pixel_format *pf,
                                   const struct yagl_gles_pixelstore *ps,
                                   GLsizei width,
                                   GLsizei height,
                                   GLsizei depth,
                                   GLsizei *size);

const GLvoid *yagl_pixel_format_unpack(struct yagl_pixel_format *pf,
                                       const struct yagl_gles_pixelstore *ps,
                                       GLsizei width,
                                       GLsizei height,
                                       GLsizei depth,
                                       const GLvoid *pixels);

GLvoid *yagl_pixel_format_pack_alloc(struct yagl_pixel_format *pf,
                                     const struct yagl_gles_pixelstore *ps,
                                     GLsizei width,
                                     GLsizei height,
                                     GLvoid *pixels);

void yagl_pixel_format_pack(struct yagl_pixel_format *pf,
                            const struct yagl_gles_pixelstore *ps,
                            GLsizei width,
                            GLsizei height,
                            const GLvoid *pixels_src,
                            GLvoid *pixels_dst);

#endif
