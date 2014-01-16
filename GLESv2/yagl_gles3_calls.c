#include "GLES3/gl3.h"
#include "yagl_host_gles_calls.h"
#include "yagl_gles3_context.h"
#include "yagl_gles3_program.h"
#include "yagl_gles3_transform_feedback.h"
#include "yagl_gles3_buffer_binding.h"
#include "yagl_gles3_query.h"
#include "yagl_gles3_validate.h"
#include "yagl_gles2_shader.h"
#include "yagl_gles2_validate.h"
#include "yagl_gles_buffer.h"
#include "yagl_gles_texture.h"
#include "yagl_gles_framebuffer.h"
#include "yagl_gles_validate.h"
#include "yagl_sharegroup.h"
#include "yagl_log.h"
#include "yagl_impl.h"
#include "yagl_malloc.h"

#define YAGL_SET_ERR(err) \
    yagl_gles_context_set_error(&ctx->base.base, err); \
    YAGL_LOG_ERROR("error = 0x%X", err)

#define YAGL_GET_CTX_IMPL(ret_expr) \
    struct yagl_gles3_context *ctx = \
        (struct yagl_gles3_context*)yagl_get_client_context(); \
    if (!ctx || ((ctx->base.base.base.client_api & yagl_client_api_gles3) == 0)) { \
        YAGL_LOG_WARN("no current context"); \
        YAGL_LOG_FUNC_EXIT(NULL); \
        ret_expr; \
    }

#define YAGL_GET_CTX_RET(ret) YAGL_GET_CTX_IMPL(return ret)

#define YAGL_GET_CTX() YAGL_GET_CTX_IMPL(return)

YAGL_API void glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glInvalidateFramebuffer, GLenum, GLsizei, const GLenum*, target, numAttachments, attachments);

    YAGL_GET_CTX();

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    struct yagl_gles_buffer *buffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glBindBufferBase, GLenum, GLuint, GLuint, target, index, buffer);

    YAGL_GET_CTX();

    if (buffer != 0) {
        buffer_obj = (struct yagl_gles_buffer*)yagl_sharegroup_acquire_object(ctx->base.sg,
            YAGL_NS_BUFFER, buffer);

        if (!buffer_obj) {
            buffer_obj = yagl_gles_buffer_create();

            if (!buffer_obj) {
                goto out;
            }

            buffer_obj = (struct yagl_gles_buffer*)yagl_sharegroup_add_named(ctx->base.sg,
               YAGL_NS_BUFFER, buffer, &buffer_obj->base);
        }
    }

    yagl_gles3_context_bind_buffer_base(ctx, target, index, buffer_obj);

