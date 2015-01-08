/*
 * YaGL
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Stanislav Vorobiov <s.vorobiov@samsung.com>
 * Jinhyung Jo <jinhyung.jo@samsung.com>
 * YeongKyoon Lee <yeongkyoon.lee@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * - S-Core Co., Ltd
 *
 */

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
