#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yagl_host_gles_calls.h"
#include "yagl_impl.h"
#include "yagl_malloc.h"
#include "yagl_gles_context.h"
#include "yagl_gles_utils.h"
#include "yagl_gles_image.h"
#include "yagl_render.h"

static void yagl_update_pack_alignment(void)
{
    struct yagl_gles_context *ctx = yagl_gles_context_get();

    if (!ctx || ctx->pack_alignment) {
        return;
    }

    ctx->pack_alignment = yagl_get_integer(GL_PACK_ALIGNMENT);

    if (!ctx->pack_alignment) {
        ctx->pack_alignment = 1;
    }
}

static void yagl_update_unpack_alignment(void)
{
    struct yagl_gles_context *ctx = yagl_gles_context_get();

    if (!ctx || ctx->unpack_alignment) {
        return;
    }

    ctx->unpack_alignment = yagl_get_integer(GL_UNPACK_ALIGNMENT);

    if (!ctx->unpack_alignment) {
        ctx->unpack_alignment = 1;
    }
}

static inline void yagl_update_ebo(void)
{
    struct yagl_gles_context *ctx = yagl_gles_context_get();

    if (!ctx) {
        return;
    }

    if (!ctx->ebo_valid) {
        ctx->ebo = yagl_get_integer(GL_ELEMENT_ARRAY_BUFFER_BINDING);
        ctx->ebo_valid = 1;
    }
}

static void yagl_get_minmax_index(const GLvoid *indices,
                                  GLsizei count,
                                  GLenum type,
                                  GLint *range_first,
                                  GLsizei *range_count)
{
    uint32_t first_idx = UINT32_MAX, last_idx = 0;
    int i;

    if (!indices) {
        *range_count = 0;
        return;
    }

    switch (type) {
    case GL_UNSIGNED_BYTE:
        for (i = 0; i < count; ++i) {
            uint32_t idx = ((uint8_t*)indices)[i];
            if (idx < first_idx) {
                first_idx = idx;
            }
            if (idx > last_idx) {
                last_idx = idx;
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        for (i = 0; i < count; ++i) {
            uint32_t idx = ((uint16_t*)indices)[i];
            if (idx < first_idx) {
                first_idx = idx;
            }
            if (idx > last_idx) {
                last_idx = idx;
            }
        }
        break;
    default:
        *range_count = 0;
        return;
    }

    if (count > 0) {
        *range_first = first_idx;
        *range_count = last_idx + 1 - first_idx;
    } else {
        *range_count = 0;
    }
}

YAGL_IMPLEMENT_API_NORET1(glActiveTexture, GLenum, texture)

YAGL_API void glBindBuffer(GLenum target, GLuint buffer)
{
    struct yagl_gles_context *ctx;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glBindBuffer, GLenum, GLuint, target, buffer);

    ctx = yagl_gles_context_get();

    if (ctx) {
        /*
         * The trick is: we only update VBO/EBO here when they've
         * already been fetched from the host, otherwise we don't care,
         * we'll fetch them later anyway.
         */
        switch (target) {
        case GL_ARRAY_BUFFER:
            if (ctx->vbo_valid) {
                ctx->vbo = buffer;
            }
            break;
        case GL_ELEMENT_ARRAY_BUFFER:
            if (ctx->ebo_valid) {
                ctx->ebo = buffer;
            }
            break;
        default:
            break;
        }
    }

    yagl_host_glBindBuffer(target, buffer);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET2(glBindFramebuffer, GLenum, GLuint, target, framebuffer)
YAGL_IMPLEMENT_API_NORET2(glBindRenderbuffer, GLenum, GLuint, target, renderbuffer)
YAGL_IMPLEMENT_API_NORET2(glBindTexture, GLenum, GLuint, target, texture)
YAGL_IMPLEMENT_API_NORET1(glBlendEquation, GLenum, mode)
YAGL_IMPLEMENT_API_NORET2(glBlendEquationSeparate, GLenum, GLenum, modeRGB, modeAlpha)
YAGL_IMPLEMENT_API_NORET2(glBlendFunc, GLenum, GLenum, sfactor, dfactor)
YAGL_IMPLEMENT_API_NORET4(glBlendFuncSeparate, GLenum, GLenum, GLenum, GLenum, srcRGB, dstRGB, srcAlpha, dstAlpha)

YAGL_API void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glBufferData, GLenum, GLsizeiptr, const GLvoid*, GLenum, target, size, data, usage);
    yagl_host_glBufferData(target, data, size, usage);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glBufferSubData, GLenum, GLintptr, GLsizeiptr, const GLvoid*, target, offset, size, data);
    yagl_host_glBufferSubData(target, offset, data, size);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_RET1(GLenum, glCheckFramebufferStatus, GLenum, target)

