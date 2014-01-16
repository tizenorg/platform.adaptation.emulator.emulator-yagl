#include "GLES2/gl2.h"
#include "yagl_host_gles_calls.h"
#include "yagl_gles2_program.h"
#include "yagl_gles2_shader.h"
#include "yagl_gles2_validate.h"
#include "yagl_gles2_context.h"
#include "yagl_gles_vertex_array.h"
#include "yagl_gles_array.h"
#include "yagl_gles_buffer.h"
#include "yagl_gles_utils.h"
#include "yagl_impl.h"
#include "yagl_malloc.h"
#include "yagl_sharegroup.h"
#include "yagl_transport.h"
#include "yagl_utils.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define YAGL_SET_ERR(err) \
    yagl_gles_context_set_error(&ctx->base, err); \
    YAGL_LOG_ERROR("error = 0x%X", err)

#define YAGL_GET_CTX_IMPL(ret_expr) \
    struct yagl_gles2_context *ctx = \
        (struct yagl_gles2_context*)yagl_get_client_context(); \
    if (!ctx || ((ctx->base.base.client_api & (yagl_client_api_gles2|yagl_client_api_gles3)) == 0)) { \
        YAGL_LOG_WARN("no current context"); \
        YAGL_LOG_FUNC_EXIT(NULL); \
        ret_expr; \
    }

#define YAGL_GET_CTX_RET(ret) YAGL_GET_CTX_IMPL(return ret)

#define YAGL_GET_CTX() YAGL_GET_CTX_IMPL(return)

/*
 * TODO: Passthrough for now.
 * @{
 */

YAGL_IMPLEMENT_API_NORET4(glStencilFuncSeparate, GLenum, GLenum, GLint, GLuint, face, func, ref, mask)
YAGL_IMPLEMENT_API_NORET2(glStencilMaskSeparate, GLenum, GLuint, face, mask)
YAGL_IMPLEMENT_API_NORET4(glStencilOpSeparate, GLenum, GLenum, GLenum, GLenum, face, fail, zfail, zpass)

/*
 * @}
 */

YAGL_API void glAttachShader(GLuint program, GLuint shader)
{
    struct yagl_gles2_program *program_obj = NULL;
    struct yagl_gles2_shader *shader_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glAttachShader, GLuint, GLuint, program, shader);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    shader_obj = (struct yagl_gles2_shader*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, shader);

    if (!shader_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!shader_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_attach_shader(program_obj, shader_obj)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    yagl_gles2_shader_release(shader_obj);
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glBindAttribLocation, GLuint, GLuint, const GLchar*, program, index, name);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (index >= ctx->base.num_arrays) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (name) {
        if (strncmp(name, "gl_", 3) == 0) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }

        YAGL_LOG_TRACE("binding attrib %s location to %d", name, index);
    }

    yagl_host_glBindAttribLocation(program_obj->global_name,
                                   index,
                                   name,
                                   yagl_transport_string_count(name));

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glBlendColor(GLclampf red,
                           GLclampf green,
                           GLclampf blue,
                           GLclampf alpha)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glBlendColor, GLclampf, GLclampf, GLclampf, GLclampf, red, green, blue, alpha);

    YAGL_GET_CTX();

    ctx->blend_color[0] = yagl_clampf(red);
    ctx->blend_color[1] = yagl_clampf(green);
    ctx->blend_color[2] = yagl_clampf(blue);
    ctx->blend_color[3] = yagl_clampf(alpha);

    yagl_host_glBlendColor(red, green, blue, alpha);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glCompileShader(GLuint shader)
{
    struct yagl_gles2_shader *shader_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glCompileShader, GLuint, shader);

    YAGL_GET_CTX();

    shader_obj = (struct yagl_gles2_shader*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, shader);

    if (!shader_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!shader_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_host_glCompileShader(shader_obj->global_name);

out:
    yagl_gles2_shader_release(shader_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API GLuint glCreateProgram(void)
{
    GLuint res = 0;
    struct yagl_gles2_program *program = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT0(glCreateProgram);

    YAGL_GET_CTX_RET(0);

    program = yagl_gles2_program_create(ctx->gen_locations);

    if (!program) {
        goto out;
    }

    yagl_sharegroup_add(ctx->sg, YAGL_NS_SHADER_PROGRAM, &program->base);
    res = program->base.local_name;

out:
    yagl_gles2_program_release(program);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLuint, res);

    return res;
}

YAGL_API GLuint glCreateShader(GLenum type)
{
    GLuint res = 0;
    struct yagl_gles2_shader *shader = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glCreateShader, GLenum, type);

    YAGL_GET_CTX_RET(0);

    if (yagl_gles2_is_shader_type_valid(type)) {
        shader = yagl_gles2_shader_create(type);

        if (!shader) {
            goto out;
        }

        yagl_sharegroup_add(ctx->sg, YAGL_NS_SHADER_PROGRAM, &shader->base);
        res = shader->base.local_name;
    } else {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    }

out:
    yagl_gles2_shader_release(shader);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLuint, res);

    return res;
}

