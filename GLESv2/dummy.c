#include "GLES3/gl3.h"
#include "yagl_export.h"
#include <stdlib.h>
#include <assert.h>

YAGL_API void glDrawRangeElements(GLenum mode, GLuint start, GLuint end,
                                  GLsizei count, GLenum type,
                                  const void *indices)
{
    assert(0);
    exit(5);
}

YAGL_API GLint glGetFragDataLocation(GLuint program, const GLchar *name)
{
    assert(0);
    exit(5);
    return 0;
}

YAGL_API void glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)
{
    assert(0);
    exit(5);
}

YAGL_API void glProgramBinary(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length)
{
    assert(0);
    exit(5);
}

YAGL_API void glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    assert(0);
    exit(5);
}

YAGL_API void glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
    assert(0);
    exit(5);
}

YAGL_API void glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params)
{
    assert(0);
    exit(5);
}
