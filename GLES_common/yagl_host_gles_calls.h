/*
 * Generated by gen-yagl-calls.py, do not modify!
 */
#ifndef _YAGL_HOST_GLES_CALLS_H_
#define _YAGL_HOST_GLES_CALLS_H_

#include "yagl_export.h"
#include "yagl_types.h"

/*
 * glDrawArrays wrapper. id = 1
 */
void yagl_host_glDrawArrays(GLenum mode, GLint first, GLsizei count);

/*
 * glDrawElements wrapper. id = 2
 */
void yagl_host_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, int32_t indices_count);

/*
 * glReadPixels wrapper. id = 3
 */
void yagl_host_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels, int32_t pixels_maxcount, int32_t *pixels_count);

/*
 * glGenVertexArrays wrapper. id = 4
 */
void yagl_host_glGenVertexArrays(const GLuint *arrays, int32_t arrays_count);

/*
 * glBindVertexArray wrapper. id = 5
 */
void yagl_host_glBindVertexArray(GLuint array);

/*
 * glDisableVertexAttribArray wrapper. id = 6
 */
void yagl_host_glDisableVertexAttribArray(GLuint index);

/*
 * glEnableVertexAttribArray wrapper. id = 7
 */
void yagl_host_glEnableVertexAttribArray(GLuint index);

/*
 * glVertexAttribPointerData wrapper. id = 8
 */
void yagl_host_glVertexAttribPointerData(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLint first, const GLvoid *data, int32_t data_count);

/*
 * glVertexAttribPointerOffset wrapper. id = 9
 */
void yagl_host_glVertexAttribPointerOffset(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLsizei offset);

/*
 * glVertexPointerData wrapper. id = 10
 */
void yagl_host_glVertexPointerData(GLint size, GLenum type, GLsizei stride, GLint first, const GLvoid *data, int32_t data_count);

/*
 * glVertexPointerOffset wrapper. id = 11
 */
void yagl_host_glVertexPointerOffset(GLint size, GLenum type, GLsizei stride, GLsizei offset);

/*
 * glNormalPointerData wrapper. id = 12
 */
void yagl_host_glNormalPointerData(GLenum type, GLsizei stride, GLint first, const GLvoid *data, int32_t data_count);

/*
 * glNormalPointerOffset wrapper. id = 13
 */
void yagl_host_glNormalPointerOffset(GLenum type, GLsizei stride, GLsizei offset);

/*
 * glColorPointerData wrapper. id = 14
 */
void yagl_host_glColorPointerData(GLint size, GLenum type, GLsizei stride, GLint first, const GLvoid *data, int32_t data_count);

/*
 * glColorPointerOffset wrapper. id = 15
 */
void yagl_host_glColorPointerOffset(GLint size, GLenum type, GLsizei stride, GLsizei offset);

/*
 * glTexCoordPointerData wrapper. id = 16
 */
void yagl_host_glTexCoordPointerData(GLint tex_id, GLint size, GLenum type, GLsizei stride, GLint first, const GLvoid *data, int32_t data_count);

/*
 * glTexCoordPointerOffset wrapper. id = 17
 */
void yagl_host_glTexCoordPointerOffset(GLint size, GLenum type, GLsizei stride, GLsizei offset);

/*
 * glDisableClientState wrapper. id = 18
 */
void yagl_host_glDisableClientState(GLenum array);

/*
 * glEnableClientState wrapper. id = 19
 */
void yagl_host_glEnableClientState(GLenum array);

/*
 * glGenBuffers wrapper. id = 20
 */
void yagl_host_glGenBuffers(const GLuint *buffers, int32_t buffers_count);

/*
 * glBindBuffer wrapper. id = 21
 */
void yagl_host_glBindBuffer(GLenum target, GLuint buffer);

/*
 * glBufferData wrapper. id = 22
 */
void yagl_host_glBufferData(GLenum target, const GLvoid *data, int32_t data_count, GLenum usage);

/*
 * glBufferSubData wrapper. id = 23
 */
void yagl_host_glBufferSubData(GLenum target, GLsizei offset, const GLvoid *data, int32_t data_count);

/*
 * glGenTextures wrapper. id = 24
 */
void yagl_host_glGenTextures(const GLuint *textures, int32_t textures_count);

/*
 * glBindTexture wrapper. id = 25
 */
void yagl_host_glBindTexture(GLenum target, GLuint texture);