YAGL_API void glDeleteProgram(GLuint program)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glDeleteProgram, GLuint, program);

    YAGL_GET_CTX();

    if (program == 0) {
        goto out;
    }

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        goto out;
    }

    yagl_gles2_context_unuse_program(ctx, program_obj);

    yagl_sharegroup_remove(ctx->sg,
                           YAGL_NS_SHADER_PROGRAM,
                           program);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDeleteShader(GLuint shader)
{
    struct yagl_gles2_shader *shader_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glDeleteShader, GLuint, shader);

    YAGL_GET_CTX();

    if (shader == 0) {
        goto out;
    }

    shader_obj = (struct yagl_gles2_shader*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, shader);

    if (!shader_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!shader_obj->is_shader) {
        goto out;
    }

    yagl_sharegroup_remove(ctx->sg,
                           YAGL_NS_SHADER_PROGRAM,
                           shader);

out:
    yagl_gles2_shader_release(shader_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDetachShader(GLuint program, GLuint shader)
{
    struct yagl_gles2_program *program_obj = NULL;
    struct yagl_gles2_shader *shader_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glDetachShader, GLuint, GLuint, program, shader);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    shader_obj = (struct yagl_gles2_shader*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, shader);

    if (!shader_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!shader_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_detach_shader(program_obj, shader_obj)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    yagl_gles2_shader_release(shader_obj);
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glEnableVertexAttribArray(GLuint index)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glEnableVertexAttribArray, GLuint, index);

    YAGL_GET_CTX();

    if (index < ctx->base.num_arrays) {
        yagl_gles_array_enable(&ctx->base.vao->arrays[index], 1);
    }

    yagl_host_glEnableVertexAttribArray(index);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDisableVertexAttribArray(GLuint index)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glDisableVertexAttribArray, GLuint, index);

    YAGL_GET_CTX();

    if (index < ctx->base.num_arrays) {
        yagl_gles_array_enable(&ctx->base.vao->arrays[index], 0);
    }

    yagl_host_glDisableVertexAttribArray(index);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT7(glGetActiveAttrib, GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*, program, index, bufsize, length, size, type, name);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (bufsize < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (index >= program_obj->num_active_attribs) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    yagl_gles2_program_get_active_attrib(program_obj, index, bufsize, length, size, type, name);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT7(glGetActiveUniform, GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*, program, index, bufsize, length, size, type, name);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (bufsize < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (index >= program_obj->num_active_uniforms) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    yagl_gles2_program_get_active_uniform(program_obj, index, bufsize, length, size, type, name);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei *count, GLuint *shaders)
{
    struct yagl_gles2_program *program_obj = NULL;
    GLsizei tmp = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetAttachedShaders, GLuint, GLsizei, GLsizei*, GLuint*, program, maxcount, count, shaders);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (maxcount < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    tmp = 0;

    if (program_obj->vertex_shader) {
        if (tmp < maxcount) {
            if (shaders) {
                shaders[tmp] = program_obj->vertex_shader->base.local_name;
            }
            ++tmp;
        }
    }

    if (program_obj->fragment_shader) {
        if (tmp < maxcount) {
            if (shaders) {
                shaders[tmp] = program_obj->fragment_shader->base.local_name;
            }
            ++tmp;
        }
    }

    if (count) {
        *count = tmp;
    }

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API int glGetAttribLocation(GLuint program, const GLchar *name)
{
    struct yagl_gles2_program *program_obj = NULL;
    int ret = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetAttribLocation, GLuint, const GLchar*, program, name);

    YAGL_GET_CTX_RET(0);

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!program_obj->linked) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    ret = yagl_gles2_program_get_attrib_location(program_obj, name);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT_SPLIT(int, ret);

    return ret;
}

