/*
 * Generated by gen-yagl-calls.sh, do not modify!
 */
#ifndef _YAGL_HOST_GLES2_CALLS_H_
#define _YAGL_HOST_GLES2_CALLS_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include <GLES2/gl2.h>

/*
 * glActiveTexture wrapper. id = 1
 */
int yagl_host_glActiveTexture(GLenum texture);

/*
 * glAttachShader wrapper. id = 2
 */
int yagl_host_glAttachShader(GLuint program, GLuint shader);

/*
 * glBindAttribLocation wrapper. id = 3
 */
int yagl_host_glBindAttribLocation(GLuint program, GLuint index, const GLchar* name);

/*
 * glBindBuffer wrapper. id = 4
 */
int yagl_host_glBindBuffer(GLenum target, GLuint buffer);

/*
 * glBindFramebuffer wrapper. id = 5
 */
int yagl_host_glBindFramebuffer(GLenum target, GLuint framebuffer);

/*
 * glBindRenderbuffer wrapper. id = 6
 */
int yagl_host_glBindRenderbuffer(GLenum target, GLuint renderbuffer);

/*
 * glBindTexture wrapper. id = 7
 */
int yagl_host_glBindTexture(GLenum target, GLuint texture);

/*
 * glBlendColor wrapper. id = 8
 */
int yagl_host_glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

/*
 * glBlendEquation wrapper. id = 9
 */
int yagl_host_glBlendEquation(GLenum mode);

/*
 * glBlendEquationSeparate wrapper. id = 10
 */
int yagl_host_glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);

/*
 * glBlendFunc wrapper. id = 11
 */
int yagl_host_glBlendFunc(GLenum sfactor, GLenum dfactor);

/*
 * glBlendFuncSeparate wrapper. id = 12
 */
int yagl_host_glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

/*
 * glBufferData wrapper. id = 13
 */
int yagl_host_glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);

/*
 * glBufferSubData wrapper. id = 14
 */
int yagl_host_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);

/*
 * glCheckFramebufferStatus wrapper. id = 15
 */
int yagl_host_glCheckFramebufferStatus(GLenum* retval, GLenum target);

/*
 * glClear wrapper. id = 16
 */
int yagl_host_glClear(GLbitfield mask);

/*
 * glClearColor wrapper. id = 17
 */
int yagl_host_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

/*
 * glClearDepthf wrapper. id = 18
 */
int yagl_host_glClearDepthf(GLclampf depth);

/*
 * glClearStencil wrapper. id = 19
 */
int yagl_host_glClearStencil(GLint s);

/*
 * glColorMask wrapper. id = 20
 */
int yagl_host_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

/*
 * glCompileShader wrapper. id = 21
 */
int yagl_host_glCompileShader(GLuint shader);

/*
 * glCompressedTexImage2D wrapper. id = 22
 */
int yagl_host_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);

/*
 * glCompressedTexSubImage2D wrapper. id = 23
 */
int yagl_host_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);

/*
 * glCopyTexImage2D wrapper. id = 24
 */
int yagl_host_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);

/*
 * glCopyTexSubImage2D wrapper. id = 25
 */
int yagl_host_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

/*
 * glCreateProgram wrapper. id = 26
 */
int yagl_host_glCreateProgram(GLuint* retval);

/*
 * glCreateShader wrapper. id = 27
 */
int yagl_host_glCreateShader(GLuint* retval, GLenum type);

/*
 * glCullFace wrapper. id = 28
 */
int yagl_host_glCullFace(GLenum mode);

/*
 * glDeleteBuffers wrapper. id = 29
 */
int yagl_host_glDeleteBuffers(GLsizei n, const GLuint* buffers);

/*
 * glDeleteFramebuffers wrapper. id = 30
 */
int yagl_host_glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers);

/*
 * glDeleteProgram wrapper. id = 31
 */
int yagl_host_glDeleteProgram(GLuint program);

/*
 * glDeleteRenderbuffers wrapper. id = 32
 */
int yagl_host_glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);

/*
 * glDeleteShader wrapper. id = 33
 */
int yagl_host_glDeleteShader(GLuint shader);

/*
 * glDeleteTextures wrapper. id = 34
 */
int yagl_host_glDeleteTextures(GLsizei n, const GLuint* textures);

/*
 * glDepthFunc wrapper. id = 35
 */
int yagl_host_glDepthFunc(GLenum func);

/*
 * glDepthMask wrapper. id = 36
 */
int yagl_host_glDepthMask(GLboolean flag);

/*
 * glDepthRangef wrapper. id = 37
 */
