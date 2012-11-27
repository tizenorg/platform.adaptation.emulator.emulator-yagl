#include "yagl_host_gles2_calls.h"
#include "yagl_impl.h"
#include "yagl_malloc.h"
#include "yagl_mem_gl.h"
#include "yagl_batch_gl.h"
#include "yagl_gles_context.h"
#include <GLES2/gl2ext.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yagl_gles_utils.h"

static void yagl_update_vbo()
{
    struct yagl_gles_context *ctx = yagl_gles_context_get();

    if (!ctx || ctx->vbo_valid) {
        return;
    }

    ctx->vbo = yagl_get_integer(GL_ARRAY_BUFFER_BINDING);
    ctx->vbo_valid = 1;
}

static GLint yagl_get_array_param(GLuint index, GLenum pname)
{
    GLint param = 0;

    do {
        yagl_mem_probe_write_GLint(&param);
    } while (!yagl_host_glGetVertexAttribiv(index,
                                            pname,
                                            &param));

    return param;
}

void yagl_update_arrays(void)
{
    struct yagl_gles_context *ctx = yagl_gles_context_get();

    if (!ctx) {
        return;
    }

    if (!ctx->arrays) {
        GLuint i;

        ctx->num_arrays = yagl_get_integer(GL_MAX_VERTEX_ATTRIBS);

        ctx->arrays = yagl_malloc0(sizeof(*ctx->arrays) * ctx->num_arrays);

        for (i = 0; i < ctx->num_arrays; ++i) {
            ctx->arrays[i].enabled = yagl_get_array_param(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED);
            ctx->arrays[i].vbo = yagl_get_array_param(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING);
            ctx->arrays[i].stride = yagl_get_array_param(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE);
            if (!ctx->arrays[i].vbo) {
                do {
                    yagl_mem_probe_write_ptr(&ctx->arrays[i].ptr);
                } while (!yagl_host_glGetVertexAttribPointerv(i,
                                                              GL_VERTEX_ATTRIB_ARRAY_POINTER,
                                                              &ctx->arrays[i].ptr));
            }
        }
    }
}

YAGL_IMPLEMENT_API_NORET2(glAttachShader, GLuint, GLuint, program, shader)