YAGL_API void glGetProgramiv(GLuint program, GLenum pname, GLint *params)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetProgramiv, GLuint, GLenum, GLint*, program, pname, params);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    switch (pname) {
    case GL_ATTACHED_SHADERS:
        *params = (program_obj->fragment_shader != NULL) ? 1 : 0;
        *params += (program_obj->vertex_shader != NULL) ? 1 : 0;
        break;
    case GL_INFO_LOG_LENGTH:
        *params = program_obj->info_log_length;
        break;
    case GL_ACTIVE_ATTRIBUTES:
        *params = program_obj->num_active_attribs;
        break;
    case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
        *params = program_obj->max_active_attrib_bufsize;
        break;
    case GL_ACTIVE_UNIFORMS:
        *params = program_obj->num_active_uniforms;
        break;
    case GL_ACTIVE_UNIFORM_MAX_LENGTH:
        *params = program_obj->max_active_uniform_bufsize;
        break;
    case GL_DELETE_STATUS:
        *params = GL_FALSE;
        break;
    case GL_LINK_STATUS:
        *params = program_obj->link_status;
        break;
    case GL_VALIDATE_STATUS:
        yagl_host_glGetProgramiv(program_obj->global_name, pname, params);
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
    }

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei *length, GLchar *infolog)
{
    struct yagl_gles2_program *program_obj = NULL;
    int32_t tmp = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetProgramInfoLog, GLuint, GLsizei, GLsizei*, GLchar*, program, bufsize, length, infolog);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (bufsize < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (yagl_host_glGetProgramInfoLog(program_obj->global_name, infolog, bufsize, &tmp)) {
        if (length) {
            *length = (tmp > 0) ? (tmp - 1) : 0;
        }
        YAGL_LOG_TRACE("infolog = %s", infolog);
    }

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetShaderiv(GLuint shader, GLenum pname, GLint *params)
{
    struct yagl_gles2_shader *shader_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetShaderiv, GLuint, GLenum, GLint*, shader, pname, params);

    YAGL_GET_CTX();

    shader_obj = (struct yagl_gles2_shader*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, shader);

    if (!shader_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!shader_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    switch (pname) {
    case GL_SHADER_TYPE:
        *params = shader_obj->type;
        break;
    case GL_SHADER_SOURCE_LENGTH:
        *params = shader_obj->source ? (strlen(shader_obj->source) + 1) : 0;
        break;
    case GL_INFO_LOG_LENGTH:
    case GL_DELETE_STATUS:
    case GL_COMPILE_STATUS:
        yagl_host_glGetShaderiv(shader_obj->global_name, pname, params);
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        break;
    }

out:
    yagl_gles2_shader_release(shader_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *infolog)
{
    int32_t tmp = 0;
    struct yagl_gles2_shader *shader_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetShaderInfoLog, GLuint, GLsizei, GLsizei*, GLchar*, shader, bufsize, length, infolog);

    YAGL_GET_CTX();

    shader_obj = (struct yagl_gles2_shader*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, shader);

    if (!shader_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!shader_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (bufsize < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (yagl_host_glGetShaderInfoLog(shader_obj->global_name, infolog, bufsize, &tmp)) {
        if (length) {
            *length = (tmp > 0) ? (tmp - 1) : 0;
        }
        YAGL_LOG_TRACE("infolog = %s", infolog);
    }

out:
    yagl_gles2_shader_release(shader_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetShaderPrecisionFormat, GLenum, GLenum, GLint*, GLint*, shadertype, precisiontype, range, precision);

    YAGL_GET_CTX();

    switch (precisiontype) {
    case GL_LOW_INT:
    case GL_MEDIUM_INT:
    case GL_HIGH_INT:
        if (range) {
            range[0] = range[1] = 16;
        }
        if (precision) {
            *precision = 0;
        }
        break;
    case GL_LOW_FLOAT:
    case GL_MEDIUM_FLOAT:
    case GL_HIGH_FLOAT:
        if (range) {
            range[0] = range[1] = 127;
        }
        if (precision) {
            *precision = 24;
        }
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        break;
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *source)
{
    struct yagl_gles2_shader *shader_obj = NULL;
    GLsizei tmp;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetShaderSource, GLuint, GLsizei, GLsizei*, GLchar*, shader, bufsize, length, source);

    YAGL_GET_CTX();

    shader_obj = (struct yagl_gles2_shader*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, shader);

    if (!shader_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!shader_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (bufsize < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!shader_obj->source) {
        if (length) {
            *length = 0;
        }
        goto out;
    }

    tmp = strlen(shader_obj->source) + 1;
    tmp = (bufsize <= tmp) ? bufsize : tmp;

    if (source && (tmp > 0)) {
        strncpy(source, shader_obj->source, tmp);
        source[tmp - 1] = '\0';
    }

    if (tmp > 0) {
        --tmp;
    }

    if (length) {
        *length = tmp;
    }

out:
    yagl_gles2_shader_release(shader_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetUniformfv(GLuint program, GLint location, GLfloat *params)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetUniformfv, GLuint, GLint, GLfloat*, program, location, params);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!program_obj->linked) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_get_uniformfv(program_obj, location, params)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetUniformiv(GLuint program, GLint location, GLint *params)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetUniformiv, GLuint, GLint, GLint*, program, location, params);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!program_obj->linked) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_get_uniformiv(program_obj, location, params)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API int glGetUniformLocation(GLuint program, const GLchar *name)
{
    struct yagl_gles2_program *program_obj = NULL;
    int ret = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetUniformLocation, GLuint, const GLchar*, program, name);

    YAGL_GET_CTX_RET(0);

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!program_obj->linked) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (name) {
        YAGL_LOG_TRACE("getting uniform %s location", name);
    }

    ret = yagl_gles2_program_get_uniform_location(program_obj, name);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT_SPLIT(int, ret);

    return ret;
}

