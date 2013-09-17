#include "yagl_state.h"
#include "yagl_host_gles1_calls.h"
#include "yagl_impl.h"
#include "yagl_gles_context.h"
#include "yagl_malloc.h"
#include "GLES/glext.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yagl_gles_utils.h"

/*
 * GLES1 has arrays of vertices, normals, colors, texture coordinates and
 * point sizes. Every texture unit has its own texture coordinates array
 */
typedef enum {
    YAGL_GLES1_ARRAY_VERTEX = 0,
    YAGL_GLES1_ARRAY_COLOR,
    YAGL_GLES1_ARRAY_NORMAL,
    YAGL_GLES1_ARRAY_POINTSIZE,
    YAGL_GLES1_ARRAY_TEX_COORD,
} YaglGles1ArrayType;

static inline int yagl_get_active_tex_index(void)
{
    return YAGL_GLES1_ARRAY_TEX_COORD +
                 yagl_get_integer(GL_CLIENT_ACTIVE_TEXTURE) - GL_TEXTURE0;
}

static unsigned yagl_gles1_array_idx_get(struct yagl_gles_context *ctx,
                                         GLenum array)
{
    unsigned ret;

    switch (array) {
    case GL_VERTEX_ARRAY:
        ret = YAGL_GLES1_ARRAY_VERTEX;
        break;
    case GL_COLOR_ARRAY:
        ret = YAGL_GLES1_ARRAY_COLOR;
        break;
    case GL_NORMAL_ARRAY:
        ret = YAGL_GLES1_ARRAY_NORMAL;
        break;
    case GL_TEXTURE_COORD_ARRAY:
        ret = yagl_get_active_tex_index();
        break;
    case GL_POINT_SIZE_ARRAY_OES:
        ret = YAGL_GLES1_ARRAY_POINTSIZE;
        break;
    default:
        ret = -1;
        break;
    }

    if (ret >= ctx->num_arrays) {
        fprintf(stderr, "Error! Array with index %d doesn't exist at %s:%d\n",
                ret, __func__, __LINE__);
    }

    return ret;
}

static void yagl_query_gles1_array(struct yagl_gles_array *arr,
                                   YaglGles1ArrayType arr_type)
{
    GLenum name, buff_bind, stride, pointer;

    switch (arr_type) {
    case YAGL_GLES1_ARRAY_VERTEX:
        name = GL_VERTEX_ARRAY;
        buff_bind = GL_VERTEX_ARRAY_BUFFER_BINDING;
        stride = GL_VERTEX_ARRAY_STRIDE;
        pointer = GL_VERTEX_ARRAY_POINTER;
        break;
    case YAGL_GLES1_ARRAY_COLOR:
        name = GL_COLOR_ARRAY;
        buff_bind = GL_COLOR_ARRAY_BUFFER_BINDING;
        stride = GL_COLOR_ARRAY_STRIDE;
        pointer = GL_COLOR_ARRAY_POINTER;
        break;
    case YAGL_GLES1_ARRAY_NORMAL:
        name = GL_NORMAL_ARRAY;
        buff_bind = GL_NORMAL_ARRAY_BUFFER_BINDING;
        stride = GL_NORMAL_ARRAY_STRIDE;
        pointer = GL_NORMAL_ARRAY_POINTER;
        break;
    case YAGL_GLES1_ARRAY_POINTSIZE:
        name = GL_POINT_SIZE_ARRAY_OES;
        buff_bind = GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES;
        stride = GL_POINT_SIZE_ARRAY_STRIDE_OES;
        pointer = GL_POINT_SIZE_ARRAY_POINTER_OES;
        break;
    case YAGL_GLES1_ARRAY_TEX_COORD:
        name = GL_TEXTURE_COORD_ARRAY;
        buff_bind = GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING;
        stride = GL_TEXTURE_COORD_ARRAY_STRIDE;
        pointer = GL_TEXTURE_COORD_ARRAY_POINTER;
        break;
    default:
        fprintf(stderr, "Programming error! Unknown array %d type at %s:%d\n",
                arr_type, __func__, __LINE__);
        exit(1);
    }

    arr->enabled = yagl_get_integer(name);
    arr->vbo = yagl_get_integer(buff_bind);
    arr->stride = yagl_get_integer(stride);

    if (!arr->vbo) {
        yagl_host_glGetPointerv(pointer, &arr->ptr);
    }
}

