#include "GL/gl.h"
#include "GL/glext.h"
#include "yagl_gles_calls.h"
#include "yagl_host_gles_calls.h"
#include "yagl_impl.h"
#include "yagl_malloc.h"
#include "yagl_render.h"
#include "yagl_utils.h"
#include "yagl_sharegroup.h"
#include "yagl_state.h"
#include "yagl_gles_context.h"
#include "yagl_gles_vertex_array.h"
#include "yagl_gles_buffer.h"
#include "yagl_gles_texture.h"
#include "yagl_gles_texture_unit.h"
#include "yagl_gles_framebuffer.h"
#include "yagl_gles_renderbuffer.h"
#include "yagl_gles_image.h"
#include "yagl_gles_array.h"
#include "yagl_gles_validate.h"
#include "yagl_gles_utils.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define YAGL_SET_ERR(err) \
    yagl_gles_context_set_error(ctx, err); \
    YAGL_LOG_ERROR("error = 0x%X", err)

#define YAGL_GET_CTX_IMPL(ret_expr) \
    struct yagl_gles_context *ctx = \
        (struct yagl_gles_context*)yagl_get_client_context(); \
    if (!ctx) { \
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

YAGL_IMPLEMENT_API_NORET1(glClearStencil, GLint, s)
YAGL_IMPLEMENT_API_NORET2(glHint, GLenum, GLenum, target, mode)

YAGL_API void glLineWidth(GLfloat width)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glLineWidth, GLfloat, width);

    YAGL_GET_CTX();

    yagl_gles_context_line_width(ctx, width);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glPolygonOffset(GLfloat factor, GLfloat units)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glPolygonOffset, GLfloat, GLfloat, factor, units);

    YAGL_GET_CTX();

    yagl_gles_context_polygon_offset(ctx, factor, units);

    YAGL_LOG_FUNC_EXIT(NULL);
}

void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glRenderbufferStorage, GLenum, GLenum, GLsizei, GLsizei, target, internalformat, width, height);

    YAGL_GET_CTX();

    yagl_host_glRenderbufferStorage(target, internalformat, width, height);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glSampleCoverage(GLclampf value, GLboolean invert)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glSampleCoverage, GLclampf, GLboolean, value, invert);

    YAGL_GET_CTX();

    yagl_gles_context_sample_coverage(ctx, value, invert);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glScissor, GLint, GLint, GLsizei, GLsizei, x, y, width, height);
    yagl_render_invalidate(0);
    yagl_host_glScissor(x, y, width, height);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET3(glStencilFunc, GLenum, GLint, GLuint, func, ref, mask)
YAGL_IMPLEMENT_API_NORET1(glStencilMask, GLuint, mask)
YAGL_IMPLEMENT_API_NORET3(glStencilOp, GLenum, GLenum, GLenum, fail, zfail, zpass)

YAGL_API void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexParameterf, GLenum, GLenum, GLfloat, target, pname, param);

    YAGL_GET_CTX();

    yagl_gles_context_tex_parameterf(ctx, target, pname, param);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexParameterfv, GLenum, GLenum, const GLfloat*, target, pname, params);

    YAGL_GET_CTX();

    yagl_gles_context_tex_parameterfv(ctx, target, pname, params);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET3(glTexParameteri, GLenum, GLenum, GLint, target, pname, param)

