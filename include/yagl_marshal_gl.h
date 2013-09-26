#ifndef _YAGL_MARSHAL_GL_H_
#define _YAGL_MARSHAL_GL_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_marshal.h"

#define yagl_marshal_put_GLboolean(buff, value) yagl_marshal_put_uint8((buff), (value))
#define yagl_marshal_get_GLboolean(buff) yagl_marshal_get_uint8(buff)
#define yagl_marshal_put_GLenum(buff, value) yagl_marshal_put_uint32((buff), (value))
#define yagl_marshal_get_GLenum(buff) yagl_marshal_get_uint32(buff)
#define yagl_marshal_put_GLuint(buff, value) yagl_marshal_put_uint32((buff), (value))
#define yagl_marshal_get_GLuint(buff) yagl_marshal_get_uint32(buff)
#define yagl_marshal_put_GLbitfield(buff, value) yagl_marshal_put_uint32((buff), (value))
#define yagl_marshal_get_GLbitfield(buff) yagl_marshal_get_uint32(buff)
#define yagl_marshal_put_GLclampf(buff, value) yagl_marshal_put_float((buff), (value))
#define yagl_marshal_get_GLclampf(buff) yagl_marshal_get_float(buff)
#define yagl_marshal_put_GLfloat(buff, value) yagl_marshal_put_float((buff), (value))
#define yagl_marshal_get_GLfloat(buff) yagl_marshal_get_float(buff)
#define yagl_marshal_put_GLint(buff, value) yagl_marshal_put_int32((buff), (value))
#define yagl_marshal_get_GLint(buff) yagl_marshal_get_int32(buff)
#define yagl_marshal_put_GLsizei(buff, value) yagl_marshal_put_int32((buff), (value))
#define yagl_marshal_get_GLsizei(buff) yagl_marshal_get_int32(buff)
#define yagl_marshal_put_GLclampx(buff, value) yagl_marshal_put_int32((buff), (value))
#define yagl_marshal_get_GLclampx(buff) yagl_marshal_get_int32(buff)
#define yagl_marshal_put_GLfixed(buff, value) yagl_marshal_put_int32((buff), (value))
#define yagl_marshal_get_GLfixed(buff) yagl_marshal_get_int32(buff)
#define yagl_marshal_put_GLubyte(buff, value) yagl_marshal_put_uint8((buff), (value))
#define yagl_marshal_get_GLubyte(buff) yagl_marshal_get_uint8(buff)

/*
 * GLintptr and GLsizeiptr are used for pointer differences/buffer sizes,
 * cast them to int32_t since we're sure that even on x64 targets
 * we won't be passing data that big.
 */
#define yagl_marshal_put_GLintptr(buff, value) yagl_marshal_put_int32((buff), (int32_t)(value))
#define yagl_marshal_get_GLintptr(buff) (GLintptr)yagl_marshal_get_int32(buff)
#define yagl_marshal_put_GLsizeiptr(buff, value) yagl_marshal_put_int32((buff), (int32_t)(value))
#define yagl_marshal_get_GLsizeiptr(buff) (GLsizeiptr)yagl_marshal_get_int32(buff)

#endif
