#include "yagl_host_gles2_calls.h"
#include "yagl_impl.h"
#include "yagl_malloc.h"
#include "yagl_gles_context.h"
#include "GLES2/gl2ext.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yagl_gles_utils.h"
#include "yagl_transport.h"
#include "yagl_state.h"

static GLint yagl_get_array_param(GLuint index, GLenum pname)
{
    GLint param = 0;

    yagl_host_glGetVertexAttribiv(index, pname, &param, 1, NULL);

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
                yagl_host_glGetVertexAttribPointerv(i,
                                                    GL_VERTEX_ATTRIB_ARRAY_POINTER,
                                                    &ctx->arrays[i].ptr);
            }
        }
    }
}

YAGL_IMPLEMENT_API_NORET2(glAttachShader, GLuint, GLuint, program, shader)

YAGL_API void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glBindAttribLocation, GLuint, GLuint, const GLchar*, program, index, name);
    yagl_host_glBindAttribLocation(program, index, name, yagl_transport_string_count(name));
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET4(glBlendColor, GLclampf, GLclampf, GLclampf, GLclampf, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET1(glCompileShader, GLuint, shader)
YAGL_IMPLEMENT_API_RET0(GLuint, glCreateProgram)
YAGL_IMPLEMENT_API_RET1(GLuint, glCreateShader, GLenum, type)
YAGL_IMPLEMENT_API_NORET1(glDeleteProgram, GLuint, program)
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

    yagl_host_glEnableVertexAttribArray(index);

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

    yagl_host_glDisableVertexAttribArray(index);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    int32_t tmp = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT7(glGetActiveAttrib, GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*, program, index, bufsize, length, size, type, name);
    yagl_host_glGetActiveAttrib(program, index, size, type, name, bufsize, &tmp);
    if (length && (tmp > 0)) {
        *length = tmp - 1;
    }
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    int32_t tmp = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT7(glGetActiveUniform, GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*, program, index, bufsize, length, size, type, name);
    yagl_host_glGetActiveUniform(program, index, size, type, name, bufsize, &tmp);
    if (length && (tmp > 0)) {
        *length = tmp - 1;
    }
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei *count, GLuint *shaders)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetAttachedShaders, GLuint, GLsizei, GLsizei*, GLuint*, program, maxcount, count, shaders);
    yagl_host_glGetAttachedShaders(program, shaders, maxcount, count);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API int glGetAttribLocation(GLuint program, const GLchar *name)
{
    int ret;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetAttribLocation, GLuint, const GLchar*, program, name);
    ret = yagl_host_glGetAttribLocation(program, name, yagl_transport_string_count(name));
    YAGL_LOG_FUNC_EXIT_SPLIT(int, ret);

    return ret;
}

YAGL_API void glGetProgramiv(GLuint program, GLenum pname, GLint *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetProgramiv, GLuint, GLenum, GLint*, program, pname, params);
    yagl_host_glGetProgramiv(program, pname, params);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei *length, GLchar *infolog)
{
    int32_t tmp = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetProgramInfoLog, GLuint, GLsizei, GLsizei*, GLchar*, program, bufsize, length, infolog);
    yagl_host_glGetProgramInfoLog(program, infolog, bufsize, &tmp);
    if (length && (tmp > 0)) {
        *length = tmp - 1;
    }
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetShaderiv(GLuint shader, GLenum pname, GLint *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetShaderiv, GLuint, GLenum, GLint*, shader, pname, params);
    yagl_host_glGetShaderiv(shader, pname, params);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *infolog)
{
    int32_t tmp = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetShaderInfoLog, GLuint, GLsizei, GLsizei*, GLchar*, shader, bufsize, length, infolog);
    yagl_host_glGetShaderInfoLog(shader, infolog, bufsize, &tmp);
    if (length && (tmp > 0)) {
        *length = tmp - 1;
    }
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetShaderPrecisionFormat, GLenum, GLenum, GLint*, GLint*, shadertype, precisiontype, range, precision);
    yagl_host_glGetShaderPrecisionFormat(shadertype, precisiontype, range, 2, NULL, precision);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *source)
{
    int32_t tmp = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetShaderSource, GLuint, GLsizei, GLsizei*, GLchar*, shader, bufsize, length, source);
    yagl_host_glGetShaderSource(shader, source, bufsize, &tmp);
    if (length && (tmp > 0)) {
        *length = tmp - 1;
    }
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API const GLubyte *glGetString(GLenum name)
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
        str = "OpenGL ES 2.0";
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
                int32_t size = 0;
                yagl_host_glGetExtensionStringYAGL(NULL, 0, &size);
                ctx->extensions = yagl_malloc0(size);
                yagl_host_glGetExtensionStringYAGL(ctx->extensions, size, NULL);
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