YAGL_API void glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexParameteriv, GLenum, GLenum, const GLint*, target, pname, params);
    yagl_host_glTexParameteriv(target, pname, params, 1);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glViewport, GLint, GLint, GLsizei, GLsizei, x, y, width, height);

    YAGL_GET_CTX();

    if ((width < 0) || (height < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    yagl_render_invalidate(0);

    ctx->have_viewport = 1;
    ctx->viewport[0] = x;
    ctx->viewport[1] = y;
    ctx->viewport[2] = width;
    ctx->viewport[3] = height;

    yagl_host_glViewport(x, y, width, height);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

/*
 * @}
 */

YAGL_API const GLubyte *glGetString(GLenum name)
{
    struct yagl_gles_context *ctx;
    const char *str = NULL;

    YAGL_LOG_FUNC_ENTER(glGetString, "name = 0x%X", name);

    ctx = (struct yagl_gles_context*)yagl_get_client_context();

    switch (name) {
    case GL_VENDOR:
        str = "Samsung";
        break;
    case GL_EXTENSIONS:
        if (ctx) {
            str = ctx->extension_string;
        } else {
            str = "";
        }
        break;
    default:
        if (ctx) {
            str = ctx->get_string(ctx, name);
        } else {
            str = "";
        }
    }

    YAGL_LOG_FUNC_EXIT("%s", str);

    return (const GLubyte*)str;
}

YAGL_API void glActiveTexture(GLenum texture)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glActiveTexture, GLenum, texture);

    YAGL_GET_CTX();

    yagl_gles_context_set_active_texture(ctx, texture);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glBindBuffer(GLenum target, GLuint buffer)
{
    struct yagl_gles_buffer *buffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glBindBuffer, GLenum, GLuint, target, buffer);

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

    yagl_gles_context_bind_buffer(ctx, target, buffer_obj);

out:
    yagl_gles_buffer_release(buffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

void glBindFramebuffer(GLenum target, GLuint framebuffer)
{
    struct yagl_gles_framebuffer *framebuffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glBindFramebuffer, GLenum, GLuint, target, framebuffer);

    YAGL_GET_CTX();

    if (framebuffer != 0) {
        framebuffer_obj = (struct yagl_gles_framebuffer*)yagl_namespace_acquire(&ctx->framebuffers,
            framebuffer);

        if (!framebuffer_obj) {
            framebuffer_obj = yagl_gles_framebuffer_create();

            if (!framebuffer_obj) {
                goto out;
            }

            framebuffer_obj = (struct yagl_gles_framebuffer*)yagl_namespace_add_named(&ctx->framebuffers,
               framebuffer, &framebuffer_obj->base);
        }
    }

    yagl_gles_context_bind_framebuffer(ctx, target, framebuffer_obj);

out:
    yagl_gles_framebuffer_release(framebuffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

void glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    struct yagl_gles_renderbuffer *renderbuffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glBindRenderbuffer, GLenum, GLuint, target, renderbuffer);

    YAGL_GET_CTX();

    if (renderbuffer != 0) {
        renderbuffer_obj = (struct yagl_gles_renderbuffer*)yagl_sharegroup_acquire_object(ctx->base.sg,
            YAGL_NS_RENDERBUFFER, renderbuffer);

        if (!renderbuffer_obj) {
            renderbuffer_obj = yagl_gles_renderbuffer_create();

            if (!renderbuffer_obj) {
                goto out;
            }

            renderbuffer_obj = (struct yagl_gles_renderbuffer*)yagl_sharegroup_add_named(ctx->base.sg,
               YAGL_NS_RENDERBUFFER, renderbuffer, &renderbuffer_obj->base);
        }
    }

    yagl_gles_context_bind_renderbuffer(ctx, target, renderbuffer_obj);

out:
    yagl_gles_renderbuffer_release(renderbuffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glBindTexture(GLenum target, GLuint texture)
{
    struct yagl_gles_texture *texture_obj = NULL;
    yagl_gles_texture_target texture_target;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glBindTexture, GLenum, GLuint, target, texture);

    YAGL_GET_CTX();

    if (!yagl_gles_context_validate_texture_target(ctx, target, &texture_target)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (texture != 0) {
        texture_obj = (struct yagl_gles_texture*)yagl_sharegroup_acquire_object(ctx->base.sg,
            YAGL_NS_TEXTURE, texture);

        if (!texture_obj) {
            texture_obj = yagl_gles_texture_create();

            if (!texture_obj) {
                goto out;
            }

            texture_obj = (struct yagl_gles_texture*)yagl_sharegroup_add_named(ctx->base.sg,
               YAGL_NS_TEXTURE, texture, &texture_obj->base);
        }
    }

    yagl_gles_context_bind_texture(ctx, target, texture_obj);

out:
    yagl_gles_texture_release(texture_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

void glBlendEquation(GLenum mode)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glBlendEquation, GLenum, mode);

    YAGL_GET_CTX();

    if (yagl_gles_is_blend_equation_valid(mode)) {
        ctx->blend_equation_rgb = mode;
        ctx->blend_equation_alpha = mode;

        yagl_host_glBlendEquation(mode);
    } else {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glBlendEquationSeparate, GLenum, GLenum, modeRGB, modeAlpha);

    YAGL_GET_CTX();

    if (yagl_gles_is_blend_equation_valid(modeRGB) &&
        yagl_gles_is_blend_equation_valid(modeAlpha)) {
        ctx->blend_equation_rgb = modeRGB;
        ctx->blend_equation_alpha = modeAlpha;

        yagl_host_glBlendEquationSeparate(modeRGB, modeAlpha);
    } else {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glBlendFunc, GLenum, GLenum, sfactor, dfactor);

    YAGL_GET_CTX();

    if (yagl_gles_is_blend_func_valid(sfactor) &&
        yagl_gles_is_blend_func_valid(dfactor)) {
        ctx->blend_src_rgb = sfactor;
        ctx->blend_src_alpha = sfactor;

        ctx->blend_dst_rgb = dfactor;
        ctx->blend_dst_alpha = dfactor;

        yagl_host_glBlendFunc(sfactor, dfactor);
    } else {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

void glBlendFuncSeparate(GLenum srcRGB,
                         GLenum dstRGB,
                         GLenum srcAlpha,
                         GLenum dstAlpha)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glBlendFuncSeparate, GLenum, GLenum, GLenum, GLenum, srcRGB, dstRGB, srcAlpha, dstAlpha);

    YAGL_GET_CTX();

    if (yagl_gles_is_blend_func_valid(srcRGB) &&
        yagl_gles_is_blend_func_valid(dstRGB) &&
        yagl_gles_is_blend_func_valid(srcAlpha) &&
        yagl_gles_is_blend_func_valid(dstAlpha)) {
        ctx->blend_src_rgb = srcRGB;
        ctx->blend_src_alpha = srcAlpha;

        ctx->blend_dst_rgb = dstRGB;
        ctx->blend_dst_alpha = dstAlpha;

        yagl_host_glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
    } else {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    struct yagl_gles_buffer *buffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glBufferData, GLenum, GLsizeiptr, const GLvoid*, GLenum, target, size, data, usage);

    YAGL_GET_CTX();

    if (size < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles_is_buffer_usage_valid(usage)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!yagl_gles_context_acquire_binded_buffer(ctx, target, &buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!buffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_gles_buffer_set_data(buffer_obj, size, data, usage);

out:
    yagl_gles_buffer_release(buffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
    struct yagl_gles_buffer *buffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glBufferSubData, GLenum, GLintptr, GLsizeiptr, const GLvoid*, target, offset, size, data);

    YAGL_GET_CTX();

    if ((offset < 0) || (size < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles_context_acquire_binded_buffer(ctx, target, &buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!buffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (size == 0) {
        goto out;
    }

    if (!yagl_gles_buffer_update_data(buffer_obj, offset, size, data)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

out:
    yagl_gles_buffer_release(buffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

GLenum glCheckFramebufferStatus(GLenum target)
{
    GLenum res = 0;
    struct yagl_gles_framebuffer *framebuffer_obj = NULL;
    struct yagl_gles_texture *texture_obj = NULL;
    struct yagl_gles_renderbuffer *renderbuffer_obj = NULL;
    int i, missing = 1;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glCheckFramebufferStatus, GLenum, target);

    YAGL_GET_CTX_RET(0);

    if (!yagl_gles_context_acquire_binded_framebuffer(ctx, target, &framebuffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!framebuffer_obj) {
        res = GL_FRAMEBUFFER_COMPLETE;
        goto out;
    }

    for (i = 0; i < (yagl_gles_framebuffer_attachment_color0 +
                     ctx->max_color_attachments); ++i) {
        if (framebuffer_obj->attachment_states[i].type != GL_NONE) {
            missing = 0;
            break;
        }
    }

    if (missing) {
        res = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
        goto out;
    }

    for (i = 0; i < (yagl_gles_framebuffer_attachment_color0 +
                     ctx->max_color_attachments); ++i) {
        switch (framebuffer_obj->attachment_states[i].type) {
        case GL_TEXTURE:
            texture_obj = (struct yagl_gles_texture*)yagl_sharegroup_acquire_object(ctx->base.sg,
                YAGL_NS_TEXTURE, framebuffer_obj->attachment_states[i].local_name);

            if (!texture_obj) {
                res = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            }

            yagl_gles_texture_release(texture_obj);
            break;
        case GL_RENDERBUFFER:
            renderbuffer_obj = (struct yagl_gles_renderbuffer*)yagl_sharegroup_acquire_object(ctx->base.sg,
                YAGL_NS_RENDERBUFFER, framebuffer_obj->attachment_states[i].local_name);

            if (!renderbuffer_obj) {
                res = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            }

            yagl_gles_renderbuffer_release(renderbuffer_obj);
            break;
        default:
            break;
        }
    }

    if (res != 0) {
        goto out;
    }

    res = GL_FRAMEBUFFER_COMPLETE;

out:
    yagl_gles_framebuffer_release(framebuffer_obj);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLenum, res);

    return res;
}

YAGL_API void glClear(GLbitfield mask)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glClear, GLbitfield, mask);

    YAGL_GET_CTX();

    yagl_render_invalidate((mask & GL_COLOR_BUFFER_BIT));

    yagl_host_glClear(mask);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClearColor(GLclampf red,
                           GLclampf green,
                           GLclampf blue,
                           GLclampf alpha)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glClearColor, GLclampf, GLclampf, GLclampf, GLclampf, red, green, blue, alpha);

    YAGL_GET_CTX();

    yagl_gles_context_clear_color(ctx,
                                  red,
                                  green,
                                  blue,
                                  alpha);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClearDepthf(GLclampf depth)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glClearDepthf, GLclampf, depth);

    YAGL_GET_CTX();

    yagl_gles_context_clear_depthf(ctx, depth);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glColorMask(GLboolean red,
                          GLboolean green,
                          GLboolean blue,
                          GLboolean alpha)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glColorMask, GLboolean, GLboolean, GLboolean, GLboolean, red, green, blue, alpha);

    YAGL_GET_CTX();

    ctx->color_writemask[0] = red;
    ctx->color_writemask[1] = green;
    ctx->color_writemask[2] = blue;
    ctx->color_writemask[3] = alpha;

    yagl_host_glColorMask(red, green, blue, alpha);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
    int using_pbo = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT8(glCompressedTexImage2D, GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*, target, level, internalformat, width, height, border, imageSize, data);

    YAGL_GET_CTX();

    if ((width < 0) || (height < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if ((width == 0) || (height == 0)) {
        width = height = 0;
    }

    if ((width != 0) && !yagl_gles_context_pre_unpack(ctx, &data, 1, &using_pbo)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    assert(!using_pbo);

    if (target == GL_TEXTURE_2D) {
        struct yagl_gles_texture_target_state *tex_target_state =
            yagl_gles_context_get_active_texture_target_state(ctx,
                                                              yagl_gles_texture_target_2d);
        if (tex_target_state->texture) {
            /*
             * This operation should orphan EGLImage according
             * to OES_EGL_image specs.
             *
             * This operation should release TexImage according
             * to eglBindTexImage spec.
             */
            yagl_gles_texture_unset_image(tex_target_state->texture);
        }
    }

    ctx->compressed_tex_image_2d(ctx,
                                 target,
                                 level,
                                 internalformat,
                                 width,
                                 height,
                                 border,
                                 imageSize,
                                 data);

    if (width != 0) {
        yagl_gles_context_post_unpack(ctx, 1);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    int using_pbo = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT9(glCompressedTexSubImage2D, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid*, target, level, xoffset, yoffset, width, height, format, imageSize, data);

    YAGL_GET_CTX();

    if ((width < 0) || (height < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if ((width == 0) || (height == 0)) {
        width = height = 0;
    }

    if ((width != 0) && !yagl_gles_context_pre_unpack(ctx, &data, 1, &using_pbo)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    assert(!using_pbo);

    ctx->compressed_tex_sub_image_2d(ctx,
                                     target,
                                     level,
                                     xoffset,
                                     yoffset,
                                     width,
                                     height,
                                     format,
                                     imageSize,
                                     data);

    if (width != 0) {
        yagl_gles_context_post_unpack(ctx, 1);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glCopyTexImage2D(GLenum target,
                               GLint level,
                               GLenum internalformat,
                               GLint x,
                               GLint y,
                               GLsizei width,
                               GLsizei height,
                               GLint border)
{
    YAGL_LOG_FUNC_ENTER_SPLIT8(glCopyTexImage2D, GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, target, level, internalformat, x, y, width, height, border);

    YAGL_GET_CTX();

    if ((ctx->base.client_api == yagl_client_api_gles1) &&
        (target != GL_TEXTURE_2D)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (border != 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    /*
     * TODO: Passing internalformat here is not correct since
     * it may be unsupported in OpenGL 3.1 core profile
     * (such as GL_ALPHA). Simply changing it to GL_RGBA
     * is no good either since in that case the texture will
     * contain more information than it actually supposed to,
     * thus, leading to incorrect rendering. Perhaps we need to
     * glReadPixels here, convert unsupported formats and then
     * glTexImage2D. m.b. do it on host since it won't incur
     * extra vmexit.
     */

    yagl_host_glCopyTexImage2D(target,
                               level,
                               internalformat,
                               x,
                               y,
                               width,
                               height,
                               border);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glCopyTexSubImage2D(GLenum target,
                                  GLint level,
                                  GLint xoffset,
                                  GLint yoffset,
                                  GLint x,
                                  GLint y,
                                  GLsizei width,
                                  GLsizei height)
{
    YAGL_LOG_FUNC_ENTER_SPLIT8(glCopyTexSubImage2D, GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, target, level, xoffset, yoffset, x, y, width, height);

    YAGL_GET_CTX();

    if ((ctx->base.client_api == yagl_client_api_gles1) &&
        (target != GL_TEXTURE_2D)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glCopyTexSubImage2D(target,
                                  level,
                                  xoffset,
                                  yoffset,
                                  x,
                                  y,
                                  width,
                                  height);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glCullFace(GLenum mode)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glCullFace, GLenum, mode);

    YAGL_GET_CTX();

    if (yagl_gles_is_cull_face_mode_valid(mode)) {
        ctx->cull_face_mode = mode;

        yagl_host_glCullFace(mode);
    } else {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteBuffers, GLsizei, const GLuint*, n, buffers);

    YAGL_GET_CTX();

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (buffers) {
        for (i = 0; i < n; ++i) {
            yagl_gles_context_unbind_buffer(ctx, buffers[i]);

            yagl_sharegroup_remove(ctx->base.sg,
                                   YAGL_NS_BUFFER,
                                   buffers[i]);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
{
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteFramebuffers, GLsizei, const GLuint*, n, framebuffers);

    YAGL_GET_CTX();

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (framebuffers) {
        for (i = 0; i < n; ++i) {
            yagl_gles_context_unbind_framebuffer(ctx, framebuffers[i]);

            yagl_namespace_remove(&ctx->framebuffers,
                                  framebuffers[i]);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
{
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteRenderbuffers, GLsizei, const GLuint*, n, renderbuffers);

    YAGL_GET_CTX();

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (renderbuffers) {
        for (i = 0; i < n; ++i) {
            yagl_gles_context_unbind_renderbuffer(ctx, renderbuffers[i]);

            yagl_sharegroup_remove(ctx->base.sg,
                                   YAGL_NS_RENDERBUFFER,
                                   renderbuffers[i]);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDeleteTextures(GLsizei n, const GLuint *textures)
{
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteTextures, GLsizei, const GLuint*, n, textures);

    YAGL_GET_CTX();

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (textures) {
        for (i = 0; i < n; ++i) {
            yagl_gles_context_unbind_texture(ctx, textures[i]);

            yagl_sharegroup_remove(ctx->base.sg,
                                   YAGL_NS_TEXTURE,
                                   textures[i]);

        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDepthFunc(GLenum func)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glDepthFunc, GLenum, func);

    YAGL_GET_CTX();

    if (yagl_gles_is_depth_func_valid(func)) {
        ctx->depth_func = func;

        yagl_host_glDepthFunc(func);
    } else {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDepthMask(GLboolean flag)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glDepthMask, GLboolean, flag);

    YAGL_GET_CTX();

    ctx->depth_writemask = flag;

    yagl_host_glDepthMask(flag);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDepthRangef(GLclampf zNear, GLclampf zFar)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glDepthRangef, GLclampf, GLclampf, zNear, zFar);

    YAGL_GET_CTX();

    yagl_gles_context_depth_rangef(ctx, zNear, zFar);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDisable(GLenum cap)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glDisable, GLenum, cap);

    YAGL_GET_CTX();

    yagl_gles_context_enable(ctx, cap, GL_FALSE);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glDrawArrays, GLenum, GLint, GLsizei, mode, first, count);

    YAGL_GET_CTX();

    yagl_gles_context_draw_arrays(ctx, mode, first, count, -1);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glDrawElements, GLenum, GLsizei, GLenum, const GLvoid*, mode, count, type, indices);

    YAGL_GET_CTX();

    yagl_gles_context_draw_elements(ctx, mode, count, type, indices, -1);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glEnable(GLenum cap)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glEnable, GLenum, cap);

    YAGL_GET_CTX();

    yagl_gles_context_enable(ctx, cap, GL_TRUE);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glFinish()
{
    YAGL_LOG_FUNC_ENTER_SPLIT0(glFinish);

    yagl_render_finish();

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_IMPLEMENT_API_NORET0(glFlush)

void glFramebufferRenderbuffer(GLenum target,
                               GLenum attachment,
                               GLenum renderbuffertarget,
                               GLuint renderbuffer)
{
    struct yagl_gles_framebuffer *framebuffer_obj = NULL;
    struct yagl_gles_renderbuffer *renderbuffer_obj = NULL;
    yagl_gles_framebuffer_attachment framebuffer_attachment;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glFramebufferRenderbuffer, GLenum, GLenum, GLenum, GLuint, target, attachment, renderbuffertarget, renderbuffer);

    YAGL_GET_CTX();

    if (!yagl_gles_context_acquire_binded_framebuffer(ctx, target, &framebuffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!framebuffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (renderbuffer) {
        renderbuffer_obj = (struct yagl_gles_renderbuffer*)yagl_sharegroup_acquire_object(ctx->base.sg,
            YAGL_NS_RENDERBUFFER, renderbuffer);

        if (!renderbuffer_obj) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }
    }

    if (renderbuffer_obj && (renderbuffertarget != GL_RENDERBUFFER)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if ((ctx->base.client_api == yagl_client_api_gles3) &&
        (attachment == GL_DEPTH_STENCIL_ATTACHMENT)) {
        yagl_gles_framebuffer_renderbuffer(framebuffer_obj,
                                           target, GL_DEPTH_ATTACHMENT,
                                           yagl_gles_framebuffer_attachment_depth,
                                           renderbuffertarget,
                                           renderbuffer_obj);
        yagl_gles_framebuffer_renderbuffer(framebuffer_obj,
                                           target, GL_STENCIL_ATTACHMENT,
                                           yagl_gles_framebuffer_attachment_stencil,
                                           renderbuffertarget,
                                           renderbuffer_obj);
    } else {
        if (!yagl_gles_validate_framebuffer_attachment(attachment,
                                                       ctx->max_color_attachments,
                                                       &framebuffer_attachment)) {
            YAGL_SET_ERR(GL_INVALID_ENUM);
            goto out;
        }

        yagl_gles_framebuffer_renderbuffer(framebuffer_obj,
                                           target, attachment,
                                           framebuffer_attachment,
                                           renderbuffertarget,
                                           renderbuffer_obj);
    }

out:
    yagl_gles_renderbuffer_release(renderbuffer_obj);
    yagl_gles_framebuffer_release(framebuffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

void glFramebufferTexture2D(GLenum target,
                            GLenum attachment,
                            GLenum textarget,
                            GLuint texture,
                            GLint level)
{
    struct yagl_gles_framebuffer *framebuffer_obj = NULL;
    struct yagl_gles_texture *texture_obj = NULL;
    yagl_gles_framebuffer_attachment framebuffer_attachment;
    GLenum squashed_textarget;

    YAGL_LOG_FUNC_ENTER_SPLIT5(glFramebufferTexture2D, GLenum, GLenum, GLenum, GLuint, GLint, target, attachment, textarget, texture, level);

    YAGL_GET_CTX();

    if (!yagl_gles_context_acquire_binded_framebuffer(ctx, target, &framebuffer_obj)) {
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
    }

    if (texture_obj && (level != 0)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!yagl_gles_validate_texture_target_squash(textarget,
                                                  &squashed_textarget)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (texture_obj && (texture_obj->target != squashed_textarget)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if ((ctx->base.client_api == yagl_client_api_gles3) &&
        (attachment == GL_DEPTH_STENCIL_ATTACHMENT)) {
        yagl_gles_framebuffer_texture2d(framebuffer_obj,
                                        target, GL_DEPTH_ATTACHMENT,
                                        yagl_gles_framebuffer_attachment_depth,
                                        textarget,
                                        level,
                                        texture_obj);
        yagl_gles_framebuffer_texture2d(framebuffer_obj,
                                        target, GL_STENCIL_ATTACHMENT,
                                        yagl_gles_framebuffer_attachment_stencil,
                                        textarget,
                                        level,
                                        texture_obj);
    } else {
        if (!yagl_gles_validate_framebuffer_attachment(attachment,
                                                       ctx->max_color_attachments,
                                                       &framebuffer_attachment)) {
            YAGL_SET_ERR(GL_INVALID_ENUM);
            goto out;
        }

        yagl_gles_framebuffer_texture2d(framebuffer_obj,
                                        target, attachment,
                                        framebuffer_attachment,
                                        textarget,
                                        level,
                                        texture_obj);
    }

out:
    yagl_gles_texture_release(texture_obj);
    yagl_gles_framebuffer_release(framebuffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glFrontFace(GLenum mode)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glFrontFace, GLenum, mode);

    YAGL_GET_CTX();

    if (yagl_gles_is_front_face_mode_valid(mode)) {
        ctx->front_face_mode = mode;

        yagl_host_glFrontFace(mode);
    } else {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGenBuffers(GLsizei n, GLuint *buffer_names)
{
    struct yagl_gles_buffer **buffers = NULL;
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenBuffers, GLsizei, GLuint*, n, buffer_names);

    YAGL_GET_CTX();

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    buffers = yagl_malloc0(n * sizeof(*buffers));

    for (i = 0; i < n; ++i) {
        buffers[i] = yagl_gles_buffer_create();

        if (!buffers[i]) {
            goto out;
        }
    }

    for (i = 0; i < n; ++i) {
        yagl_sharegroup_add(ctx->base.sg,
                            YAGL_NS_BUFFER,
                            &buffers[i]->base);

        if (buffer_names) {
            buffer_names[i] = buffers[i]->base.local_name;
        }
    }

out:
    for (i = 0; i < n; ++i) {
        yagl_gles_buffer_release(buffers[i]);
    }
    yagl_free(buffers);

    YAGL_LOG_FUNC_EXIT(NULL);
}

void glGenerateMipmap(GLenum target)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glGenerateMipmap, GLenum, target);

    YAGL_GET_CTX();

    yagl_host_glGenerateMipmap(target);

    YAGL_LOG_FUNC_EXIT(NULL);
}

void glGenFramebuffers(GLsizei n, GLuint *framebuffer_names)
{
    struct yagl_gles_framebuffer **framebuffers = NULL;
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenFramebuffers, GLsizei, GLuint*, n, framebuffer_names);

    YAGL_GET_CTX();

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    framebuffers = yagl_malloc0(n * sizeof(*framebuffers));

    for (i = 0; i < n; ++i) {
        framebuffers[i] = yagl_gles_framebuffer_create();

        if (!framebuffers[i]) {
            goto out;
        }
    }

    for (i = 0; i < n; ++i) {
        yagl_namespace_add(&ctx->framebuffers,
                           &framebuffers[i]->base);

        if (framebuffer_names) {
            framebuffer_names[i] = framebuffers[i]->base.local_name;
        }
    }

out:
    for (i = 0; i < n; ++i) {
        yagl_gles_framebuffer_release(framebuffers[i]);
    }
    yagl_free(framebuffers);

    YAGL_LOG_FUNC_EXIT(NULL);
}

void glGenRenderbuffers(GLsizei n, GLuint *renderbuffer_names)
{
    struct yagl_gles_renderbuffer **renderbuffers = NULL;
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenRenderbuffers, GLsizei, GLuint*, n, renderbuffer_names);

    YAGL_GET_CTX();

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    renderbuffers = yagl_malloc0(n * sizeof(*renderbuffers));

    for (i = 0; i < n; ++i) {
        renderbuffers[i] = yagl_gles_renderbuffer_create();

        if (!renderbuffers[i]) {
            goto out;
        }
    }

    for (i = 0; i < n; ++i) {
        yagl_sharegroup_add(ctx->base.sg,
                            YAGL_NS_RENDERBUFFER,
                            &renderbuffers[i]->base);

        if (renderbuffer_names) {
            renderbuffer_names[i] = renderbuffers[i]->base.local_name;
        }
    }

out:
    for (i = 0; i < n; ++i) {
        yagl_gles_renderbuffer_release(renderbuffers[i]);
    }
    yagl_free(renderbuffers);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGenTextures(GLsizei n, GLuint *texture_names)
{
    struct yagl_gles_texture **textures = NULL;
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenTextures, GLsizei, GLuint*, n, texture_names);

    YAGL_GET_CTX();

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    textures = yagl_malloc0(n * sizeof(*textures));

    for (i = 0; i < n; ++i) {
        textures[i] = yagl_gles_texture_create();

        if (!textures[i]) {
            goto out;
        }
    }

    for (i = 0; i < n; ++i) {
        yagl_sharegroup_add(ctx->base.sg,
                            YAGL_NS_TEXTURE,
                            &textures[i]->base);

        if (texture_names) {
            texture_names[i] = textures[i]->base.local_name;
        }
    }

out:
    for (i = 0; i < n; ++i) {
        yagl_gles_texture_release(textures[i]);
    }
    yagl_free(textures);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetBooleanv(GLenum pname, GLboolean *params)
{
    GLint ints[100]; // This fits all cases.
    uint32_t i, num = 0;
    int needs_map;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetBooleanv, GLenum, GLboolean*, pname, params);

    YAGL_GET_CTX();

    if (yagl_gles_context_get_integerv(ctx, pname, ints, &num)) {
        for (i = 0; i < num; ++i) {
            params[i] = ints[i] ? GL_TRUE : GL_FALSE;
        }
    } else {
        GLfloat floats[100]; // This fits all cases.
        if (yagl_gles_context_get_floatv(ctx, pname, floats, &num, &needs_map)) {
            for (i = 0; i < num; ++i) {
                params[i] = (floats[i] == 0.0f) ? GL_FALSE : GL_TRUE;
            }
        } else {
            YAGL_SET_ERR(GL_INVALID_ENUM);
        }
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    struct yagl_gles_buffer *buffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetBufferParameteriv, GLenum, GLenum, GLint*, target, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles_context_acquire_binded_buffer(ctx, target, &buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!buffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles_buffer_get_parameter(buffer_obj,
                                        pname,
                                        params)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

out:
    yagl_gles_buffer_release(buffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API GLenum glGetError(void)
{
    GLenum ret;

    YAGL_LOG_FUNC_ENTER_SPLIT0(glGetError);

    YAGL_GET_CTX_RET(GL_NO_ERROR);

    ret = yagl_gles_context_get_error(ctx);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLenum, ret);

    return ret;
}

YAGL_API void glGetFloatv(GLenum pname, GLfloat *params)
{
    uint32_t i, num = 0;
    int needs_map;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetFloatv, GLenum, GLfloat*, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles_context_get_floatv(ctx, pname, params, &num, &needs_map)) {
        GLint ints[100]; // This fits all cases.
        if (yagl_gles_context_get_integerv(ctx, pname, ints, &num)) {
            for (i = 0; i < num; ++i) {
                params[i] = ints[i];
            }
        } else {
            YAGL_SET_ERR(GL_INVALID_ENUM);
        }
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
    struct yagl_gles_framebuffer *framebuffer_obj = NULL;
    yagl_gles_framebuffer_attachment framebuffer_attachment;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetFramebufferAttachmentParameteriv, GLenum, GLenum, GLenum, GLint*, target, attachment, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles_context_acquire_binded_framebuffer(ctx, target, &framebuffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!framebuffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if ((ctx->base.client_api == yagl_client_api_gles3) &&
        (attachment == GL_DEPTH_STENCIL_ATTACHMENT)) {
        if (memcmp(&framebuffer_obj->attachment_states[yagl_gles_framebuffer_attachment_depth],
                   &framebuffer_obj->attachment_states[yagl_gles_framebuffer_attachment_stencil],
                   sizeof(struct yagl_gles_framebuffer_attachment_state)) == 0) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }
        framebuffer_attachment = yagl_gles_framebuffer_attachment_depth;
    } else if (!yagl_gles_validate_framebuffer_attachment(attachment,
                                                          ctx->max_color_attachments,
                                                          &framebuffer_attachment)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    switch (pname) {
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
        *params = framebuffer_obj->attachment_states[framebuffer_attachment].type;
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
        *params = framebuffer_obj->attachment_states[framebuffer_attachment].local_name;
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
        if (framebuffer_obj->attachment_states[framebuffer_attachment].type == GL_TEXTURE) {
            *params = 0;
        } else {
            YAGL_SET_ERR(GL_INVALID_ENUM);
            goto out;
        }
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
        if (framebuffer_obj->attachment_states[framebuffer_attachment].type == GL_TEXTURE) {
            *params =
                (framebuffer_obj->attachment_states[framebuffer_attachment].textarget == GL_TEXTURE_2D)
                ? 0 : framebuffer_obj->attachment_states[framebuffer_attachment].textarget;
        } else {
            YAGL_SET_ERR(GL_INVALID_ENUM);
            goto out;
        }
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

out:
    yagl_gles_framebuffer_release(framebuffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetIntegerv(GLenum pname, GLint *params)
{
    uint32_t i, num = 0;
    int needs_map = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetIntegerv, GLenum, GLint*, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles_context_get_integerv(ctx, pname, params, &num)) {
        GLfloat floats[100]; // This fits all cases.
        if (yagl_gles_context_get_floatv(ctx, pname, floats, &num, &needs_map)) {
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
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetRenderbufferParameteriv, GLenum, GLenum, GLint*, target, pname, params);

    YAGL_GET_CTX();

    /*
     * TODO: Passthrough for now.
     */

    yagl_host_glGetRenderbufferParameteriv(target, pname, params);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetTexParameterfv, GLenum, GLenum, GLfloat*, target, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles_context_get_tex_parameterfv(ctx, target, pname, params)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetTexParameteriv, GLenum, GLenum, GLint*, target, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles_context_get_tex_parameteriv(ctx, target, pname, params)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API GLboolean glIsBuffer(GLuint buffer)
{
    GLboolean res = GL_FALSE;
    struct yagl_gles_buffer *buffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsBuffer, GLuint, buffer);

    YAGL_GET_CTX_RET(GL_FALSE);

    buffer_obj = (struct yagl_gles_buffer*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_BUFFER, buffer);

    if (buffer_obj && yagl_gles_buffer_was_bound(buffer_obj)) {
        res = GL_TRUE;
    }

    yagl_gles_buffer_release(buffer_obj);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, res);

    return res;
}

YAGL_API GLboolean glIsEnabled(GLenum cap)
{
    GLboolean res;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsEnabled, GLenum, cap);

    YAGL_GET_CTX_RET(GL_FALSE);

    res = yagl_gles_context_is_enabled(ctx, cap);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, res);

    return res;
}

GLboolean glIsFramebuffer(GLuint framebuffer)
{
    GLboolean res = GL_FALSE;
    struct yagl_gles_framebuffer *framebuffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsFramebuffer, GLuint, framebuffer);

    YAGL_GET_CTX_RET(GL_FALSE);

    framebuffer_obj = (struct yagl_gles_framebuffer*)yagl_namespace_acquire(&ctx->framebuffers,
        framebuffer);

    if (framebuffer_obj && yagl_gles_framebuffer_was_bound(framebuffer_obj)) {
        res = GL_TRUE;
    }

    yagl_gles_framebuffer_release(framebuffer_obj);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, res);

    return res;
}

GLboolean glIsRenderbuffer(GLuint renderbuffer)
{
    GLboolean res = GL_FALSE;
    struct yagl_gles_renderbuffer *renderbuffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsRenderbuffer, GLuint, renderbuffer);

    YAGL_GET_CTX_RET(GL_FALSE);

    renderbuffer_obj = (struct yagl_gles_renderbuffer*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_RENDERBUFFER, renderbuffer);

    if (renderbuffer_obj && yagl_gles_renderbuffer_was_bound(renderbuffer_obj)) {
        res = GL_TRUE;
    }

    yagl_gles_renderbuffer_release(renderbuffer_obj);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, res);

    return res;
}

YAGL_API GLboolean glIsTexture(GLuint texture)
{
    GLboolean res = GL_FALSE;
    struct yagl_gles_texture *texture_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsTexture, GLuint, texture);

    YAGL_GET_CTX_RET(GL_FALSE);

    texture_obj = (struct yagl_gles_texture*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_TEXTURE, texture);

    if (texture_obj && (texture_obj->target != 0)) {
        res = GL_TRUE;
    }

    yagl_gles_texture_release(texture_obj);

    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, res);

    return res;
}

YAGL_API void glPixelStorei(GLenum pname, GLint param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glPixelStorei, GLenum, GLint, pname, param);

    YAGL_GET_CTX();

    /*
     * We don't pass GL_XXX_SKIP_YYY to host, this
     * is intentionally so that we can pass less data later in glTexImage*
     * and glReadPixels, i.e. we process skips ourselves and feed biased
     * data to host.
     */

    switch (pname) {
    case GL_PACK_ALIGNMENT:
        if (yagl_gles_is_alignment_valid(param)) {
            ctx->pack.alignment = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
        break;
    case GL_PACK_ROW_LENGTH:
        if (param >= 0) {
            ctx->pack.row_length = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
        break;
    case GL_PACK_IMAGE_HEIGHT:
        if (param >= 0) {
            ctx->pack.image_height = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
        break;
    case GL_PACK_SKIP_PIXELS:
        if (param >= 0) {
            ctx->pack.skip_pixels = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
        goto out;
        break;
    case GL_PACK_SKIP_ROWS:
        if (param >= 0) {
            ctx->pack.skip_rows = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
        goto out;
        break;
    case GL_PACK_SKIP_IMAGES:
        if (param >= 0) {
            ctx->pack.skip_images = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
        goto out;
        break;
    case GL_UNPACK_ALIGNMENT:
        if (yagl_gles_is_alignment_valid(param)) {
            ctx->unpack.alignment = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
        break;
    case GL_UNPACK_ROW_LENGTH:
        if (param >= 0) {
            ctx->unpack.row_length = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
        break;
    case GL_UNPACK_IMAGE_HEIGHT:
        if (param >= 0) {
            ctx->unpack.image_height = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
        break;
    case GL_UNPACK_SKIP_PIXELS:
        if (param >= 0) {
            ctx->unpack.skip_pixels = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
        goto out;
        break;
    case GL_UNPACK_SKIP_ROWS:
        if (param >= 0) {
            ctx->unpack.skip_rows = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
        goto out;
        break;
    case GL_UNPACK_SKIP_IMAGES:
        if (param >= 0) {
            ctx->unpack.skip_images = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
        goto out;
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glPixelStorei(pname, param);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
    GLsizei bpp, row_stride, size;
    int need_convert;
    int using_pbo = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT7(glReadPixels, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*, x, y, width, height, format, type, pixels);

    YAGL_GET_CTX();

    if ((width < 0) || (height < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if ((width == 0) || (height == 0)) {
        width = height = 0;
    }

    if (!yagl_gles_context_validate_format(ctx,
                                           format,
                                           type,
                                           &bpp,
                                           &need_convert)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_render_invalidate(0);

    row_stride = yagl_gles_get_stride(&ctx->pack,
                                      width, height, bpp,
                                      NULL);

    if ((width != 0) && !yagl_gles_context_pre_pack(ctx, &pixels, need_convert, &using_pbo)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    pixels += yagl_gles_get_offset(&ctx->pack,
                                   width, height, 1, bpp, &size);

    if (using_pbo) {
        yagl_host_glReadPixelsOffset(x, y,
                                     width, height,
                                     yagl_gles_get_actual_format(format),
                                     yagl_gles_get_actual_type(type),
                                     (GLsizei)pixels);
    } else {
        GLvoid *pixels_from;

        pixels_from = yagl_gles_convert_from_host_start(&ctx->pack,
                                                        width,
                                                        height,
                                                        format,
                                                        type,
                                                        pixels);

        yagl_host_glReadPixelsData(x, y,
                                   width, height,
                                   yagl_gles_get_actual_format(format),
                                   yagl_gles_get_actual_type(type),
                                   pixels_from,
                                   size,
                                   NULL);

        yagl_gles_convert_from_host_end(&ctx->pack,
                                        width,
                                        height,
                                        format,
                                        type,
                                        row_stride,
                                        pixels_from,
                                        pixels);
    }

    if (width != 0) {
        yagl_gles_context_post_pack(ctx, pixels, size, need_convert);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    GLsizei bpp, row_stride, size;
    int need_convert;
    int using_pbo = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT9(glTexImage2D, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*, target, level, internalformat, width, height, border, format, type, pixels);

    YAGL_GET_CTX();

    if ((width < 0) || (height < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if ((width == 0) || (height == 0)) {
        width = height = 0;
    }

    if (!yagl_gles_context_validate_format(ctx,
                                           format,
                                           type,
                                           &bpp,
                                           &need_convert)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    row_stride = yagl_gles_get_stride(&ctx->unpack,
                                      width, height, bpp,
                                      NULL);

    if ((width != 0) && !yagl_gles_context_pre_unpack(ctx, &pixels, need_convert, &using_pbo)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (target == GL_TEXTURE_2D) {
        struct yagl_gles_texture_target_state *tex_target_state =
            yagl_gles_context_get_active_texture_target_state(ctx,
                                                              yagl_gles_texture_target_2d);
        if (tex_target_state->texture) {
            /*
             * This operation should orphan EGLImage according
             * to OES_EGL_image specs.
             *
             * This operation should release TexImage according
             * to eglBindTexImage spec.
             */
            yagl_gles_texture_unset_image(tex_target_state->texture);
        }
    }

    pixels += yagl_gles_get_offset(&ctx->unpack,
                                   width, height, 1, bpp, &size);

    if (using_pbo) {
        yagl_host_glTexImage2DOffset(target,
                                     level,
                                     yagl_gles_get_actual_internalformat(internalformat),
                                     width,
                                     height,
                                     border,
                                     yagl_gles_get_actual_format(format),
                                     yagl_gles_get_actual_type(type),
                                     (GLsizei)pixels);
    } else {
        yagl_host_glTexImage2DData(target,
                                   level,
                                   yagl_gles_get_actual_internalformat(internalformat),
                                   width,
                                   height,
                                   border,
                                   yagl_gles_get_actual_format(format),
                                   yagl_gles_get_actual_type(type),
                                   yagl_gles_convert_to_host(&ctx->unpack,
                                                             width,
                                                             height,
                                                             1,
                                                             format,
                                                             type,
                                                             pixels),
                                   size);
    }

    if (width != 0) {
        yagl_gles_context_post_unpack(ctx, need_convert);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    GLsizei bpp, row_stride, size;
    int need_convert;
    int using_pbo = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT9(glTexSubImage2D, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*, target, level, xoffset, yoffset, width, height, format, type, pixels);

    YAGL_GET_CTX();

    if ((width < 0) || (height < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if ((width == 0) || (height == 0)) {
        width = height = 0;
    }

    if (!yagl_gles_context_validate_format(ctx,
                                           format,
                                           type,
                                           &bpp,
                                           &need_convert)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    row_stride = yagl_gles_get_stride(&ctx->unpack,
                                      width, height, bpp,
                                      NULL);

    if ((width != 0) && !yagl_gles_context_pre_unpack(ctx, &pixels, need_convert, &using_pbo)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    pixels += yagl_gles_get_offset(&ctx->unpack,
                                   width, height, 1, bpp, &size);

    if (using_pbo) {
        yagl_host_glTexSubImage2DOffset(target,
                                        level,
                                        xoffset,
                                        yoffset,
                                        width,
                                        height,
                                        yagl_gles_get_actual_format(format),
                                        yagl_gles_get_actual_type(type),
                                        (GLsizei)pixels);
    } else {
        yagl_host_glTexSubImage2DData(target,
                                      level,
                                      xoffset,
                                      yoffset,
                                      width,
                                      height,
                                      yagl_gles_get_actual_format(format),
                                      yagl_gles_get_actual_type(type),
                                      yagl_gles_convert_to_host(&ctx->unpack,
                                                                width,
                                                                height,
                                                                1,
                                                                format,
                                                                type,
                                                                pixels),
                                      size);
    }

    if (width != 0) {
        yagl_gles_context_post_unpack(ctx, need_convert);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image)
{
    struct yagl_gles_image *image_obj = NULL;
    struct yagl_gles_texture_target_state *tex_target_state;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glEGLImageTargetTexture2DOES, GLenum, GLeglImageOES, target, image);

    YAGL_GET_CTX();

    if (target != GL_TEXTURE_2D) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    image_obj = (struct yagl_gles_image*)yagl_acquire_client_image((yagl_host_handle)image);

    if (!image_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    tex_target_state =
        yagl_gles_context_get_active_texture_target_state(ctx,
                                                          yagl_gles_texture_target_2d);

    if (!tex_target_state->texture) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_gles_texture_set_image(tex_target_state->texture, image_obj);

out:
    yagl_gles_image_release(image_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glEGLImageTargetRenderbufferStorageOES(GLenum target, GLeglImageOES image)
{
    fprintf(stderr, "glEGLImageTargetRenderbufferStorageOES not supported in YaGL\n");
}

/*
 *  GL_ANGLE_framebuffer_blit.
 * @{
 */

YAGL_API void glBlitFramebuffer(GLint srcX0, GLint srcY0,
                                GLint srcX1, GLint srcY1,
                                GLint dstX0, GLint dstY0,
                                GLint dstX1, GLint dstY1,
                                GLbitfield mask, GLenum filter)
{
    YAGL_LOG_FUNC_ENTER_SPLIT10(glBlitFramebuffer, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);

    YAGL_GET_CTX();

    yagl_render_invalidate(0);

    yagl_host_glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);

    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glBlitFramebuffer, glBlitFramebufferANGLE);

/*
 * @}
 */

/*
 * GL_OES_vertex_array_object.
 * @{
 */

YAGL_API void glBindVertexArray(GLuint array)
{
    struct yagl_gles_vertex_array *va_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glBindVertexArray, GLuint, array);

    YAGL_GET_CTX();

    if (!ctx->vertex_arrays_supported) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (array != 0) {
        va_obj = (struct yagl_gles_vertex_array*)yagl_namespace_acquire(&ctx->vertex_arrays,
            array);

        if (!va_obj) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }
    }

    yagl_gles_context_bind_vertex_array(ctx, va_obj);

out:
    yagl_gles_vertex_array_release(va_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glBindVertexArray, glBindVertexArrayOES);

YAGL_API void glDeleteVertexArrays(GLsizei n, const GLuint *arrays)
{
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteVertexArrays, GLsizei, const GLuint*, n, arrays);

    YAGL_GET_CTX();

    if (!ctx->vertex_arrays_supported) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (arrays) {
        for (i = 0; i < n; ++i) {
            yagl_gles_context_unbind_vertex_array(ctx, arrays[i]);
            yagl_namespace_remove(&ctx->vertex_arrays, arrays[i]);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glDeleteVertexArrays, glDeleteVertexArraysOES);

YAGL_API void glGenVertexArrays(GLsizei n, GLuint *array_names)
{
    struct yagl_gles_vertex_array **arrays = NULL;
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenVertexArrays, GLsizei, GLuint*, n, array_names);

    YAGL_GET_CTX();

    if (!ctx->vertex_arrays_supported) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (n < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    arrays = yagl_malloc0(n * sizeof(*arrays));

    for (i = 0; i < n; ++i) {
        arrays[i] = yagl_gles_vertex_array_create(0,
                                                  ctx->create_arrays(ctx),
                                                  ctx->num_arrays);

        if (!arrays[i]) {
            goto out;
        }
    }

    for (i = 0; i < n; ++i) {
        yagl_namespace_add(&ctx->vertex_arrays,
                           &arrays[i]->base);

        if (array_names) {
            array_names[i] = arrays[i]->base.local_name;
        }
    }

out:
    for (i = 0; i < n; ++i) {
        yagl_gles_vertex_array_release(arrays[i]);
    }
    yagl_free(arrays);

    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glGenVertexArrays, glGenVertexArraysOES);

YAGL_API GLboolean glIsVertexArray(GLuint array)
{
    GLboolean res = GL_FALSE;
    struct yagl_gles_vertex_array *va_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsVertexArray, GLuint, array);

    YAGL_GET_CTX_RET(GL_FALSE);

    if (!ctx->vertex_arrays_supported) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    va_obj = (struct yagl_gles_vertex_array*)yagl_namespace_acquire(&ctx->vertex_arrays,
        array);

    if (va_obj && yagl_gles_vertex_array_was_bound(va_obj)) {
        res = GL_TRUE;
    }

    yagl_gles_vertex_array_release(va_obj);

out:
    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, res);

    return res;
}
YAGL_API YAGL_ALIAS(glIsVertexArray, glIsVertexArrayOES);

/*
 * @}
 */

/*
 * GL_EXT_draw_buffers.
 * @{
 */

YAGL_API void glDrawBuffers(GLsizei n, const GLenum *bufs)
{
    GLsizei i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glDrawBuffers, GLsizei, const GLenum*, n, bufs);

    YAGL_GET_CTX();

    if ((n < 0) || (n > ctx->max_draw_buffers)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (ctx->fbo_draw) {
        for (i = 0; i < n; ++i) {
            if (bufs[i] == GL_NONE) {
                continue;
            }

            if ((bufs[i] < GL_COLOR_ATTACHMENT0) ||
                (bufs[i] >= (GL_COLOR_ATTACHMENT0 + ctx->max_color_attachments))) {
                YAGL_SET_ERR(GL_INVALID_OPERATION);
                goto out;
            }

            if ((bufs[i] - GL_COLOR_ATTACHMENT0) != i) {
                YAGL_SET_ERR(GL_INVALID_OPERATION);
                goto out;
            }
        }
    } else if ((n != 1) || ((bufs[0] != GL_NONE) && (bufs[0] != GL_BACK))) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    memcpy(&ctx->draw_buffers[0], bufs, n * sizeof(bufs[0]));

    for (i = n; i < ctx->max_draw_buffers; ++i) {
        ctx->draw_buffers[i] = GL_NONE;
    }

    yagl_host_glDrawBuffers(bufs, n);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glDrawBuffers, glDrawBuffersEXT);

/*
 * @}
 */

/*
 * GL_NV_read_buffer.
 * @{
 */

YAGL_API void glReadBuffer(GLenum mode)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glReadBuffer, GLenum, mode);

    YAGL_GET_CTX();

    if (ctx->fbo_read) {
        if (mode != GL_NONE) {
            if ((mode < GL_COLOR_ATTACHMENT0) ||
                (mode >= (GL_COLOR_ATTACHMENT0 + ctx->max_color_attachments))) {
                YAGL_SET_ERR(GL_INVALID_OPERATION);
                goto out;
            }
        }
    } else if ((mode != GL_NONE) && (mode != GL_BACK)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    ctx->read_buffer = mode;

    yagl_host_glReadBuffer(mode);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glReadBuffer, glReadBufferNV);

/*
 * @}
 */

/*
 * GL_OES_mapbuffer.
 * @{
 */

YAGL_API void *glMapBuffer(GLenum target, GLenum access)
{
    void *ptr = NULL;
    struct yagl_gles_buffer *buffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glMapBuffer, GLenum, GLenum, target, access);

    YAGL_GET_CTX_RET(NULL);

    if (access != GL_WRITE_ONLY) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!yagl_gles_context_acquire_binded_buffer(ctx, target, &buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!buffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (yagl_gles_buffer_mapped(buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles_buffer_map(buffer_obj, 0, buffer_obj->size, GL_MAP_WRITE_BIT)) {
        YAGL_SET_ERR(GL_OUT_OF_MEMORY);
        goto out;
    }

    ptr = buffer_obj->map_pointer;

out:
    yagl_gles_buffer_release(buffer_obj);

    YAGL_LOG_FUNC_EXIT("%p", ptr);

    return ptr;
}
YAGL_API YAGL_ALIAS(glMapBuffer, glMapBufferOES);

YAGL_API GLboolean glUnmapBuffer(GLenum target)
{
    GLboolean ret = GL_FALSE;
    struct yagl_gles_buffer *buffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glUnmapBuffer, GLenum, target);

    YAGL_GET_CTX_RET(GL_FALSE);

    if (!yagl_gles_context_acquire_binded_buffer(ctx, target, &buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!buffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles_buffer_mapped(buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_gles_buffer_unmap(buffer_obj);

    ret = GL_TRUE;

out:
    yagl_gles_buffer_release(buffer_obj);

    YAGL_LOG_FUNC_EXIT("%u", ret);

    return ret;
}
YAGL_API YAGL_ALIAS(glUnmapBuffer, glUnmapBufferOES);

YAGL_API void glGetBufferPointerv(GLenum target,
                                  GLenum pname,
                                  GLvoid **params)
{
    struct yagl_gles_buffer *buffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetBufferPointerv, GLenum, GLenum, GLvoid**, target, pname, params);

    YAGL_GET_CTX();

    if (pname != GL_BUFFER_MAP_POINTER) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!yagl_gles_context_acquire_binded_buffer(ctx, target, &buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!buffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    *params = buffer_obj->map_pointer;

out:
    yagl_gles_buffer_release(buffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glGetBufferPointerv, glGetBufferPointervOES);

/*
 * @}
 */

/*
 * GL_EXT_map_buffer_range.
 * @{
 */

YAGL_API void *glMapBufferRange(GLenum target, GLintptr offset,
                                GLsizeiptr length, GLbitfield access)
{
    void *ptr = NULL;
    struct yagl_gles_buffer *buffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glMapBufferRange, GLenum, GLintptr, GLsizeiptr, GLbitfield, target, offset, length, access);

    YAGL_GET_CTX_RET(NULL);

    if ((offset < 0) || (length < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (length == 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (access & ~(GL_MAP_READ_BIT |
                   GL_MAP_WRITE_BIT |
                   GL_MAP_INVALIDATE_RANGE_BIT |
                   GL_MAP_INVALIDATE_BUFFER_BIT |
                   GL_MAP_FLUSH_EXPLICIT_BIT |
                   GL_MAP_UNSYNCHRONIZED_BIT)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if ((access & (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT)) == 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if ((access & GL_MAP_READ_BIT) &&
        (access & (GL_MAP_INVALIDATE_RANGE_BIT |
                   GL_MAP_INVALIDATE_BUFFER_BIT |
                   GL_MAP_UNSYNCHRONIZED_BIT))) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if ((access & GL_MAP_FLUSH_EXPLICIT_BIT) &&
        ((access & GL_MAP_WRITE_BIT) == 0)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles_context_acquire_binded_buffer(ctx, target, &buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!buffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (yagl_gles_buffer_mapped(buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles_buffer_map(buffer_obj, offset, length, access)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    ptr = buffer_obj->map_pointer;

out:
    yagl_gles_buffer_release(buffer_obj);

    YAGL_LOG_FUNC_EXIT("%p", ptr);

    return ptr;
}
YAGL_API YAGL_ALIAS(glMapBufferRange, glMapBufferRangeEXT);

YAGL_API void glFlushMappedBufferRange(GLenum target, GLintptr offset,
                                       GLsizeiptr length)
{
    struct yagl_gles_buffer *buffer_obj = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glFlushMappedBufferRange, GLenum, GLintptr, GLsizeiptr, target, offset, length);

    YAGL_GET_CTX();

    if ((offset < 0) || (length < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles_context_acquire_binded_buffer(ctx, target, &buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!buffer_obj) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles_buffer_mapped(buffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if ((buffer_obj->map_access & GL_MAP_FLUSH_EXPLICIT_BIT) == 0) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    if (!yagl_gles_buffer_flush_mapped_range(buffer_obj, offset, length)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

out:
    yagl_gles_buffer_release(buffer_obj);

    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glFlushMappedBufferRange, glFlushMappedBufferRangeEXT);

/*
 * @}
 */

/*
 * GL_EXT_texture_storage.
 * @{
 */

YAGL_API void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    yagl_gles_texture_target texture_target;
    struct yagl_gles_texture *texture_obj;
    GLenum format, type;
    GLsizei i, j;
    GLenum cubemap_targets[] =
    {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    YAGL_LOG_FUNC_ENTER_SPLIT5(glTexStorage2D, GLenum, GLsizei, GLenum, GLsizei, GLsizei, target, levels, internalformat, width, height);

    YAGL_GET_CTX();

    if ((levels <= 0) || (width <= 0) || (height <= 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles_context_validate_texture_internalformat(ctx,
                                                           &internalformat,
                                                           &format,
                                                           &type)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!yagl_gles_context_validate_texture_target(ctx, target, &texture_target)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    texture_obj = yagl_gles_context_get_active_texture_target_state(ctx,
                                                                    texture_target)->texture;

    if (!texture_obj || texture_obj->immutable) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    internalformat = yagl_gles_get_actual_internalformat(internalformat);
    format = yagl_gles_get_actual_format(format);
    type = yagl_gles_get_actual_type(type);

    switch (texture_target) {
    case yagl_gles_texture_target_2d:
        for (i = 0; i < levels; ++i) {
            yagl_host_glTexImage2DData(target, i, internalformat,
                                       width, height, 0, format, type,
                                       NULL, 0);

            width >>= 1;
            if (width == 0) {
                width = 1;
            }

            height >>= 1;
            if (height == 0) {
                height = 1;
            }
        }
        break;
    case yagl_gles_texture_target_cubemap:
        for (i = 0; i < levels; ++i) {
            for (j = 0;
                 j < sizeof(cubemap_targets)/sizeof(cubemap_targets[0]);
                 ++j) {
                yagl_host_glTexImage2DData(cubemap_targets[j], i, internalformat,
                                           width, height, 0, format, type,
                                           NULL, 0);
            }

            width >>= 1;
            if (width == 0) {
                width = 1;
            }

            height >>= 1;
            if (height == 0) {
                height = 1;
            }
        }
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_gles_texture_set_immutable(texture_obj);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}
YAGL_API YAGL_ALIAS(glTexStorage2D, glTexStorage2DEXT);

/*
 * @}
 */
