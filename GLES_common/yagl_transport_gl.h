#ifndef _YAGL_TRANSPORT_GL_H_
#define _YAGL_TRANSPORT_GL_H_

#include "yagl_transport.h"

static __inline void yagl_transport_put_out_GLenum(struct yagl_transport *t,
                                                   GLenum value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_GLuint(struct yagl_transport *t,
                                                   GLuint value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_GLint(struct yagl_transport *t,
                                                   GLint value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_GLbitfield(struct yagl_transport *t,
                                                       GLbitfield value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_GLboolean(struct yagl_transport *t,
                                                      GLboolean value)
{
    yagl_transport_put_out_uint8_t(t, value);
}

static __inline void yagl_transport_put_out_GLubyte(struct yagl_transport *t,
                                                    GLubyte value)
{
    yagl_transport_put_out_uint8_t(t, value);
}

static __inline void yagl_transport_put_out_GLsizei(struct yagl_transport *t,
                                                    GLsizei value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_GLclampf(struct yagl_transport *t,
                                                     GLclampf value)
{
    yagl_transport_put_out_float(t, value);
}

static __inline void yagl_transport_put_out_GLfloat(struct yagl_transport *t,
                                                    GLfloat value)
{
    yagl_transport_put_out_float(t, value);
}

static __inline void yagl_transport_put_in_GLenum(struct yagl_transport *t,
                                                  GLenum *value)
{
    yagl_transport_put_in_uint32_t(t, (uint32_t*)value);
}

static __inline void yagl_transport_put_in_GLuint(struct yagl_transport *t,
                                                  GLuint *value)
{
    yagl_transport_put_in_uint32_t(t, value);
}

static __inline void yagl_transport_put_in_GLint(struct yagl_transport *t,
                                                 GLint *value)
{
    yagl_transport_put_in_uint32_t(t, (uint32_t*)value);
}

static __inline void yagl_transport_put_in_GLsizei(struct yagl_transport *t,
                                                   GLsizei *value)
{
    yagl_transport_put_in_uint32_t(t, (uint32_t*)value);
}

static __inline void yagl_transport_put_in_GLboolean(struct yagl_transport *t,
                                                     GLboolean *value)
{
    yagl_transport_put_in_uint8_t(t, value);
}

static __inline void yagl_transport_put_in_GLfloat(struct yagl_transport *t,
                                                   GLfloat *value)
{
    yagl_transport_put_in_float(t, value);
}

#endif