/*
 * glActiveTexture wrapper. id = 26
 */
void yagl_host_glActiveTexture(GLenum texture);

/*
 * glCompressedTexImage2D wrapper. id = 27
 */
void yagl_host_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, const GLvoid *data, int32_t data_count);

/*
 * glCompressedTexSubImage2D wrapper. id = 28
 */
void yagl_host_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, const GLvoid *data, int32_t data_count);

/*
 * glCopyTexImage2D wrapper. id = 29
 */
void yagl_host_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);

/*
 * glCopyTexSubImage2D wrapper. id = 30
 */
void yagl_host_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

/*
 * glGetTexParameterfv wrapper. id = 31
 */
void yagl_host_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *param);

/*
 * glGetTexParameteriv wrapper. id = 32
 */
void yagl_host_glGetTexParameteriv(GLenum target, GLenum pname, GLint *param);

/*
 * glTexImage2D wrapper. id = 33
 */
void yagl_host_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels, int32_t pixels_count);

/*
 * glTexParameterf wrapper. id = 34
 */
void yagl_host_glTexParameterf(GLenum target, GLenum pname, GLfloat param);

/*
 * glTexParameterfv wrapper. id = 35
 */
void yagl_host_glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params, int32_t params_count);

/*
 * glTexParameteri wrapper. id = 36
 */
void yagl_host_glTexParameteri(GLenum target, GLenum pname, GLint param);

/*
 * glTexParameteriv wrapper. id = 37
 */
void yagl_host_glTexParameteriv(GLenum target, GLenum pname, const GLint *params, int32_t params_count);

/*
 * glTexSubImage2D wrapper. id = 38
 */
void yagl_host_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels, int32_t pixels_count);

/*
 * glClientActiveTexture wrapper. id = 39
 */
void yagl_host_glClientActiveTexture(GLenum texture);

/*
 * glTexEnvi wrapper. id = 40
 */
void yagl_host_glTexEnvi(GLenum target, GLenum pname, GLint param);

/*
 * glTexEnvf wrapper. id = 41
 */
void yagl_host_glTexEnvf(GLenum target, GLenum pname, GLfloat param);

/*
 * glMultiTexCoord4f wrapper. id = 42
 */
void yagl_host_glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat tt, GLfloat r, GLfloat q);

/*
 * glTexEnviv wrapper. id = 43
 */
void yagl_host_glTexEnviv(GLenum target, GLenum pname, const GLint *params, int32_t params_count);

/*
 * glTexEnvfv wrapper. id = 44
 */
void yagl_host_glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params, int32_t params_count);

/*
 * glGetTexEnviv wrapper. id = 45
 */
void yagl_host_glGetTexEnviv(GLenum env, GLenum pname, GLint *params, int32_t params_maxcount, int32_t *params_count);

/*
 * glGetTexEnvfv wrapper. id = 46
 */
void yagl_host_glGetTexEnvfv(GLenum env, GLenum pname, GLfloat *params, int32_t params_maxcount, int32_t *params_count);

/*
 * glGenFramebuffers wrapper. id = 47
 */
void yagl_host_glGenFramebuffers(const GLuint *framebuffers, int32_t framebuffers_count);

/*
 * glBindFramebuffer wrapper. id = 48
 */
void yagl_host_glBindFramebuffer(GLenum target, GLuint framebuffer);

/*
 * glFramebufferTexture2D wrapper. id = 49
 */
void yagl_host_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);

/*
 * glFramebufferRenderbuffer wrapper. id = 50
 */
void yagl_host_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

/*
 * glBlitFramebuffer wrapper. id = 51
 */
void yagl_host_glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

/*
 * glGenRenderbuffers wrapper. id = 52
 */
void yagl_host_glGenRenderbuffers(const GLuint *renderbuffers, int32_t renderbuffers_count);

/*
 * glBindRenderbuffer wrapper. id = 53
 */
void yagl_host_glBindRenderbuffer(GLenum target, GLuint renderbuffer);

/*
 * glRenderbufferStorage wrapper. id = 54
 */
void yagl_host_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

/*
 * glGetRenderbufferParameteriv wrapper. id = 55
 */
void yagl_host_glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *param);

/*
 * glCreateProgram wrapper. id = 56
 */
void yagl_host_glCreateProgram(GLuint program);

/*
 * glCreateShader wrapper. id = 57
 */
void yagl_host_glCreateShader(GLuint shader, GLenum type);

