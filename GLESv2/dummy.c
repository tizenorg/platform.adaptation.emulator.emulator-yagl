#include "GLES3/gl3.h"
#include "yagl_export.h"
#include <stdlib.h>

YAGL_API void glReadBuffer(GLenum mode)
{
    exit(5);
}

YAGL_API void glDrawRangeElements(GLenum mode, GLuint start, GLuint end,
                                  GLsizei count, GLenum type,
                                  const void *indices)
{
    exit(5);
}

YAGL_API void glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64 *params)
{
    exit(5);
}

YAGL_API void glGetIntegeri_v(GLenum target, GLuint index, GLint *data)
{
    exit(5);
}

YAGL_API void glGetInteger64i_v(GLenum target, GLuint index, GLint64 *data)
{
    exit(5);
}

YAGL_API void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    exit(5);
}

YAGL_API void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
    exit(5);
}

YAGL_API void glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value)
{
    exit(5);
}

YAGL_API void glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    exit(5);
}

YAGL_API void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    exit(5);
}

YAGL_API void glGetInteger64v(GLenum pname, GLint64 *params)
{
    exit(5);
}

YAGL_API GLint glGetFragDataLocation(GLuint program, const GLchar *name)
{
    exit(5);
    return 0;
}

YAGL_API void glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)
{
    exit(5);
}

YAGL_API void glProgramBinary(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length)
{
    exit(5);
}

YAGL_API GLsync glFenceSync(GLenum condition, GLbitfield flags)
{
    exit(5);
    return 0;
}

YAGL_API GLboolean glIsSync(GLsync sync)
{
    exit(5);
    return 0;
}

YAGL_API void glDeleteSync(GLsync sync)
{
    exit(5);
}

YAGL_API GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    exit(5);
    return 0;
}

YAGL_API void glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    exit(5);
}

YAGL_API void glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
    exit(5);
}

YAGL_API void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    exit(5);
}

YAGL_API void glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    exit(5);
}

YAGL_API void glGenSamplers(GLsizei count, GLuint *samplers)
{
    exit(5);
}

YAGL_API void glDeleteSamplers(GLsizei count, const GLuint *samplers)
{
    exit(5);
}

YAGL_API GLboolean glIsSampler(GLuint sampler)
{
    exit(5);
    return 0;
}

YAGL_API void glBindSampler(GLuint unit, GLuint sampler)
{
    exit(5);
}

YAGL_API void glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
    exit(5);
}

YAGL_API void glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint *param)
{
    exit(5);
}

YAGL_API void glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
    exit(5);
}

YAGL_API void glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *param)
{
    exit(5);
}

YAGL_API void glGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params)
{
    exit(5);
}

YAGL_API void glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value)
{
    exit(5);
}

YAGL_API void glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params)
{
    exit(5);
}

YAGL_API void glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    exit(5);
}

YAGL_API void glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    exit(5);
}

YAGL_API void glVertexAttribI4iv(GLuint index, const GLint *v)
{
    exit(5);
}

YAGL_API void glVertexAttribI4uiv(GLuint index, const GLuint *v)
{
    exit(5);
}

YAGL_API void glGetUniformuiv(GLuint program, GLint location, GLuint *params)
{
    exit(5);
}

YAGL_API void glUniform2ui(GLint location, GLuint v0, GLuint v1)
{
    exit(5);
}

YAGL_API void glUniform2uiv(GLint location, GLsizei count, const GLuint *value)
{
    exit(5);
}

YAGL_API const GLubyte *glGetStringi(GLenum name, GLuint index)
{
    exit(5);
    return NULL;
}

YAGL_API void glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    exit(5);
}

YAGL_API void glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params)
{
    exit(5);
}

YAGL_API void glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params)
{
    exit(5);
}

YAGL_API void glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    exit(5);
}

YAGL_API void glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
    exit(5);
}