YAGL_API void glGetUniformfv(GLuint program, GLint location, GLfloat *params)
{
    GLfloat tmp[100]; // This fits all cases.
    int32_t num = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetUniformfv, GLuint, GLint, GLfloat*, program, location, params);
    yagl_host_glGetUniformfv(program, location, tmp, sizeof(tmp)/sizeof(tmp[0]), &num);
    if (params) {
        memcpy(params, tmp, num * sizeof(tmp[0]));
    }
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetUniformiv(GLuint program, GLint location, GLint *params)
{
    GLint tmp[100]; // This fits all cases.
    int32_t num = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetUniformiv, GLuint, GLint, GLint*, program, location, params);
    yagl_host_glGetUniformiv(program, location, tmp, sizeof(tmp)/sizeof(tmp[0]), &num);
    if (params) {
        memcpy(params, tmp, num * sizeof(tmp[0]));
    }
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API int glGetUniformLocation(GLuint program, const GLchar *name)
{
    int ret;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetUniformLocation, GLuint, const GLchar*, program, name);
    ret = yagl_host_glGetUniformLocation(program, name, yagl_transport_string_count(name));
    YAGL_LOG_FUNC_EXIT_SPLIT(int, ret);

    return ret;
}

YAGL_API void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
    GLfloat tmp[100]; // This fits all cases.
    int32_t num = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetVertexAttribfv, GLuint, GLenum, GLfloat*, index, pname, params);
    yagl_host_glGetVertexAttribfv(index, pname, tmp, sizeof(tmp)/sizeof(tmp[0]), &num);
    if (params) {
        memcpy(params, tmp, num * sizeof(tmp[0]));
    }
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
    GLint tmp[100]; // This fits all cases.
    int32_t num = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetVertexAttribiv, GLuint, GLenum, GLint*, index, pname, params);
    yagl_host_glGetVertexAttribiv(index, pname, tmp, sizeof(tmp)/sizeof(tmp[0]), &num);
    if (params) {
        memcpy(params, tmp, num * sizeof(tmp[0]));
    }
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid **pointer)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetVertexAttribPointerv, GLuint, GLenum, GLvoid**, index, pname, pointer);
    yagl_host_glGetVertexAttribPointerv(index, pname, pointer);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_RET1(GLboolean, glIsProgram, GLuint, program)
YAGL_IMPLEMENT_API_RET1(GLboolean, glIsShader, GLuint, shader)
YAGL_IMPLEMENT_API_NORET1(glLinkProgram, GLuint, program)
YAGL_IMPLEMENT_API_NORET0(glReleaseShaderCompiler)

YAGL_API void glShaderBinary(GLsizei n, const GLuint *shaders, GLenum binaryformat, const GLvoid *binary, GLsizei length)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glShaderBinary, GLsizei, const GLuint*, GLenum, const GLvoid*, GLsizei, n, shaders, binaryformat, binary, length);

    /*
     * This is not implemented on the host.
     */

    yagl_host_glShaderBinary(shaders, n, binaryformat, binary, length);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length)
{
    int have_strings = 0;
    uint32_t total_length = 0;
    uint8_t *tmp_buff = NULL;
    int i;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glShaderSource, GLuint, GLsizei, const GLchar**, const GLint*, shader, count, string, length);

    if (string) {
        for (i = 0; i < count; ++i) {
            if (string[i]) {
                if (length && (length[i] >= 0)) {
                    total_length += length[i];
                    have_strings = 1;
                } else {
                    total_length += strlen(string[i]);
                    have_strings = 1;
                }
            }
        }
    }

    if (have_strings) {
        ++total_length;

        tmp_buff = yagl_get_tmp_buffer(total_length);
        tmp_buff[0] = '\0';

        for (i = 0; i < count; ++i) {
            if (string[i]) {
                if (length && (length[i] >= 0)) {
                    strncat((char*)tmp_buff, string[i], length[i]);
                } else {
                    strcat((char*)tmp_buff, string[i]);
                }
            }
        }

        tmp_buff[total_length - 1] = '\0';
    }

    yagl_host_glShaderSource(shader,
                             (const GLchar*)tmp_buff,
                             ((count < 0) ? -1 : total_length));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET4(glStencilFuncSeparate, GLenum, GLenum, GLint, GLuint, face, func, ref, mask)
YAGL_IMPLEMENT_API_NORET2(glStencilMaskSeparate, GLenum, GLuint, face, mask)
YAGL_IMPLEMENT_API_NORET4(glStencilOpSeparate, GLenum, GLenum, GLenum, GLenum, face, fail, zfail, zpass)
YAGL_IMPLEMENT_API_NORET2(glUniform1f, GLint, GLfloat, location, x)

