#include "yagl_state.h"
#include "yagl_host_gles1_calls.h"
#include "yagl_impl.h"
#include "yagl_mem.h"
#include "yagl_gles_context.h"
#include "yagl_malloc.h"
#include <GLES/glext.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yagl_gles_utils.h"

/* GLES1 has arrays of vertices, normals, colors, texture coordinates and
 * point sizes. Every texture unit has its own texture coordinates array */
typedef enum {
    YAGL_GLES1_ARRAY_VERTEX = 0,
    YAGL_GLES1_ARRAY_COLOR,
    YAGL_GLES1_ARRAY_NORMAL,
    YAGL_GLES1_ARRAY_POINTSIZE,
    YAGL_GLES1_ARRAY_TEX_COORD,
} YaglGles1ArrayType;

static inline void yagl_query_array(struct yagl_gles_array *arr,
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
        /* The rest are texture arrays */
        name = GL_TEXTURE_COORD_ARRAY;
        buff_bind = GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING;
        stride = GL_TEXTURE_COORD_ARRAY_STRIDE;
        pointer = GL_TEXTURE_COORD_ARRAY_POINTER;
        break;
    }

    arr->enabled = yagl_get_integer(name);
    arr->vbo = yagl_get_integer(buff_bind);
    arr->stride = yagl_get_integer(stride);

    if (!arr->vbo) {
        do {
            yagl_mem_probe_write_ptr(&arr->ptr);
        } while (!yagl_host_glGetPointerv(pointer, &arr->ptr));
    }
}

static inline void yagl_set_client_active_texture(GLenum tex)
{
    yagl_host_glClientActiveTexture(tex);

    if (!yagl_batch_sync()) {
        fprintf(stderr, "Critical error! Couldn't set active texture!\n");
        exit(1);
    }
}

static void yagl_query_texture_arrays(struct yagl_gles_array *arrays,
                                             int num_texture_units)
{
    GLint cur_text;
    int i;

    cur_text = yagl_get_integer(GL_CLIENT_ACTIVE_TEXTURE);

    for (i = 0; i < num_texture_units; ++i) {
        yagl_set_client_active_texture(GL_TEXTURE0 + i);
        yagl_query_array(&arrays[i], YAGL_GLES1_ARRAY_TEX_COORD);
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
        yagl_query_array(&ctx->arrays[i], i);
    }

    yagl_query_texture_arrays(&ctx->arrays[YAGL_GLES1_ARRAY_TEX_COORD],
                              num_texture_units);
}