YAGL_API void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
    GLint param = 0;
    GLfloat tmp[100]; // This fits all cases.
    int32_t num = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetVertexAttribfv, GLuint, GLenum, GLfloat*, index, pname, params);

    YAGL_GET_CTX();

    if (yagl_gles2_context_get_array_param(ctx,
                                           index,
                                           pname,
                                           &param)) {
        if (params) {
            params[0] = param;
        }
    } else {
        yagl_host_glGetVertexAttribfv(index, pname, tmp, sizeof(tmp)/sizeof(tmp[0]), &num);
        if (params) {
            memcpy(params, tmp, num * sizeof(tmp[0]));
        }
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
    GLint tmp[100]; // This fits all cases.
    int32_t num = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetVertexAttribiv, GLuint, GLenum, GLint*, index, pname, params);

    YAGL_GET_CTX();

    if (yagl_gles2_context_get_array_param(ctx,
                                           index,
                                           pname,
                                           params)) {
    } else {
        yagl_host_glGetVertexAttribiv(index, pname, tmp, sizeof(tmp)/sizeof(tmp[0]), &num);
        if (params) {
            memcpy(params, tmp, num * sizeof(tmp[0]));
        }
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid **pointer)
{
    struct yagl_gles_array *array = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetVertexAttribPointerv, GLuint, GLenum, GLvoid**, index, pname, pointer);

    YAGL_GET_CTX();

    if (index >= ctx->base.num_arrays) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    array = &ctx->base.vao->arrays[index];

    if (pointer) {
        if (array->vbo) {
            *pointer = (GLvoid*)array->offset;
        } else {
            *pointer = (GLvoid*)array->ptr;
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API GLboolean glIsProgram(GLuint program)
{
    GLboolean res = GL_FALSE;
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsProgram, GLuint, program);

    YAGL_GET_CTX_RET(GL_FALSE);

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (program_obj) {
        res = program_obj->is_shader ? GL_FALSE : GL_TRUE;
    }

    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, res);

    return res;
}

