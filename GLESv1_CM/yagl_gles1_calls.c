#include "GLES/gl.h"
#include "GLES/glext.h"
#include "yagl_gles_calls.h"
#include "yagl_host_gles_calls.h"
#include "yagl_gles1_validate.h"
#include "yagl_gles1_context.h"
#include "yagl_gles_vertex_array.h"
#include "yagl_gles_array.h"
#include "yagl_gles_buffer.h"
#include "yagl_impl.h"
#include "yagl_malloc.h"
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
    struct yagl_gles1_context *ctx = \
        (struct yagl_gles1_context*)yagl_get_client_context(); \
    if (!ctx || (ctx->base.base.client_api != yagl_client_api_gles1)) { \
        YAGL_LOG_WARN("no current context"); \
        YAGL_LOG_FUNC_EXIT(NULL); \
        ret_expr; \
    }

#define YAGL_GET_CTX_RET(ret) YAGL_GET_CTX_IMPL(return ret)

#define YAGL_GET_CTX() YAGL_GET_CTX_IMPL(return)

static __inline int yagl_gles1_array_idx_get(struct yagl_gles1_context *ctx,
                                             GLenum array,
                                             unsigned *arr_idx_p)
{
    switch (array) {
    case GL_VERTEX_ARRAY:
        *arr_idx_p = yagl_gles1_array_vertex;
        break;
    case GL_COLOR_ARRAY:
        *arr_idx_p = yagl_gles1_array_color;
        break;
    case GL_NORMAL_ARRAY:
        *arr_idx_p = yagl_gles1_array_normal;
        break;
    case GL_TEXTURE_COORD_ARRAY:
        *arr_idx_p = yagl_gles1_array_texcoord + ctx->client_active_texture;
        break;
    case GL_POINT_SIZE_ARRAY_OES:
        *arr_idx_p = yagl_gles1_array_pointsize;
        break;
    default:
        return 0;
    }

    return 1;
}

/*
 * TODO: Passthrough for now.
 * @{
 */

