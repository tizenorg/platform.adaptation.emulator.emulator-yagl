#include "GLES3/gl3.h"
#include "yagl_host_gles_calls.h"
#include "yagl_gles3_context.h"
#include "yagl_gles3_program.h"
#include "yagl_gles3_transform_feedback.h"
#include "yagl_gles3_buffer_binding.h"
#include "yagl_gles3_query.h"
#include "yagl_gles3_sync.h"
#include "yagl_gles3_validate.h"
#include "yagl_gles2_shader.h"
#include "yagl_gles2_validate.h"
#include "yagl_gles_buffer.h"
#include "yagl_gles_texture.h"
#include "yagl_gles_framebuffer.h"
#include "yagl_gles_sampler.h"
#include "yagl_gles_vertex_array.h"
#include "yagl_gles_array.h"
#include "yagl_gles_validate.h"
#include "yagl_sharegroup.h"
#include "yagl_log.h"
#include "yagl_impl.h"
#include "yagl_malloc.h"
#include "yagl_transport.h"
#include "yagl_state.h"
#include "yagl_render.h"
#include "yagl_egl_fence.h"

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

YAGL_API void glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    YAGL_LOG_FUNC_ENTER_SPLIT7(glInvalidateSubFramebuffer, GLenum, GLsizei, const GLenum*, GLint, GLint, GLsizei, GLsizei, target, numAttachments, attachments, x, y, width, height);

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

        if (!yagl_gles2_is_texture_target_layered(texture_obj->target)) {
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

YAGL_API void glGenSamplers(GLsizei count, GLuint *sampler_names)
{
    struct yagl_gles_sampler **samplers = NULL;
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenSamplers, GLsizei, GLuint*, count, sampler_names);

    YAGL_GET_CTX();

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    samplers = yagl_malloc0(count * sizeof(*samplers));

    for (i = 0; i < count; ++i) {
        samplers[i] = yagl_gles_sampler_create();

        if (!samplers[i]) {
            goto out;
        }
    }

    for (i = 0; i < count; ++i) {
        yagl_sharegroup_add(ctx->base.sg,
                            YAGL_NS_SAMPLER,
                            &samplers[i]->base);

        if (sampler_names) {
            sampler_names[i] = samplers[i]->base.local_name;
        }
    }

out:
    for (i = 0; i < count; ++i) {
        yagl_gles_sampler_release(samplers[i]);
    }
    yagl_free(samplers);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDeleteSamplers(GLsizei count, const GLuint *samplers)
{
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteSamplers, GLsizei, const GLuint*, count, samplers);

    YAGL_GET_CTX();

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (samplers) {
        for (i = 0; i < count; ++i) {
            yagl_gles3_context_unbind_sampler(ctx, samplers[i]);

            yagl_sharegroup_remove(ctx->base.sg,
                                   YAGL_NS_SAMPLER,
                                   samplers[i]);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API GLboolean glIsSampler(GLuint sampler)
{
    GLboolean res = GL_FALSE;
    struct yagl_gles_sampler *sampler_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsSampler, GLuint, sampler);

    YAGL_GET_CTX_RET(GL_FALSE);

    sampler_obj = (struct yagl_gles_sampler*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SAMPLER, sampler);

    if (sampler_obj && yagl_gles_sampler_was_bound(sampler_obj)) {
        res = GL_TRUE;
    }

    yagl_gles_sampler_release(sampler_obj);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, res);

    return res;
}

YAGL_API void glBindSampler(GLuint unit, GLuint sampler)
{
    struct yagl_gles_sampler *sampler_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glBindSampler, GLuint, GLuint, unit, sampler);

    YAGL_GET_CTX();

    if (sampler != 0) {
        sampler_obj = (struct yagl_gles_sampler*)yagl_sharegroup_acquire_object(ctx->base.sg,
            YAGL_NS_SAMPLER, sampler);

        if (!sampler_obj) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }
    }

    if (!yagl_gles3_context_bind_sampler(ctx, unit, sampler_obj)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

out:
    yagl_gles_sampler_release(sampler_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
    struct yagl_gles_sampler *sampler_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glSamplerParameteri, GLuint, GLenum, GLint, sampler, pname, param);

    YAGL_GET_CTX();

    sampler_obj = (struct yagl_gles_sampler*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SAMPLER, sampler);

    if (!sampler_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles_sampler_set_parameteriv(sampler_obj, pname, &param)) {
        GLfloat float_param = param;

        if (!yagl_gles_sampler_set_parameterfv(sampler_obj, pname, &float_param)) {
            YAGL_SET_ERR(GL_INVALID_ENUM);
            goto out;
        }
    }

out:
    yagl_gles_sampler_release(sampler_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint *param)
{
    struct yagl_gles_sampler *sampler_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glSamplerParameteriv, GLuint, GLenum, const GLint*, sampler, pname, param);

    YAGL_GET_CTX();

    sampler_obj = (struct yagl_gles_sampler*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SAMPLER, sampler);

    if (!sampler_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles_sampler_set_parameteriv(sampler_obj, pname, param)) {
        GLfloat float_param = param[0];

        if (!yagl_gles_sampler_set_parameterfv(sampler_obj, pname, &float_param)) {
            YAGL_SET_ERR(GL_INVALID_ENUM);
            goto out;
        }
    }

out:
    yagl_gles_sampler_release(sampler_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
    struct yagl_gles_sampler *sampler_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glSamplerParameterf, GLuint, GLenum, GLfloat, sampler, pname, param);

    YAGL_GET_CTX();

    sampler_obj = (struct yagl_gles_sampler*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SAMPLER, sampler);

    if (!sampler_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles_sampler_set_parameterfv(sampler_obj, pname, &param)) {
        GLint int_param = param;

        if (!yagl_gles_sampler_set_parameteriv(sampler_obj, pname, &int_param)) {
            YAGL_SET_ERR(GL_INVALID_ENUM);
            goto out;
        }
    }

out:
    yagl_gles_sampler_release(sampler_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *param)
{
    struct yagl_gles_sampler *sampler_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glSamplerParameterfv, GLuint, GLenum, const GLfloat*, sampler, pname, param);

    YAGL_GET_CTX();

    sampler_obj = (struct yagl_gles_sampler*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SAMPLER, sampler);

    if (!sampler_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles_sampler_set_parameterfv(sampler_obj, pname, param)) {
        GLint int_param = param[0];

        if (!yagl_gles_sampler_set_parameteriv(sampler_obj, pname, &int_param)) {
            YAGL_SET_ERR(GL_INVALID_ENUM);
            goto out;
        }
    }

out:
    yagl_gles_sampler_release(sampler_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params)
{
    struct yagl_gles_sampler *sampler_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetSamplerParameteriv, GLuint, GLenum, GLint*, sampler, pname, params);

    YAGL_GET_CTX();

    sampler_obj = (struct yagl_gles_sampler*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SAMPLER, sampler);

    if (!sampler_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles_sampler_get_parameteriv(sampler_obj, pname, params)) {
        GLfloat float_param;

        if (!yagl_gles_sampler_get_parameterfv(sampler_obj, pname, &float_param)) {
            YAGL_SET_ERR(GL_INVALID_ENUM);
            goto out;
        }

        params[0] = float_param;
    }

out:
    yagl_gles_sampler_release(sampler_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params)
{
    struct yagl_gles_sampler *sampler_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetSamplerParameterfv, GLuint, GLenum, GLfloat*, sampler, pname, params);

    YAGL_GET_CTX();

    sampler_obj = (struct yagl_gles_sampler*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SAMPLER, sampler);

    if (!sampler_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles_sampler_get_parameterfv(sampler_obj, pname, params)) {
        GLint int_param;

        if (!yagl_gles_sampler_get_parameteriv(sampler_obj, pname, &int_param)) {
            YAGL_SET_ERR(GL_INVALID_ENUM);
            goto out;
        }

        params[0] = int_param;
    }

out:
    yagl_gles_sampler_release(sampler_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glRenderbufferStorageMultisample, GLenum, GLsizei, GLenum, GLsizei, GLsizei, target, samples, internalformat, width, height);

    YAGL_GET_CTX();

    yagl_host_glRenderbufferStorageMultisample(target, samples, internalformat, width, height);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    struct yagl_gles_buffer *read_buffer_obj = NULL;
    struct yagl_gles_buffer *write_buffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT5(glCopyBufferSubData, GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr, readTarget, writeTarget, readOffset, writeOffset, size);

    YAGL_GET_CTX();

    if (!yagl_gles_context_acquire_binded_buffer(&ctx->base.base, readTarget, &read_buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!read_buffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles_context_acquire_binded_buffer(&ctx->base.base, writeTarget, &write_buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!write_buffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_gles_buffer_bind(read_buffer_obj,
                          0,
                          0,
                          readTarget);
    yagl_gles_buffer_bind(write_buffer_obj,
                          0,
                          0,
                          writeTarget);
    yagl_gles_buffer_transfer(read_buffer_obj,
                              0,
                              readTarget,
                              0);
    yagl_gles_buffer_transfer(write_buffer_obj,
                              0,
                              writeTarget,
                              0);

    if (yagl_gles_buffer_copy_gpu(read_buffer_obj, readTarget,
                                  write_buffer_obj, writeTarget,
                                  readOffset,
                                  writeOffset,
                                  size)) {
        yagl_gles_buffer_set_gpu_dirty(write_buffer_obj,
                                       writeOffset,
                                       size);
    } else {
        YAGL_SET_ERR(GL_INVALID_VALUE);
    }

    yagl_host_glBindBuffer(writeTarget, 0);
    yagl_host_glBindBuffer(readTarget, 0);

out:
    yagl_gles_buffer_release(read_buffer_obj);
    yagl_gles_buffer_release(write_buffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API GLsync glFenceSync(GLenum condition, GLbitfield flags)
{
    GLsync res = NULL;
    struct yagl_gles3_sync *sync = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glFenceSync, GLenum, GLbitfield, condition, flags);

    YAGL_GET_CTX_RET(NULL);

    if (!yagl_egl_fence_supported()) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (condition != GL_SYNC_GPU_COMMANDS_COMPLETE) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (flags != 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    sync = yagl_gles3_sync_create();

    if (!sync) {
        goto out;
    }

    yagl_sharegroup_add(ctx->base.sg, YAGL_NS_SYNC, &sync->base);
    res = (GLsync)sync->base.local_name;

    yagl_transport_flush(yagl_get_transport(), sync->egl_fence);

out:
    yagl_gles3_sync_release(sync);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLsync, res);

    return res;
}

YAGL_API GLboolean glIsSync(GLsync sync)
{
    GLboolean res = GL_FALSE;
    struct yagl_gles3_sync *sync_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsSync, GLsync, sync);

    YAGL_GET_CTX_RET(GL_FALSE);

    sync_obj = (struct yagl_gles3_sync*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SYNC, (yagl_object_name)sync);

    res = (sync_obj != NULL);

    yagl_gles3_sync_release(sync_obj);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, res);

    return res;
}

YAGL_API void glDeleteSync(GLsync sync)
{
    struct yagl_gles3_sync *sync_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glDeleteSync, GLsync, sync);

    YAGL_GET_CTX();

    if (sync == NULL) {
        goto out;
    }

    sync_obj = (struct yagl_gles3_sync*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SYNC, (yagl_object_name)sync);

    if (!sync_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    yagl_sharegroup_remove(ctx->base.sg,
                           YAGL_NS_SYNC,
                           (yagl_object_name)sync);

out:
    yagl_gles3_sync_release(sync_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    GLenum status = GL_WAIT_FAILED;
    struct yagl_gles3_sync *sync_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glClientWaitSync, GLsync, GLbitfield, GLuint64, sync, flags, timeout);

    YAGL_GET_CTX_RET(status);

    if (flags & ~(GL_SYNC_FLUSH_COMMANDS_BIT)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    sync_obj = (struct yagl_gles3_sync*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SYNC, (yagl_object_name)sync);

    if (!sync_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (timeout == 0) {
        status = sync_obj->egl_fence->signaled(sync_obj->egl_fence) ? GL_CONDITION_SATISFIED
                                                                    : GL_TIMEOUT_EXPIRED;
    } else if (sync_obj->egl_fence->wait(sync_obj->egl_fence)) {
        status = GL_CONDITION_SATISFIED;
    } else {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
    }

out:
    yagl_gles3_sync_release(sync_obj);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLenum, status);

    return status;
}

YAGL_API void glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    struct yagl_gles3_sync *sync_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glWaitSync, GLsync, GLbitfield, GLuint64, sync, flags, timeout);

    YAGL_GET_CTX();

    if ((flags != 0) || (timeout != GL_TIMEOUT_IGNORED)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    sync_obj = (struct yagl_gles3_sync*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SYNC, (yagl_object_name)sync);

    if (!sync_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    /*
     * All our OpenGL commands are serialized, this is a no-op.
     */

out:
    yagl_gles3_sync_release(sync_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
    struct yagl_gles3_sync *sync_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT5(glGetSynciv, GLsync, GLenum, GLsizei, GLsizei*, GLint*, sync, pname, bufSize, length, values);

    YAGL_GET_CTX();

    sync_obj = (struct yagl_gles3_sync*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_SYNC, (yagl_object_name)sync);

    if (!sync_obj) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    switch (pname) {
    case GL_OBJECT_TYPE:
        if (bufSize > 0) {
            *values = GL_SYNC_FENCE;
            if (length) {
                *length = 1;
            }
        } else if (length) {
            *length = 0;
        }
        break;
    case GL_SYNC_STATUS:
        if (bufSize > 0) {
            *values = (sync_obj->egl_fence->signaled(sync_obj->egl_fence) ? GL_SIGNALED : GL_UNSIGNALED);
            if (length) {
                *length = 1;
            }
        } else if (length) {
            *length = 0;
        }
        break;
    case GL_SYNC_CONDITION:
        if (bufSize > 0) {
            *values = GL_SYNC_GPU_COMMANDS_COMPLETE;
            if (length) {
                *length = 1;
            }
        } else if (length) {
            *length = 0;
        }
        break;
    case GL_SYNC_FLAGS:
        if (length) {
            *length = 0;
        }
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
    }

out:
    yagl_gles3_sync_release(sync_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetInteger64v(GLenum pname, GLint64 *params)
{
    uint32_t i, num = 0;
    int needs_map = 0;
    GLint ints[100]; // This fits all cases.

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetInteger64v, GLenum, GLint64*, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles_context_get_integerv(&ctx->base.base, pname, ints, &num)) {
        GLfloat floats[100]; // This fits all cases.
        if (yagl_gles_context_get_floatv(&ctx->base.base, pname, floats, &num, &needs_map)) {
            for (i = 0; i < num; ++i) {
                if (needs_map) {
                    params[i] = 2147483647.0 * floats[i];
                } else {
                    params[i] = floats[i];
                }
            }
        } else {
            YAGL_SET_ERR(GL_INVALID_ENUM);
        }
    } else {
        for (i = 0; i < num; ++i) {
            params[i] = ints[i];
        }
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetIntegeri_v(GLenum target, GLuint index, GLint *data)
{
    uint32_t num = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetIntegeri_v, GLenum, GLuint, GLint*, target, index, data);

    YAGL_GET_CTX();

    yagl_gles3_context_get_integerv_indexed(ctx,
                                            target,
                                            index,
                                            data,
                                            &num);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetInteger64i_v(GLenum target, GLuint index, GLint64 *data)
{
    uint32_t i, num = 0;
    GLint ints[100]; // This fits all cases.

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetInteger64i_v, GLenum, GLuint, GLint64*, target, index, data);

    YAGL_GET_CTX();

    if (yagl_gles3_context_get_integerv_indexed(ctx,
                                                target,
                                                index,
                                                ints,
                                                &num)) {
        for (i = 0; i < num; ++i) {
            data[i] = ints[i];
        }
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API const GLubyte *glGetStringi(GLenum name, GLuint index)
{
    const char *str = NULL;

    YAGL_LOG_FUNC_ENTER(glGetStringi, "name = 0x%X, index = %u", name, index);

    YAGL_GET_CTX_RET(NULL);

    switch (name) {
    case GL_EXTENSIONS:
        if (index >= ctx->base.base.num_extensions) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            break;
        }
        str = ctx->base.base.extensions[index];
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        break;
    }

    YAGL_LOG_FUNC_EXIT("%s", str);

    return (const GLubyte*)str;
}

YAGL_API void glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64 *params)
{
    struct yagl_gles_buffer *buffer_obj = NULL;
    GLint int_param = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetBufferParameteri64v, GLenum, GLenum, GLint64*, target, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles_context_acquire_binded_buffer(&ctx->base.base, target, &buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!buffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (yagl_gles_buffer_get_parameter(buffer_obj,
                                       pname,
                                       &int_param)) {
        params[0] = int_param;
    } else {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

out:
    yagl_gles_buffer_release(buffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    struct yagl_gles_array *array;

    YAGL_LOG_FUNC_ENTER_SPLIT5(glVertexAttribIPointer, GLuint, GLint, GLenum, GLsizei, const void*, index, size, type, stride, pointer);

    YAGL_GET_CTX();

    if (index >= ctx->base.base.num_arrays) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    array = &ctx->base.base.vao->arrays[index];

    if (ctx->base.base.vbo) {
        if (!yagl_gles_array_update_vbo(array,
                                        size,
                                        type,
                                        0,
                                        GL_FALSE,
                                        stride,
                                        ctx->base.base.vbo,
                                        (GLint)pointer,
                                        1)) {
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
        if ((ctx->base.base.vao != ctx->base.base.va_zero) && pointer) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }

        if (!yagl_gles_array_update(array,
                                    size,
                                    type,
                                    0,
                                    GL_FALSE,
                                    stride,
                                    pointer,
                                    1)) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params)
{
    GLint tmp[100]; // This fits all cases.
    int32_t num = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetVertexAttribIiv, GLuint, GLenum, GLint*, index, pname, params);

    YAGL_GET_CTX();

    if (yagl_gles2_context_get_array_param(&ctx->base,
                                           index,
                                           pname,
                                           params)) {
    } else {
        yagl_host_glGetVertexAttribIiv(index, pname, tmp, sizeof(tmp)/sizeof(tmp[0]), &num);
        if (params) {
            memcpy(params, tmp, num * sizeof(tmp[0]));
        }
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params)
{
    GLint param = 0;
    GLuint tmp[100]; // This fits all cases.
    int32_t num = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetVertexAttribIuiv, GLuint, GLenum, GLuint*, index, pname, params);

    YAGL_GET_CTX();

    if (yagl_gles2_context_get_array_param(&ctx->base,
                                           index,
                                           pname,
                                           &param)) {
        if (params) {
            params[0] = param;
        }
    } else {
        yagl_host_glGetVertexAttribIuiv(index, pname, tmp, sizeof(tmp)/sizeof(tmp[0]), &num);
        if (params) {
            memcpy(params, tmp, num * sizeof(tmp[0]));
        }
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET5(glVertexAttribI4i, GLuint, GLint, GLint, GLint, GLint, index, x, y, z, w)
YAGL_IMPLEMENT_API_NORET5(glVertexAttribI4ui, GLuint, GLuint, GLuint, GLuint, GLuint, index, x, y, z, w)

YAGL_API void glVertexAttribI4iv(GLuint index, const GLint *v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glVertexAttribI4iv, GLuint, const GLint*, index, v);
    yagl_host_glVertexAttribI4iv(index, v, 4);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glVertexAttribI4uiv(GLuint index, const GLuint *v)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glVertexAttribI4uiv, GLuint, const GLuint*, index, v);
    yagl_host_glVertexAttribI4uiv(index, v, 4);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetUniformuiv(GLuint program, GLint location, GLuint *params)
{
    struct yagl_gles2_program *program_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetUniformuiv, GLuint, GLint, GLuint*, program, location, params);

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

    if (location < 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles3_program_get_uniformuiv(program_obj, location, params)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform1ui(GLint location, GLuint v0)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glUniform1ui, GLint, GLuint, location, v0);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform1ui(ctx->base.program, location, v0)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform2ui(GLint location, GLuint v0, GLuint v1)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform2ui, GLint, GLuint, GLuint, location, v0, v1);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform2ui(ctx->base.program, location, v0, v1)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniform3ui, GLint, GLuint, GLuint, GLuint, location, v0, v1, v2);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform3ui(ctx->base.program, location, v0, v1, v2)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glUniform4ui, GLint, GLuint, GLuint, GLuint, GLuint, location, v0, v1, v2, v3);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform4ui(ctx->base.program, location, v0, v1, v2, v3)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform1uiv(GLint location, GLsizei count, const GLuint *value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform1uiv, GLint, GLsizei, const GLuint*, location, count, value);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform1uiv(ctx->base.program, location, count, value)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform2uiv(GLint location, GLsizei count, const GLuint *value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform2uiv, GLint, GLsizei, const GLuint*, location, count, value);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform2uiv(ctx->base.program, location, count, value)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform3uiv(GLint location, GLsizei count, const GLuint *value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform3uiv, GLint, GLsizei, const GLuint*, location, count, value);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform3uiv(ctx->base.program, location, count, value)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniform4uiv(GLint location, GLsizei count, const GLuint *value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniform4uiv, GLint, GLsizei, const GLuint*, location, count, value);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform4uiv(ctx->base.program, location, count, value)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix2x3fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform_matrix2x3fv(ctx->base.program, location, count, transpose, value)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix2x4fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform_matrix2x4fv(ctx->base.program, location, count, transpose, value)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix3x2fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform_matrix3x2fv(ctx->base.program, location, count, transpose, value)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix3x4fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform_matrix3x4fv(ctx->base.program, location, count, transpose, value)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix4x2fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform_matrix4x2fv(ctx->base.program, location, count, transpose, value)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glUniformMatrix4x3fv, GLint, GLsizei, GLboolean, const GLfloat*, location, count, transpose, value);

    YAGL_GET_CTX();

    if (!ctx->base.program) {
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

    if (!yagl_gles3_program_uniform_matrix4x3fv(ctx->base.program, location, count, transpose, value)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glClearBufferiv, GLenum, GLint, const GLint*, buffer, drawbuffer, value);

    YAGL_GET_CTX();

    if (!yagl_gles3_is_buffer_valid(buffer)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    switch (buffer) {
    case GL_DEPTH:
    case GL_STENCIL:
        if (drawbuffer != 0) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
        yagl_host_glClearBufferiv(buffer, drawbuffer, value, 1);
        break;
    default:
        if ((drawbuffer < 0) ||
            (drawbuffer >= ctx->base.base.max_draw_buffers)) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
        yagl_host_glClearBufferiv(buffer, drawbuffer, value, 4);
        break;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glClearBufferuiv, GLenum, GLint, const GLuint*, buffer, drawbuffer, value);

    YAGL_GET_CTX();

    if (!yagl_gles3_is_buffer_valid(buffer)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    switch (buffer) {
    case GL_DEPTH:
    case GL_STENCIL:
        if (drawbuffer != 0) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
        yagl_host_glClearBufferuiv(buffer, drawbuffer, value, 1);
        break;
    default:
        if ((drawbuffer < 0) ||
            (drawbuffer >= ctx->base.base.max_draw_buffers)) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
        yagl_host_glClearBufferuiv(buffer, drawbuffer, value, 4);
        break;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glClearBufferfi, GLenum, GLint, GLfloat, GLint, buffer, drawbuffer, depth, stencil);

    YAGL_GET_CTX();

    if (buffer != GL_DEPTH_STENCIL) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (drawbuffer != 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    yagl_host_glClearBufferfi(buffer, drawbuffer, depth, stencil);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glClearBufferfv, GLenum, GLint, const GLfloat*, buffer, drawbuffer, value);

    YAGL_GET_CTX();

    if (!yagl_gles3_is_buffer_valid(buffer)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    switch (buffer) {
    case GL_DEPTH:
    case GL_STENCIL:
        if (drawbuffer != 0) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
        yagl_host_glClearBufferfv(buffer, drawbuffer, value, 1);
        break;
    default:
        if ((drawbuffer < 0) ||
            (drawbuffer >= ctx->base.base.max_draw_buffers)) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
        yagl_host_glClearBufferfv(buffer, drawbuffer, value, 4);
        break;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API GLint glGetFragDataLocation(GLuint program, const GLchar *name)
{
    struct yagl_gles2_program *program_obj = NULL;
    GLint ret = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetFragDataLocation, GLuint, const GLchar*, program, name);

    YAGL_GET_CTX_RET(0);

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

    ret = yagl_gles3_program_get_frag_data_location(program_obj, name);

out:
    yagl_gles2_program_release(program_obj);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLint, ret);

    return ret;
}

YAGL_API void glDrawRangeElements(GLenum mode, GLuint start, GLuint end,
                                  GLsizei count, GLenum type,
                                  const void *indices)
{
    int index_size = 0;
    GLuint i;

    YAGL_LOG_FUNC_ENTER_SPLIT6(glDrawRangeElements, GLenum, GLuint, GLuint, GLsizei, GLenum, const void*, mode, start, end, count, type, indices);

    YAGL_GET_CTX();

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (end < start) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles_get_index_size(type, &index_size)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!ctx->base.base.vao->ebo && !indices) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (count == 0) {
        goto out;
    }

    yagl_render_invalidate(0);

    for (i = 0; i < ctx->base.base.vao->num_arrays; ++i) {
        if (!ctx->base.base.vao->arrays[i].enabled) {
            continue;
        }

        yagl_gles_array_transfer(&ctx->base.base.vao->arrays[i],
                                 start,
                                 end + 1 - start,
                                 -1);
    }

    if (ctx->base.base.vao->ebo) {
        yagl_gles_buffer_bind(ctx->base.base.vao->ebo,
                              type,
                              0,
                              GL_ELEMENT_ARRAY_BUFFER);
        yagl_gles_buffer_transfer(ctx->base.base.vao->ebo,
                                  type,
                                  GL_ELEMENT_ARRAY_BUFFER,
                                  0);
        yagl_gles3_context_draw_range_elements(ctx, mode, start, end, count,
                                               type, NULL, (int32_t)indices);
        yagl_host_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    } else {
        yagl_gles3_context_draw_range_elements(ctx, mode, start, end, count,
                                               type, indices, count * index_size);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glGetProgramBinary, GLuint, GLsizei, GLsizei*, GLenum*, void*, program, bufSize, length, binaryFormat, binary);

    YAGL_GET_CTX();

    YAGL_SET_ERR(GL_INVALID_OPERATION);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glProgramBinary(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glProgramBinary, GLuint, GLenum, const void*, GLsizei, program, binaryFormat, binary, length);

    YAGL_GET_CTX();

    YAGL_SET_ERR(GL_INVALID_OPERATION);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glProgramParameteri, GLuint, GLenum, GLint, program, pname, value);

    YAGL_GET_CTX();

    YAGL_SET_ERR(GL_INVALID_OPERATION);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glGetInternalformativ, GLenum, GLenum, GLenum, GLsizei, GLint*, target, internalformat, pname, bufSize, params);

    YAGL_GET_CTX();

    YAGL_SET_ERR(GL_INVALID_OPERATION);

    YAGL_LOG_FUNC_EXIT(NULL);
}
