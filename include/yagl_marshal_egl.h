#ifndef _YAGL_MARSHAL_EGL_H_
#define _YAGL_MARSHAL_EGL_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_marshal.h"
#include <EGL/egl.h>

#define yagl_marshal_put_EGLBoolean(buff, value) yagl_marshal_put_uint32((buff), (value))
#define yagl_marshal_get_EGLBoolean(buff) yagl_marshal_get_uint32(buff)
#define yagl_marshal_put_EGLenum(buff, value) yagl_marshal_put_uint32((buff), (value))
#define yagl_marshal_get_EGLenum(buff) yagl_marshal_get_uint32(buff)
#define yagl_marshal_put_EGLint(buff, value) yagl_marshal_put_int32((buff), (value))
#define yagl_marshal_get_EGLint(buff) yagl_marshal_get_int32(buff)

#define yagl_marshal_put_EGLClientBuffer(buff, value) yagl_marshal_skip(buff)
#define yagl_marshal_put_EGLNativePixmapType(buff, value) yagl_marshal_skip(buff)
#define yagl_marshal_put_EGLNativeWindowType(buff, value) yagl_marshal_skip(buff)

#endif