static inline void yagl_set_client_active_texture(GLenum tex)
{
    yagl_host_glClientActiveTexture(tex);
}

static void yagl_query_texture_arrays(struct yagl_gles_array *arrays,
                                      int num_texture_units)
{
    GLint cur_text;
    int i;

    cur_text = yagl_get_integer(GL_CLIENT_ACTIVE_TEXTURE);

    for (i = 0; i < num_texture_units; ++i) {
        yagl_set_client_active_texture(GL_TEXTURE0 + i);
        yagl_query_gles1_array(&arrays[i], YAGL_GLES1_ARRAY_TEX_COORD);
    }

    yagl_set_client_active_texture(cur_text);
}

void yagl_update_arrays(void)
{
    struct yagl_gles_context *ctx = yagl_gles_context_get();
    int i;
    int num_texture_units;

    if (!ctx || ctx->arrays) {
        return;
    }

    num_texture_units = yagl_get_integer(GL_MAX_TEXTURE_UNITS);

    ctx->num_arrays = YAGL_GLES1_ARRAY_TEX_COORD + num_texture_units;

    ctx->arrays = yagl_malloc0(sizeof(*ctx->arrays) * ctx->num_arrays);

    for (i = YAGL_GLES1_ARRAY_VERTEX; i < YAGL_GLES1_ARRAY_TEX_COORD; ++i) {
        yagl_query_gles1_array(&ctx->arrays[i], i);
    }

    yagl_query_texture_arrays(&ctx->arrays[YAGL_GLES1_ARRAY_TEX_COORD],
                              num_texture_units);
}

static void yagl_set_array_pointer(GLenum array_type,
                                   GLint size,
                                   GLenum type,
                                   GLsizei stride,
                                   const GLvoid* pointer)
{
    struct yagl_gles_context *ctx = yagl_gles_context_get();
    int el_size = 0;
    unsigned arr_idx;

    if (ctx && ctx->arrays && yagl_get_el_size(type, &el_size)) {
        arr_idx = yagl_gles1_array_idx_get(ctx, array_type);
        yagl_update_vbo();
        ctx->arrays[arr_idx].vbo = 0;
        ctx->arrays[arr_idx].stride = 0;
        ctx->arrays[arr_idx].ptr = NULL;
        if (ctx->vbo) {
            ctx->arrays[arr_idx].vbo = ctx->vbo;
        } else {
            if (stride) {
                ctx->arrays[arr_idx].stride = stride;
            } else {
                ctx->arrays[arr_idx].stride = size * el_size;
            }
            ctx->arrays[arr_idx].ptr = (GLvoid *)pointer;
        }
    }
}

static inline unsigned yagl_get_light_param_len(GLenum pname)
{
    switch (pname) {
    case GL_AMBIENT:
    case GL_DIFFUSE:
    case GL_SPECULAR:
    case GL_POSITION:
        return 4;
    case GL_SPOT_DIRECTION:
        return 3;
    case GL_SPOT_EXPONENT:
    case GL_SPOT_CUTOFF:
    case GL_CONSTANT_ATTENUATION:
    case GL_LINEAR_ATTENUATION:
    case GL_QUADRATIC_ATTENUATION:
        return 1;
    default:
        return 0;
    }
}

static inline unsigned yagl_get_material_param_len(GLenum pname)
{
    switch (pname) {
    case GL_AMBIENT:
    case GL_DIFFUSE:
    case GL_SPECULAR:
    case GL_EMISSION:
    case GL_AMBIENT_AND_DIFFUSE:
        return 4;
    case GL_SHININESS:
        return 1;
    default:
        return 0;
    }
}

static inline unsigned yagl_get_texenv_param_len(GLenum pname)
{
    if (pname == GL_TEXTURE_ENV_COLOR) {
        return 4;
    }

    return 1;
}

static inline unsigned yagl_get_point_param_len(GLenum pname)
{
    if (pname == GL_POINT_DISTANCE_ATTENUATION) {
        return 3;
    }

    return 1;
}

static inline unsigned yagl_get_fog_param_len(GLenum pname)
{
    if (pname == GL_FOG_COLOR) {
        return 4;
    }

    return 1;
}