YAGL_API void glClear(GLbitfield mask)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glClear, GLbitfield, mask);
    yagl_render_invalidate();
    yagl_host_glClear(mask);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET4(glClearColor, GLclampf, GLclampf, GLclampf, GLclampf, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET1(glClearDepthf, GLclampf, depth)
YAGL_IMPLEMENT_API_NORET1(glClearStencil, GLint, s)
YAGL_IMPLEMENT_API_NORET4(glColorMask, GLboolean, GLboolean, GLboolean, GLboolean, red, green, blue, alpha)

YAGL_API void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
    YAGL_LOG_FUNC_ENTER_SPLIT8(glCompressedTexImage2D, GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*, target, level, internalformat, width, height, border, imageSize, data);
    yagl_host_glCompressedTexImage2D(target, level, internalformat, width, height, border, data, imageSize);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    YAGL_LOG_FUNC_ENTER_SPLIT9(glCompressedTexSubImage2D, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid*, target, level, xoffset, yoffset, width, height, format, imageSize, data);
    yagl_host_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, data, imageSize);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET8(glCopyTexImage2D, GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, target, level, internalformat, x, y, width, height, border)
YAGL_IMPLEMENT_API_NORET8(glCopyTexSubImage2D, GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, target, level, xoffset, yoffset, x, y, width, height)
YAGL_IMPLEMENT_API_NORET1(glCullFace, GLenum, mode)

YAGL_API void glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
    struct yagl_gles_context *ctx;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteBuffers, GLsizei, const GLuint*, n, buffers);

    ctx = yagl_gles_context_get();

    if (ctx) {
        /*
         * When glDeleteBuffers is called VBO/EBOs are unbound from the
         * context.
         */
        GLsizei i;

        for (i = 0; i < n; ++i) {
            if (ctx->vbo_valid && (buffers[i] == ctx->vbo)) {
                ctx->vbo = 0;
            }
            if (ctx->ebo_valid && (buffers[i] == ctx->ebo)) {
                ctx->ebo = 0;
            }
        }
    }

    yagl_host_glDeleteBuffers(buffers, n);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteFramebuffers, GLsizei, const GLuint*, n, framebuffers);
    yagl_host_glDeleteFramebuffers(framebuffers, n);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteRenderbuffers, GLsizei, const GLuint*, n, renderbuffers);
    yagl_host_glDeleteRenderbuffers(renderbuffers, n);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDeleteTextures(GLsizei n, const GLuint *textures)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteTextures, GLsizei, const GLuint*, n, textures);
    yagl_host_glDeleteTextures(textures, n);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET1(glDepthFunc, GLenum, func)
YAGL_IMPLEMENT_API_NORET1(glDepthMask, GLboolean, flag)
YAGL_IMPLEMENT_API_NORET2(glDepthRangef, GLclampf, GLclampf, zNear, zFar)
YAGL_IMPLEMENT_API_NORET1(glDisable, GLenum, cap)