YAGL_IMPLEMENT_API_NORET2(glAlphaFunc, GLenum, GLclampf, func, ref)
YAGL_IMPLEMENT_API_NORET3(glTexEnvf, GLenum, GLenum, GLfloat, target, pname, param)
YAGL_IMPLEMENT_API_NORET1(glMatrixMode, GLenum, mode)
YAGL_IMPLEMENT_API_NORET0(glLoadIdentity)
YAGL_IMPLEMENT_API_NORET0(glPopMatrix)
YAGL_IMPLEMENT_API_NORET0(glPushMatrix)
YAGL_IMPLEMENT_API_NORET4(glRotatef, GLfloat, GLfloat, GLfloat, GLfloat, angle, x, y, z)
YAGL_IMPLEMENT_API_NORET3(glTranslatef, GLfloat, GLfloat, GLfloat, x, y, z)
YAGL_IMPLEMENT_API_NORET3(glScalef, GLfloat, GLfloat, GLfloat, x, y, z)
YAGL_IMPLEMENT_API_NORET4(glColor4f, GLfloat, GLfloat, GLfloat, GLfloat, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET4(glColor4ub, GLubyte, GLubyte, GLubyte, GLubyte, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET3(glNormal3f, GLfloat, GLfloat, GLfloat, nx, ny, nz)
YAGL_IMPLEMENT_API_NORET1(glShadeModel, GLenum, mode)
YAGL_IMPLEMENT_API_NORET1(glLogicOp, GLenum, opcode)

/*
 * @}
 */

YAGL_API void glAlphaFuncx(GLenum func, GLclampx ref)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glAlphaFuncx, GLenum, GLclampx, func, ref);

    YAGL_GET_CTX();

    yagl_host_glAlphaFunc(func, yagl_fixed_to_float(ref));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexEnvi(GLenum target, GLenum pname, GLint param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexEnvi, GLenum, GLenum, GLint, target, pname, param);

    YAGL_GET_CTX();

    if ((target != GL_TEXTURE_ENV) && (target != GL_POINT_SPRITE_OES)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glTexEnvi(target, pname, param);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexEnvx(GLenum target, GLenum pname, GLfixed param)
{
    GLfloat paramf;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexEnvx, GLenum, GLenum, GLfixed, target, pname, param);

    YAGL_GET_CTX();

    if ((target != GL_TEXTURE_ENV) && (target != GL_POINT_SPRITE_OES)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if ((pname == GL_RGB_SCALE) || (pname == GL_ALPHA_SCALE)) {
        paramf = yagl_fixed_to_float(param);
    } else {
        paramf = (GLfloat)param;
    }

    yagl_host_glTexEnvf(target, pname, paramf);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glRotatex(GLfixed angle,
                        GLfixed x,
                        GLfixed y,
                        GLfixed z)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glRotatex, GLfixed, GLfixed, GLfixed, GLfixed, angle, x, y, z);

    YAGL_GET_CTX();

    yagl_host_glRotatef(yagl_fixed_to_float(angle),
                        yagl_fixed_to_float(x),
                        yagl_fixed_to_float(y),
                        yagl_fixed_to_float(z));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTranslatex(GLfixed x,
                           GLfixed y,
                           GLfixed z)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glTranslatex, GLfixed, GLfixed, GLfixed, x, y, z);

    YAGL_GET_CTX();

    yagl_host_glTranslatef(yagl_fixed_to_float(x),
                           yagl_fixed_to_float(y),
                           yagl_fixed_to_float(z));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glScalex(GLfixed x,
                       GLfixed y,
                       GLfixed z)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glScalex, GLfixed, GLfixed, GLfixed, x, y, z);

    YAGL_GET_CTX();

    yagl_host_glScalef(yagl_fixed_to_float(x),
                       yagl_fixed_to_float(y),
                       yagl_fixed_to_float(z));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glOrthof(GLfloat left,
                       GLfloat right,
                       GLfloat bottom,
                       GLfloat top,
                       GLfloat zNear,
                       GLfloat zFar)
{
    YAGL_LOG_FUNC_ENTER_SPLIT6(glOrthof, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, left, right, bottom, top, zNear, zFar);

    YAGL_GET_CTX();

    if ((left == right) || (bottom == top) || (zNear == zFar)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
    } else {
        yagl_host_glOrthof(left, right, bottom, top, zNear, zFar);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glOrthox(GLfixed left,
                       GLfixed right,
                       GLfixed bottom,
                       GLfixed top,
                       GLfixed zNear,
                       GLfixed zFar)
{
    YAGL_LOG_FUNC_ENTER_SPLIT6(glOrthox, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, left, right, bottom, top, zNear, zFar);

    YAGL_GET_CTX();

    if ((left == right) || (bottom == top) || (zNear == zFar)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
    } else {
        yagl_host_glOrthof(yagl_fixed_to_float(left),
                           yagl_fixed_to_float(right),
                           yagl_fixed_to_float(bottom),
                           yagl_fixed_to_float(top),
                           yagl_fixed_to_float(zNear),
                           yagl_fixed_to_float(zFar));
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glPointSize(GLfloat size)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glPointSize, GLfloat, size);

    YAGL_GET_CTX();

    if (size <= 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
    } else {
        yagl_host_glPointSize(size);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glPointSizex(GLfixed size)
{
    GLfloat sizef;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glPointSizex, GLfixed, size);

    YAGL_GET_CTX();

    sizef = yagl_fixed_to_float(size);

    if (sizef <= 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
    } else {
        yagl_host_glPointSize(sizef);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLineWidthx(GLfixed width)
{
    GLfloat widthf;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glLineWidthx, GLfixed, width);

    YAGL_GET_CTX();

    widthf = yagl_fixed_to_float(width);

    if (widthf <= 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    yagl_gles_context_line_width(&ctx->base, widthf);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexParameterx(GLenum target,
                              GLenum pname,
                              GLfixed param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexParameterx, GLenum, GLenum, GLfixed, target, pname, param);

    YAGL_GET_CTX();

    if (target != GL_TEXTURE_2D) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_gles_context_tex_parameterf(&ctx->base, target, pname, (GLfloat)param);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glColor4x(GLfixed red,
                        GLfixed green,
                        GLfixed blue,
                        GLfixed alpha)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glColor4x, GLfixed, GLfixed, GLfixed, GLfixed, red, green, blue, alpha);

    YAGL_GET_CTX();

    yagl_host_glColor4f(yagl_fixed_to_float(red),
                        yagl_fixed_to_float(green),
                        yagl_fixed_to_float(blue),
                        yagl_fixed_to_float(alpha));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glNormal3x(GLfixed nx,
                         GLfixed ny,
                         GLfixed nz)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glNormal3x, GLfixed, GLfixed, GLfixed, nx, ny, nz);

    YAGL_GET_CTX();

    yagl_host_glNormal3f(yagl_fixed_to_float(nx),
                         yagl_fixed_to_float(ny),
                         yagl_fixed_to_float(nz));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClearColorx(GLclampx red,
                            GLclampx green,
                            GLclampx blue,
                            GLclampx alpha)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glClearColorx, GLclampx, GLclampx, GLclampx, GLclampx, red, green, blue, alpha);

    YAGL_GET_CTX();

    yagl_gles_context_clear_color(&ctx->base,
                                  yagl_fixed_to_float(red),
                                  yagl_fixed_to_float(green),
                                  yagl_fixed_to_float(blue),
                                  yagl_fixed_to_float(alpha));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClearDepthx(GLclampx depth)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glClearDepthx, GLclampx, depth);

    YAGL_GET_CTX();

    yagl_gles_context_clear_depthf(&ctx->base, yagl_fixed_to_float(depth));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glMultiTexCoord4f(GLenum target,
                                GLfloat s,
                                GLfloat t,
                                GLfloat r,
                                GLfloat q)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glMultiTexCoord4f, GLenum, GLfloat, GLfloat, GLfloat, GLfloat, target, s, t, r, q);

    YAGL_GET_CTX();

    if (target >= GL_TEXTURE0 &&
        target < (GL_TEXTURE0 + ctx->base.num_texture_units)) {
        yagl_host_glMultiTexCoord4f(target, s, t, r, q);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glMultiTexCoord4x(GLenum target,
                                GLfixed s,
                                GLfixed t,
                                GLfixed r,
                                GLfixed q)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glMultiTexCoord4x, GLenum, GLfixed, GLfixed, GLfixed, GLfixed, target, s, t, r, q);

    YAGL_GET_CTX();

    if (target >= GL_TEXTURE0 &&
        target < (GL_TEXTURE0 + ctx->base.num_texture_units)) {
        yagl_host_glMultiTexCoord4f(target,
                                    yagl_fixed_to_float(s),
                                    yagl_fixed_to_float(t),
                                    yagl_fixed_to_float(r),
                                    yagl_fixed_to_float(q));
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glPointParameterf(GLenum pname,
                                GLfloat param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glPointParameterf, GLenum, GLfloat, pname, param);

    YAGL_GET_CTX();

    if (pname != GL_POINT_SIZE_MIN && pname != GL_POINT_SIZE_MIN &&
        pname != GL_POINT_SIZE_MAX && pname != GL_POINT_FADE_THRESHOLD_SIZE) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    } else {
        yagl_host_glPointParameterf(pname, param);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glPointParameterx(GLenum pname,
                                GLfixed param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glPointParameterx, GLenum, GLfixed, pname, param);

    YAGL_GET_CTX();

    if (pname != GL_POINT_SIZE_MIN && pname != GL_POINT_SIZE_MIN &&
        pname != GL_POINT_SIZE_MAX && pname != GL_POINT_FADE_THRESHOLD_SIZE) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    } else {
        yagl_host_glPointParameterf(pname, yagl_fixed_to_float(param));
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glFogf(GLenum pname, GLfloat param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glFogf, GLenum, GLfloat, pname, param);

    YAGL_GET_CTX();

    if (pname != GL_FOG_MODE && pname != GL_FOG_DENSITY &&
        pname != GL_FOG_START && pname != GL_FOG_END) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    } else {
        yagl_host_glFogf(pname, param);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glFogx(GLenum pname, GLfixed param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glFogx, GLenum, GLfixed, pname, param);

    YAGL_GET_CTX();

    if (pname != GL_FOG_MODE && pname != GL_FOG_DENSITY &&
        pname != GL_FOG_START && pname != GL_FOG_END) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    } else {
        if (pname == GL_FOG_MODE) {
            yagl_host_glFogf(pname, (GLfloat)param);
        } else {
            yagl_host_glFogf(pname, yagl_fixed_to_float(param));
        }
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glFrustumf(GLfloat left,
                         GLfloat right,
                         GLfloat bottom,
                         GLfloat top,
                         GLfloat zNear,
                         GLfloat zFar)
{
    YAGL_LOG_FUNC_ENTER_SPLIT6(glFrustumf, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, left, right, bottom, top, zNear, zFar);

    YAGL_GET_CTX();

    if (zNear <= 0 || zFar <= 0 || left == right ||
        bottom == top || zNear == zFar) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
    } else {
        yagl_host_glFrustumf(left, right, bottom, top, zNear, zFar);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glFrustumx(GLfixed left,
                         GLfixed right,
                         GLfixed bottom,
                         GLfixed top,
                         GLfixed zNear,
                         GLfixed zFar)
{
    YAGL_LOG_FUNC_ENTER_SPLIT6(glFrustumx, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, left, right, bottom, top, zNear, zFar);

    YAGL_GET_CTX();

    if (zNear <= 0 || zFar <= 0 ||left == right ||
        bottom == top || zNear == zFar) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
    } else {
        yagl_host_glFrustumf(yagl_fixed_to_float(left),
                             yagl_fixed_to_float(right),
                             yagl_fixed_to_float(bottom),
                             yagl_fixed_to_float(top),
                             yagl_fixed_to_float(zNear),
                             yagl_fixed_to_float(zFar));
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLightf(GLenum light,
                       GLenum pname,
                       GLfloat param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glLightf, GLenum, GLenum, GLfloat, light, pname, param);

    YAGL_GET_CTX();

    if (light < GL_LIGHT0 || light >= (GL_LIGHT0 + ctx->max_lights)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    } else {
        yagl_host_glLightf(light, pname, param);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLightx(GLenum light,
                       GLenum pname,
                       GLfixed param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glLightx, GLenum, GLenum, GLfixed, light, pname, param);

    YAGL_GET_CTX();

    if (light < GL_LIGHT0 || light >= (GL_LIGHT0 + ctx->max_lights)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    } else {
        yagl_host_glLightf(light, pname, yagl_fixed_to_float(param));
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLightModelf(GLenum pname, GLfloat param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glLightModelf, GLenum, GLfloat, pname, param);

    YAGL_GET_CTX();

    if (pname != GL_LIGHT_MODEL_TWO_SIDE) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    } else {
        yagl_host_glLightModelf(pname, param);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLightModelx(GLenum pname,
                            GLfixed param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glLightModelx, GLenum, GLfixed, pname, param);

    YAGL_GET_CTX();

    if (pname != GL_LIGHT_MODEL_TWO_SIDE) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    } else {
        yagl_host_glLightModelf(pname, (GLfloat)param);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glMaterialf(GLenum face,
                          GLenum pname,
                          GLfloat param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glMaterialf, GLenum, GLenum, GLfloat, face, pname, param);

    YAGL_GET_CTX();

    if (face != GL_FRONT_AND_BACK) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    } else {
        yagl_host_glMaterialf(face, pname, param);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glMaterialx(GLenum face,
                          GLenum pname,
                          GLfixed param)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glMaterialx, GLenum, GLenum, GLfixed, face, pname, param);

    YAGL_GET_CTX();

    if (face != GL_FRONT_AND_BACK) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
    } else {
        yagl_host_glMaterialf(face, pname, yagl_fixed_to_float(param));
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glSampleCoveragex(GLclampx value,
                                GLboolean invert)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glSampleCoveragex, GLclampx, GLboolean, value, invert);

    YAGL_GET_CTX();

    yagl_gles_context_sample_coverage(&ctx->base,
                                      yagl_fixed_to_float(value),
                                      invert);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDepthRangex(GLclampx zNear, GLclampx zFar)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glDepthRangex, GLclampx, GLclampx, zNear, zFar);

    YAGL_GET_CTX();

    yagl_gles_context_depth_rangef(&ctx->base,
                                   yagl_fixed_to_float(zNear),
                                   yagl_fixed_to_float(zFar));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glPolygonOffsetx(GLfixed factor, GLfixed units)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glPolygonOffsetx, GLfixed, GLfixed, factor, units);

    YAGL_GET_CTX();

    yagl_gles_context_polygon_offset(&ctx->base,
                                     yagl_fixed_to_float(factor),
                                     yagl_fixed_to_float(units));

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
    int count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexEnviv, GLenum, GLenum, const GLint*, target, pname, params);

    YAGL_GET_CTX();

    if (target != GL_TEXTURE_ENV && target != GL_POINT_SPRITE_OES) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!yagl_gles1_get_texenv_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glTexEnviv(target, pname, params, count);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
    int count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexEnvfv, GLenum, GLenum, const GLfloat *, target, pname, params);

    YAGL_GET_CTX();

    if (target != GL_TEXTURE_ENV && target != GL_POINT_SPRITE_OES) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!yagl_gles1_get_texenv_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glTexEnvfv(target, pname, params, count);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexEnvxv(GLenum target, GLenum pname, const GLfixed *params)
{
    GLfloat paramsf[100]; // This fits all cases.
    int i, count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexEnvxv, GLenum, GLenum, const GLfixed *, target, pname, params);

    YAGL_GET_CTX();

    if (target != GL_TEXTURE_ENV && target != GL_POINT_SPRITE_OES) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!yagl_gles1_get_texenv_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (params) {
        if ((pname == GL_TEXTURE_ENV_COLOR) ||
            (pname == GL_RGB_SCALE) ||
            (pname == GL_ALPHA_SCALE)) {
            for (i = 0; i < count; ++i) {
                paramsf[i] = yagl_fixed_to_float(params[i]);
            }
        } else {
            for (i = 0; i < count; ++i) {
                paramsf[i] = (GLfloat)params[i];
            }
        }

        yagl_host_glTexEnvfv(target, pname, paramsf, count);
    } else {
        yagl_host_glTexEnvfv(target, pname, NULL, 0);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClientActiveTexture(GLenum texture)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glClientActiveTexture, GLenum, texture);

    YAGL_GET_CTX();

    if ((texture < GL_TEXTURE0) ||
        (texture >= (GL_TEXTURE0 + ctx->base.num_texture_units))) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    ctx->client_active_texture = texture - GL_TEXTURE0;

    yagl_host_glClientActiveTexture(texture);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDisableClientState(GLenum array_name)
{
    unsigned arr_idx;
    struct yagl_gles_array *array;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glDisableClientState, GLenum, array_name);

    YAGL_GET_CTX();

    if (!yagl_gles1_array_idx_get(ctx, array_name, &arr_idx)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    array = &ctx->base.vao->arrays[arr_idx];

    yagl_gles_array_enable(array, 0);

    if (array_name != GL_POINT_SIZE_ARRAY_OES) {
        yagl_host_glDisableClientState(array_name);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glEnableClientState(GLenum array_name)
{
    unsigned arr_idx;
    struct yagl_gles_array *array;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glEnableClientState, GLenum, array_name);

    YAGL_GET_CTX();

    if (!yagl_gles1_array_idx_get(ctx, array_name, &arr_idx)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    array = &ctx->base.vao->arrays[arr_idx];

    yagl_gles_array_enable(array, 1);

    if (array_name != GL_POINT_SIZE_ARRAY_OES) {
        yagl_host_glEnableClientState(array_name);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
    int count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetTexEnviv, GLenum, GLenum, GLint *, target, pname, params);

    YAGL_GET_CTX();

    if (target != GL_TEXTURE_ENV && target != GL_POINT_SPRITE_OES) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!yagl_gles1_get_texenv_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glGetTexEnviv(target, pname, params, count, NULL);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
    int count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetTexEnvfv, GLenum, GLenum, GLfloat *, target, pname, params);

    YAGL_GET_CTX();

    if (target != GL_TEXTURE_ENV && target != GL_POINT_SPRITE_OES) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!yagl_gles1_get_texenv_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glGetTexEnvfv(target, pname, params, count, NULL);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetTexEnvxv(GLenum target, GLenum pname, GLfixed *params)
{
    int i, count = 0;
    GLfloat paramsf[100]; // This fits all cases.

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetTexEnvxv, GLenum, GLenum, GLint *, target, pname, params);

    YAGL_GET_CTX();

    if (target != GL_TEXTURE_ENV && target != GL_POINT_SPRITE_OES) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (!yagl_gles1_get_texenv_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glGetTexEnvfv(target, pname, paramsf, count, NULL);

    if (params) {
        if ((pname == GL_TEXTURE_ENV_COLOR) ||
            (pname == GL_RGB_SCALE) ||
            (pname == GL_ALPHA_SCALE)) {
            for (i = 0; i < count; ++i) {
                params[i] = yagl_float_to_fixed(paramsf[i]);
            }
        } else {
            for (i = 0; i < count; ++i) {
                params[i] = (GLfixed)paramsf[i];
            }
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetPointerv(GLenum pname, GLvoid** pointer)
{
    struct yagl_gles_array *array = NULL;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetPointerv, GLenum, GLvoid**, pname, pointer);

    YAGL_GET_CTX();

    switch (pname) {
    case GL_VERTEX_ARRAY_POINTER:
        array = &ctx->base.vao->arrays[yagl_gles1_array_vertex];
        break;
    case GL_COLOR_ARRAY_POINTER:
        array = &ctx->base.vao->arrays[yagl_gles1_array_color];
        break;
    case GL_NORMAL_ARRAY_POINTER:
        array = &ctx->base.vao->arrays[yagl_gles1_array_normal];
        break;
    case GL_TEXTURE_COORD_ARRAY_POINTER:
        array = &ctx->base.vao->arrays[yagl_gles1_array_texcoord + ctx->client_active_texture];
        break;
    case GL_POINT_SIZE_ARRAY_POINTER_OES:
        array = &ctx->base.vao->arrays[yagl_gles1_array_pointsize];
        break;
    default:
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

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

YAGL_API void glNormalPointer(GLenum type, GLsizei stride, const GLvoid* pointer)
{
    struct yagl_gles_array *array;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glNormalPointer, GLenum, GLsizei, const GLvoid*, type, stride, pointer);

    YAGL_GET_CTX();

    if (stride < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (type != GL_FLOAT && type != GL_FIXED &&
        type != GL_SHORT && type != GL_BYTE) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    array = &ctx->base.vao->arrays[yagl_gles1_array_normal];

    if (ctx->base.vbo) {
        if (!yagl_gles_array_update_vbo(array,
                                        3,
                                        type,
                                        type == GL_FIXED,
                                        GL_FALSE,
                                        stride,
                                        ctx->base.vbo,
                                        (GLint)pointer)) {
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
        if ((ctx->base.vao != ctx->base.va_zero) && pointer) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }

        if (!yagl_gles_array_update(array,
                                    3,
                                    type,
                                    type == GL_FIXED,
                                    GL_FALSE,
                                    stride,
                                    pointer)) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    struct yagl_gles_array *array;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glVertexPointer, GLint, GLenum, GLsizei, const GLvoid*, size, type, stride, pointer);

    YAGL_GET_CTX();

    if ((size < 2) || (size > 4) || (stride < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (type != GL_FLOAT && type != GL_FIXED &&
        type != GL_SHORT && type != GL_BYTE) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    array = &ctx->base.vao->arrays[yagl_gles1_array_vertex];

    if (ctx->base.vbo) {
        if (!yagl_gles_array_update_vbo(array,
                                        size,
                                        type,
                                        type == GL_FIXED || type == GL_BYTE,
                                        GL_FALSE,
                                        stride,
                                        ctx->base.vbo,
                                        (GLint)pointer)) {
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
        if ((ctx->base.vao != ctx->base.va_zero) && pointer) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }

        if (!yagl_gles_array_update(array,
                                    size,
                                    type,
                                    type == GL_FIXED || type == GL_BYTE,
                                    GL_FALSE,
                                    stride,
                                    pointer)) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    struct yagl_gles_array *array;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glColorPointer, GLint, GLenum, GLsizei, const GLvoid*, size, type, stride, pointer);

    YAGL_GET_CTX();

    if (size != 4 || stride < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (type != GL_FLOAT && type != GL_FIXED && type != GL_UNSIGNED_BYTE) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    array = &ctx->base.vao->arrays[yagl_gles1_array_color];

    if (ctx->base.vbo) {
        if (!yagl_gles_array_update_vbo(array,
                                        size,
                                        type,
                                        type == GL_FIXED,
                                        GL_FALSE,
                                        stride,
                                        ctx->base.vbo,
                                        (GLint)pointer)) {
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
        if ((ctx->base.vao != ctx->base.va_zero) && pointer) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }

        if (!yagl_gles_array_update(array,
                                    size,
                                    type,
                                    type == GL_FIXED,
                                    GL_FALSE,
                                    stride,
                                    pointer)) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    struct yagl_gles_array *array;

    YAGL_LOG_FUNC_ENTER_SPLIT4(glTexCoordPointer, GLint, GLenum, GLsizei, const GLvoid*, size, type, stride, pointer);

    YAGL_GET_CTX();

    if ((size < 2) || (size > 4) || (stride < 0)) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (type != GL_FLOAT && type != GL_FIXED &&
        type != GL_SHORT && type != GL_BYTE) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    array = &ctx->base.vao->arrays[yagl_gles1_array_texcoord + ctx->client_active_texture];

    if (ctx->base.vbo) {
        if (!yagl_gles_array_update_vbo(array,
                                        size,
                                        type,
                                        type == GL_FIXED || type == GL_BYTE,
                                        GL_FALSE,
                                        stride,
                                        ctx->base.vbo,
                                        (GLint)pointer)) {
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
        if ((ctx->base.vao != ctx->base.va_zero) && pointer) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }

        if (!yagl_gles_array_update(array,
                                    size,
                                    type,
                                    type == GL_FIXED || type == GL_BYTE,
                                    GL_FALSE,
                                    stride,
                                    pointer)) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glPointSizePointerOES(GLenum type, GLsizei stride, const GLvoid* pointer)
{
    struct yagl_gles_array *array;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glPointSizePointerOES, GLenum, GLsizei, const GLvoid*, type, stride, pointer);

    YAGL_GET_CTX();

    if (stride < 0) {
        YAGL_SET_ERR(GL_INVALID_VALUE);
        goto out;
    }

    if (type != GL_FLOAT && type != GL_FIXED) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    array = &ctx->base.vao->arrays[yagl_gles1_array_pointsize];

    if (ctx->base.vbo) {
        if (!yagl_gles_array_update_vbo(array,
                                        1,
                                        type,
                                        type == GL_FIXED,
                                        GL_FALSE,
                                        stride,
                                        ctx->base.vbo,
                                        (GLint)pointer)) {
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
        if ((ctx->base.vao != ctx->base.va_zero) && pointer) {
            YAGL_SET_ERR(GL_INVALID_OPERATION);
            goto out;
        }

        if (!yagl_gles_array_update(array,
                                    1,
                                    type,
                                    type == GL_FIXED,
                                    GL_FALSE,
                                    stride,
                                    pointer)) {
            YAGL_SET_ERR(GL_INVALID_VALUE);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexParameterxv(GLenum target, GLenum pname, const GLfixed *params)
{
    GLfloat paramf;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexParameterxv, GLenum, GLenum, const GLfixed*, target, pname, params);

    YAGL_GET_CTX();

    if (target != GL_TEXTURE_2D) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (params) {
        paramf = (GLfloat)params[0];
        yagl_gles_context_tex_parameterfv(&ctx->base, target, pname, &paramf);
    } else {
        yagl_gles_context_tex_parameterfv(&ctx->base, target, pname, NULL);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetTexParameterxv(GLenum target, GLenum pname, GLfixed *params)
{
    GLfloat paramf;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetTexParameterxv, GLenum, GLenum, GLfixed*, target, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles_context_get_tex_parameterfv(&ctx->base, target, pname, &paramf)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (params) {
        *params = (GLfixed)paramf;
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glMultMatrixf(const GLfloat* m)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glMultMatrixf, const GLfloat*, m);
    yagl_host_glMultMatrixf(m, 16);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glMultMatrixx(const GLfixed* m)
{
    GLfloat tmp[16];
    int i;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glMultMatrixx, const GLfixed*, m);

    YAGL_GET_CTX();

    if (m) {
        for (i = 0; i < 16; ++i) {
            tmp[i] = yagl_fixed_to_float(m[i]);
        }
        yagl_host_glMultMatrixf(tmp, 16);
    } else {
        yagl_host_glMultMatrixf(NULL, 16);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLoadMatrixf(const GLfloat* m)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glLoadMatrixf, const GLfloat*, m);
    yagl_host_glLoadMatrixf(m, 16);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLoadMatrixx(const GLfixed* m)
{
    GLfloat tmp[16];
    int i;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glLoadMatrixx, const GLfixed*, m);

    YAGL_GET_CTX();

    if (m) {
        for (i = 0; i < 16; ++i) {
            tmp[i] = yagl_fixed_to_float(m[i]);
        }
        yagl_host_glLoadMatrixf(tmp, 16);
    } else {
        yagl_host_glLoadMatrixf(NULL, 16);
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glPointParameterfv(GLenum pname, const GLfloat *params)
{
    int count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glPointParameterfv, GLenum, const GLfloat *, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles1_get_point_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glPointParameterfv(pname, params, count);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glPointParameterxv(GLenum pname, const GLfixed *params)
{
    GLfloat paramsf[100]; // This fits all cases.
    int i, count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glPointParameterxv, GLenum, const GLfixed *, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles1_get_point_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (params) {
        for (i = 0; i < count; ++i) {
            paramsf[i] = yagl_fixed_to_float(params[i]);
        }
        yagl_host_glPointParameterfv(pname, paramsf, count);
    } else {
        yagl_host_glPointParameterfv(pname, NULL, count);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glFogfv(GLenum pname, const GLfloat *params)
{
    int count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glFogfv, GLenum, const GLfloat *, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles1_get_fog_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glFogfv(pname, params, count);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glFogxv(GLenum pname, const GLfixed *params)
{
    GLfloat paramsf[100]; // This fits all cases.
    int i, count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glFogxv, GLenum, const GLfixed *, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles1_get_fog_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (params) {
        if (pname == GL_FOG_MODE) {
            for (i = 0; i < count; ++i) {
                paramsf[i] = (GLfloat)params[i];
            }
        } else {
            for (i = 0; i < count; ++i) {
                paramsf[i] = yagl_fixed_to_float(params[i]);
            }
        }
        yagl_host_glFogfv(pname, paramsf, count);
    } else {
        yagl_host_glFogfv(pname, NULL, count);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClipPlanef(GLenum plane, const GLfloat *equation)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glClipPlanef, GLenum, const GLfloat *, plane, equation);

    YAGL_GET_CTX();

    if (plane < GL_CLIP_PLANE0 || plane >= (GL_CLIP_PLANE0 + ctx->max_clip_planes)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glClipPlanef(plane, equation, 4);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClipPlanex(GLenum plane, const GLfixed *equation)
{
    GLfloat equationf[4];
    int i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glClipPlanex, GLenum, const GLfixed *, plane, equation);

    YAGL_GET_CTX();

    if (plane < GL_CLIP_PLANE0 || plane >= (GL_CLIP_PLANE0 + ctx->max_clip_planes)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (equation) {
        for (i = 0; i < 4; ++i) {
            equationf[i] = yagl_fixed_to_float(equation[i]);
        }
        yagl_host_glClipPlanef(plane, equationf, 4);
    } else {
        yagl_host_glClipPlanef(plane, NULL, 4);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetClipPlanef(GLenum pname, GLfloat *eqn)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetClipPlanef, GLenum, GLfloat*, pname, eqn);

    YAGL_GET_CTX();

    if (pname < GL_CLIP_PLANE0 || pname >= (GL_CLIP_PLANE0 + ctx->max_clip_planes)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glGetClipPlanef(pname, eqn, 4, NULL);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetClipPlanex(GLenum pname, GLfixed *eqn)
{
    GLfloat equationf[4];
    int i;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetClipPlanex, GLenum, GLfixed*, pname, eqn);

    YAGL_GET_CTX();

    if (pname < GL_CLIP_PLANE0 ||
        pname >= (GL_CLIP_PLANE0 + ctx->max_clip_planes)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glGetClipPlanef(pname, equationf, 4, NULL);

    if (eqn) {
        for (i = 0; i < 4; ++i) {
            eqn[i] = yagl_float_to_fixed(equationf[i]);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    int count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glLightfv, GLenum, GLenum, const GLfloat *, light, pname, params);

    YAGL_GET_CTX();

    if (light < GL_LIGHT0 || light >= (GL_LIGHT0 + ctx->max_lights) ||
        !yagl_gles1_get_light_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glLightfv(light, pname, params, count);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLightxv(GLenum light, GLenum pname, const GLfixed *params)
{
    GLfloat paramsf[100]; // This fits all cases.
    int i, count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glLightxv, GLenum, GLenum, const GLfixed *, light, pname, params);

    YAGL_GET_CTX();

    if (light < GL_LIGHT0 || light >= (GL_LIGHT0 + ctx->max_lights) ||
        !yagl_gles1_get_light_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (params) {
        for (i = 0; i < count; ++i) {
            paramsf[i] = yagl_fixed_to_float(params[i]);
        }
        yagl_host_glLightfv(light, pname, paramsf, count);
    } else {
        yagl_host_glLightfv(light, pname, NULL, count);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLightModelfv(GLenum pname, const GLfloat *params)
{
    int count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glLightModelfv, GLenum, const GLfloat *, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles1_get_light_model_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glLightModelfv(pname, params, count);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLightModelxv(GLenum pname, const GLfixed *params)
{
    GLfloat paramsf[100]; // This fits all cases.
    int i, count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glLightModelxv, GLenum, const GLfixed *, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles1_get_light_model_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (params) {
        if (pname == GL_LIGHT_MODEL_TWO_SIDE) {
            for (i = 0; i < count; ++i) {
                paramsf[i] = (GLfloat)(params[i]);
            }
        } else {
            for (i = 0; i < count; ++i) {
                paramsf[i] = yagl_fixed_to_float(params[i]);
            }
        }
        yagl_host_glLightModelfv(pname, paramsf, count);
    } else {
        yagl_host_glLightModelfv(pname, NULL, count);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
    int count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetLightfv, GLenum, GLenum, GLfloat*, light, pname, params);

    YAGL_GET_CTX();

    if (light < GL_LIGHT0 || light >= (GL_LIGHT0 + ctx->max_lights) ||
        !yagl_gles1_get_light_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glGetLightfv(light, pname, params, count, NULL);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetLightxv(GLenum light, GLenum pname, GLfixed *params)
{
    GLfloat paramsf[100]; // This fits all cases.
    int i, count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetLightxv, GLenum, GLenum, GLfixed*, light, pname, params);

    YAGL_GET_CTX();

    if (light < GL_LIGHT0 || light >= (GL_LIGHT0 + ctx->max_lights) ||
        !yagl_gles1_get_light_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glGetLightfv(light, pname, paramsf, count, NULL);

    if (params) {
        for (i = 0; i < count; ++i) {
            params[i] = yagl_float_to_fixed(paramsf[i]);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    int count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glMaterialfv, GLenum, GLenum, const GLfloat *, face, pname, params);

    YAGL_GET_CTX();

    if (face != GL_FRONT_AND_BACK ||
        !yagl_gles1_get_material_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glMaterialfv(face, pname, params, count);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glMaterialxv(GLenum face, GLenum pname, const GLfixed *params)
{
    GLfloat paramsf[100]; // This fits all cases.
    int i, count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glMaterialxv, GLenum, GLenum, const GLfixed *, face, pname, params);

    YAGL_GET_CTX();

    if (face != GL_FRONT_AND_BACK ||
        !yagl_gles1_get_material_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    if (params) {
        for (i = 0; i < count; ++i) {
            paramsf[i] = yagl_fixed_to_float(params[i]);
        }
        yagl_host_glMaterialfv(face, pname, paramsf, count);
    } else {
        yagl_host_glMaterialfv(face, pname, NULL, count);
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
    int count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetMaterialfv, GLenum, GLenum, GLfloat*, face, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles1_get_material_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glGetMaterialfv(face, pname, params, count, NULL);

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetMaterialxv(GLenum light, GLenum pname, GLfixed *params)
{
    GLfloat paramsf[100]; // This fits all cases.
    int i, count = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetMaterialxv, GLenum, GLenum, GLfixed*, light, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles1_get_material_param_count(pname, &count)) {
        YAGL_SET_ERR(GL_INVALID_ENUM);
        goto out;
    }

    yagl_host_glGetMaterialfv(light, pname, paramsf, count, NULL);

    if (params) {
        for (i = 0; i < count; ++i) {
            params[i] = yagl_float_to_fixed(paramsf[i]);
        }
    }

out:
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetFixedv(GLenum pname, GLfixed* params)
{
    GLfloat floats[100]; // This fits all cases.
    uint32_t i, num = 0;
    int needs_map;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetFixedv, GLenum, GLfixed*, pname, params);

    YAGL_GET_CTX();

    if (!yagl_gles_context_get_floatv(&ctx->base, pname, floats, &num, &needs_map)) {
        GLint ints[100]; // This fits all cases.
        if (yagl_gles_context_get_integerv(&ctx->base, pname, ints, &num)) {
            for (i = 0; i < num; ++i) {
                floats[i] = ints[i];
            }
        } else {
            YAGL_SET_ERR(GL_INVALID_ENUM);
        }
    }

    if (params) {
        for (i = 0; i < num; ++i) {
            params[i] = yagl_float_to_fixed(floats[i]);
        }
    }

    YAGL_LOG_FUNC_EXIT(NULL);
}

/* GL_OES_framebuffer_object */

YAGL_API GLboolean glIsRenderbufferOES(GLuint renderbuffer)
{
    return glIsRenderbuffer(renderbuffer);
}

YAGL_API void glBindRenderbufferOES(GLenum target, GLuint renderbuffer)
{
    glBindRenderbuffer(target, renderbuffer);
}

YAGL_API void glDeleteRenderbuffersOES(GLsizei n, const GLuint* renderbuffers)
{
    glDeleteRenderbuffers(n, renderbuffers);
}

YAGL_API void glGenRenderbuffersOES(GLsizei n, GLuint* renderbuffers)
{
    glGenRenderbuffers(n, renderbuffers);
}

YAGL_API void glRenderbufferStorageOES(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    glRenderbufferStorage(target, internalformat, width, height);
}

YAGL_API void glGetRenderbufferParameterivOES(GLenum target, GLenum pname, GLint* params)
{
    glGetRenderbufferParameteriv(target, pname, params);
}

YAGL_API GLboolean glIsFramebufferOES(GLuint framebuffer)
{
    return glIsFramebuffer(framebuffer);
}

YAGL_API void glBindFramebufferOES(GLenum target, GLuint framebuffer)
{
    glBindFramebuffer(target, framebuffer);
}

YAGL_API void glDeleteFramebuffersOES(GLsizei n, const GLuint* framebuffers)
{
    glDeleteFramebuffers(n, framebuffers);
}

YAGL_API void glGenFramebuffersOES(GLsizei n, GLuint* framebuffers)
{
    glGenFramebuffers(n, framebuffers);
}

YAGL_API GLenum glCheckFramebufferStatusOES(GLenum target)
{
    return glCheckFramebufferStatus(target);
}

YAGL_API void glFramebufferTexture2DOES(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

YAGL_API void glFramebufferRenderbufferOES(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

YAGL_API void glGetFramebufferAttachmentParameterivOES(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

YAGL_API void glGenerateMipmapOES(GLenum target)
{
    glGenerateMipmap(target);
}

/* GL_OES_blend_subtract */

YAGL_API void glBlendEquationOES(GLenum mode)
{
    glBlendEquation(mode);
}

/* GL_OES_blend_equation_separate */

YAGL_API void glBlendEquationSeparateOES(GLenum modeRGB, GLenum modeAlpha)
{
    glBlendEquationSeparate(modeRGB, modeAlpha);
}

/* GL_OES_blend_func_separate */

YAGL_API void glBlendFuncSeparateOES(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
}
