#ifndef _YAGL_GLES_CALLS_H_
#define _YAGL_GLES_CALLS_H_

#include "yagl_export.h"

YAGL_API GLboolean glIsRenderbuffer(GLuint renderbuffer);

YAGL_API void glBindRenderbuffer(GLenum target, GLuint renderbuffer);

YAGL_API void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers);

YAGL_API void glGenRenderbuffers(GLsizei n, GLuint *renderbuffer_names);

YAGL_API void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

YAGL_API void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params);

YAGL_API GLboolean glIsFramebuffer(GLuint framebuffer);

YAGL_API void glBindFramebuffer(GLenum target, GLuint framebuffer);

YAGL_API void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers);

YAGL_API void glGenFramebuffers(GLsizei n, GLuint *framebuffer_names);

YAGL_API GLenum glCheckFramebufferStatus(GLenum target);

YAGL_API void glFramebufferRenderbuffer(GLenum target,
                                        GLenum attachment,
                                        GLenum renderbuffertarget,
                                        GLuint renderbuffer);

YAGL_API void glFramebufferTexture2D(GLenum target,
                                     GLenum attachment,
                                     GLenum textarget,
                                     GLuint texture,
                                     GLint level);

YAGL_API void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params);

YAGL_API void glGenerateMipmap(GLenum target);

YAGL_API void glBlendEquation(GLenum mode);

YAGL_API void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);

YAGL_API void glBlendFuncSeparate(GLenum srcRGB,
                                  GLenum dstRGB,
                                  GLenum srcAlpha,
                                  GLenum dstAlpha);

#endif