YAGL_API void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    struct yagl_gles_context *ctx;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glDrawArrays, GLenum, GLint, GLsizei, mode, first, count);

    yagl_render_invalidate();

    yagl_update_ebo();
    yagl_update_arrays();

    ctx = yagl_gles_context_get();

    if (ctx) {
        GLuint i;
        for (i = 0; i < ctx->num_arrays; ++i) {
            if (ctx->arrays[i].enabled &&
                (ctx->arrays[i].vbo == 0) &&
                ctx->arrays[i].ptr &&
                (first >= 0) &&
                (count > 0)) {
                /*
                 * Enabled array without bounded VBO, transfer it.
                 */
                yagl_host_glTransferArrayYAGL(i,
                    first,
                    (const uint8_t*)ctx->arrays[i].ptr + first * ctx->arrays[i].stride,
                    count * ctx->arrays[i].stride);
            }
        }
    }

    yagl_host_glDrawArrays(mode, first, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    struct yagl_gles_context *ctx;
    int have_range = 0;
    GLint range_first;
    GLsizei range_count = 0;
    GLuint i;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glDrawElements, GLenum, GLsizei, GLenum, const GLvoid*, mode, count, type, indices);

    yagl_render_invalidate();

    yagl_update_ebo();
    yagl_update_arrays();

    ctx = yagl_gles_context_get();

    if (!ctx) {
        /*
         * Just to report about missing context.
         */

        yagl_host_glDrawElementsOffsetYAGL(mode, type, (GLsizei)indices, count);

        YAGL_LOG_FUNC_EXIT(NULL);

        return;
    }

    for (i = 0; i < ctx->num_arrays; ++i) {
        if (!ctx->arrays[i].enabled ||
            (ctx->arrays[i].vbo != 0) ||
            !ctx->arrays[i].ptr) {
            continue;
        }

        /*
         * Enabled array without bounded VBO, transfer it.
         */

        if (!have_range) {
            if (ctx->ebo == 0) {
                yagl_get_minmax_index(indices, count, type,
                                      &range_first, &range_count);
            } else {
                yagl_host_glGetVertexAttribRangeYAGL(type,
                                                     (GLsizei)indices,
                                                     count,
                                                     &range_first,
                                                     &range_count);
            }
            have_range = 1;
        }

        if (range_count > 0) {
            yagl_host_glTransferArrayYAGL(i,
                range_first,
                (const uint8_t*)ctx->arrays[i].ptr + range_first * ctx->arrays[i].stride,
                range_count * ctx->arrays[i].stride);
        }
    }

    if (ctx->ebo == 0) {
        int index_size;

        if (yagl_get_index_size(type, &index_size)) {
            count *= index_size;
        } else {
            count = 0;
        }

        yagl_host_glDrawElementsIndicesYAGL(mode, type, indices, count);
    } else {
        yagl_host_glDrawElementsOffsetYAGL(mode, type, (GLsizei)indices, count);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET1(glEnable, GLenum, cap)

YAGL_API void glFinish()
{
    YAGL_LOG_FUNC_ENTER_SPLIT0(glFinish);
    yagl_render_finish();
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET0(glFlush)
YAGL_IMPLEMENT_API_NORET4(glFramebufferRenderbuffer, GLenum, GLenum, GLenum, GLuint, target, attachment, renderbuffertarget, renderbuffer)
YAGL_IMPLEMENT_API_NORET5(glFramebufferTexture2D, GLenum, GLenum, GLenum, GLuint, GLint, target, attachment, textarget, texture, level)
YAGL_IMPLEMENT_API_NORET1(glFrontFace, GLenum, mode)

YAGL_API void glGenBuffers(GLsizei n, GLuint *buffers)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenBuffers, GLsizei, GLuint*, n, buffers);
    yagl_host_glGenBuffers(buffers, n, NULL);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET1(glGenerateMipmap, GLenum, target)

YAGL_API void glGenFramebuffers(GLsizei n, GLuint *framebuffers)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenFramebuffers, GLsizei, GLuint*, n, framebuffers);
    yagl_host_glGenFramebuffers(framebuffers, n, NULL);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenRenderbuffers, GLsizei, GLuint*, n, renderbuffers);
    yagl_host_glGenRenderbuffers(renderbuffers, n, NULL);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGenTextures(GLsizei n, GLuint *textures)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenTextures, GLsizei, GLuint*, n, textures);
    yagl_host_glGenTextures(textures, n, NULL);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetBooleanv(GLenum pname, GLboolean *params)
{
    GLboolean tmp[100]; // This fits all cases.
    int32_t num = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetBooleanv, GLenum, GLboolean*, pname, params);
    yagl_host_glGetBooleanv(pname, tmp, sizeof(tmp)/sizeof(tmp[0]), &num);
    if (params) {
        memcpy(params, tmp, num * sizeof(tmp[0]));
    }
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetBufferParameteriv, GLenum, GLenum, GLint*, target, pname, params);
    yagl_host_glGetBufferParameteriv(target, pname, params);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_RET0(GLenum, glGetError)