YAGL_API void glBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glBindAttribLocation, GLuint, GLuint, const GLchar*, program, index, name);
    while (!yagl_host_glBindAttribLocation(program, index, yagl_batch_put_GLchars(name))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET2(glBindFramebuffer, GLenum, GLuint, target, framebuffer)
YAGL_IMPLEMENT_API_NORET2(glBindRenderbuffer, GLenum, GLuint, target, renderbuffer)
YAGL_IMPLEMENT_API_NORET4(glBlendColor, GLclampf, GLclampf, GLclampf, GLclampf, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET1(glBlendEquation, GLenum, mode)
YAGL_IMPLEMENT_API_NORET2(glBlendEquationSeparate, GLenum, GLenum, modeRGB, modeAlpha)
YAGL_IMPLEMENT_API_NORET4(glBlendFuncSeparate, GLenum, GLenum, GLenum, GLenum, srcRGB, dstRGB, srcAlpha, dstAlpha)
YAGL_IMPLEMENT_API_RET1(GLenum, glCheckFramebufferStatus, GLenum, target)
YAGL_IMPLEMENT_API_NORET1(glCompileShader, GLuint, shader)
YAGL_IMPLEMENT_API_RET0(GLuint, glCreateProgram)
YAGL_IMPLEMENT_API_RET1(GLuint, glCreateShader, GLenum, type)

YAGL_API void glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteFramebuffers, GLsizei, const GLuint*, n, framebuffers);
    while (!yagl_host_glDeleteFramebuffers(n, yagl_batch_put_GLuints(framebuffers, n))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET1(glDeleteProgram, GLuint, program)

YAGL_API void glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteRenderbuffers, GLsizei, const GLuint*, n, renderbuffers);
    while (!yagl_host_glDeleteRenderbuffers(n, yagl_batch_put_GLuints(renderbuffers, n))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET1(glDeleteShader, GLuint, shader)
YAGL_IMPLEMENT_API_NORET2(glDetachShader, GLuint, GLuint, program, shader)

YAGL_API void glEnableVertexAttribArray(GLuint index)
{
    struct yagl_gles_context *ctx;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glEnableVertexAttribArray, GLuint, index);

    ctx = yagl_gles_context_get();

    if (ctx && ctx->arrays) {
        if (index < ctx->num_arrays) {
            ctx->arrays[index].enabled = 1;
        }
    }

    YAGL_HOST_CALL_ASSERT(yagl_host_glEnableVertexAttribArray(index));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDisableVertexAttribArray(GLuint index)
{
    struct yagl_gles_context *ctx;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glDisableVertexAttribArray, GLuint, index);

    ctx = yagl_gles_context_get();

    if (ctx && ctx->arrays) {
        if (index < ctx->num_arrays) {
            ctx->arrays[index].enabled = 0;
        }
    }

    YAGL_HOST_CALL_ASSERT(yagl_host_glDisableVertexAttribArray(index));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET4(glFramebufferRenderbuffer, GLenum, GLenum, GLenum, GLuint, target, attachment, renderbuffertarget, renderbuffer)
YAGL_IMPLEMENT_API_NORET5(glFramebufferTexture2D, GLenum, GLenum, GLenum, GLuint, GLint, target, attachment, textarget, texture, level)
YAGL_IMPLEMENT_API_NORET1(glGenerateMipmap, GLenum, target)

YAGL_API void glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenFramebuffers, GLsizei, GLuint*, n, framebuffers);

    do {
        yagl_mem_probe_write(framebuffers, sizeof(*framebuffers) * n);
    } while (!yagl_host_glGenFramebuffers(n, framebuffers));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenRenderbuffers, GLsizei, GLuint*, n, renderbuffers);

    do {
        yagl_mem_probe_write(renderbuffers, sizeof(*renderbuffers) * n);
    } while (!yagl_host_glGenRenderbuffers(n, renderbuffers));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    YAGL_LOG_FUNC_ENTER_SPLIT7(glGetActiveAttrib, GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*, program, index, bufsize, length, size, type, name);

    do {
        yagl_mem_probe_write_GLsizei(length);
        yagl_mem_probe_write_GLint(size);
        yagl_mem_probe_write_GLenum(type);
        yagl_mem_probe_write(name, bufsize);
    } while (!yagl_host_glGetActiveAttrib(program, index, bufsize, length, size, type, name));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    YAGL_LOG_FUNC_ENTER_SPLIT7(glGetActiveUniform, GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*, program, index, bufsize, length, size, type, name);

    do {
        yagl_mem_probe_write_GLsizei(length);
        yagl_mem_probe_write_GLint(size);
        yagl_mem_probe_write_GLenum(type);
        yagl_mem_probe_write(name, bufsize);
    } while (!yagl_host_glGetActiveUniform(program, index, bufsize, length, size, type, name));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetAttachedShaders, GLuint, GLsizei, GLsizei*, GLuint*, program, maxcount, count, shaders);

    do {
        yagl_mem_probe_write_GLsizei(count);
        yagl_mem_probe_write(shaders, sizeof(*shaders) * maxcount);
    } while (!yagl_host_glGetAttachedShaders(program, maxcount, count, shaders));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API int glGetAttribLocation(GLuint program, const GLchar* name)
{
    int retval;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetAttribLocation, GLuint, const GLchar*, program, name);

    do {
        yagl_mem_probe_read_GLchars(name);
    } while (!yagl_host_glGetAttribLocation(&retval, program, name));

    YAGL_LOG_FUNC_EXIT_SPLIT(int, retval);

    return retval;
}