/*
 * glShaderSource wrapper. id = 58
 */
void yagl_host_glShaderSource(GLuint shader, const GLchar *string, int32_t string_count);

/*
 * glAttachShader wrapper. id = 59
 */
void yagl_host_glAttachShader(GLuint program, GLuint shader);

/*
 * glDetachShader wrapper. id = 60
 */
void yagl_host_glDetachShader(GLuint program, GLuint shader);

/*
 * glCompileShader wrapper. id = 61
 */
void yagl_host_glCompileShader(GLuint shader);

/*
 * glBindAttribLocation wrapper. id = 62
 */
void yagl_host_glBindAttribLocation(GLuint program, GLuint index, const GLchar *name, int32_t name_count);

/*
 * glGetActiveAttrib wrapper. id = 63
 */
GLboolean yagl_host_glGetActiveAttrib(GLuint program, GLuint index, GLint *size, GLenum *type, GLchar *name, int32_t name_maxcount, int32_t *name_count);

/*
 * glGetActiveUniform wrapper. id = 64
 */
GLboolean yagl_host_glGetActiveUniform(GLuint program, GLuint index, GLint *size, GLenum *type, GLchar *name, int32_t name_maxcount, int32_t *name_count);

/*
 * glGetAttribLocation wrapper. id = 65
 */
int yagl_host_glGetAttribLocation(GLuint program, const GLchar *name, int32_t name_count);

/*
 * glGetProgramiv wrapper. id = 66
 */
void yagl_host_glGetProgramiv(GLuint program, GLenum pname, GLint *param);

/*
 * glGetProgramInfoLog wrapper. id = 67
 */
GLboolean yagl_host_glGetProgramInfoLog(GLuint program, GLchar *infolog, int32_t infolog_maxcount, int32_t *infolog_count);

/*
 * glGetShaderiv wrapper. id = 68
 */
void yagl_host_glGetShaderiv(GLuint shader, GLenum pname, GLint *param);

/*
 * glGetShaderInfoLog wrapper. id = 69
 */
GLboolean yagl_host_glGetShaderInfoLog(GLuint shader, GLchar *infolog, int32_t infolog_maxcount, int32_t *infolog_count);

/*
 * glGetUniformfv wrapper. id = 70
 */
void yagl_host_glGetUniformfv(GLboolean tl, GLuint program, uint32_t location, GLfloat *params, int32_t params_maxcount, int32_t *params_count);

/*
 * glGetUniformiv wrapper. id = 71
 */
void yagl_host_glGetUniformiv(GLboolean tl, GLuint program, uint32_t location, GLint *params, int32_t params_maxcount, int32_t *params_count);

/*
 * glGetUniformLocation wrapper. id = 72
 */
int yagl_host_glGetUniformLocation(GLuint program, const GLchar *name, int32_t name_count);

/*
 * glGetVertexAttribfv wrapper. id = 73
 */
void yagl_host_glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params, int32_t params_maxcount, int32_t *params_count);

/*
 * glGetVertexAttribiv wrapper. id = 74
 */
void yagl_host_glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params, int32_t params_maxcount, int32_t *params_count);

/*
 * glLinkProgram wrapper. id = 75
 */
void yagl_host_glLinkProgram(GLuint program);

/*
 * glUniform1f wrapper. id = 76
 */
void yagl_host_glUniform1f(GLboolean tl, uint32_t location, GLfloat x);

/*
 * glUniform1fv wrapper. id = 77
 */
void yagl_host_glUniform1fv(GLboolean tl, uint32_t location, const GLfloat *v, int32_t v_count);

/*
 * glUniform1i wrapper. id = 78
 */
void yagl_host_glUniform1i(GLboolean tl, uint32_t location, GLint x);

/*
 * glUniform1iv wrapper. id = 79
 */
void yagl_host_glUniform1iv(GLboolean tl, uint32_t location, const GLint *v, int32_t v_count);

/*
 * glUniform2f wrapper. id = 80
 */
void yagl_host_glUniform2f(GLboolean tl, uint32_t location, GLfloat x, GLfloat y);

/*
 * glUniform2fv wrapper. id = 81
 */
void yagl_host_glUniform2fv(GLboolean tl, uint32_t location, const GLfloat *v, int32_t v_count);

/*
 * glUniform2i wrapper. id = 82
 */
void yagl_host_glUniform2i(GLboolean tl, uint32_t location, GLint x, GLint y);

