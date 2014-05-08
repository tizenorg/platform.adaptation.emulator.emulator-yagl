/*
* Emulator YaGL dummy library for GLESv1
*
* Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
*
* Contact : Jinhyung Jo <jinhyung.jo@samsung.conm>
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
*/

#include "GLES/gl.h"

/* Available only in Common profile */
GL_API void GL_APIENTRY glAlphaFunc (GLenum func, GLclampf ref) { return; }
GL_API void GL_APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) { return; }
GL_API void GL_APIENTRY glClearDepthf (GLclampf depth) { return; }
GL_API void GL_APIENTRY glClipPlanef (GLenum plane, const GLfloat *equation) { return; }
GL_API void GL_APIENTRY glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { return; }
GL_API void GL_APIENTRY glDepthRangef (GLclampf zNear, GLclampf zFar) { return; }
GL_API void GL_APIENTRY glFogf (GLenum pname, GLfloat param) { return; }
GL_API void GL_APIENTRY glFogfv (GLenum pname, const GLfloat *params) { return; }
GL_API void GL_APIENTRY glFrustumf (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) { return; }
GL_API void GL_APIENTRY glGetClipPlanef (GLenum pname, GLfloat eqn[4]) { return; }
GL_API void GL_APIENTRY glGetFloatv (GLenum pname, GLfloat *params) { return; }
GL_API void GL_APIENTRY glGetLightfv (GLenum light, GLenum pname, GLfloat *params) { return; }
GL_API void GL_APIENTRY glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params) { return; }
GL_API void GL_APIENTRY glGetTexEnvfv (GLenum env, GLenum pname, GLfloat *params) { return; }
GL_API void GL_APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params) { return; }
GL_API void GL_APIENTRY glLightModelf (GLenum pname, GLfloat param) { return; }
GL_API void GL_APIENTRY glLightModelfv (GLenum pname, const GLfloat *params) { return; }
GL_API void GL_APIENTRY glLightf (GLenum light, GLenum pname, GLfloat param) { return; }
GL_API void GL_APIENTRY glLightfv (GLenum light, GLenum pname, const GLfloat *params) { return; }
GL_API void GL_APIENTRY glLineWidth (GLfloat width) { return; }
GL_API void GL_APIENTRY glLoadMatrixf (const GLfloat *m) { return; }
GL_API void GL_APIENTRY glMaterialf (GLenum face, GLenum pname, GLfloat param) { return; }
GL_API void GL_APIENTRY glMaterialfv (GLenum face, GLenum pname, const GLfloat *params) { return; }
GL_API void GL_APIENTRY glMultMatrixf (const GLfloat *m) { return; }
GL_API void GL_APIENTRY glMultiTexCoord4f (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) { return; }
GL_API void GL_APIENTRY glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz) { return; }
GL_API void GL_APIENTRY glOrthof (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) { return; }
GL_API void GL_APIENTRY glPointParameterf (GLenum pname, GLfloat param) { return; }
GL_API void GL_APIENTRY glPointParameterfv (GLenum pname, const GLfloat *params) { return; }
GL_API void GL_APIENTRY glPointSize (GLfloat size) { return; }
GL_API void GL_APIENTRY glPolygonOffset (GLfloat factor, GLfloat units) { return; }
GL_API void GL_APIENTRY glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z) { return; }
GL_API void GL_APIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z) { return; }
GL_API void GL_APIENTRY glTexEnvf (GLenum target, GLenum pname, GLfloat param) { return; }
GL_API void GL_APIENTRY glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params) { return; }
GL_API void GL_APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param) { return; }
GL_API void GL_APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params) { return; }
GL_API void GL_APIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z) { return; }