YAGL_API void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetFramebufferAttachmentParameteriv, GLenum, GLenum, GLenum, GLint*, target, attachment, pname, params);

    do {
        yagl_mem_probe_write_GLint(params);
    } while (!yagl_host_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetProgramiv, GLuint, GLenum, GLint*, program, pname, params);

    do {
        yagl_mem_probe_write_GLint(params);
    } while (!yagl_host_glGetProgramiv(program, pname, params));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetProgramInfoLog, GLuint, GLsizei, GLsizei*, GLchar*, program, bufsize, length, infolog);

    do {
        yagl_mem_probe_write_GLsizei(length);
        yagl_mem_probe_write(infolog, bufsize);
    } while (!yagl_host_glGetProgramInfoLog(program, bufsize, length, infolog));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetRenderbufferParameteriv, GLenum, GLenum, GLint*, target, pname, params);

    do {
        yagl_mem_probe_write_GLint(params);
    } while (!yagl_host_glGetRenderbufferParameteriv(target, pname, params));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetShaderiv, GLuint, GLenum, GLint*, shader, pname, params);

    do {
        yagl_mem_probe_write_GLint(params);
    } while (!yagl_host_glGetShaderiv(shader, pname, params));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetShaderInfoLog, GLuint, GLsizei, GLsizei*, GLchar*, shader, bufsize, length, infolog);

    do {
        yagl_mem_probe_write_GLsizei(length);
        yagl_mem_probe_write(infolog, bufsize);
    } while (!yagl_host_glGetShaderInfoLog(shader, bufsize, length, infolog));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetShaderPrecisionFormat, GLenum, GLenum, GLint*, GLint*, shadertype, precisiontype, range, precision);

    do {
        yagl_mem_probe_write(range, 2 * sizeof(*range));
        yagl_mem_probe_write_GLint(precision);
    } while (!yagl_host_glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetShaderSource, GLuint, GLsizei, GLsizei*, GLchar*, shader, bufsize, length, source);

    do {
        yagl_mem_probe_write_GLsizei(length);
        yagl_mem_probe_write(source, bufsize);
    } while (!yagl_host_glGetShaderSource(shader, bufsize, length, source));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API const GLubyte* glGetString(GLenum name)
{
    struct yagl_gles_context *ctx;
    const char *str = NULL;

    YAGL_LOG_FUNC_ENTER(glGetString, "name = %d", name);

    ctx = yagl_gles_context_get();

    switch (name) {
    case GL_VENDOR:
        str = "Samsung";
        break;
    case GL_VERSION:
        str = "2.0";
        break;
    case GL_RENDERER:
        str = "YaGL GLESv2";
        break;
    case GL_SHADING_LANGUAGE_VERSION:
        str = "OpenGL ES GLSL ES 1.4";
        break;
    case GL_EXTENSIONS:
        if (ctx) {
            if (!ctx->extensions) {
                GLuint size = 0, tmp;
                YAGL_HOST_CALL_ASSERT(yagl_host_glGetExtensionStringYAGL(&size, NULL));
                ctx->extensions = yagl_malloc0(size);
                do {
                    yagl_mem_probe_write(ctx->extensions, size);
                } while (!yagl_host_glGetExtensionStringYAGL(&tmp, ctx->extensions));
            }
            str = ctx->extensions;
        } else {
            str = "";
        }
        break;
    default:
        str = "";
    }

    YAGL_LOG_FUNC_EXIT(NULL);

    return (const GLubyte*)str;
}

YAGL_API void glGetUniformfv(GLuint program, GLint location, GLfloat* params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetUniformfv, GLuint, GLint, GLfloat*, program, location, params);

    do {
        yagl_mem_probe_write_GLfloat(params);
    } while (!yagl_host_glGetUniformfv(program, location, params));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetUniformiv(GLuint program, GLint location, GLint* params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetUniformiv, GLuint, GLint, GLint*, program, location, params);

    do {
        yagl_mem_probe_write_GLint(params);
    } while (!yagl_host_glGetUniformiv(program, location, params));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API int glGetUniformLocation(GLuint program, const GLchar* name)
{
    int retval;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetUniformLocation, GLuint, const GLchar*, program, name);

    do {
        yagl_mem_probe_read_GLchars(name);
    } while (!yagl_host_glGetUniformLocation(&retval, program, name));

    YAGL_LOG_FUNC_EXIT_SPLIT(int, retval);

    return retval;
}

YAGL_API void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetVertexAttribfv, GLuint, GLenum, GLfloat*, index, pname, params);

    do {
        yagl_mem_probe_write_GLfloat(params);
    } while (!yagl_host_glGetVertexAttribfv(index, pname, params));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetVertexAttribiv, GLuint, GLenum, GLint*, index, pname, params);

    do {
        yagl_mem_probe_write_GLint(params);
    } while (!yagl_host_glGetVertexAttribiv(index, pname, params));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid** pointer)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetVertexAttribPointerv, GLuint, GLenum, GLvoid**, index, pname, pointer);

    do {
        yagl_mem_probe_write(pointer, sizeof(*pointer));
    } while (!yagl_host_glGetVertexAttribPointerv(index, pname, pointer));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_RET1(GLboolean, glIsFramebuffer, GLuint, framebuffer)