YAGL_API void glUniform1fv(GLint location, GLsizei count, const GLfloat* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform1fv, GLint, GLsizei, const GLfloat*, location, count, v);
    yagl_host_glUniform1fv(location, v, count);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET2(glUniform1i, GLint, GLint, location, x)

YAGL_API void glUniform1iv(GLint location, GLsizei count, const GLint* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform1iv, GLint, GLsizei, const GLint*, location, count, v);
    yagl_host_glUniform1iv(location, v, count);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET3(glUniform2f, GLint, GLfloat, GLfloat, location, x, y)

YAGL_API void glUniform2fv(GLint location, GLsizei count, const GLfloat* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform2fv, GLint, GLsizei, const GLfloat*, location, count, v);
    yagl_host_glUniform2fv(location, v, 2 * count);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET3(glUniform2i, GLint, GLint, GLint, location, x, y)

YAGL_API void glUniform2iv(GLint location, GLsizei count, const GLint* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform2iv, GLint, GLsizei, const GLint*, location, count, v);
    yagl_host_glUniform2iv(location, v, 2 * count);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET4(glUniform3f, GLint, GLfloat, GLfloat, GLfloat, location, x, y, z)

YAGL_API void glUniform3fv(GLint location, GLsizei count, const GLfloat* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform3fv, GLint, GLsizei, const GLfloat*, location, count, v);
    yagl_host_glUniform3fv(location, v, 3 * count);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET4(glUniform3i, GLint, GLint, GLint, GLint, location, x, y, z)

YAGL_API void glUniform3iv(GLint location, GLsizei count, const GLint* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform3iv, GLint, GLsizei, const GLint*, location, count, v);
    yagl_host_glUniform3iv(location, v, 3 * count);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET5(glUniform4f, GLint, GLfloat, GLfloat, GLfloat, GLfloat, location, x, y, z, w)

YAGL_API void glUniform4fv(GLint location, GLsizei count, const GLfloat* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform4fv, GLint, GLsizei, const GLfloat*, location, count, v);
    yagl_host_glUniform4fv(location, v, 4 * count);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET5(glUniform4i, GLint, GLint, GLint, GLint, GLint, location, x, y, z, w)

YAGL_API void glUniform4iv(GLint location, GLsizei count, const GLint* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform4iv, GLint, GLsizei, const GLint*, location, count, v);
    yagl_host_glUniform4iv(location, v, 4 * count);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix2fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);
    yagl_host_glUniformMatrix2fv(location, transpose, value, 2 * 2 * count);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix3fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);
    yagl_host_glUniformMatrix3fv(location, transpose, value, 3 * 3 * count);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix4fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);
    yagl_host_glUniformMatrix4fv(location, transpose, value, 4 * 4 * count);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET1(glUseProgram, GLuint, program)
YAGL_IMPLEMENT_API_NORET1(glValidateProgram, GLuint, program)
YAGL_IMPLEMENT_API_NORET2(glVertexAttrib1f, GLuint, GLfloat, indx, x)

YAGL_API void glVertexAttrib1fv(GLuint indx, const GLfloat* values)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glVertexAttrib1fv, GLuint, const GLfloat*, indx, values);
    yagl_host_glVertexAttrib1fv(indx, values, 1);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET3(glVertexAttrib2f, GLuint, GLfloat, GLfloat, indx, x, y)

YAGL_API void glVertexAttrib2fv(GLuint indx, const GLfloat* values)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glVertexAttrib2fv, GLuint, const GLfloat*, indx, values);
    yagl_host_glVertexAttrib2fv(indx, values, 2);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET4(glVertexAttrib3f, GLuint, GLfloat, GLfloat, GLfloat, indx, x, y, z)

YAGL_API void glVertexAttrib3fv(GLuint indx, const GLfloat* values)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glVertexAttrib3fv, GLuint, const GLfloat*, indx, values);
    yagl_host_glVertexAttrib3fv(indx, values, 3);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET5(glVertexAttrib4f, GLuint, GLfloat, GLfloat, GLfloat, GLfloat, indx, x, y, z, w)

YAGL_API void glVertexAttrib4fv(GLuint indx, const GLfloat* values)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glVertexAttrib4fv, GLuint, const GLfloat*, indx, values);
    yagl_host_glVertexAttrib4fv(indx, values, 4);
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

    yagl_host_glVertexAttribPointer(indx, size, type, normalized, stride, ptr);

    YAGL_LOG_FUNC_EXIT(NULL);
}