/* Available in both Common and Common-Lite profiles */
GL_API void GL_APIENTRY glActiveTexture (GLenum texture) { return; }
GL_API void GL_APIENTRY glAlphaFuncx (GLenum func, GLclampx ref) { return; }
GL_API void GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer) { return; }
GL_API void GL_APIENTRY glBindTexture (GLenum target, GLuint texture) { return; }
GL_API void GL_APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor) { return; }
GL_API void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) { return; }
GL_API void GL_APIENTRY glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data) { return; }
GL_API void GL_APIENTRY glClear (GLbitfield mask) { return; }
GL_API void GL_APIENTRY glClearColorx (GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha) { return; }
GL_API void GL_APIENTRY glClearDepthx (GLclampx depth) { return; }
GL_API void GL_APIENTRY glClearStencil (GLint s) { return; }
GL_API void GL_APIENTRY glClientActiveTexture (GLenum texture) { return; }
GL_API void GL_APIENTRY glClipPlanex (GLenum plane, const GLfixed *equation) { return; }
GL_API void GL_APIENTRY glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) { return; }
GL_API void GL_APIENTRY glColor4x (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) { return; }
GL_API void GL_APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) { return; }
GL_API void GL_APIENTRY glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) { return; }
GL_API void GL_APIENTRY glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) { return; }
GL_API void GL_APIENTRY glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) { return; }
GL_API void GL_APIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) { return; }
GL_API void GL_APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) { return; }
GL_API void GL_APIENTRY glCullFace (GLenum mode) { return; }
GL_API void GL_APIENTRY glDeleteBuffers (GLsizei n, const GLuint *buffers) { return; }
GL_API void GL_APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures) { return; }
GL_API void GL_APIENTRY glDepthFunc (GLenum func) { return; }
GL_API void GL_APIENTRY glDepthMask (GLboolean flag) { return; }
GL_API void GL_APIENTRY glDepthRangex (GLclampx zNear, GLclampx zFar) { return; }
GL_API void GL_APIENTRY glDisable (GLenum cap) { return; }
GL_API void GL_APIENTRY glDisableClientState (GLenum array) { return; }
GL_API void GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count) { return; }
GL_API void GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) { return; }
GL_API void GL_APIENTRY glEnable (GLenum cap) { return; }
GL_API void GL_APIENTRY glEnableClientState (GLenum array) { return; }
GL_API void GL_APIENTRY glFinish (void) { return; }
GL_API void GL_APIENTRY glFlush (void) { return; }
GL_API void GL_APIENTRY glFogx (GLenum pname, GLfixed param) { return; }
GL_API void GL_APIENTRY glFogxv (GLenum pname, const GLfixed *params) { return; }
GL_API void GL_APIENTRY glFrontFace (GLenum mode) { return; }
GL_API void GL_APIENTRY glFrustumx (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar) { return; }
GL_API void GL_APIENTRY glGetBooleanv (GLenum pname, GLboolean *params) { return; }
GL_API void GL_APIENTRY glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params) { return; }
GL_API void GL_APIENTRY glGetClipPlanex (GLenum pname, GLfixed eqn[4]) { return; }
GL_API void GL_APIENTRY glGenBuffers (GLsizei n, GLuint *buffers) { return; }
GL_API void GL_APIENTRY glGenTextures (GLsizei n, GLuint *textures) { return; }
GL_API GLenum GL_APIENTRY glGetError (void) { return 0; }
GL_API void GL_APIENTRY glGetFixedv (GLenum pname, GLfixed *params) { return; }
GL_API void GL_APIENTRY glGetIntegerv (GLenum pname, GLint *params) { return; }
GL_API void GL_APIENTRY glGetLightxv (GLenum light, GLenum pname, GLfixed *params) { return; }
GL_API void GL_APIENTRY glGetMaterialxv (GLenum face, GLenum pname, GLfixed *params) { return; }
GL_API void GL_APIENTRY glGetPointerv (GLenum pname, GLvoid **params) { return; }
GL_API const GLubyte * GL_APIENTRY glGetString (GLenum name) { return 0; }
GL_API void GL_APIENTRY glGetTexEnviv (GLenum env, GLenum pname, GLint *params) { return; }
GL_API void GL_APIENTRY glGetTexEnvxv (GLenum env, GLenum pname, GLfixed *params) { return; }
GL_API void GL_APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params) { return; }
GL_API void GL_APIENTRY glGetTexParameterxv (GLenum target, GLenum pname, GLfixed *params) { return; }
GL_API void GL_APIENTRY glHint (GLenum target, GLenum mode) { return; }
GL_API GLboolean GL_APIENTRY glIsBuffer (GLuint buffer) { return GL_FALSE; }
GL_API GLboolean GL_APIENTRY glIsEnabled (GLenum cap) { return GL_FALSE; }
GL_API GLboolean GL_APIENTRY glIsTexture (GLuint texture) { return GL_FALSE; }
GL_API void GL_APIENTRY glLightModelx (GLenum pname, GLfixed param) { return; }
GL_API void GL_APIENTRY glLightModelxv (GLenum pname, const GLfixed *params) { return; }
GL_API void GL_APIENTRY glLightx (GLenum light, GLenum pname, GLfixed param) { return; }
GL_API void GL_APIENTRY glLightxv (GLenum light, GLenum pname, const GLfixed *params) { return; }
GL_API void GL_APIENTRY glLineWidthx (GLfixed width) { return; }
GL_API void GL_APIENTRY glLoadIdentity (void) { return; }
GL_API void GL_APIENTRY glLoadMatrixx (const GLfixed *m) { return; }
GL_API void GL_APIENTRY glLogicOp (GLenum opcode) { return; }
GL_API void GL_APIENTRY glMaterialx (GLenum face, GLenum pname, GLfixed param) { return; }
GL_API void GL_APIENTRY glMaterialxv (GLenum face, GLenum pname, const GLfixed *params) { return; }
GL_API void GL_APIENTRY glMatrixMode (GLenum mode) { return; }
GL_API void GL_APIENTRY glMultMatrixx (const GLfixed *m) { return; }
GL_API void GL_APIENTRY glMultiTexCoord4x (GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q) { return; }
GL_API void GL_APIENTRY glNormal3x (GLfixed nx, GLfixed ny, GLfixed nz) { return; }
GL_API void GL_APIENTRY glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer) { return; }
GL_API void GL_APIENTRY glOrthox (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar) { return; }
GL_API void GL_APIENTRY glPixelStorei (GLenum pname, GLint param) { return; }
GL_API void GL_APIENTRY glPointParameterx (GLenum pname, GLfixed param) { return; }
GL_API void GL_APIENTRY glPointParameterxv (GLenum pname, const GLfixed *params) { return; }
GL_API void GL_APIENTRY glPointSizex (GLfixed size) { return; }
GL_API void GL_APIENTRY glPolygonOffsetx (GLfixed factor, GLfixed units) { return; }
GL_API void GL_APIENTRY glPopMatrix (void) { return; }
GL_API void GL_APIENTRY glPushMatrix (void) { return; }
GL_API void GL_APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels) { return; }
GL_API void GL_APIENTRY glRotatex (GLfixed angle, GLfixed x, GLfixed y, GLfixed z) { return; }
GL_API void GL_APIENTRY glSampleCoverage (GLclampf value, GLboolean invert) { return; }
GL_API void GL_APIENTRY glSampleCoveragex (GLclampx value, GLboolean invert) { return; }
GL_API void GL_APIENTRY glScalex (GLfixed x, GLfixed y, GLfixed z) { return; }
GL_API void GL_APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height) { return; }
GL_API void GL_APIENTRY glShadeModel (GLenum mode) { return; }
GL_API void GL_APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask) { return; }
GL_API void GL_APIENTRY glStencilMask (GLuint mask) { return; }
GL_API void GL_APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass) { return; }
GL_API void GL_APIENTRY glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) { return; }
GL_API void GL_APIENTRY glTexEnvi (GLenum target, GLenum pname, GLint param) { return; }
GL_API void GL_APIENTRY glTexEnvx (GLenum target, GLenum pname, GLfixed param) { return; }
GL_API void GL_APIENTRY glTexEnviv (GLenum target, GLenum pname, const GLint *params) { return; }
GL_API void GL_APIENTRY glTexEnvxv (GLenum target, GLenum pname, const GLfixed *params) { return; }
GL_API void GL_APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) { return; }
GL_API void GL_APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param) { return; }
GL_API void GL_APIENTRY glTexParameterx (GLenum target, GLenum pname, GLfixed param) { return; }
GL_API void GL_APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params) { return; }
GL_API void GL_APIENTRY glTexParameterxv (GLenum target, GLenum pname, const GLfixed *params) { return; }
GL_API void GL_APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) { return; }
GL_API void GL_APIENTRY glTranslatex (GLfixed x, GLfixed y, GLfixed z) { return; }
GL_API void GL_APIENTRY glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) { return; }
GL_API void GL_APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height) { return; }
GL_API void GL_APIENTRY glPointSizePointerOES (GLenum type, GLsizei stride, const GLvoid *pointer) { return; }