YAGL_API GLboolean glIsShader(GLuint shader)
{
    GLboolean res = GL_FALSE;
    struct yagl_gles2_shader *shader_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsShader, GLuint, shader);

    YAGL_GET_CTX_RET(GL_FALSE);

    shader_obj = (struct yagl_gles2_shader*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, shader);

    if (shader_obj) {
        res = shader_obj->is_shader ? GL_TRUE : GL_FALSE;
    }

    yagl_gles2_shader_release(shader_obj);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, res);

    return res;
}

YAGL_API void glLinkProgram(GLuint program)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glLinkProgram, GLuint, program);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_gles2_program_link(program_obj);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glReleaseShaderCompiler(void)
{
    YAGL_LOG_FUNC_ENTER_SPLIT0(glReleaseShaderCompiler);

    YAGL_GET_CTX();

    /*
     * No-op.
     */

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glShaderBinary(GLsizei n, const GLuint *shaders, GLenum binaryformat, const GLvoid *binary, GLsizei length)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glShaderBinary, GLsizei, const GLuint*, GLenum, const GLvoid*, GLsizei, n, shaders, binaryformat, binary, length);

    YAGL_GET_CTX();

    /*
     * No-op.
     */

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glShaderSource(GLuint shader, GLsizei count, const GLchar * const *string, const GLint *length)
{
    struct yagl_gles2_shader *shader_obj = NULL;
    int have_strings = 0;
    uint32_t total_length = 0;
    int i;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glShaderSource, GLuint, GLsizei, const GLchar**, const GLint*, shader, count, string, length);

    YAGL_GET_CTX();

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    shader_obj = (struct yagl_gles2_shader*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, shader);

    if (!shader_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!shader_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

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
        uint8_t *tmp_buff;
        int patched_len = 0;
        char *patched_source;

        tmp_buff = yagl_malloc(total_length + 1);
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

        tmp_buff[total_length] = '\0';

        YAGL_LOG_TRACE("orig source = %s", tmp_buff);

        patched_source = ctx->shader_patch(ctx,
                                           (char*)tmp_buff,
                                           total_length,
                                           &patched_len);

        if (patched_source) {
            YAGL_LOG_TRACE("patched source = %s", patched_source);

            yagl_gles2_shader_source(shader_obj,
                                     (GLchar*)tmp_buff,
                                     patched_source,
                                     patched_len);

            yagl_free(patched_source);
        } else {
            yagl_gles2_shader_source(shader_obj,
                                     (GLchar*)tmp_buff,
                                     (GLchar*)tmp_buff,
                                     total_length);
        }
    }