int yagl_host_glDepthRangef(GLclampf zNear, GLclampf zFar);

/*
 * glDetachShader wrapper. id = 38
 */
int yagl_host_glDetachShader(GLuint program, GLuint shader);

/*
 * glDisable wrapper. id = 39
 */
int yagl_host_glDisable(GLenum cap);

/*
 * glDisableVertexAttribArray wrapper. id = 40
 */
int yagl_host_glDisableVertexAttribArray(GLuint index);

/*
 * glDrawArrays wrapper. id = 41
 */
int yagl_host_glDrawArrays(GLenum mode, GLint first, GLsizei count);

/*
 * glDrawElements wrapper. id = 42
 */
int yagl_host_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);

/*
 * glEnable wrapper. id = 43
 */
int yagl_host_glEnable(GLenum cap);

/*
 * glEnableVertexAttribArray wrapper. id = 44
 */
int yagl_host_glEnableVertexAttribArray(GLuint index);

/*
 * glFinish wrapper. id = 45
 */
int yagl_host_glFinish();

/*
 * glFlush wrapper. id = 46
 */
int yagl_host_glFlush();

/*
 * glFramebufferRenderbuffer wrapper. id = 47
 */
int yagl_host_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

/*
 * glFramebufferTexture2D wrapper. id = 48
 */
int yagl_host_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);

/*
 * glFrontFace wrapper. id = 49
 */
int yagl_host_glFrontFace(GLenum mode);

/*
 * glGenBuffers wrapper. id = 50
 */
int yagl_host_glGenBuffers(GLsizei n, GLuint* buffers);

/*
 * glGenerateMipmap wrapper. id = 51
 */
int yagl_host_glGenerateMipmap(GLenum target);

/*
 * glGenFramebuffers wrapper. id = 52
 */
int yagl_host_glGenFramebuffers(GLsizei n, GLuint* framebuffers);

/*
 * glGenRenderbuffers wrapper. id = 53
 */
int yagl_host_glGenRenderbuffers(GLsizei n, GLuint* renderbuffers);

/*
 * glGenTextures wrapper. id = 54
 */
int yagl_host_glGenTextures(GLsizei n, GLuint* textures);

/*
 * glGetActiveAttrib wrapper. id = 55
 */
int yagl_host_glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);

/*
 * glGetActiveUniform wrapper. id = 56
 */
int yagl_host_glGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);

/*
 * glGetAttachedShaders wrapper. id = 57
 */
int yagl_host_glGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);

/*
 * glGetAttribLocation wrapper. id = 58
 */
int yagl_host_glGetAttribLocation(int* retval, GLuint program, const GLchar* name);

/*
 * glGetBooleanv wrapper. id = 59
 */
int yagl_host_glGetBooleanv(GLenum pname, GLboolean* params);

/*
 * glGetBufferParameteriv wrapper. id = 60
 */
int yagl_host_glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params);

/*
 * glGetError wrapper. id = 61
 */
int yagl_host_glGetError(GLenum* retval);

/*
 * glGetFloatv wrapper. id = 62
 */
int yagl_host_glGetFloatv(GLenum pname, GLfloat* params);

/*
 * glGetFramebufferAttachmentParameteriv wrapper. id = 63
 */
int yagl_host_glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params);

/*
 * glGetIntegerv wrapper. id = 64
 */
int yagl_host_glGetIntegerv(GLenum pname, GLint* params);

/*
 * glGetProgramiv wrapper. id = 65
 */
int yagl_host_glGetProgramiv(GLuint program, GLenum pname, GLint* params);

/*
 * glGetProgramInfoLog wrapper. id = 66
 */
int yagl_host_glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);

/*
 * glGetRenderbufferParameteriv wrapper. id = 67
 */
int yagl_host_glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params);

/*
 * glGetShaderiv wrapper. id = 68
 */
int yagl_host_glGetShaderiv(GLuint shader, GLenum pname, GLint* params);

/*
 * glGetShaderInfoLog wrapper. id = 69
 */
int yagl_host_glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);

/*
 * glGetShaderPrecisionFormat wrapper. id = 70
 */
int yagl_host_glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);

/*
 * glGetShaderSource wrapper. id = 71
 */
int yagl_host_glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source);

/*
 * glGetTexParameterfv wrapper. id = 72
 */
int yagl_host_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params);

/*
 * glGetTexParameteriv wrapper. id = 73
 */
int yagl_host_glGetTexParameteriv(GLenum target, GLenum pname, GLint* params);

/*
 * glGetUniformfv wrapper. id = 74
 */
int yagl_host_glGetUniformfv(GLuint program, GLint location, GLfloat* params);