YAGL_IMPLEMENT_API_RET1(GLboolean, glIsProgram, GLuint, program)
YAGL_IMPLEMENT_API_RET1(GLboolean, glIsRenderbuffer, GLuint, renderbuffer)
YAGL_IMPLEMENT_API_RET1(GLboolean, glIsShader, GLuint, shader)
YAGL_IMPLEMENT_API_NORET1(glLinkProgram, GLuint, program)
YAGL_IMPLEMENT_API_NORET0(glReleaseShaderCompiler)
YAGL_IMPLEMENT_API_NORET4(glRenderbufferStorage, GLenum, GLenum, GLsizei, GLsizei, target, internalformat, width, height)

YAGL_API void glShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glShaderBinary, GLsizei, const GLuint*, GLenum, const GLvoid*, GLsizei, n, shaders, binaryformat, binary, length);

    /*
     * This is not implemented on the host.
     */

    yagl_host_glShaderBinary(n, shaders, binaryformat, binary, length);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glShaderSource, GLuint, GLsizei, const GLchar**, const GLint*, shader, count, string, length);

    /*
     * We'll just break the batch here, we don't care, this function
     * is not something that gets called every frame.
     */

    do {
        int i;
        yagl_mem_probe_read(string, sizeof(*string) * count);
        yagl_mem_probe_read(length, sizeof(*length) * count);
        for (i = 0; i < count; ++i) {
            if (length && (length[i] >= 0)) {
                yagl_mem_probe_read(string[i], length[i]);
            } else {
                yagl_mem_probe_read_GLchars(string[i]);
            }
        }
    } while (!yagl_host_glShaderSource(shader, count, string, length) ||
             !yagl_batch_sync());

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET4(glStencilFuncSeparate, GLenum, GLenum, GLint, GLuint, face, func, ref, mask)
YAGL_IMPLEMENT_API_NORET2(glStencilMaskSeparate, GLenum, GLuint, face, mask)
YAGL_IMPLEMENT_API_NORET4(glStencilOpSeparate, GLenum, GLenum, GLenum, GLenum, face, fail, zfail, zpass)
YAGL_IMPLEMENT_API_NORET2(glUniform1f, GLint, GLfloat, location, x)

