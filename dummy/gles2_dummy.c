/*
 * Emulator YaGL dummy library for GLESv2
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Jinhyung Jo <jinhyung.jo@samsung.conm>
 * Stanislav Vorobiov <s.vorobiov@samsung.com>
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

#include "GLES2/gl2.h"
#include "dummy.h"

/* GL core functions */
DUMMY_API GL_APICALL void         GL_APIENTRY glActiveTexture (GLenum texture) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glAttachShader (GLuint program, GLuint shader) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glBindAttribLocation (GLuint program, GLuint index, const GLchar* name) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glBindFramebuffer (GLenum target, GLuint framebuffer) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glBindRenderbuffer (GLenum target, GLuint renderbuffer) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glBindTexture (GLenum target, GLuint texture) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glBlendColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glBlendEquation ( GLenum mode ) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glBlendFuncSeparate (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data) { return; }
DUMMY_API GL_APICALL GLenum       GL_APIENTRY glCheckFramebufferStatus (GLenum target) { return 0; }
DUMMY_API GL_APICALL void         GL_APIENTRY glClear (GLbitfield mask) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glClearDepthf (GLclampf depth) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glClearStencil (GLint s) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glCompileShader (GLuint shader) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) { return; }
DUMMY_API GL_APICALL GLuint       GL_APIENTRY glCreateProgram (void) { return 0; }
DUMMY_API GL_APICALL GLuint       GL_APIENTRY glCreateShader (GLenum type) { return 0; }
DUMMY_API GL_APICALL void         GL_APIENTRY glCullFace (GLenum mode) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDeleteBuffers (GLsizei n, const GLuint* buffers) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDeleteFramebuffers (GLsizei n, const GLuint* framebuffers) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDeleteProgram (GLuint program) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDeleteRenderbuffers (GLsizei n, const GLuint* renderbuffers) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDeleteShader (GLuint shader) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDeleteTextures (GLsizei n, const GLuint* textures) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDepthFunc (GLenum func) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDepthMask (GLboolean flag) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDepthRangef (GLclampf zNear, GLclampf zFar) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDetachShader (GLuint program, GLuint shader) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDisable (GLenum cap) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDisableVertexAttribArray (GLuint index) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glEnable (GLenum cap) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glEnableVertexAttribArray (GLuint index) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glFinish (void) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glFlush (void) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glFrontFace (GLenum mode) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGenBuffers (GLsizei n, GLuint* buffers) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGenerateMipmap (GLenum target) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGenFramebuffers (GLsizei n, GLuint* framebuffers) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGenRenderbuffers (GLsizei n, GLuint* renderbuffers) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGenTextures (GLsizei n, GLuint* textures) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetActiveUniform (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetAttachedShaders (GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders) { return; }
DUMMY_API GL_APICALL GLint          GL_APIENTRY glGetAttribLocation (GLuint program, const GLchar* name) { return 0; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetBooleanv (GLenum pname, GLboolean* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetBufferParameteriv (GLenum target, GLenum pname, GLint* params) { return; }
DUMMY_API GL_APICALL GLenum       GL_APIENTRY glGetError (void) { return 0; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetFloatv (GLenum pname, GLfloat* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetFramebufferAttachmentParameteriv (GLenum target, GLenum attachment, GLenum pname, GLint* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetIntegerv (GLenum pname, GLint* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetProgramiv (GLuint program, GLenum pname, GLint* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetProgramInfoLog (GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetRenderbufferParameteriv (GLenum target, GLenum pname, GLint* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetShaderiv (GLuint shader, GLenum pname, GLint* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetShaderInfoLog (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetShaderPrecisionFormat (GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetShaderSource (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source) { return; }
DUMMY_API GL_APICALL const GLubyte* GL_APIENTRY glGetString (GLenum name) { return (GLubyte*)0; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetUniformfv (GLuint program, GLint location, GLfloat* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetUniformiv (GLuint program, GLint location, GLint* params) { return; }
DUMMY_API GL_APICALL GLint          GL_APIENTRY glGetUniformLocation (GLuint program, const GLchar* name) { return 0; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetVertexAttribiv (GLuint index, GLenum pname, GLint* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glGetVertexAttribPointerv (GLuint index, GLenum pname, GLvoid** pointer) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glHint (GLenum target, GLenum mode) { return; }
DUMMY_API GL_APICALL GLboolean    GL_APIENTRY glIsBuffer (GLuint buffer) { return GL_FALSE; }
DUMMY_API GL_APICALL GLboolean    GL_APIENTRY glIsEnabled (GLenum cap) { return GL_FALSE; }
DUMMY_API GL_APICALL GLboolean    GL_APIENTRY glIsFramebuffer (GLuint framebuffer) { return GL_FALSE; }
DUMMY_API GL_APICALL GLboolean    GL_APIENTRY glIsProgram (GLuint program) { return GL_FALSE; }
DUMMY_API GL_APICALL GLboolean    GL_APIENTRY glIsRenderbuffer (GLuint renderbuffer) { return GL_FALSE; }
DUMMY_API GL_APICALL GLboolean    GL_APIENTRY glIsShader (GLuint shader) { return GL_FALSE; }
DUMMY_API GL_APICALL GLboolean    GL_APIENTRY glIsTexture (GLuint texture) { return GL_FALSE; }
DUMMY_API GL_APICALL void         GL_APIENTRY glLineWidth (GLfloat width) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glLinkProgram (GLuint program) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glPixelStorei (GLenum pname, GLint param) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glPolygonOffset (GLfloat factor, GLfloat units) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glReleaseShaderCompiler (void) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glSampleCoverage (GLclampf value, GLboolean invert) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glShaderBinary (GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glShaderSource (GLuint shader, GLsizei count, const GLchar*const* string, const GLint* length) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glStencilMask (GLuint mask) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glStencilMaskSeparate (GLenum face, GLuint mask) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glStencilOpSeparate (GLenum face, GLenum fail, GLenum zfail, GLenum zpass) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint* params) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform1f (GLint location, GLfloat x) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform1fv (GLint location, GLsizei count, const GLfloat* v) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform1i (GLint location, GLint x) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform1iv (GLint location, GLsizei count, const GLint* v) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform2f (GLint location, GLfloat x, GLfloat y) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform2fv (GLint location, GLsizei count, const GLfloat* v) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform2i (GLint location, GLint x, GLint y) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform2iv (GLint location, GLsizei count, const GLint* v) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform3f (GLint location, GLfloat x, GLfloat y, GLfloat z) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform3fv (GLint location, GLsizei count, const GLfloat* v) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform3i (GLint location, GLint x, GLint y, GLint z) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform3iv (GLint location, GLsizei count, const GLint* v) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform4f (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform4fv (GLint location, GLsizei count, const GLfloat* v) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform4i (GLint location, GLint x, GLint y, GLint z, GLint w) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniform4iv (GLint location, GLsizei count, const GLint* v) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glUseProgram (GLuint program) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glValidateProgram (GLuint program) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glVertexAttrib1f (GLuint indx, GLfloat x) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glVertexAttrib1fv (GLuint indx, const GLfloat* values) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glVertexAttrib2f (GLuint indx, GLfloat x, GLfloat y) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glVertexAttrib2fv (GLuint indx, const GLfloat* values) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glVertexAttrib3f (GLuint indx, GLfloat x, GLfloat y, GLfloat z) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glVertexAttrib3fv (GLuint indx, const GLfloat* values) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glVertexAttrib4f (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glVertexAttrib4fv (GLuint indx, const GLfloat* values) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glVertexAttribPointer (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr) { return; }
DUMMY_API GL_APICALL void         GL_APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height) { return; }