/*
 * glGetUniformiv wrapper. id = 75
 */
int yagl_host_glGetUniformiv(GLuint program, GLint location, GLint* params);

/*
 * glGetUniformLocation wrapper. id = 76
 */
int yagl_host_glGetUniformLocation(int* retval, GLuint program, const GLchar* name);

/*
 * glGetVertexAttribfv wrapper. id = 77
 */
int yagl_host_glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params);

/*
 * glGetVertexAttribiv wrapper. id = 78
 */
int yagl_host_glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params);

/*
 * glGetVertexAttribPointerv wrapper. id = 79
 */
int yagl_host_glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid** pointer);

/*
 * glHint wrapper. id = 80
 */
int yagl_host_glHint(GLenum target, GLenum mode);

/*
 * glIsBuffer wrapper. id = 81
 */
int yagl_host_glIsBuffer(GLboolean* retval, GLuint buffer);

/*
 * glIsEnabled wrapper. id = 82
 */
int yagl_host_glIsEnabled(GLboolean* retval, GLenum cap);

/*
 * glIsFramebuffer wrapper. id = 83
 */
int yagl_host_glIsFramebuffer(GLboolean* retval, GLuint framebuffer);

/*
 * glIsProgram wrapper. id = 84
 */
int yagl_host_glIsProgram(GLboolean* retval, GLuint program);

/*
 * glIsRenderbuffer wrapper. id = 85
 */
int yagl_host_glIsRenderbuffer(GLboolean* retval, GLuint renderbuffer);

/*
 * glIsShader wrapper. id = 86
 */
int yagl_host_glIsShader(GLboolean* retval, GLuint shader);

/*
 * glIsTexture wrapper. id = 87
 */
int yagl_host_glIsTexture(GLboolean* retval, GLuint texture);

/*
 * glLineWidth wrapper. id = 88
 */
int yagl_host_glLineWidth(GLfloat width);

/*
 * glLinkProgram wrapper. id = 89
 */
int yagl_host_glLinkProgram(GLuint program);

/*
 * glPixelStorei wrapper. id = 90
 */
int yagl_host_glPixelStorei(GLenum pname, GLint param);

/*
 * glPolygonOffset wrapper. id = 91
 */
int yagl_host_glPolygonOffset(GLfloat factor, GLfloat units);

/*
 * glReadPixels wrapper. id = 92
 */
int yagl_host_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);

/*
 * glReleaseShaderCompiler wrapper. id = 93
 */
int yagl_host_glReleaseShaderCompiler();

/*
 * glRenderbufferStorage wrapper. id = 94
 */
int yagl_host_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

/*
 * glSampleCoverage wrapper. id = 95
 */
int yagl_host_glSampleCoverage(GLclampf value, GLboolean invert);

/*
 * glScissor wrapper. id = 96
 */
int yagl_host_glScissor(GLint x, GLint y, GLsizei width, GLsizei height);

/*
 * glShaderBinary wrapper. id = 97
 */
int yagl_host_glShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);

/*
 * glShaderSource wrapper. id = 98
 */
int yagl_host_glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);

/*
 * glStencilFunc wrapper. id = 99
 */
int yagl_host_glStencilFunc(GLenum func, GLint ref, GLuint mask);

/*
 * glStencilFuncSeparate wrapper. id = 100
 */
int yagl_host_glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);

/*
 * glStencilMask wrapper. id = 101
 */
int yagl_host_glStencilMask(GLuint mask);

/*
 * glStencilMaskSeparate wrapper. id = 102
 */
int yagl_host_glStencilMaskSeparate(GLenum face, GLuint mask);

/*
 * glStencilOp wrapper. id = 103
 */
int yagl_host_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);

/*
 * glStencilOpSeparate wrapper. id = 104
 */
int yagl_host_glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);

/*
 * glTexImage2D wrapper. id = 105
 */
int yagl_host_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);

/*
 * glTexParameterf wrapper. id = 106
 */
int yagl_host_glTexParameterf(GLenum target, GLenum pname, GLfloat param);

/*
 * glTexParameterfv wrapper. id = 107
 */
int yagl_host_glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params);

/*
 * glTexParameteri wrapper. id = 108
 */
int yagl_host_glTexParameteri(GLenum target, GLenum pname, GLint param);

/*
 * glTexParameteriv wrapper. id = 109
 */
int yagl_host_glTexParameteriv(GLenum target, GLenum pname, const GLint* params);

/*
 * glTexSubImage2D wrapper. id = 110
 */
int yagl_host_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);

/*
 * glUniform1f wrapper. id = 111
 */