out:
    yagl_gles_buffer_release(buffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glBindBufferRange(GLenum target, GLuint index, GLuint buffer,
                                GLintptr offset, GLsizeiptr size)
{
    struct yagl_gles_buffer *buffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT5(glBindBufferRange, GLenum, GLuint, GLuint, GLintptr, GLsizeiptr, target, index, buffer, offset, size);

    YAGL_GET_CTX();

    if (buffer != 0) {
        buffer_obj = (struct yagl_gles_buffer*)yagl_sharegroup_acquire_object(ctx->base.sg,
            YAGL_NS_BUFFER, buffer);

        if (!buffer_obj) {
            buffer_obj = yagl_gles_buffer_create();

            if (!buffer_obj) {
                goto out;
            }

            buffer_obj = (struct yagl_gles_buffer*)yagl_sharegroup_add_named(ctx->base.sg,
               YAGL_NS_BUFFER, buffer, &buffer_obj->base);
        }
    }

    yagl_gles3_context_bind_buffer_range(ctx, target, index, offset, size,
                                         buffer_obj);

out:
    yagl_gles_buffer_release(buffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API GLuint glGetUniformBlockIndex(GLuint program,
                                       const GLchar *uniformBlockName)
{
    struct yagl_gles2_program *program_obj = NULL;
    GLuint index = GL_INVALID_INDEX;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetUniformBlockIndex, GLuint, const GLchar*, program, uniformBlockName);

    YAGL_GET_CTX_RET(GL_INVALID_INDEX);

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->base.sg,
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

    index = yagl_gles3_program_get_uniform_block_index(program_obj,
                                                       uniformBlockName);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT("%u", index);

    return index;
}

YAGL_API void glGetUniformIndices(GLuint program, GLsizei uniformCount,
                                  const GLchar *const *uniformNames,
                                  GLuint *uniformIndices)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetUniformIndices, GLuint, GLsizei, const GLchar* const*, GLuint*, program, uniformCount, uniformNames, uniformIndices);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_gles3_program_get_uniform_indices(program_obj,
                                           uniformNames,
                                           uniformCount,
                                           uniformIndices);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetActiveUniformsiv(GLuint program, GLsizei uniformCount,
                                    const GLuint *uniformIndices,
                                    GLenum pname, GLint *params)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT5(glGetActiveUniformsiv, GLuint, GLsizei, const GLuint*, GLenum, GLint*, program, uniformCount, uniformIndices, pname, params);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles3_is_uniform_param_valid(pname)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (uniformCount >= program_obj->num_active_uniforms) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles3_program_get_active_uniformsiv(program_obj, uniformIndices, uniformCount, pname, params)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex,
                                    GLuint uniformBlockBinding)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniformBlockBinding, GLuint, GLuint, GLuint, program, uniformBlockIndex, uniformBlockBinding);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (uniformBlockIndex >= program_obj->num_active_uniform_blocks) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (uniformBlockBinding >= ctx->num_uniform_buffer_bindings) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    yagl_gles3_program_set_uniform_block_binding(program_obj,
                                                 uniformBlockIndex,
                                                 uniformBlockBinding);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetActiveUniformBlockName(GLuint program,
                                          GLuint uniformBlockIndex,
                                          GLsizei bufSize,
                                          GLsizei *length,
                                          GLchar *uniformBlockName)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT5(glGetActiveUniformBlockName, GLuint, GLuint, GLsizei, GLsizei*, GLchar*, program, uniformBlockIndex, bufSize, length, uniformBlockName);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (uniformBlockIndex >= program_obj->num_active_uniform_blocks) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    yagl_gles3_program_get_active_uniform_block_name(program_obj,
                                                     uniformBlockIndex,
                                                     bufSize,
                                                     length,
                                                     uniformBlockName);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetActiveUniformBlockiv(GLuint program,
                                        GLuint uniformBlockIndex,
                                        GLenum pname, GLint *params)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetActiveUniformBlockiv, GLuint, GLuint, GLenum, GLint*, program, uniformBlockIndex, pname, params);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (uniformBlockIndex >= program_obj->num_active_uniform_blocks) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles3_program_get_active_uniform_blockiv(program_obj,
                                                       uniformBlockIndex,
                                                       pname,
                                                       params)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGenTransformFeedbacks(GLsizei n, GLuint *ids)
{
    struct yagl_gles3_transform_feedback **transform_feedbacks = NULL;
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenTransformFeedbacks, GLsizei, GLuint*, n, ids);

    YAGL_GET_CTX();

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    transform_feedbacks = yagl_malloc0(n * sizeof(*transform_feedbacks));

    for (i = 0; i < n; ++i) {
        transform_feedbacks[i] =
            yagl_gles3_transform_feedback_create(0,
                                                 ctx->max_transform_feedback_separate_attribs);

        if (!transform_feedbacks[i]) {
            goto out;
        }
    }

    for (i = 0; i < n; ++i) {
        yagl_namespace_add(&ctx->transform_feedbacks,
                           &transform_feedbacks[i]->base);

        if (ids) {
            ids[i] = transform_feedbacks[i]->base.local_name;
        }
    }

out:
    for (i = 0; i < n; ++i) {
        yagl_gles3_transform_feedback_release(transform_feedbacks[i]);
    }
    yagl_free(transform_feedbacks);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glBindTransformFeedback(GLenum target, GLuint id)
{
    struct yagl_gles3_transform_feedback *tf = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glBindTransformFeedback, GLenum, GLuint, target, id);

    YAGL_GET_CTX();

    if (id != 0) {
        tf = (struct yagl_gles3_transform_feedback*)yagl_namespace_acquire(&ctx->transform_feedbacks,
            id);

        if (!tf) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }
    }

    yagl_gles3_context_bind_transform_feedback(ctx, target, tf);