static inline unsigned yagl_get_light_model_param_len(GLenum pname)
{
    if (pname == GL_LIGHT_MODEL_AMBIENT) {
        return 4;
    }

    return 1;
}

YAGL_IMPLEMENT_API_NORET2(glAlphaFunc, GLenum, GLclampf, func, ref)
YAGL_IMPLEMENT_API_NORET2(glAlphaFuncx, GLenum, GLclampx, func, ref)
YAGL_IMPLEMENT_API_NORET3(glTexEnvi, GLenum, GLenum, GLint, target, pname, param)
YAGL_IMPLEMENT_API_NORET3(glTexEnvf, GLenum, GLenum, GLfloat, target, pname, param)
YAGL_IMPLEMENT_API_NORET3(glTexEnvx, GLenum, GLenum, GLfixed, target, pname, param)
YAGL_IMPLEMENT_API_NORET1(glMatrixMode, GLenum, mode)
YAGL_IMPLEMENT_API_NORET0(glLoadIdentity)
YAGL_IMPLEMENT_API_NORET0(glPopMatrix)
YAGL_IMPLEMENT_API_NORET0(glPushMatrix)
YAGL_IMPLEMENT_API_NORET4(glRotatef, GLfloat, GLfloat, GLfloat, GLfloat, angle, x, y, z)
YAGL_IMPLEMENT_API_NORET4(glRotatex, GLfixed, GLfixed, GLfixed, GLfixed, angle, x, y, z)
YAGL_IMPLEMENT_API_NORET3(glTranslatef, GLfloat, GLfloat, GLfloat, x, y, z)
YAGL_IMPLEMENT_API_NORET3(glTranslatex, GLfixed, GLfixed, GLfixed, x, y, z)
YAGL_IMPLEMENT_API_NORET3(glScalef, GLfloat, GLfloat, GLfloat, x, y, z)
YAGL_IMPLEMENT_API_NORET3(glScalex, GLfixed, GLfixed, GLfixed, x, y, z)
YAGL_IMPLEMENT_API_NORET6(glOrthof, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, left, right, bottom, top, zNear, zFar)
YAGL_IMPLEMENT_API_NORET6(glOrthox, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, left, right, bottom, top, zNear, zFar)
YAGL_IMPLEMENT_API_NORET1(glPointSize, GLfloat, size)
YAGL_IMPLEMENT_API_NORET1(glPointSizex, GLfixed, size)
YAGL_IMPLEMENT_API_NORET1(glLineWidthx, GLfixed, width)
YAGL_IMPLEMENT_API_NORET3(glTexParameterx, GLenum, GLenum, GLfixed, target, pname, param)
YAGL_IMPLEMENT_API_NORET4(glColor4f, GLfloat, GLfloat, GLfloat, GLfloat, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET4(glColor4ub, GLubyte, GLubyte, GLubyte, GLubyte, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET4(glColor4x, GLfixed, GLfixed, GLfixed, GLfixed, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET3(glNormal3f, GLfloat, GLfloat, GLfloat, nx, ny, nz)
YAGL_IMPLEMENT_API_NORET3(glNormal3x, GLfixed, GLfixed, GLfixed, nx, ny, nz)
YAGL_IMPLEMENT_API_NORET4(glClearColorx, GLclampx, GLclampx, GLclampx, GLclampx, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET1(glClearDepthx, GLclampx, depth)
YAGL_IMPLEMENT_API_NORET5(glMultiTexCoord4f, GLenum, GLfloat, GLfloat, GLfloat, GLfloat, target, s, t, r, q)
YAGL_IMPLEMENT_API_NORET5(glMultiTexCoord4x, GLenum, GLfixed, GLfixed, GLfixed, GLfixed, target, s, t, r, q)
YAGL_IMPLEMENT_API_NORET2(glPointParameterf, GLenum, GLfloat, pname, param)
YAGL_IMPLEMENT_API_NORET2(glPointParameterx, GLenum, GLfixed, pname, param)
YAGL_IMPLEMENT_API_NORET2(glFogf, GLenum, GLfloat, pname, param)
YAGL_IMPLEMENT_API_NORET2(glFogx, GLenum, GLfixed, pname, param)
YAGL_IMPLEMENT_API_NORET6(glFrustumf, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, left, right, bottom, top, zNear, zFar)
YAGL_IMPLEMENT_API_NORET6(glFrustumx, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, left, right, bottom, top, zNear, zFar)
YAGL_IMPLEMENT_API_NORET3(glLightf, GLenum, GLenum, GLfloat, light, pname, param)
YAGL_IMPLEMENT_API_NORET3(glLightx, GLenum, GLenum, GLfixed, light, pname, param)
YAGL_IMPLEMENT_API_NORET2(glLightModelf, GLenum, GLfloat, pname, param)
YAGL_IMPLEMENT_API_NORET2(glLightModelx, GLenum, GLfixed, pname, param)
YAGL_IMPLEMENT_API_NORET3(glMaterialf, GLenum, GLenum, GLfloat, face, pname, param)
YAGL_IMPLEMENT_API_NORET3(glMaterialx, GLenum, GLenum, GLfixed, face, pname, param)
YAGL_IMPLEMENT_API_NORET1(glShadeModel, GLenum, mode)
YAGL_IMPLEMENT_API_NORET2(glSampleCoveragex, GLclampx, GLboolean, value, invert)
YAGL_IMPLEMENT_API_NORET2(glDepthRangex, GLclampx, GLclampx, zNear, zFar)
YAGL_IMPLEMENT_API_NORET1(glLogicOp, GLenum, opcode)
YAGL_IMPLEMENT_API_NORET2(glPolygonOffsetx, GLfixed, GLfixed, factor, units)

YAGL_API void glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
    unsigned count = yagl_get_texenv_param_len(pname);

    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexEnviv, GLenum, GLenum, const GLint*, target, pname, params);

    yagl_host_glTexEnviv(target, pname, params, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
    unsigned count = yagl_get_texenv_param_len(pname);

    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexEnvfv, GLenum, GLenum, const GLfloat *, target, pname, params);

    yagl_host_glTexEnvfv(target, pname, params, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexEnvxv(GLenum target, GLenum pname, const GLfixed *params)
{
    unsigned count = yagl_get_texenv_param_len(pname);

    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexEnvxv, GLenum, GLenum, const GLfixed *, target, pname, params);

    yagl_host_glTexEnvxv(target, pname, params, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClientActiveTexture(GLenum texture)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glClientActiveTexture, GLenum, texture);

    yagl_host_glClientActiveTexture(texture);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDisableClientState(GLenum array)
{
    struct yagl_gles_context *ctx;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glDisableClientState, GLenum, array);

    ctx = yagl_gles_context_get();

    if (ctx && ctx->arrays) {
        ctx->arrays[yagl_gles1_array_idx_get(ctx, array)].enabled = 0;
    }

    yagl_host_glDisableClientState(array);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glEnableClientState(GLenum array)
{
    struct yagl_gles_context *ctx;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glEnableClientState, GLenum, array);

    ctx = yagl_gles_context_get();

    if (ctx && ctx->arrays) {
        ctx->arrays[yagl_gles1_array_idx_get(ctx, array)].enabled = 1;
    }

    yagl_host_glEnableClientState(array);

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
        str = "OpenGL ES-CM 1.1";
        break;
    case GL_RENDERER:
        str = "YaGL GLESv1_CM";
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
        break;
    }

    YAGL_LOG_FUNC_EXIT(NULL);

    return (const GLubyte*)str;
}

YAGL_API void glGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
    unsigned count = yagl_get_texenv_param_len(pname);

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetTexEnviv, GLenum, GLenum, GLint *, target, pname, params);

    yagl_host_glGetTexEnviv(target, pname, params, count, NULL);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
    unsigned count = yagl_get_texenv_param_len(pname);

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetTexEnvfv, GLenum, GLenum, GLfloat *, target, pname, params);

    yagl_host_glGetTexEnvfv(target, pname, params, count, NULL);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetTexEnvxv(GLenum target, GLenum pname, GLfixed *params)
{
    unsigned count = yagl_get_texenv_param_len(pname);

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetTexEnvxv, GLenum, GLenum, GLint *, target, pname, params);

    yagl_host_glGetTexEnvxv(target, pname, params, count, NULL);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetPointerv(GLenum pname, GLvoid** pointer)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetPointerv, GLenum, GLvoid**, pname, pointer);

    yagl_host_glGetPointerv(pname, pointer);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glNormalPointer(GLenum type, GLsizei stride, const GLvoid* pointer)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glNormalPointer, GLenum, GLsizei, const GLvoid*, type, stride, pointer);

    yagl_set_array_pointer(GL_NORMAL_ARRAY, 3, type, stride, pointer);

    yagl_host_glNormalPointer(type, stride, pointer);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glVertexPointer, GLint, GLenum, GLsizei, const GLvoid*, size, type, stride, pointer);

    yagl_set_array_pointer(GL_VERTEX_ARRAY, size, type, stride, pointer);

    yagl_host_glVertexPointer(size, type, stride, pointer);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glColorPointer, GLint, GLenum, GLsizei, const GLvoid*, size, type, stride, pointer);

    yagl_set_array_pointer(GL_COLOR_ARRAY, size, type, stride, pointer);

    yagl_host_glColorPointer(size, type, stride, pointer);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glTexCoordPointer, GLint, GLenum, GLsizei, const GLvoid*, size, type, stride, pointer);

    yagl_set_array_pointer(GL_TEXTURE_COORD_ARRAY, size, type, stride, pointer);

    yagl_host_glTexCoordPointer(size, type, stride, pointer);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glPointSizePointerOES(GLenum type, GLsizei stride, const GLvoid* pointer)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glPointSizePointerOES, GLenum, GLsizei, const GLvoid*, type, stride, pointer);

    yagl_set_array_pointer(GL_POINT_SIZE_ARRAY_OES, 1, type, stride, pointer);

    yagl_host_glPointSizePointerOES(type, stride, pointer);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glTexParameterxv(GLenum target, GLenum pname, const GLfixed *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glTexParameterxv, GLenum, GLenum, const GLfixed*, target, pname, params);

    yagl_host_glTexParameterxv(target, pname, params, 1);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetTexParameterxv(GLenum target, GLenum pname, GLfixed *params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetTexParameterxv, GLenum, GLenum, GLfixed*, target, pname, params);

    yagl_host_glGetTexParameterxv(target, pname, params);

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
    YAGL_LOG_FUNC_ENTER_SPLIT1(glMultMatrixx, const GLfixed*, m);
    yagl_host_glMultMatrixx(m, 16);
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
    YAGL_LOG_FUNC_ENTER_SPLIT1(glLoadMatrixx, const GLfixed*, m);
    yagl_host_glLoadMatrixx(m, 16);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glPointParameterfv(GLenum pname, const GLfloat *params)
{
    unsigned count = yagl_get_point_param_len(pname);

    YAGL_LOG_FUNC_ENTER_SPLIT2(glPointParameterfv, GLenum, const GLfloat *, pname, params);

    yagl_host_glPointParameterfv(pname, params, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glPointParameterxv(GLenum pname, const GLfixed *params)
{
    unsigned count = yagl_get_point_param_len(pname);

    YAGL_LOG_FUNC_ENTER_SPLIT2(glPointParameterxv, GLenum, const GLfixed *, pname, params);

    yagl_host_glPointParameterxv(pname, params, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glFogfv(GLenum pname, const GLfloat *params)
{
    unsigned count = yagl_get_fog_param_len(pname);

    YAGL_LOG_FUNC_ENTER_SPLIT2(glFogfv, GLenum, const GLfloat *, pname, params);

    yagl_host_glFogfv(pname, params, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glFogxv(GLenum pname, const GLfixed *params)
{
    unsigned count = yagl_get_fog_param_len(pname);

    YAGL_LOG_FUNC_ENTER_SPLIT2(glFogxv, GLenum, const GLfixed *, pname, params);

    yagl_host_glFogxv(pname, params, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClipPlanef(GLenum plane, const GLfloat *equation)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glClipPlanef, GLenum, const GLfloat *, plane, equation);

    yagl_host_glClipPlanef(plane, equation, 4);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glClipPlanex(GLenum plane, const GLfixed *equation)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glClipPlanex, GLenum, const GLfixed *, plane, equation);

    yagl_host_glClipPlanex(plane, equation, 4);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetClipPlanef(GLenum pname, GLfloat *eqn)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetClipPlanef, GLenum, GLfloat*, pname, eqn);

    yagl_host_glGetClipPlanef(pname, eqn, 4, NULL);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetClipPlanex(GLenum pname, GLfixed *eqn)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetClipPlanex, GLenum, GLfixed*, pname, eqn);

    yagl_host_glGetClipPlanex(pname, eqn, 4, NULL);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    unsigned count;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glLightfv, GLenum, GLenum, const GLfloat *, light, pname, params);

    count = yagl_get_light_param_len(pname);

    yagl_host_glLightfv(light, pname, params, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLightxv(GLenum light, GLenum pname, const GLfixed *params)
{
    unsigned count;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glLightxv, GLenum, GLenum, const GLfixed *, light, pname, params);

    count = yagl_get_light_param_len(pname);

    yagl_host_glLightxv(light, pname, params, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLightModelfv(GLenum pname, const GLfloat *params)
{
    unsigned count = yagl_get_light_model_param_len(pname);

    YAGL_LOG_FUNC_ENTER_SPLIT2(glLightModelfv, GLenum, const GLfloat *, pname, params);

    yagl_host_glLightModelfv(pname, params, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glLightModelxv(GLenum pname, const GLfixed *params)
{
    unsigned count = yagl_get_light_model_param_len(pname);

    YAGL_LOG_FUNC_ENTER_SPLIT2(glLightModelxv, GLenum, const GLfixed *, pname, params);

    yagl_host_glLightModelxv(pname, params, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
    unsigned count;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetLightfv, GLenum, GLenum, GLfloat*, light, pname, params);

    count = yagl_get_light_param_len(pname);

    yagl_host_glGetLightfv(light, pname, params, count, NULL);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetLightxv(GLenum light, GLenum pname, GLfixed *params)
{
    unsigned count;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetLightxv, GLenum, GLenum, GLfixed*, light, pname, params);

    count = yagl_get_light_param_len(pname);

    yagl_host_glGetLightxv(light, pname, params, count, NULL);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    unsigned count;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glMaterialfv, GLenum, GLenum, const GLfloat *, face, pname, params);

    count = yagl_get_material_param_len(pname);

    yagl_host_glMaterialfv(face, pname, params, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glMaterialxv(GLenum face, GLenum pname, const GLfixed *params)
{
    unsigned count;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glMaterialxv, GLenum, GLenum, const GLfixed *, face, pname, params);

    count = yagl_get_material_param_len(pname);

    yagl_host_glMaterialxv(face, pname, params, count);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
    unsigned count;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetMaterialfv, GLenum, GLenum, GLfloat*, face, pname, params);

    count = yagl_get_material_param_len(pname);

    yagl_host_glGetMaterialfv(face, pname, params, count, NULL);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetMaterialxv(GLenum light, GLenum pname, GLfixed *params)
{
    unsigned count;

    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetMaterialxv, GLenum, GLenum, GLfixed*, light, pname, params);

    count = yagl_get_material_param_len(pname);

    yagl_host_glGetMaterialxv(light, pname, params, count, NULL);

    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetFixedv(GLenum pname, GLfixed* params)
{
    GLfixed tmp[100]; // This fits all cases.
    int32_t num = 0;

    YAGL_LOG_FUNC_ENTER_SPLIT2(glGetFixedv, GLenum, GLfixed*, pname, params);
    yagl_host_glGetFixedv(pname, tmp, sizeof(tmp)/sizeof(tmp[0]), &num);
    if (params) {
        memcpy(params, tmp, num * sizeof(tmp[0]));
    }
    YAGL_LOG_FUNC_EXIT(NULL);
}

/* GL_OES_framebuffer_object */

YAGL_API GLboolean glIsRenderbufferOES(GLuint renderbuffer)
{
    GLboolean tmp;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsRenderbufferOES, GLuint, renderbuffer);
    tmp = yagl_host_glIsRenderbuffer(renderbuffer);
    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, tmp);

    return tmp;
}

YAGL_API void glBindRenderbufferOES(GLenum target, GLuint renderbuffer)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glBindRenderbufferOES, GLenum, GLuint, target, renderbuffer);
    yagl_host_glBindRenderbuffer(target, renderbuffer);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDeleteRenderbuffersOES(GLsizei n, const GLuint* renderbuffers)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteRenderbuffersOES, GLsizei, const GLuint*, n, renderbuffers);
    yagl_host_glDeleteRenderbuffers(renderbuffers, n);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGenRenderbuffersOES(GLsizei n, GLuint* renderbuffers)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenRenderbuffersOES, GLsizei, GLuint*, n, renderbuffers);
    yagl_host_glGenRenderbuffers(renderbuffers, n, NULL);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glRenderbufferStorageOES(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glRenderbufferStorageOES, GLenum, GLenum, GLsizei, GLsizei, target, internalformat, width, height);
    yagl_host_glRenderbufferStorage(target, internalformat, width, height);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetRenderbufferParameterivOES(GLenum target, GLenum pname, GLint* params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT3(glGetRenderbufferParameterivOES, GLenum, GLenum, GLint*, target, pname, params);
    yagl_host_glGetRenderbufferParameteriv(target, pname, params);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API GLboolean glIsFramebufferOES(GLuint framebuffer)
{
    GLboolean tmp;

    YAGL_LOG_FUNC_ENTER_SPLIT1(glIsFramebufferOES, GLuint, framebuffer);
    tmp = yagl_host_glIsFramebuffer(framebuffer);
    YAGL_LOG_FUNC_EXIT_SPLIT(GLboolean, tmp);

    return tmp;
}

YAGL_API void glBindFramebufferOES(GLenum target, GLuint framebuffer)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glBindFramebufferOES, GLenum, GLuint, target, framebuffer);
    yagl_host_glBindFramebuffer(target, framebuffer);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glDeleteFramebuffersOES(GLsizei n, const GLuint* framebuffers)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glDeleteFramebuffersOES, GLsizei, const GLuint*, n, framebuffers);
    yagl_host_glDeleteFramebuffers(framebuffers, n);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGenFramebuffersOES(GLsizei n, GLuint* framebuffers)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glGenFramebuffersOES, GLsizei, GLuint*, n, framebuffers);
    yagl_host_glGenFramebuffers(framebuffers, n, NULL);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API GLenum glCheckFramebufferStatusOES(GLenum target)
{
    GLenum tmp;
    YAGL_LOG_FUNC_ENTER_SPLIT1(glCheckFramebufferStatusOES, GLenum, target);
    tmp = yagl_host_glCheckFramebufferStatus(target);
    YAGL_LOG_FUNC_EXIT_SPLIT(GLenum, tmp);
    return tmp;
}