int yagl_host_glUniform1f(GLint location, GLfloat x);

/*
 * glUniform1fv wrapper. id = 112
 */
int yagl_host_glUniform1fv(GLint location, GLsizei count, const GLfloat* v);

/*
 * glUniform1i wrapper. id = 113
 */
int yagl_host_glUniform1i(GLint location, GLint x);

/*
 * glUniform1iv wrapper. id = 114
 */
int yagl_host_glUniform1iv(GLint location, GLsizei count, const GLint* v);

/*
 * glUniform2f wrapper. id = 115
 */
int yagl_host_glUniform2f(GLint location, GLfloat x, GLfloat y);

/*
 * glUniform2fv wrapper. id = 116
 */
int yagl_host_glUniform2fv(GLint location, GLsizei count, const GLfloat* v);

/*
 * glUniform2i wrapper. id = 117
 */
int yagl_host_glUniform2i(GLint location, GLint x, GLint y);

/*
 * glUniform2iv wrapper. id = 118
 */
int yagl_host_glUniform2iv(GLint location, GLsizei count, const GLint* v);

/*
 * glUniform3f wrapper. id = 119
 */
int yagl_host_glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z);

/*
 * glUniform3fv wrapper. id = 120
 */
int yagl_host_glUniform3fv(GLint location, GLsizei count, const GLfloat* v);

/*
 * glUniform3i wrapper. id = 121
 */
int yagl_host_glUniform3i(GLint location, GLint x, GLint y, GLint z);

/*
 * glUniform3iv wrapper. id = 122
 */
int yagl_host_glUniform3iv(GLint location, GLsizei count, const GLint* v);

/*
 * glUniform4f wrapper. id = 123
 */
int yagl_host_glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

/*
 * glUniform4fv wrapper. id = 124
 */
int yagl_host_glUniform4fv(GLint location, GLsizei count, const GLfloat* v);

/*
 * glUniform4i wrapper. id = 125
 */
int yagl_host_glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w);

/*
 * glUniform4iv wrapper. id = 126
 */
int yagl_host_glUniform4iv(GLint location, GLsizei count, const GLint* v);

/*
 * glUniformMatrix2fv wrapper. id = 127
 */
int yagl_host_glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

/*
 * glUniformMatrix3fv wrapper. id = 128
 */
int yagl_host_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

/*
 * glUniformMatrix4fv wrapper. id = 129
 */
int yagl_host_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

/*
 * glUseProgram wrapper. id = 130
 */
int yagl_host_glUseProgram(GLuint program);

/*
 * glValidateProgram wrapper. id = 131
 */
int yagl_host_glValidateProgram(GLuint program);

/*
 * glVertexAttrib1f wrapper. id = 132
 */
int yagl_host_glVertexAttrib1f(GLuint indx, GLfloat x);

/*
 * glVertexAttrib1fv wrapper. id = 133
 */
int yagl_host_glVertexAttrib1fv(GLuint indx, const GLfloat* values);

/*
 * glVertexAttrib2f wrapper. id = 134
 */
int yagl_host_glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y);

/*
 * glVertexAttrib2fv wrapper. id = 135
 */
int yagl_host_glVertexAttrib2fv(GLuint indx, const GLfloat* values);

/*
 * glVertexAttrib3f wrapper. id = 136
 */
int yagl_host_glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z);

/*
 * glVertexAttrib3fv wrapper. id = 137
 */
int yagl_host_glVertexAttrib3fv(GLuint indx, const GLfloat* values);

/*
 * glVertexAttrib4f wrapper. id = 138
 */
int yagl_host_glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

/*
 * glVertexAttrib4fv wrapper. id = 139
 */
int yagl_host_glVertexAttrib4fv(GLuint indx, const GLfloat* values);

/*
 * glVertexAttribPointer wrapper. id = 140
 */
int yagl_host_glVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);

/*
 * glViewport wrapper. id = 141
 */
int yagl_host_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

/*
 * glGetExtensionStringYAGL wrapper. id = 142
 */
int yagl_host_glGetExtensionStringYAGL(GLuint* retval, GLchar* str);

/*
 * glEGLImageTargetTexture2DYAGL wrapper. id = 143
 */
int yagl_host_glEGLImageTargetTexture2DYAGL(GLenum target, uint32_t width, uint32_t height, uint32_t bpp, const void* pixels);

/*
 * glGetVertexAttribRangeYAGL wrapper. id = 144
 */
int yagl_host_glGetVertexAttribRangeYAGL(GLsizei count, GLenum type, const GLvoid* indices, GLint* range_first, GLsizei* range_count);

#endif