out:
    yagl_gles3_transform_feedback_release(tf);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glBeginTransformFeedback(GLenum primitiveMode)
{
    GLuint i, num_active_buffer_bindings;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glBeginTransformFeedback, GLenum, primitiveMode);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    num_active_buffer_bindings = ctx->base.program->linked_transform_feedback_info.num_varyings;

    if (num_active_buffer_bindings == 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles3_is_primitive_mode_valid(primitiveMode)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (ctx->tfo->active) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (ctx->base.program->linked_transform_feedback_info.buffer_mode == GL_INTERLEAVED_ATTRIBS) {
        /*
         * "In interleaved mode, only the first buffer object binding point is ever written to."
         */
        num_active_buffer_bindings = 1;
    }

    for (i = 0; i < num_active_buffer_bindings; ++i) {
        if (!ctx->tfo->buffer_bindings[i].buffer) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }
    }

    yagl_gles3_transform_feedback_begin(ctx->tfo,
                                        primitiveMode,
                                        num_active_buffer_bindings);

    ctx->tf_primitive_mode = primitiveMode;

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glEndTransformFeedback(void)
{
    YAGL_LOG_FUNC_ENTER_SPLIT0(glEndTransformFeedback);

    YAGL_GET_CTX();

    if (!ctx->tfo->active) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_gles3_transform_feedback_end(ctx->tfo);

    ctx->tf_primitive_mode = 0;

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glPauseTransformFeedback(void)
{
    YAGL_LOG_FUNC_ENTER_SPLIT0(glPauseTransformFeedback);

    YAGL_GET_CTX();

    if (!ctx->tfo->active || ctx->tfo->paused) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_gles3_transform_feedback_pause(ctx->tfo);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glResumeTransformFeedback(void)
{
    YAGL_LOG_FUNC_ENTER_SPLIT0(glResumeTransformFeedback);

    YAGL_GET_CTX();

    if (!ctx->tfo->active || !ctx->tfo->paused) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_gles3_transform_feedback_resume(ctx->tfo);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDeleteTransformFeedbacks(GLsizei n, const GLuint *ids)
{
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteTransformFeedbacks, GLsizei, const GLuint*, n, ids);

    YAGL_GET_CTX();

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (ids) {
        for (i = 0; i < n; ++i) {
            yagl_namespace_remove(&ctx->transform_feedbacks, ids[i]);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API GLboolean glIsTransformFeedback(GLuint id)
{
    GLboolean res = GL_FALSE;
    struct yagl_gles3_transform_feedback *tf = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsTransformFeedback, GLuint, id);

    YAGL_GET_CTX_RET(GL_FALSE);

    tf = (struct yagl_gles3_transform_feedback*)yagl_namespace_acquire(&ctx->transform_feedbacks,
        id);

    if (tf && yagl_gles3_transform_feedback_was_bound(tf)) {
        res = GL_TRUE;
    }

    yagl_gles3_transform_feedback_release(tf);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, res);

    return res;
}

YAGL_API void glTransformFeedbackVaryings(GLuint program,
                                          GLsizei count,
                                          const GLchar *const *varyings,
                                          GLenum bufferMode)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glTransformFeedbackVaryings, GLuint, GLsizei, const GLchar* const*, GLenum, program, count, varyings, bufferMode);

    YAGL_GET_CTX();

    if (!yagl_gles3_is_transform_feedback_buffer_mode_valid(bufferMode)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SHADER_PROGRAM, program);

    if (!program_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (program_obj->is_shader) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if ((bufferMode == GL_SEPARATE_ATTRIBS) &&
        (count > ctx->max_transform_feedback_separate_attribs)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    yagl_gles3_program_set_transform_feedback_varyings(program_obj,
                                                       varyings,
                                                       count,
                                                       bufferMode);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetTransformFeedbackVarying(GLuint program,
                                            GLuint index,
                                            GLsizei bufSize,
                                            GLsizei *length,
                                            GLsizei *size,
                                            GLenum *type,
                                            GLchar *name)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT7(glGetTransformFeedbackVarying, GLuint, GLuint, GLsizei, GLsizei*, GLsizei*, GLenum*, GLchar*, program, index, bufSize, length, size, type, name);

    YAGL_GET_CTX();

    program_obj = (struct yagl_gles2_program*)yagl_sharegroup_acquire_object(ctx->base.sg,
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

    if (index >= program_obj->linked_transform_feedback_info.num_varyings) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    yagl_gles3_program_get_transform_feedback_varying(program_obj,
                                                      index,
                                                      bufSize,
                                                      length,
                                                      size,
                                                      type,
                                                      name);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGenQueries(GLsizei n, GLuint *ids)
{
    struct yagl_gles3_query **queries = NULL;
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenQueries, GLsizei, GLuint*, n, ids);

    YAGL_GET_CTX();

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    queries = yagl_malloc0(n * sizeof(*queries));

    for (i = 0; i < n; ++i) {
        queries[i] = yagl_gles3_query_create();

        if (!queries[i]) {
            goto out;
        }
    }

    for (i = 0; i < n; ++i) {
        yagl_namespace_add(&ctx->queries, &queries[i]->base);

        if (ids) {
            ids[i] = queries[i]->base.local_name;
        }
    }

out:
    for (i = 0; i < n; ++i) {
        yagl_gles3_query_release(queries[i]);
    }
    yagl_free(queries);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDeleteQueries(GLsizei n, const GLuint *ids)
{
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteQueries, GLsizei, const GLuint*, n, ids);

    YAGL_GET_CTX();

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (ids) {
        for (i = 0; i < n; ++i) {
            yagl_namespace_remove(&ctx->queries, ids[i]);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glBeginQuery(GLenum target, GLuint id)
{
    struct yagl_gles3_query *query = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glBeginQuery, GLenum, GLuint, target, id);

    YAGL_GET_CTX();

    query = (struct yagl_gles3_query*)yagl_namespace_acquire(&ctx->queries, id);

    if (!query) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_gles3_context_begin_query(ctx, target, query);

out:
    yagl_gles3_query_release(query);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glEndQuery(GLenum target)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glEndQuery, GLenum, target);

    YAGL_GET_CTX();

    yagl_gles3_context_end_query(ctx, target);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetQueryiv(GLenum target, GLenum pname, GLint *params)
{
    struct yagl_gles3_query *query = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetQueryiv, GLenum, GLenum, GLint*, target, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles3_context_acquire_active_query(ctx, target, &query)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    switch (pname) {
    case GL_CURRENT_QUERY:
        *params = query ? query->base.local_name : 0;
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

out:
    yagl_gles3_query_release(query);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params)
{
    struct yagl_gles3_query *query = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetQueryObjectuiv, GLuint, GLenum, GLuint*, id, pname, params);

    YAGL_GET_CTX();

    query = (struct yagl_gles3_query*)yagl_namespace_acquire(&ctx->queries, id);

    if (!query) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (query->active) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    switch (pname) {
    case GL_QUERY_RESULT_AVAILABLE:
        *params = yagl_gles3_query_is_result_available(query);
        break;
    case GL_QUERY_RESULT:
        *params = yagl_gles3_query_get_result(query);
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

out:
    yagl_gles3_query_release(query);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API GLboolean glIsQuery(GLuint id)
{
    GLboolean res = GL_FALSE;
    struct yagl_gles3_query *query = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsQuery, GLuint, id);

    YAGL_GET_CTX_RET(GL_FALSE);

    query = (struct yagl_gles3_query*)yagl_namespace_acquire(&ctx->queries, id);

    if (query && yagl_gles3_query_was_active(query)) {
        res = GL_TRUE;
    }

    yagl_gles3_query_release(query);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, res);

    return res;
}

YAGL_API void glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    struct yagl_gles_framebuffer *framebuffer_obj = NULL;
    struct yagl_gles_texture *texture_obj = NULL;
    yagl_gles_framebuffer_attachment framebuffer_attachment;

    YAGL_LOG_FUNC_ENTER_SPLIT5(glFramebufferTextureLayer, GLenum, GLenum, GLuint, GLint, GLint, target, attachment, texture, level, layer);

    YAGL_GET_CTX();

    if (!yagl_gles_context_acquire_binded_framebuffer(&ctx->base.base,
                                                      target,
                                                      &framebuffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!framebuffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (texture) {
        texture_obj = (struct yagl_gles_texture*)yagl_sharegroup_acquire_object(ctx->base.sg,
            YAGL_NS_TEXTURE, texture);

        if (!texture_obj) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }

        if (!yagl_gles2_is_texture_target_layered(yagl_gles_texture_get_target(texture_obj))) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }

        if (layer < 0) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
    }

    if (attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
        yagl_gles_framebuffer_texture_layer(framebuffer_obj,
                                            target, GL_DEPTH_ATTACHMENT,
                                            yagl_gles_framebuffer_attachment_depth,
                                            texture_obj, level, layer);
        yagl_gles_framebuffer_texture_layer(framebuffer_obj,
                                            target, GL_STENCIL_ATTACHMENT,
                                            yagl_gles_framebuffer_attachment_stencil,
                                            texture_obj, level, layer);
    } else {
        if (!yagl_gles_validate_framebuffer_attachment(attachment,
                                                       ctx->base.base.max_color_attachments,
                                                       &framebuffer_attachment)) {
            YAGL_SET_ERR(GL_INVALID_ENUM);
            goto out;
        }

        yagl_gles_framebuffer_texture_layer(framebuffer_obj,
                                            target, attachment,
                                            framebuffer_attachment,
                                            texture_obj, level, layer);
    }

out:
    yagl_gles_texture_release(texture_obj);
    yagl_gles_framebuffer_release(framebuffer_obj);
}