YAGL_API void glFramebufferTexture2DOES(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    YAGL_LOG_FUNC_ENTER_SPLIT5(glFramebufferTexture2DOES, GLenum, GLenum, GLenum, GLuint, GLint, target, attachment, textarget, texture, level);
    yagl_host_glFramebufferTexture2D(target, attachment, textarget, texture, level);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glFramebufferRenderbufferOES(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glFramebufferRenderbufferOES, GLenum, GLenum, GLenum, GLuint, target, attachment, renderbuffertarget, renderbuffer);
    yagl_host_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGetFramebufferAttachmentParameterivOES(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glGetFramebufferAttachmentParameterivOES, GLenum, GLenum, GLenum, GLint*, target, attachment, pname, params);
    yagl_host_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
    YAGL_LOG_FUNC_EXIT(NULL);
}

YAGL_API void glGenerateMipmapOES(GLenum target)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glGenerateMipmapOES, GLenum, target);
    yagl_host_glGenerateMipmap(target);
    YAGL_LOG_FUNC_EXIT(NULL);
}

/* GL_OES_blend_subtract */
YAGL_API void glBlendEquationOES(GLenum mode)
{
    YAGL_LOG_FUNC_ENTER_SPLIT1(glBlendEquationOES, GLenum, mode);
    yagl_host_glBlendEquation(mode);
    YAGL_LOG_FUNC_EXIT(NULL);
}

/* GL_OES_blend_equation_separate */
YAGL_API void glBlendEquationSeparateOES(GLenum modeRGB, GLenum modeAlpha)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glBlendEquationSeparateOES, GLenum, GLenum, modeRGB, modeAlpha);
    yagl_host_glBlendEquationSeparate(modeRGB, modeAlpha);
    YAGL_LOG_FUNC_EXIT(NULL);
}

/* GL_OES_blend_func_separate */
YAGL_API void glBlendFuncSeparateOES(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    YAGL_LOG_FUNC_ENTER_SPLIT4(glBlendFuncSeparateOES, GLenum, GLenum, GLenum, GLenum, srcRGB, dstRGB, srcAlpha, dstAlpha);
    yagl_host_glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
    YAGL_LOG_FUNC_EXIT(NULL);
}
