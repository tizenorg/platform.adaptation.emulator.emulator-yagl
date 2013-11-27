#include "GL/gl.h"
#include "yagl_gles_calls.h"
#include "yagl_host_gles_calls.h"
#include "yagl_impl.h"
#include "yagl_malloc.h"
#include "yagl_render.h"
#include "yagl_utils.h"
#include "yagl_sharegroup.h"
#include "yagl_gles_context.h"
#include "yagl_gles_buffer.h"
#include "yagl_gles_texture.h"
#include "yagl_gles_texture_unit.h"
#include "yagl_gles_framebuffer.h"
#include "yagl_gles_renderbuffer.h"
#include "yagl_gles_image.h"
#include "yagl_gles_array.h"
#include "yagl_gles_validate.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * We can't include GLES2/gl2ext.h here
 */
#define GL_HALF_FLOAT_OES 0x8D61

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

static void yagl_get_minmax_index(const GLvoid *indices,
                                  GLsizei count,
                                  GLenum type,
                                  uint32_t *min_idx,
                                  uint32_t *max_idx)
{
    int i;

    *min_idx = UINT32_MAX;
    *max_idx = 0;

    switch (type) {
    case GL_UNSIGNED_BYTE:
        for (i = 0; i < count; ++i) {
            uint32_t idx = ((uint8_t*)indices)[i];
            if (idx < *min_idx) {
                *min_idx = idx;
            }
            if (idx > *max_idx) {
                *max_idx = idx;
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        for (i = 0; i < count; ++i) {
            uint32_t idx = ((uint16_t*)indices)[i];
            if (idx < *min_idx) {
                *min_idx = idx;
            }
            if (idx > *max_idx) {
                *max_idx = idx;
            }
        }
        break;
    default:
        break;
    }
}

static int yagl_get_stride(GLsizei alignment,
                           GLsizei width,
                           GLenum format,
                           GLenum type,
                           GLsizei *stride)
{
    int num_components = 0;
    GLsizei bpp = 0;

    switch (format) {
    case GL_ALPHA:
        num_components = 1;
        break;
    case GL_RGB:
        num_components = 3;
        break;
    case GL_RGBA:
        num_components = 4;
        break;
    case GL_BGRA_EXT:
        num_components = 4;
        break;
    case GL_LUMINANCE:
        num_components = 1;
        break;
    case GL_LUMINANCE_ALPHA:
        num_components = 2;
        break;
    case GL_DEPTH_STENCIL_EXT:
        if ((type == GL_FLOAT) || (type == GL_HALF_FLOAT_OES)) {
            return 0;
        }
        num_components = 1;
        break;
    case GL_DEPTH_COMPONENT:
        if ((type != GL_UNSIGNED_SHORT) && (type != GL_UNSIGNED_INT)) {
            return 0;
        }
        num_components = 1;
        break;
    default:
        return 0;
    }

    switch (type) {
    case GL_UNSIGNED_BYTE:
        bpp = num_components;
        break;
    case GL_UNSIGNED_SHORT_5_6_5:
        if (format != GL_RGB) {
            return 0;
        }
        bpp = 2;
        break;
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
        if (format != GL_RGBA) {
            return 0;
        }
        bpp = 2;
        break;
    case GL_UNSIGNED_INT_24_8_EXT:
        bpp = num_components * 4;
        break;
    case GL_UNSIGNED_SHORT:
        if (format != GL_DEPTH_COMPONENT) {
            return 0;
        }
        bpp = num_components * 2;
        break;
    case GL_UNSIGNED_INT:
        if (format != GL_DEPTH_COMPONENT) {
            return 0;
        }
        bpp = num_components * 4;
        break;
    case GL_FLOAT:
        bpp = num_components * 4;
        break;
    case GL_HALF_FLOAT_OES:
        bpp = num_components * 2;
        break;
    default:
        return 0;
    }

    assert(alignment > 0);

    *stride = ((width * bpp) + alignment - 1) & ~(alignment - 1);

    return 1;
}

static GLenum yagl_get_actual_type(GLenum type)
{
    switch (type) {
    case GL_HALF_FLOAT_OES:
        return GL_HALF_FLOAT;
    default:
        return type;
    }
}

static GLint yagl_get_actual_internalformat(GLint internalformat)
{
    switch (internalformat) {
    case GL_BGRA:
        return GL_RGBA;
    default:
        return internalformat;
    }
}

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
    yagl_render_invalidate();
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

    yagl_render_invalidate();

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
        framebuffer_obj = (struct yagl_gles_framebuffer*)yagl_sharegroup_acquire_object(ctx->base.sg,
            YAGL_NS_FRAMEBUFFER, framebuffer);

        if (!framebuffer_obj) {
            framebuffer_obj = yagl_gles_framebuffer_create();

            if (!framebuffer_obj) {
                goto out;
            }

            framebuffer_obj = (struct yagl_gles_framebuffer*)yagl_sharegroup_add_named(ctx->base.sg,
               YAGL_NS_FRAMEBUFFER, framebuffer, &framebuffer_obj->base);
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

    if (!yagl_gles_validate_texture_target(target, &texture_target)) {
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

    if (!yagl_gles_is_buffer_target_valid(target)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (size < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles_is_buffer_usage_valid(usage)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    buffer_obj = yagl_gles_context_acquire_binded_buffer(ctx, target);

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

    if (!yagl_gles_is_buffer_target_valid(target)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if ((offset < 0) || (size < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    buffer_obj = yagl_gles_context_acquire_binded_buffer(ctx, target);

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

    if (!yagl_gles_is_framebuffer_target_valid(target)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    framebuffer_obj = yagl_gles_context_acquire_binded_framebuffer(ctx, target);

    if (!framebuffer_obj) {
        res = GL_FRAMEBUFFER_COMPLETE;
        goto out;
    }

    for (i = 0; i < YAGL_NUM_GLES_FRAMEBUFFER_ATTACHMENTS; ++i) {
        if (framebuffer_obj->attachment_states[i].type != GL_NONE) {
            missing = 0;
            break;
        }
    }

    if (missing) {
        res = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
        goto out;
    }

    for (i = 0; i < YAGL_NUM_GLES_FRAMEBUFFER_ATTACHMENTS; ++i) {
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

    yagl_render_invalidate();
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
    GLenum res;

    YAGL_LOG_FUNC_ENTER_SPLIT8(glCompressedTexImage2D, GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*, target, level, internalformat, width, height, border, imageSize, data);

    YAGL_GET_CTX();

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

    res = ctx->compressed_tex_image(ctx,
                                    target,
                                    level,
                                    internalformat,
                                    width,
                                    height,
                                    border,
                                    imageSize,
                                    data);

    if (res != GL_NO_ERROR) {
        YAGL_SET_ERR(res);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    YAGL_LOG_FUNC_ENTER_SPLIT9(glCompressedTexSubImage2D, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid*, target, level, xoffset, yoffset, width, height, format, imageSize, data);

    YAGL_GET_CTX();

    if (ctx->base.client_api == yagl_client_api_gles1) {
        /* No formats are supported by this call in GLES1 API */
        YAGL_SET_ERR(GL_INVALID_OPERATION);
    } else {
        yagl_host_glCompressedTexSubImage2D(target,
                                            level,
                                            xoffset,
                                            yoffset,
                                            width,
                                            height,
                                            format,
                                            data,
                                            imageSize);
    }

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

            yagl_sharegroup_remove(ctx->base.sg,
                                   YAGL_NS_FRAMEBUFFER,
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
    GLuint i;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glDrawArrays, GLenum, GLint, GLsizei, mode, first, count);

    YAGL_GET_CTX();

    if ((first < 0) || (count < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (count == 0) {
        goto out;
    }

    yagl_render_invalidate();

    for (i = 0; i < ctx->num_arrays; ++i) {
        yagl_gles_array_transfer(&ctx->arrays[i],
                                 first,
                                 count);
    }

    ctx->draw_arrays(ctx, mode, first, count);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    int index_size = 0;
    int have_range = 0;
    uint32_t min_idx = 0, max_idx = 0;
    GLuint i;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glDrawElements, GLenum, GLsizei, GLenum, const GLvoid*, mode, count, type, indices);

    YAGL_GET_CTX();

    if (count < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_gles_get_index_size(type, &index_size)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!ctx->ebo && !indices) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (count == 0) {
        goto out;
    }

    yagl_render_invalidate();

    for (i = 0; i < ctx->num_arrays; ++i) {
        if (!ctx->arrays[i].enabled) {
            continue;
        }

        if (!have_range) {
            if (ctx->ebo) {
                if (!yagl_gles_buffer_get_minmax_index(ctx->ebo,
                                                       type,
                                                       (GLint)indices,
                                                       count,
                                                       &min_idx,
                                                       &max_idx)) {
                    YAGL_LOG_WARN("unable to get min/max index from ebo");
                    goto out;
                }
            } else {
                yagl_get_minmax_index(indices, count, type,
                                      &min_idx, &max_idx);
            }
            have_range = 1;
        }

        yagl_gles_array_transfer(&ctx->arrays[i],
                                 min_idx,
                                 max_idx + 1 - min_idx);
    }

    if (!have_range) {
        goto out;
    }

    if (ctx->ebo) {
        yagl_gles_buffer_bind(ctx->ebo,
                              type,
                              0,
                              GL_ELEMENT_ARRAY_BUFFER);
        yagl_gles_buffer_transfer(ctx->ebo,
                                  type,
                                  GL_ELEMENT_ARRAY_BUFFER,
                                  0);
        ctx->draw_elements(ctx, mode, count, type, NULL, (int32_t)indices);
        yagl_host_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    } else {
        ctx->draw_elements(ctx, mode, count, type, indices, count * index_size);
    }

out:
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

    YAGL_LOG_FUNC_ENTER_SPLIT4(glFramebufferRenderbuffer, GLenum, GLenum, GLenum, GLuint, target, attachment, renderbuffertarget, renderbuffer);

    YAGL_GET_CTX();

    framebuffer_obj = yagl_gles_context_acquire_binded_framebuffer(ctx, target);

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

    if (!yagl_gles_framebuffer_renderbuffer(framebuffer_obj,
                                            target,
                                            attachment,
                                            renderbuffertarget,
                                            renderbuffer_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
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

    YAGL_LOG_FUNC_ENTER_SPLIT5(glFramebufferTexture2D, GLenum, GLenum, GLenum, GLuint, GLint, target, attachment, textarget, texture, level);

    YAGL_GET_CTX();

    framebuffer_obj = yagl_gles_context_acquire_binded_framebuffer(ctx, target);

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

    if (!yagl_gles_framebuffer_texture2d(framebuffer_obj,
                                         target,
                                         attachment,
                                         textarget,
                                         level,
                                         texture_obj)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
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
        yagl_sharegroup_add(ctx->base.sg,
                            YAGL_NS_FRAMEBUFFER,
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

    if (!yagl_gles_is_buffer_target_valid(target)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    buffer_obj = yagl_gles_context_acquire_binded_buffer(ctx, target);

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

    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetFramebufferAttachmentParameteriv, GLenum, GLenum, GLenum, GLint*, target, attachment, pname, params);

    YAGL_GET_CTX();

    framebuffer_obj = yagl_gles_context_acquire_binded_framebuffer(ctx, target);

    if (!framebuffer_obj) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!yagl_gles_framebuffer_get_attachment_parameter(framebuffer_obj,
                                                        attachment,
                                                        pname,
                                                        params)) {
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

    /*
     * TODO: Passthrough for now.
     */

    yagl_gles_context_get_tex_parameterfv(ctx, target, pname, params);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetTexParameteriv, GLenum, GLenum, GLint*, target, pname, params);

    YAGL_GET_CTX();

    /*
     * TODO: Passthrough for now.
     */
    yagl_host_glGetTexParameteriv(target, pname, params);

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

    framebuffer_obj = (struct yagl_gles_framebuffer*)yagl_sharegroup_acquire_object(ctx->base.sg,
        YAGL_NS_FRAMEBUFFER, framebuffer);

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

    if (texture_obj && (yagl_gles_texture_get_target(texture_obj) != 0)) {
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

    switch (pname) {
    case GL_PACK_ALIGNMENT:
        if (yagl_gles_is_alignment_valid(param)) {
            ctx->pack_alignment = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
        break;
    case GL_UNPACK_ALIGNMENT:
        if (yagl_gles_is_alignment_valid(param)) {
            ctx->unpack_alignment = param;
        } else {
            YAGL_SET_ERR(GL_INVALID_VALUE);
            goto out;
        }
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
    GLsizei stride;

    YAGL_LOG_FUNC_ENTER_SPLIT7(glReadPixels, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*, x, y, width, height, format, type, pixels);

    YAGL_GET_CTX();

    if (!pixels || (width < 0) || (height < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (!yagl_get_stride(ctx->pack_alignment,
                         width,
                         format,
                         type,
                         &stride)) {
        YAGL_SET_ERR(GL_INVALID_OPERATION);
        goto out;
    }

    yagl_render_invalidate();

    yagl_host_glReadPixels(x, y,
                           width, height, format,
                           yagl_get_actual_type(type),
                           pixels, stride * height, NULL);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    GLsizei stride = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT9(glTexImage2D, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*, target, level, internalformat, width, height, border, format, type, pixels);

    YAGL_GET_CTX();

    if (pixels && (width > 0) && (height > 0)) {
        if (!yagl_get_stride(ctx->unpack_alignment,
                             width,
                             format,
                             type,
                             &stride)) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }
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

    yagl_host_glTexImage2D(target,
                           level,
                           yagl_get_actual_internalformat(internalformat),
                           width,
                           height,
                           border,
                           format,
                           yagl_get_actual_type(type),
                           pixels,
                           stride * height);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    GLsizei stride = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT9(glTexSubImage2D, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*, target, level, xoffset, yoffset, width, height, format, type, pixels);

    YAGL_GET_CTX();

    if (pixels && (width > 0) && (height > 0)) {
        if (!yagl_get_stride(ctx->unpack_alignment,
                             width,
                             format,
                             type,
                             &stride)) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }
    }

    /*
     * Nvidia Windows openGL drivers doesn't account for GL_UNPACK_ALIGNMENT
     * parameter when glTexSubImage2D function is called with format GL_ALPHA.
     * Work around this by manually setting line stride.
     */
    if (format == GL_ALPHA) {
        yagl_host_glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    }

    yagl_host_glTexSubImage2D(target,
                              level,
                              xoffset,
                              yoffset,
                              width,
                              height,
                              format,
                              yagl_get_actual_type(type),
                              pixels,
                              stride * height);

    if (format == GL_ALPHA) {
        yagl_host_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
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