out:
    yagl_gles2_shader_release(shader_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform1f(GLint location, GLfloat x)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glUniform1f, GLint, GLfloat, location, x);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform1f(ctx->program, location, x)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform1fv(GLint location, GLsizei count, const GLfloat* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform1fv, GLint, GLsizei, const GLfloat*, location, count, v);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform1fv(ctx->program, location, count, v)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform1i(GLint location, GLint x)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glUniform1i, GLint, GLint, location, x);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform1i(ctx->program, location, x)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform1iv(GLint location, GLsizei count, const GLint* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform1iv, GLint, GLsizei, const GLint*, location, count, v);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform1iv(ctx->program, location, count, v)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform2f(GLint location, GLfloat x, GLfloat y)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform2f, GLint, GLfloat, GLfloat, location, x, y);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform2f(ctx->program, location, x, y)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform2fv(GLint location, GLsizei count, const GLfloat* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform2fv, GLint, GLsizei, const GLfloat*, location, count, v);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform2fv(ctx->program, location, count, v)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform2i(GLint location, GLint x, GLint y)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform2i, GLint, GLint, GLint, location, x, y);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform2i(ctx->program, location, x, y)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform2iv(GLint location, GLsizei count, const GLint* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform2iv, GLint, GLsizei, const GLint*, location, count, v);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform2iv(ctx->program, location, count, v)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniform3f, GLint, GLfloat, GLfloat, GLfloat, location, x, y, z);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform3f(ctx->program, location, x, y, z)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform3fv(GLint location, GLsizei count, const GLfloat* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform3fv, GLint, GLsizei, const GLfloat*, location, count, v);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform3fv(ctx->program, location, count, v)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform3i(GLint location, GLint x, GLint y, GLint z)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniform3i, GLint, GLint, GLint, GLint, location, x, y, z);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform3i(ctx->program, location, x, y, z)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform3iv(GLint location, GLsizei count, const GLint* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform3iv, GLint, GLsizei, const GLint*, location, count, v);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform3iv(ctx->program, location, count, v)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glUniform4f, GLint, GLfloat, GLfloat, GLfloat, GLfloat, location, x, y, z, w);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform4f(ctx->program, location, x, y, z, w)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform4fv(GLint location, GLsizei count, const GLfloat* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform4fv, GLint, GLsizei, const GLfloat*, location, count, v);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform4fv(ctx->program, location, count, v)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glUniform4i, GLint, GLint, GLint, GLint, GLint, location, x, y, z, w);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform4i(ctx->program, location, x, y, z, w)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform4iv(GLint location, GLsizei count, const GLint* v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform4iv, GLint, GLsizei, const GLint*, location, count, v);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform4iv(ctx->program, location, count, v)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix2fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform_matrix2fv(ctx->program, location, count, transpose, value)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix3fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform_matrix3fv(ctx->program, location, count, transpose, value)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix4fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);

    YAGL_GET_CTX();

    if (!ctx->program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (location == -1) {
        goto out;
    }

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles2_program_uniform_matrix4fv(ctx->program, location, count, transpose, value)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUseProgram(GLuint program)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glUseProgram, GLuint, program);

    YAGL_GET_CTX();

    if (program != 0) {
        program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
            YAGL_NS_SHADER_PROGRAM, program);

        if (!program_obj) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }

        if (program_obj->is_shader) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }
    }

    yagl_gles2_context_use_program(ctx, program_obj);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glValidateProgram(GLuint program)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glValidateProgram, GLuint, program);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_host_glValidateProgram(program_obj->global_name);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

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
    struct yagl_gles_array *array;

    YAGL_LOG_FUNC_ENTER_SPLIT6(glVertexAttribPointer, GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*, indx, size, type, normalized, stride, ptr);

    YAGL_GET_CTX();

    if (indx >= ctx->base.num_arrays) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    array = &ctx->base.vao->arrays[indx];

    if (ctx->base.vbo) {
        if (!yagl_gles_array_update_vbo(array,
                                        size,
                                        type,
                                        0,
                                        normalized,
                                        stride,
                                        ctx->base.vbo,
                                        (GLint)ptr)) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
    } else {
        /*
         * GL_OES_vertex_array_object:
         * "Binding a zero-named vertex array buffer:
         * this can be detected by *Pointer(ES1) or VertexAttribPointer(ES2);
         * if the pointer argument is not NULL:
         * this means to bind a client vertex array;
         * an INVALID_OPERATION error will be returned."
         */
        if ((ctx->base.vao != ctx->base.va_zero) && ptr) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }

        if (!yagl_gles_array_update(array,
                                    size,
                                    type,
                                    0,
                                    normalized,
                                    stride,
                                    ptr)) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

/*
 * GL_EXT_instanced_arrays.
 * @{
 */

YAGL_API void glDrawArraysInstanced(GLenum mode, GLint start, GLsizei count,
                                    GLsizei primcount)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glDrawArraysInstanced, GLenum, GLint, GLsizei, GLsizei, mode, start, count, primcount);

    YAGL_GET_CTX();

    if (!ctx->instanced_arrays) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (primcount < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    yagl_gles_context_draw_arrays(&ctx->base, mode, start, count, primcount);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glDrawArraysInstanced, glDrawArraysInstancedEXT);