/*
 * glUniform2iv wrapper. id = 83
 */
void yagl_host_glUniform2iv(GLboolean tl, uint32_t location, const GLint *v, int32_t v_count);

/*
 * glUniform3f wrapper. id = 84
 */
void yagl_host_glUniform3f(GLboolean tl, uint32_t location, GLfloat x, GLfloat y, GLfloat z);

/*
 * glUniform3fv wrapper. id = 85
 */
void yagl_host_glUniform3fv(GLboolean tl, uint32_t location, const GLfloat *v, int32_t v_count);

/*
 * glUniform3i wrapper. id = 86
 */
void yagl_host_glUniform3i(GLboolean tl, uint32_t location, GLint x, GLint y, GLint z);

/*
 * glUniform3iv wrapper. id = 87
 */
void yagl_host_glUniform3iv(GLboolean tl, uint32_t location, const GLint *v, int32_t v_count);

/*
 * glUniform4f wrapper. id = 88
 */
void yagl_host_glUniform4f(GLboolean tl, uint32_t location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

/*
 * glUniform4fv wrapper. id = 89
 */
void yagl_host_glUniform4fv(GLboolean tl, uint32_t location, const GLfloat *v, int32_t v_count);

/*
 * glUniform4i wrapper. id = 90
 */
void yagl_host_glUniform4i(GLboolean tl, uint32_t location, GLint x, GLint y, GLint z, GLint w);

/*
 * glUniform4iv wrapper. id = 91
 */
void yagl_host_glUniform4iv(GLboolean tl, uint32_t location, const GLint *v, int32_t v_count);

/*
 * glUniformMatrix2fv wrapper. id = 92
 */
void yagl_host_glUniformMatrix2fv(GLboolean tl, uint32_t location, GLboolean transpose, const GLfloat *value, int32_t value_count);

/*
 * glUniformMatrix3fv wrapper. id = 93
 */
void yagl_host_glUniformMatrix3fv(GLboolean tl, uint32_t location, GLboolean transpose, const GLfloat *value, int32_t value_count);

/*
 * glUniformMatrix4fv wrapper. id = 94
 */
void yagl_host_glUniformMatrix4fv(GLboolean tl, uint32_t location, GLboolean transpose, const GLfloat *value, int32_t value_count);

/*
 * glUseProgram wrapper. id = 95
 */
void yagl_host_glUseProgram(GLuint program);

/*
 * glValidateProgram wrapper. id = 96
 */
void yagl_host_glValidateProgram(GLuint program);

/*
 * glVertexAttrib1f wrapper. id = 97
 */
void yagl_host_glVertexAttrib1f(GLuint indx, GLfloat x);

/*
 * glVertexAttrib1fv wrapper. id = 98
 */
void yagl_host_glVertexAttrib1fv(GLuint indx, const GLfloat *values, int32_t values_count);

/*
 * glVertexAttrib2f wrapper. id = 99
 */
void yagl_host_glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y);

/*
 * glVertexAttrib2fv wrapper. id = 100
 */
void yagl_host_glVertexAttrib2fv(GLuint indx, const GLfloat *values, int32_t values_count);

/*
 * glVertexAttrib3f wrapper. id = 101
 */
void yagl_host_glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z);

/*
 * glVertexAttrib3fv wrapper. id = 102
 */
void yagl_host_glVertexAttrib3fv(GLuint indx, const GLfloat *values, int32_t values_count);

/*
 * glVertexAttrib4f wrapper. id = 103
 */
void yagl_host_glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

/*
 * glVertexAttrib4fv wrapper. id = 104
 */
void yagl_host_glVertexAttrib4fv(GLuint indx, const GLfloat *values, int32_t values_count);

/*
 * glGetIntegerv wrapper. id = 105
 */
void yagl_host_glGetIntegerv(GLenum pname, GLint *params, int32_t params_maxcount, int32_t *params_count);

/*
 * glGetFloatv wrapper. id = 106
 */
void yagl_host_glGetFloatv(GLenum pname, GLfloat *params, int32_t params_maxcount, int32_t *params_count);

/*
 * glGetString wrapper. id = 107
 */
void yagl_host_glGetString(GLenum name, GLchar *str, int32_t str_maxcount, int32_t *str_count);

/*
 * glIsEnabled wrapper. id = 108
 */
GLboolean yagl_host_glIsEnabled(GLenum cap);