YAGL_API void glUniform1fv(GLint location, GLsizei count, const GLfloat* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform1fv, GLint, GLsizei, const GLfloat*, location, count, v);
    while (!yagl_host_glUniform1fv(location, count, yagl_batch_put_GLfloats(v, count))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET2(glUniform1i, GLint, GLint, location, x)

YAGL_API void glUniform1iv(GLint location, GLsizei count, const GLint* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform1iv, GLint, GLsizei, const GLint*, location, count, v);
    while (!yagl_host_glUniform1iv(location, count, yagl_batch_put_GLints(v, count))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET3(glUniform2f, GLint, GLfloat, GLfloat, location, x, y)

YAGL_API void glUniform2fv(GLint location, GLsizei count, const GLfloat* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform2fv, GLint, GLsizei, const GLfloat*, location, count, v);
    while (!yagl_host_glUniform2fv(location, count, yagl_batch_put_GLfloats(v, 2 * count))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET3(glUniform2i, GLint, GLint, GLint, location, x, y)

YAGL_API void glUniform2iv(GLint location, GLsizei count, const GLint* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform2iv, GLint, GLsizei, const GLint*, location, count, v);
    while (!yagl_host_glUniform2iv(location, count, yagl_batch_put_GLints(v, 2 * count))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET4(glUniform3f, GLint, GLfloat, GLfloat, GLfloat, location, x, y, z)

YAGL_API void glUniform3fv(GLint location, GLsizei count, const GLfloat* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform3fv, GLint, GLsizei, const GLfloat*, location, count, v);
    while (!yagl_host_glUniform3fv(location, count, yagl_batch_put_GLfloats(v, 3 * count))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET4(glUniform3i, GLint, GLint, GLint, GLint, location, x, y, z)

YAGL_API void glUniform3iv(GLint location, GLsizei count, const GLint* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform3iv, GLint, GLsizei, const GLint*, location, count, v);
    while (!yagl_host_glUniform3iv(location, count, yagl_batch_put_GLints(v, 3 * count))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET5(glUniform4f, GLint, GLfloat, GLfloat, GLfloat, GLfloat, location, x, y, z, w)

YAGL_API void glUniform4fv(GLint location, GLsizei count, const GLfloat* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform4fv, GLint, GLsizei, const GLfloat*, location, count, v);
    while (!yagl_host_glUniform4fv(location, count, yagl_batch_put_GLfloats(v, 4 * count))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET5(glUniform4i, GLint, GLint, GLint, GLint, GLint, location, x, y, z, w)

YAGL_API void glUniform4iv(GLint location, GLsizei count, const GLint* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform4iv, GLint, GLsizei, const GLint*, location, count, v);
    while (!yagl_host_glUniform4iv(location, count, yagl_batch_put_GLints(v, 4 * count))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix2fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);
    while (!yagl_host_glUniformMatrix2fv(location, count, transpose, yagl_batch_put_GLfloats(value, 2 * 2 * count))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix3fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);
    while (!yagl_host_glUniformMatrix3fv(location, count, transpose, yagl_batch_put_GLfloats(value, 3 * 3 * count))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix4fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);
    while (!yagl_host_glUniformMatrix4fv(location, count, transpose, yagl_batch_put_GLfloats(value, 4 * 4 * count))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET1(glUseProgram, GLuint, program)
YAGL_IMPLEMENT_API_NORET1(glValidateProgram, GLuint, program)
YAGL_IMPLEMENT_API_NORET2(glVertexAttrib1f, GLuint, GLfloat, indx, x)

YAGL_API void glVertexAttrib1fv(GLuint indx, const GLfloat* values)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glVertexAttrib1fv, GLuint, const GLfloat*, indx, values);
    while (!yagl_host_glVertexAttrib1fv(indx, yagl_batch_put_GLfloats(values, 1))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET3(glVertexAttrib2f, GLuint, GLfloat, GLfloat, indx, x, y)

YAGL_API void glVertexAttrib2fv(GLuint indx, const GLfloat* values)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glVertexAttrib2fv, GLuint, const GLfloat*, indx, values);
    while (!yagl_host_glVertexAttrib2fv(indx, yagl_batch_put_GLfloats(values, 2))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET4(glVertexAttrib3f, GLuint, GLfloat, GLfloat, GLfloat, indx, x, y, z)

YAGL_API void glVertexAttrib3fv(GLuint indx, const GLfloat* values)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glVertexAttrib3fv, GLuint, const GLfloat*, indx, values);
    while (!yagl_host_glVertexAttrib3fv(indx, yagl_batch_put_GLfloats(values, 3))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET5(glVertexAttrib4f, GLuint, GLfloat, GLfloat, GLfloat, GLfloat, indx, x, y, z, w)

YAGL_API void glVertexAttrib4fv(GLuint indx, const GLfloat* values)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glVertexAttrib4fv, GLuint, const GLfloat*, indx, values);
    while (!yagl_host_glVertexAttrib4fv(indx, yagl_batch_put_GLfloats(values, 4))) {}
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
{
    struct yagl_gles_context *ctx;
    int el_size = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT6(glVertexAttribPointer, GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*, indx, size, type, normalized, stride, ptr);

    ctx = yagl_gles_context_get();

    if (ctx && ctx->arrays) {
        if ((indx < ctx->num_arrays) &&
            yagl_get_el_size(type, &el_size)) {
            yagl_update_vbo();
            ctx->arrays[indx].vbo = 0;
            ctx->arrays[indx].stride = 0;
            ctx->arrays[indx].ptr = NULL;
            if (ctx->vbo) {
                ctx->arrays[indx].vbo = ctx->vbo;
            } else {
                if (stride) {
                    ctx->arrays[indx].stride = stride;
                } else {
                    ctx->arrays[indx].stride = size * el_size;
                }
                ctx->arrays[indx].ptr = (GLvoid*)ptr;
            }
        }
    }

    YAGL_HOST_CALL_ASSERT(yagl_host_glVertexAttribPointer(indx, size, type, normalized, stride, ptr));

    YAGL_LOG_FUNC_EXIT(NULL);
}