YAGL_API void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type,
                                      const void *indices, GLsizei primcount)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glDrawElementsInstanced, GLenum, GLsizei, GLenum, const void*, GLsizei, mode, count, type, indices, primcount);

    YAGL_GET_CTX();

    if (!ctx->instanced_arrays) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (primcount < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    yagl_gles_context_draw_elements(&ctx->base, mode, count, type, indices, primcount);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glDrawElementsInstanced, glDrawElementsInstancedEXT);

YAGL_API void glVertexAttribDivisor(GLuint index, GLuint divisor)
{
    struct yagl_gles_array *array;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glVertexAttribDivisor, GLuint, GLuint, index, divisor);

    YAGL_GET_CTX();

    if (!ctx->instanced_arrays) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (index >= ctx->base.num_arrays) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    array = &ctx->base.vao->arrays[index];

    yagl_gles_array_set_divisor(array, divisor);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glVertexAttribDivisor, glVertexAttribDivisorEXT);

/*
 * @}
 */

/*
 * GL_OES_texture_3D.
 * @{
 */

YAGL_API void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)
{
    GLsizei stride = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT10(glTexImage3D, GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*, target, level, internalformat, width, height, depth, border, format, type, pixels);

    YAGL_GET_CTX();

    if (!yagl_gles2_is_texture_target_3d(target)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (pixels && (width > 0) && (height > 0) && (depth > 0)) {
        if (!yagl_gles_context_get_stride(&ctx->base,
                                          ctx->base.unpack_alignment,
                                          width,
                                          format,
                                          type,
                                          &stride)) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }
    }

    yagl_host_glTexImage3D(target,
                           level,
                           yagl_gles_get_actual_internalformat(internalformat),
                           width,
                           height,
                           depth,
                           border,
                           yagl_gles_get_actual_format(format),
                           yagl_gles_get_actual_type(type),
                           yagl_gles_convert_to_host(ctx->base.unpack_alignment,
                                                     width,
                                                     height,
                                                     depth,
                                                     format,
                                                     type,
                                                     pixels),
                           stride * height * depth);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glTexImage3D, glTexImage3DOES);

YAGL_API void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
    GLsizei stride = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT11(glTexSubImage3D, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void*, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);

    YAGL_GET_CTX();

    if (!yagl_gles2_is_texture_target_3d(target)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (pixels && (width > 0) && (height > 0) && (depth > 0)) {
        if (!yagl_gles_context_get_stride(&ctx->base,
                                          ctx->base.unpack_alignment,
                                          width,
                                          format,
                                          type,
                                          &stride)) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }
    }

    yagl_host_glTexSubImage3D(target,
                              level,
                              xoffset,
                              yoffset,
                              zoffset,
                              width,
                              height,
                              depth,
                              yagl_gles_get_actual_format(format),
                              yagl_gles_get_actual_type(type),
                              yagl_gles_convert_to_host(ctx->base.unpack_alignment,
                                                        width,
                                                        height,
                                                        depth,
                                                        format,
                                                        type,
                                                        pixels),
                              stride * height * depth);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glTexSubImage3D, glTexSubImage3DOES);

YAGL_API void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    YAGL_LOG_FUNC_ENTER_SPLIT9(glCopyTexSubImage3D, GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, target, level, xoffset, yoffset, zoffset, x, y, width, height);

    YAGL_GET_CTX();

    if (!yagl_gles2_is_texture_target_3d(target)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glCopyTexSubImage3D(target,
                                  level,
                                  xoffset,
                                  yoffset,
                                  zoffset,
                                  x,
                                  y,
                                  width,
                                  height);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glCopyTexSubImage3D, glCopyTexSubImage3DOES);

YAGL_API void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data)
{
    /*
     * TODO: Implement
     */
}
YAGL_API YAGL_ALIAS(glCompressedTexImage3D, glCompressedTexImage3DOES);

YAGL_API void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)
{
    /*
     * TODO: Implement
     */
}
YAGL_API YAGL_ALIAS(glCompressedTexSubImage3D, glCompressedTexSubImage3DOES);

YAGL_API void glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    /*
     * TODO: Implement
     */
}
YAGL_API YAGL_ALIAS(glFramebufferTexture3D, glFramebufferTexture3DOES);

/*
 * @}
 */