YAGL_IMPLEMENT_API_NORET2(glAlphaFunc, GLenum, GLclampf, func, ref)
YAGL_IMPLEMENT_API_NORET2(glClipPlanef, GLenum, const GLfloat*, plane, equation)
YAGL_IMPLEMENT_API_NORET4(glColor4f, GLfloat, GLfloat, GLfloat, GLfloat, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET2(glFogf, GLenum, GLfloat, pname, param)
YAGL_IMPLEMENT_API_NORET2(glFogfv, GLenum, const GLfloat*, pname, params)
YAGL_IMPLEMENT_API_NORET6(glFrustumf, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, left, right, bottom, top, zNear, zFar)
YAGL_IMPLEMENT_API_NORET2(glGetClipPlanef, GLenum, GLfloat*, pname, eqn)
YAGL_IMPLEMENT_API_NORET3(glGetLightfv, GLenum, GLenum, GLfloat*, light, pname, params)
YAGL_IMPLEMENT_API_NORET3(glGetMaterialfv, GLenum, GLenum, GLfloat*, face, pname, params)
YAGL_IMPLEMENT_API_NORET3(glGetTexEnvfv, GLenum, GLenum, GLfloat*, env, pname, params)
YAGL_IMPLEMENT_API_NORET2(glLightModelf, GLenum, GLfloat, pname, param)
YAGL_IMPLEMENT_API_NORET2(glLightModelfv, GLenum, const GLfloat*, pname, params)
YAGL_IMPLEMENT_API_NORET3(glLightf, GLenum, GLenum, GLfloat, light, pname, param)
YAGL_IMPLEMENT_API_NORET3(glLightfv, GLenum, GLenum, const GLfloat*, light, pname, params)
YAGL_IMPLEMENT_API_NORET1(glLoadMatrixf, const GLfloat*, m)
YAGL_IMPLEMENT_API_NORET3(glMaterialf, GLenum, GLenum, GLfloat, face, pname, param)
YAGL_IMPLEMENT_API_NORET3(glMaterialfv, GLenum, GLenum, const GLfloat*, face, pname, params)
YAGL_IMPLEMENT_API_NORET1(glMultMatrixf, const GLfloat*, m)
YAGL_IMPLEMENT_API_NORET5(glMultiTexCoord4f, GLenum, GLfloat, GLfloat, GLfloat, GLfloat, target, s, t, r, q)
YAGL_IMPLEMENT_API_NORET3(glNormal3f, GLfloat, GLfloat, GLfloat, nx, ny, nz)
YAGL_IMPLEMENT_API_NORET6(glOrthof, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, left, right, bottom, top, zNear, zFar)
YAGL_IMPLEMENT_API_NORET2(glPointParameterf, GLenum, GLfloat, pname, param)
YAGL_IMPLEMENT_API_NORET2(glPointParameterfv, GLenum, const GLfloat*, pname, params)
YAGL_IMPLEMENT_API_NORET1(glPointSize, GLfloat, size)
YAGL_IMPLEMENT_API_NORET3(glPointSizePointerOES, GLenum, GLsizei, const GLvoid*, type, stride, pointer)
YAGL_IMPLEMENT_API_NORET4(glRotatef, GLfloat, GLfloat, GLfloat, GLfloat, angle, x, y, z)
YAGL_IMPLEMENT_API_NORET3(glScalef, GLfloat, GLfloat, GLfloat, x, y, z)
YAGL_IMPLEMENT_API_NORET3(glTexEnvf, GLenum, GLenum, GLfloat, target, pname, param)
YAGL_IMPLEMENT_API_NORET3(glTexEnvfv, GLenum, GLenum, const GLfloat*, target, pname, params)
YAGL_IMPLEMENT_API_NORET3(glTranslatef, GLfloat, GLfloat, GLfloat, x, y, z)
YAGL_IMPLEMENT_API_NORET2(glAlphaFuncx, GLenum, GLclampx, func, ref)
YAGL_IMPLEMENT_API_NORET4(glClearColorx, GLclampx, GLclampx, GLclampx, GLclampx, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET1(glClearDepthx, GLclampx, depth)
YAGL_IMPLEMENT_API_NORET1(glClientActiveTexture, GLenum, texture)
YAGL_IMPLEMENT_API_NORET2(glClipPlanex, GLenum, const GLfixed*, plane, equation)
YAGL_IMPLEMENT_API_NORET4(glColor4ub, GLubyte, GLubyte, GLubyte, GLubyte, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET4(glColor4x, GLfixed, GLfixed, GLfixed, GLfixed, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET4(glColorPointer, GLint, GLenum, GLsizei, const GLvoid*, size, type, stride, pointer)
YAGL_IMPLEMENT_API_NORET2(glDepthRangex, GLclampx, GLclampx, zNear, zFar)
YAGL_IMPLEMENT_API_NORET1(glDisableClientState, GLenum, array)
YAGL_IMPLEMENT_API_NORET1(glEnableClientState, GLenum, array)
YAGL_IMPLEMENT_API_NORET2(glFogx, GLenum, GLfixed, pname, param)
YAGL_IMPLEMENT_API_NORET2(glFogxv, GLenum, const GLfixed*, pname, params)
YAGL_IMPLEMENT_API_NORET6(glFrustumx, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, left, right, bottom, top, zNear, zFar)
YAGL_IMPLEMENT_API_NORET2(glGetClipPlanex, GLenum, GLfixed*, pname, eqn)
YAGL_IMPLEMENT_API_NORET2(glGetFixedv, GLenum, GLfixed*, pname, params)
YAGL_IMPLEMENT_API_NORET3(glGetLightxv, GLenum, GLenum, GLfixed*, light, pname, params)
YAGL_IMPLEMENT_API_NORET3(glGetMaterialxv, GLenum, GLenum, GLfixed*, face, pname, params)
YAGL_IMPLEMENT_API_NORET2(glGetPointerv, GLenum, GLvoid**, pname, params)

YAGL_API const GLubyte* glGetString(GLenum name)
{
    const char *str = NULL;

    YAGL_LOG_FUNC_ENTER(glGetString, "name = %d", name);

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
        str = "";
        break;
    default:
        str = "";
    }

    YAGL_LOG_FUNC_EXIT(NULL);

    return (const GLubyte*)str;
}

YAGL_IMPLEMENT_API_NORET3(glGetTexEnviv, GLenum, GLenum, GLint*, env, pname, params)
YAGL_IMPLEMENT_API_NORET3(glGetTexEnvxv, GLenum, GLenum, GLfixed*, env, pname, params)
YAGL_IMPLEMENT_API_NORET3(glGetTexParameterxv, GLenum, GLenum, GLfixed*, target, pname, params)
YAGL_IMPLEMENT_API_NORET2(glLightModelx, GLenum, GLfixed, pname, param)
YAGL_IMPLEMENT_API_NORET2(glLightModelxv, GLenum, const GLfixed*, pname, params)
YAGL_IMPLEMENT_API_NORET3(glLightx, GLenum, GLenum, GLfixed, light, pname, param)
YAGL_IMPLEMENT_API_NORET3(glLightxv, GLenum, GLenum, const GLfixed*, light, pname, params)
YAGL_IMPLEMENT_API_NORET1(glLineWidthx, GLfixed, width)
YAGL_IMPLEMENT_API_NORET0(glLoadIdentity)
YAGL_IMPLEMENT_API_NORET1(glLoadMatrixx, const GLfixed*, m)
YAGL_IMPLEMENT_API_NORET1(glLogicOp, GLenum, opcode)
YAGL_IMPLEMENT_API_NORET3(glMaterialx, GLenum, GLenum, GLfixed, face, pname, param)
YAGL_IMPLEMENT_API_NORET3(glMaterialxv, GLenum, GLenum, const GLfixed*, face, pname, params)
YAGL_IMPLEMENT_API_NORET1(glMatrixMode, GLenum, mode)
YAGL_IMPLEMENT_API_NORET1(glMultMatrixx, const GLfixed*, m)
YAGL_IMPLEMENT_API_NORET5(glMultiTexCoord4x, GLenum, GLfixed, GLfixed, GLfixed, GLfixed, target, s, t, r, q)
YAGL_IMPLEMENT_API_NORET3(glNormal3x, GLfixed, GLfixed, GLfixed, nx, ny, nz)
YAGL_IMPLEMENT_API_NORET3(glNormalPointer, GLenum, GLsizei, const GLvoid*, type, stride, pointer)
YAGL_IMPLEMENT_API_NORET6(glOrthox, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, left, right, bottom, top, zNear, zFar)
YAGL_IMPLEMENT_API_NORET2(glPointParameterx, GLenum, GLfixed, pname, param)
YAGL_IMPLEMENT_API_NORET2(glPointParameterxv, GLenum, const GLfixed*, pname, params)
YAGL_IMPLEMENT_API_NORET1(glPointSizex, GLfixed, size)
YAGL_IMPLEMENT_API_NORET2(glPolygonOffsetx, GLfixed, GLfixed, factor, units)
YAGL_IMPLEMENT_API_NORET0(glPopMatrix)
YAGL_IMPLEMENT_API_NORET0(glPushMatrix)
YAGL_IMPLEMENT_API_NORET4(glRotatex, GLfixed, GLfixed, GLfixed, GLfixed, angle, x, y, z)
YAGL_IMPLEMENT_API_NORET2(glSampleCoveragex, GLclampx, GLboolean, value, invert)
YAGL_IMPLEMENT_API_NORET3(glScalex, GLfixed, GLfixed, GLfixed, x, y, z)
YAGL_IMPLEMENT_API_NORET1(glShadeModel, GLenum, mode)
YAGL_IMPLEMENT_API_NORET4(glTexCoordPointer, GLint, GLenum, GLsizei, const GLvoid*, size, type, stride, pointer)
YAGL_IMPLEMENT_API_NORET3(glTexEnvi, GLenum, GLenum, GLint, target, pname, param)
YAGL_IMPLEMENT_API_NORET3(glTexEnvx, GLenum, GLenum, GLfixed, target, pname, param)
YAGL_IMPLEMENT_API_NORET3(glTexEnviv, GLenum, GLenum, const GLint*, target, pname, params)
YAGL_IMPLEMENT_API_NORET3(glTexEnvxv, GLenum, GLenum, const GLfixed*, target, pname, params)
YAGL_IMPLEMENT_API_NORET3(glTexParameterx, GLenum, GLenum, GLfixed, target, pname, param)
YAGL_IMPLEMENT_API_NORET3(glTexParameterxv, GLenum, GLenum, const GLfixed*, target, pname, params)
YAGL_IMPLEMENT_API_NORET3(glTranslatex, GLfixed, GLfixed, GLfixed, x, y, z)
YAGL_IMPLEMENT_API_NORET4(glVertexPointer, GLint, GLenum, GLsizei, const GLvoid*, size, type, stride, pointer)