/*
 * glDeleteObjects wrapper. id = 109
 */
void yagl_host_glDeleteObjects(const GLuint *objects, int32_t objects_count);

/*
 * glBlendEquation wrapper. id = 110
 */
void yagl_host_glBlendEquation(GLenum mode);

/*
 * glBlendEquationSeparate wrapper. id = 111
 */
void yagl_host_glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);

/*
 * glBlendFunc wrapper. id = 112
 */
void yagl_host_glBlendFunc(GLenum sfactor, GLenum dfactor);

/*
 * glBlendFuncSeparate wrapper. id = 113
 */
void yagl_host_glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

/*
 * glBlendColor wrapper. id = 114
 */
void yagl_host_glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

/*
 * glClear wrapper. id = 115
 */
void yagl_host_glClear(GLbitfield mask);

/*
 * glClearColor wrapper. id = 116
 */
void yagl_host_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

/*
 * glClearDepthf wrapper. id = 117
 */
void yagl_host_glClearDepthf(GLclampf depth);

/*
 * glClearStencil wrapper. id = 118
 */
void yagl_host_glClearStencil(GLint s);

/*
 * glColorMask wrapper. id = 119
 */
void yagl_host_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

/*
 * glCullFace wrapper. id = 120
 */
void yagl_host_glCullFace(GLenum mode);

/*
 * glDepthFunc wrapper. id = 121
 */
void yagl_host_glDepthFunc(GLenum func);

/*
 * glDepthMask wrapper. id = 122
 */
void yagl_host_glDepthMask(GLboolean flag);

/*
 * glDepthRangef wrapper. id = 123
 */
void yagl_host_glDepthRangef(GLclampf zNear, GLclampf zFar);

/*
 * glEnable wrapper. id = 124
 */
void yagl_host_glEnable(GLenum cap);

/*
 * glDisable wrapper. id = 125
 */
void yagl_host_glDisable(GLenum cap);

/*
 * glFlush wrapper. id = 126
 */
void yagl_host_glFlush();

/*
 * glFrontFace wrapper. id = 127
 */
void yagl_host_glFrontFace(GLenum mode);

/*
 * glGenerateMipmap wrapper. id = 128
 */
void yagl_host_glGenerateMipmap(GLenum target);

/*
 * glHint wrapper. id = 129
 */
void yagl_host_glHint(GLenum target, GLenum mode);

/*
 * glLineWidth wrapper. id = 130
 */
void yagl_host_glLineWidth(GLfloat width);

/*
 * glPixelStorei wrapper. id = 131
 */
void yagl_host_glPixelStorei(GLenum pname, GLint param);

/*
 * glPolygonOffset wrapper. id = 132
 */
void yagl_host_glPolygonOffset(GLfloat factor, GLfloat units);

/*
 * glScissor wrapper. id = 133
 */
void yagl_host_glScissor(GLint x, GLint y, GLsizei width, GLsizei height);

/*
 * glStencilFunc wrapper. id = 134
 */
void yagl_host_glStencilFunc(GLenum func, GLint ref, GLuint mask);

/*
 * glStencilMask wrapper. id = 135
 */
void yagl_host_glStencilMask(GLuint mask);

/*
 * glStencilOp wrapper. id = 136
 */
void yagl_host_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);

/*
 * glSampleCoverage wrapper. id = 137
 */
void yagl_host_glSampleCoverage(GLclampf value, GLboolean invert);

/*
 * glViewport wrapper. id = 138
 */
void yagl_host_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

/*
 * glStencilFuncSeparate wrapper. id = 139
 */
void yagl_host_glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);

/*
 * glStencilMaskSeparate wrapper. id = 140
 */
void yagl_host_glStencilMaskSeparate(GLenum face, GLuint mask);

/*
 * glStencilOpSeparate wrapper. id = 141
 */
void yagl_host_glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);

/*
 * glPointSize wrapper. id = 142
 */
void yagl_host_glPointSize(GLfloat size);

/*
 * glAlphaFunc wrapper. id = 143
 */
void yagl_host_glAlphaFunc(GLenum func, GLclampf ref);

/*
 * glMatrixMode wrapper. id = 144
 */
void yagl_host_glMatrixMode(GLenum mode);

/*
 * glLoadIdentity wrapper. id = 145
 */
void yagl_host_glLoadIdentity();

/*
 * glPopMatrix wrapper. id = 146
 */
void yagl_host_glPopMatrix();