YAGL_API void glGetFloatv(GLenum pname, GLfloat *params)
{
    GLfloat tmp[100]; // This fits all cases.
    int32_t num = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetFloatv, GLenum, GLfloat*, pname, params);
    yagl_host_glGetFloatv(pname, tmp, sizeof(tmp)/sizeof(tmp[0]), &num);
    if (params) {
        memcpy(params, tmp, num * sizeof(tmp[0]));
    }
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetFramebufferAttachmentParameteriv, GLenum, GLenum, GLenum, GLint*, target, attachment, pname, params);
    yagl_host_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetIntegerv(GLenum pname, GLint *params)
{
    GLint tmp[100]; // This fits all cases.
    int32_t num = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetIntegerv, GLenum, GLint*, pname, params);
    yagl_host_glGetIntegerv(pname, tmp, sizeof(tmp)/sizeof(tmp[0]), &num);
    if (params) {
        memcpy(params, tmp, num * sizeof(tmp[0]));
    }
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetRenderbufferParameteriv, GLenum, GLenum, GLint*, target, pname, params);
    yagl_host_glGetRenderbufferParameteriv(target, pname, params);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetTexParameterfv, GLenum, GLenum, GLfloat*, target, pname, params);
    yagl_host_glGetTexParameterfv(target, pname, params);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetTexParameteriv, GLenum, GLenum, GLint*, target, pname, params);
    yagl_host_glGetTexParameteriv(target, pname, params);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET2(glHint, GLenum, GLenum, target, mode)
YAGL_IMPLEMENT_API_RET1(GLboolean, glIsBuffer, GLuint, buffer)
YAGL_IMPLEMENT_API_RET1(GLboolean, glIsEnabled, GLenum, cap)
YAGL_IMPLEMENT_API_RET1(GLboolean, glIsFramebuffer, GLuint, framebuffer)
YAGL_IMPLEMENT_API_RET1(GLboolean, glIsRenderbuffer, GLuint, renderbuffer)
YAGL_IMPLEMENT_API_RET1(GLboolean, glIsTexture, GLuint, texture)
YAGL_IMPLEMENT_API_NORET1(glLineWidth, GLfloat, width)

YAGL_API void glPixelStorei(GLenum pname, GLint param)
{
    struct yagl_gles_context *ctx;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glPixelStorei, GLenum, GLint, pname, param);

    switch (pname) {
    case GL_PACK_ALIGNMENT:
        ctx = yagl_gles_context_get();
        if (ctx) {
            /*
             * Just reset it, we'll read it again later if needed.
             */
            ctx->pack_alignment = 0;
        }
        break;
    case GL_UNPACK_ALIGNMENT:
        ctx = yagl_gles_context_get();
        if (ctx) {
            /*
             * Just reset it, we'll read it again later if needed.
             */
            ctx->unpack_alignment = 0;
        }
        break;
    default:
        break;
    }

    yagl_host_glPixelStorei(pname, param);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET2(glPolygonOffset, GLfloat, GLfloat, factor, units)

YAGL_API void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
    struct yagl_gles_context *ctx;

    YAGL_LOG_FUNC_ENTER_SPLIT7(glReadPixels, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*, x, y, width, height, format, type, pixels);

    yagl_render_invalidate();

    yagl_update_pack_alignment();

    ctx = yagl_gles_context_get();

    if (ctx) {
        GLsizei stride;
        if (yagl_gles_get_stride(ctx->pack_alignment,
                                 width,
                                 format,
                                 type,
                                 &stride)) {
            yagl_host_glReadPixels(x, y, width, height, format, type, pixels, stride * height, NULL);

            YAGL_LOG_FUNC_EXIT(NULL);

            return;
        }
    }

    /*
     * Just to report about missing context / bad format.
     */

    yagl_host_glReadPixels(x, y, width, height, format, type, pixels, 0, NULL);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET4(glRenderbufferStorage, GLenum, GLenum, GLsizei, GLsizei, target, internalformat, width, height)
