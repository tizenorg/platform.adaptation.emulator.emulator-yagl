#include "GLES3/gl3.h"
#include "yagl_host_gles_calls.h"
#include "yagl_gles3_context.h"
#include "yagl_sharegroup.h"
#include "yagl_gles_buffer.h"
#include "yagl_log.h"
#include "yagl_impl.h"

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
    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetUniformBlockIndex, GLuint, const GLchar*, program, uniformBlockName);

    YAGL_GET_CTX_RET(0);

    YAGL_LOG_FUNC_EXIT(NULL);

    return 0;
}

YAGL_API void glGetUniformIndices(GLuint program, GLsizei uniformCount,
                                  const GLchar *const *uniformNames,
                                  GLuint *uniformIndices)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetUniformIndices, GLuint, GLsizei, const GLchar* const*, GLuint*, program, uniformCount, uniformNames, uniformIndices);

    YAGL_GET_CTX();

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetActiveUniformsiv(GLuint program, GLsizei uniformCount,
                                    const GLuint *uniformIndices,
                                    GLenum pname, GLint *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glGetActiveUniformsiv, GLuint, GLsizei, const GLuint*, GLenum, GLint*, program, uniformCount, uniformIndices, pname, params);

    YAGL_GET_CTX();

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex,
                                    GLuint uniformBlockBinding)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glUniformBlockBinding, GLuint, GLuint, GLuint, program, uniformBlockIndex, uniformBlockBinding);

    YAGL_GET_CTX();

    YAGL_LOG_FUNC_EXIT(NULL);
}