/*
 * glPushMatrix wrapper. id = 147
 */
void yagl_host_glPushMatrix();

/*
 * glRotatef wrapper. id = 148
 */
void yagl_host_glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

/*
 * glTranslatef wrapper. id = 149
 */
void yagl_host_glTranslatef(GLfloat x, GLfloat y, GLfloat z);

/*
 * glScalef wrapper. id = 150
 */
void yagl_host_glScalef(GLfloat x, GLfloat y, GLfloat z);

/*
 * glOrthof wrapper. id = 151
 */
void yagl_host_glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);

/*
 * glColor4f wrapper. id = 152
 */
void yagl_host_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

/*
 * glColor4ub wrapper. id = 153
 */
void yagl_host_glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);

/*
 * glNormal3f wrapper. id = 154
 */
void yagl_host_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);

/*
 * glPointParameterf wrapper. id = 155
 */
void yagl_host_glPointParameterf(GLenum pname, GLfloat param);

/*
 * glPointParameterfv wrapper. id = 156
 */
void yagl_host_glPointParameterfv(GLenum pname, const GLfloat *params, int32_t params_count);

/*
 * glFogf wrapper. id = 157
 */
void yagl_host_glFogf(GLenum pname, GLfloat param);

/*
 * glFogfv wrapper. id = 158
 */
void yagl_host_glFogfv(GLenum pname, const GLfloat *params, int32_t params_count);

/*
 * glFrustumf wrapper. id = 159
 */
void yagl_host_glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);

/*
 * glLightf wrapper. id = 160
 */
void yagl_host_glLightf(GLenum light, GLenum pname, GLfloat param);

/*
 * glLightfv wrapper. id = 161
 */
void yagl_host_glLightfv(GLenum light, GLenum pname, const GLfloat *params, int32_t params_count);

/*
 * glGetLightfv wrapper. id = 162
 */
void yagl_host_glGetLightfv(GLenum light, GLenum pname, GLfloat *params, int32_t params_maxcount, int32_t *params_count);

/*
 * glLightModelf wrapper. id = 163
 */
void yagl_host_glLightModelf(GLenum pname, GLfloat param);

/*
 * glLightModelfv wrapper. id = 164
 */
void yagl_host_glLightModelfv(GLenum pname, const GLfloat *params, int32_t params_count);

/*
 * glMaterialf wrapper. id = 165
 */
void yagl_host_glMaterialf(GLenum face, GLenum pname, GLfloat param);

/*
 * glMaterialfv wrapper. id = 166
 */
void yagl_host_glMaterialfv(GLenum face, GLenum pname, const GLfloat *params, int32_t params_count);

/*
 * glGetMaterialfv wrapper. id = 167
 */
void yagl_host_glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params, int32_t params_maxcount, int32_t *params_count);

/*
 * glShadeModel wrapper. id = 168
 */
void yagl_host_glShadeModel(GLenum mode);

/*
 * glLogicOp wrapper. id = 169
 */
void yagl_host_glLogicOp(GLenum opcode);

/*
 * glMultMatrixf wrapper. id = 170
 */
void yagl_host_glMultMatrixf(const GLfloat *m, int32_t m_count);

/*
 * glLoadMatrixf wrapper. id = 171
 */
void yagl_host_glLoadMatrixf(const GLfloat *m, int32_t m_count);

/*
 * glClipPlanef wrapper. id = 172
 */
void yagl_host_glClipPlanef(GLenum plane, const GLfloat *equation, int32_t equation_count);

/*
 * glGetClipPlanef wrapper. id = 173
 */
void yagl_host_glGetClipPlanef(GLenum pname, GLfloat *eqn, int32_t eqn_maxcount, int32_t *eqn_count);

/*
 * glUpdateOffscreenImageYAGL wrapper. id = 174
 */
void yagl_host_glUpdateOffscreenImageYAGL(GLuint texture, uint32_t width, uint32_t height, uint32_t bpp, const void *pixels, int32_t pixels_count);

/*
 * glGenUniformLocationYAGL wrapper. id = 175
 */
void yagl_host_glGenUniformLocationYAGL(uint32_t location, GLuint program, const GLchar *name, int32_t name_count);

/*
 * glDeleteUniformLocationsYAGL wrapper. id = 176
 */
void yagl_host_glDeleteUniformLocationsYAGL(const uint32_t *locations, int32_t locations_count);

#endif