YAGL_IMPLEMENT_API_NORET2(glSampleCoverage, GLclampf, GLboolean, value, invert)

YAGL_API void glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glScissor, GLint, GLint, GLsizei, GLsizei, x, y, width, height);
    yagl_render_invalidate();
    yagl_host_glScissor(x, y, width, height);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET3(glStencilFunc, GLenum, GLint, GLuint, func, ref, mask)
YAGL_IMPLEMENT_API_NORET1(glStencilMask, GLuint, mask)
YAGL_IMPLEMENT_API_NORET3(glStencilOp, GLenum, GLenum, GLenum, fail, zfail, zpass)

YAGL_API void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    struct yagl_gles_context *ctx;

    YAGL_LOG_FUNC_ENTER_SPLIT9(glTexImage2D, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*, target, level, internalformat, width, height, border, format, type, pixels);

    yagl_update_unpack_alignment();

    ctx = yagl_gles_context_get();

    if (ctx) {
        GLsizei stride;
        if (yagl_gles_get_stride(ctx->unpack_alignment,
                                 width,
                                 format,
                                 type,
                                 &stride)) {
            yagl_host_glTexImage2D(target,
                                   level,
                                   internalformat,
                                   width,
                                   height,
                                   border,
                                   format,
                                   type,
                                   pixels,
                                   stride * height);

            YAGL_LOG_FUNC_EXIT(NULL);

            return;
        }
    }

    /*
     * Just to report about missing context / bad format.
     */

    yagl_host_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels, 0);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET3(glTexParameterf, GLenum, GLenum, GLfloat, target, pname, param)

YAGL_API void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexParameterfv, GLenum, GLenum, const GLfloat*, target, pname, params);
    yagl_host_glTexParameterfv(target, pname, params, 1);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET3(glTexParameteri, GLenum, GLenum, GLint, target, pname, param)

YAGL_API void glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexParameteriv, GLenum, GLenum, const GLint*, target, pname, params);
    yagl_host_glTexParameteriv(target, pname, params, 1);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    struct yagl_gles_context *ctx;

    YAGL_LOG_FUNC_ENTER_SPLIT9(glTexSubImage2D, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*, target, level, xoffset, yoffset, width, height, format, type, pixels);

    yagl_update_unpack_alignment();

    ctx = yagl_gles_context_get();

    if (ctx) {
        GLsizei stride;
        if (yagl_gles_get_stride(ctx->unpack_alignment,
                                 width,
                                 format,
                                 type,
                                 &stride)) {
            yagl_host_glTexSubImage2D(target,
                                      level,
                                      xoffset,
                                      yoffset,
                                      width,
                                      height,
                                      format,
                                      type,
                                      pixels,
                                      stride * height);

            YAGL_LOG_FUNC_EXIT(NULL);

            return;
        }
    }

    /*
     * Just to report about missing context / bad format.
     */

    yagl_host_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels, 0);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glViewport, GLint, GLint, GLsizei, GLsizei, x, y, width, height);
    yagl_render_invalidate();
    yagl_host_glViewport(x, y, width, height);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image)
{
    struct yagl_gles_image *image_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glEGLImageTargetTexture2DOES, GLenum, GLeglImageOES, target, image);

    image_obj = yagl_gles_image_acquire(image);

    if (!image_obj) {
        YAGL_LOG_ERROR("yagl_gles_image_acquire failed for %p", image);
        goto out;
    }

    image_obj->update(image_obj);

    yagl_host_glEGLImageTargetTexture2DOES(target, image_obj->host_image);

out:
    yagl_gles_image_release(image_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glEGLImageTargetRenderbufferStorageOES(GLenum target, GLeglImageOES image)
{
    fprintf(stderr, "glEGLImageTargetRenderbufferStorageOES not supported in YaGL\n");
}
