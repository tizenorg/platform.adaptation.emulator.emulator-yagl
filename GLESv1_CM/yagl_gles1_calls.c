#include "yagl_host_gles1_calls.h"
#include "yagl_impl.h"
#include "yagl_mem.h"
#include <GLES/glext.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

YAGL_IMPLEMENT_API_NORET2(glAlphaFunc, GLenum, GLclampf, func, ref)
YAGL_IMPLEMENT_API_NORET4(glClearColor, GLclampf, GLclampf, GLclampf, GLclampf, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET1(glClearDepthf, GLclampf, depth)
YAGL_IMPLEMENT_API_NORET2(glClipPlanef, GLenum, const GLfloat*, plane, equation)
YAGL_IMPLEMENT_API_NORET4(glColor4f, GLfloat, GLfloat, GLfloat, GLfloat, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET2(glDepthRangef, GLclampf, GLclampf, zNear, zFar)
YAGL_IMPLEMENT_API_NORET2(glFogf, GLenum, GLfloat, pname, param)
YAGL_IMPLEMENT_API_NORET2(glFogfv, GLenum, const GLfloat*, pname, params)
YAGL_IMPLEMENT_API_NORET6(glFrustumf, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, left, right, bottom, top, zNear, zFar)
YAGL_IMPLEMENT_API_NORET2(glGetClipPlanef, GLenum, GLfloat*, pname, eqn)
YAGL_IMPLEMENT_API_NORET2(glGetFloatv, GLenum, GLfloat*, pname, params)
YAGL_IMPLEMENT_API_NORET3(glGetLightfv, GLenum, GLenum, GLfloat*, light, pname, params)
YAGL_IMPLEMENT_API_NORET3(glGetMaterialfv, GLenum, GLenum, GLfloat*, face, pname, params)
YAGL_IMPLEMENT_API_NORET3(glGetTexEnvfv, GLenum, GLenum, GLfloat*, env, pname, params)
YAGL_IMPLEMENT_API_NORET3(glGetTexParameterfv, GLenum, GLenum, GLfloat*, target, pname, params)
YAGL_IMPLEMENT_API_NORET2(glLightModelf, GLenum, GLfloat, pname, param)
YAGL_IMPLEMENT_API_NORET2(glLightModelfv, GLenum, const GLfloat*, pname, params)
YAGL_IMPLEMENT_API_NORET3(glLightf, GLenum, GLenum, GLfloat, light, pname, param)
YAGL_IMPLEMENT_API_NORET3(glLightfv, GLenum, GLenum, const GLfloat*, light, pname, params)
YAGL_IMPLEMENT_API_NORET1(glLineWidth, GLfloat, width)
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
YAGL_IMPLEMENT_API_NORET2(glPolygonOffset, GLfloat, GLfloat, factor, units)
YAGL_IMPLEMENT_API_NORET4(glRotatef, GLfloat, GLfloat, GLfloat, GLfloat, angle, x, y, z)
YAGL_IMPLEMENT_API_NORET3(glScalef, GLfloat, GLfloat, GLfloat, x, y, z)
YAGL_IMPLEMENT_API_NORET3(glTexEnvf, GLenum, GLenum, GLfloat, target, pname, param)
YAGL_IMPLEMENT_API_NORET3(glTexEnvfv, GLenum, GLenum, const GLfloat*, target, pname, params)
YAGL_IMPLEMENT_API_NORET3(glTexParameterf, GLenum, GLenum, GLfloat, target, pname, param)
YAGL_IMPLEMENT_API_NORET3(glTexParameterfv, GLenum, GLenum, const GLfloat*, target, pname, params)
YAGL_IMPLEMENT_API_NORET3(glTranslatef, GLfloat, GLfloat, GLfloat, x, y, z)
YAGL_IMPLEMENT_API_NORET1(glActiveTexture, GLenum, texture)
YAGL_IMPLEMENT_API_NORET2(glAlphaFuncx, GLenum, GLclampx, func, ref)
YAGL_IMPLEMENT_API_NORET2(glBindBuffer, GLenum, GLuint, target, buffer)
YAGL_IMPLEMENT_API_NORET2(glBindTexture, GLenum, GLuint, target, texture)
YAGL_IMPLEMENT_API_NORET2(glBlendFunc, GLenum, GLenum, sfactor, dfactor)
YAGL_IMPLEMENT_API_NORET4(glBufferData, GLenum, GLsizeiptr, const GLvoid*, GLenum, target, size, data, usage)
YAGL_IMPLEMENT_API_NORET4(glBufferSubData, GLenum, GLintptr, GLsizeiptr, const GLvoid*, target, offset, size, data)
YAGL_IMPLEMENT_API_NORET1(glClear, GLbitfield, mask)
YAGL_IMPLEMENT_API_NORET4(glClearColorx, GLclampx, GLclampx, GLclampx, GLclampx, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET1(glClearDepthx, GLclampx, depth)
YAGL_IMPLEMENT_API_NORET1(glClearStencil, GLint, s)
YAGL_IMPLEMENT_API_NORET1(glClientActiveTexture, GLenum, texture)
YAGL_IMPLEMENT_API_NORET2(glClipPlanex, GLenum, const GLfixed*, plane, equation)
YAGL_IMPLEMENT_API_NORET4(glColor4ub, GLubyte, GLubyte, GLubyte, GLubyte, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET4(glColor4x, GLfixed, GLfixed, GLfixed, GLfixed, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET4(glColorMask, GLboolean, GLboolean, GLboolean, GLboolean, red, green, blue, alpha)
YAGL_IMPLEMENT_API_NORET4(glColorPointer, GLint, GLenum, GLsizei, const GLvoid*, size, type, stride, pointer)
YAGL_IMPLEMENT_API_NORET8(glCompressedTexImage2D, GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*, target, level, internalformat, width, height, border, imageSize, data)
YAGL_IMPLEMENT_API_NORET9(glCompressedTexSubImage2D, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid*, target, level, xoffset, yoffset, width, height, format, imageSize, data)
YAGL_IMPLEMENT_API_NORET8(glCopyTexImage2D, GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, target, level, internalformat, x, y, width, height, border)
YAGL_IMPLEMENT_API_NORET8(glCopyTexSubImage2D, GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, target, level, xoffset, yoffset, x, y, width, height)
YAGL_IMPLEMENT_API_NORET1(glCullFace, GLenum, mode)
YAGL_IMPLEMENT_API_NORET2(glDeleteBuffers, GLsizei, const GLuint*, n, buffers)
YAGL_IMPLEMENT_API_NORET2(glDeleteTextures, GLsizei, const GLuint*, n, textures)
YAGL_IMPLEMENT_API_NORET1(glDepthFunc, GLenum, func)
YAGL_IMPLEMENT_API_NORET1(glDepthMask, GLboolean, flag)
YAGL_IMPLEMENT_API_NORET2(glDepthRangex, GLclampx, GLclampx, zNear, zFar)
YAGL_IMPLEMENT_API_NORET1(glDisable, GLenum, cap)
YAGL_IMPLEMENT_API_NORET1(glDisableClientState, GLenum, array)
YAGL_IMPLEMENT_API_NORET3(glDrawArrays, GLenum, GLint, GLsizei, mode, first, count)
YAGL_IMPLEMENT_API_NORET4(glDrawElements, GLenum, GLsizei, GLenum, const GLvoid*, mode, count, type, indices)
YAGL_IMPLEMENT_API_NORET1(glEnable, GLenum, cap)
YAGL_IMPLEMENT_API_NORET1(glEnableClientState, GLenum, array)
YAGL_IMPLEMENT_API_NORET0(glFinish)
YAGL_IMPLEMENT_API_NORET0(glFlush)
YAGL_IMPLEMENT_API_NORET2(glFogx, GLenum, GLfixed, pname, param)
YAGL_IMPLEMENT_API_NORET2(glFogxv, GLenum, const GLfixed*, pname, params)
YAGL_IMPLEMENT_API_NORET1(glFrontFace, GLenum, mode)
YAGL_IMPLEMENT_API_NORET6(glFrustumx, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, GLfixed, left, right, bottom, top, zNear, zFar)
YAGL_IMPLEMENT_API_NORET2(glGetBooleanv, GLenum, GLboolean*, pname, params)
YAGL_IMPLEMENT_API_NORET3(glGetBufferParameteriv, GLenum, GLenum, GLint*, target, pname, params)
YAGL_IMPLEMENT_API_NORET2(glGetClipPlanex, GLenum, GLfixed*, pname, eqn)
YAGL_IMPLEMENT_API_NORET2(glGenBuffers, GLsizei, GLuint*, n, buffers)
YAGL_IMPLEMENT_API_NORET2(glGenTextures, GLsizei, GLuint*, n, textures)
YAGL_IMPLEMENT_API_RET0(GLenum, glGetError)
YAGL_IMPLEMENT_API_NORET2(glGetFixedv, GLenum, GLfixed*, pname, params)
YAGL_IMPLEMENT_API_NORET2(glGetIntegerv, GLenum, GLint*, pname, params)
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
        str = "1.0";
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
YAGL_IMPLEMENT_API_NORET3(glGetTexParameteriv, GLenum, GLenum, GLint*, target, pname, params)
YAGL_IMPLEMENT_API_NORET3(glGetTexParameterxv, GLenum, GLenum, GLfixed*, target, pname, params)
YAGL_IMPLEMENT_API_NORET2(glHint, GLenum, GLenum, target, mode)
YAGL_IMPLEMENT_API_RET1(GLboolean, glIsBuffer, GLuint, buffer)
YAGL_IMPLEMENT_API_RET1(GLboolean, glIsEnabled, GLenum, cap)
YAGL_IMPLEMENT_API_RET1(GLboolean, glIsTexture, GLuint, texture)
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
YAGL_IMPLEMENT_API_NORET2(glPixelStorei, GLenum, GLint, pname, param)
YAGL_IMPLEMENT_API_NORET2(glPointParameterx, GLenum, GLfixed, pname, param)
YAGL_IMPLEMENT_API_NORET2(glPointParameterxv, GLenum, const GLfixed*, pname, params)
YAGL_IMPLEMENT_API_NORET1(glPointSizex, GLfixed, size)
YAGL_IMPLEMENT_API_NORET2(glPolygonOffsetx, GLfixed, GLfixed, factor, units)
YAGL_IMPLEMENT_API_NORET0(glPopMatrix)
YAGL_IMPLEMENT_API_NORET0(glPushMatrix)
YAGL_IMPLEMENT_API_NORET7(glReadPixels, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*, x, y, width, height, format, type, pixels)
YAGL_IMPLEMENT_API_NORET4(glRotatex, GLfixed, GLfixed, GLfixed, GLfixed, angle, x, y, z)
YAGL_IMPLEMENT_API_NORET2(glSampleCoverage, GLclampf, GLboolean, value, invert)
YAGL_IMPLEMENT_API_NORET2(glSampleCoveragex, GLclampx, GLboolean, value, invert)
YAGL_IMPLEMENT_API_NORET3(glScalex, GLfixed, GLfixed, GLfixed, x, y, z)
YAGL_IMPLEMENT_API_NORET4(glScissor, GLint, GLint, GLsizei, GLsizei, x, y, width, height)
YAGL_IMPLEMENT_API_NORET1(glShadeModel, GLenum, mode)
YAGL_IMPLEMENT_API_NORET3(glStencilFunc, GLenum, GLint, GLuint, func, ref, mask)
YAGL_IMPLEMENT_API_NORET1(glStencilMask, GLuint, mask)
YAGL_IMPLEMENT_API_NORET3(glStencilOp, GLenum, GLenum, GLenum, fail, zfail, zpass)
YAGL_IMPLEMENT_API_NORET4(glTexCoordPointer, GLint, GLenum, GLsizei, const GLvoid*, size, type, stride, pointer)
YAGL_IMPLEMENT_API_NORET3(glTexEnvi, GLenum, GLenum, GLint, target, pname, param)
YAGL_IMPLEMENT_API_NORET3(glTexEnvx, GLenum, GLenum, GLfixed, target, pname, param)
YAGL_IMPLEMENT_API_NORET3(glTexEnviv, GLenum, GLenum, const GLint*, target, pname, params)
YAGL_IMPLEMENT_API_NORET3(glTexEnvxv, GLenum, GLenum, const GLfixed*, target, pname, params)
YAGL_IMPLEMENT_API_NORET9(glTexImage2D, GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*, target, level, internalformat, width, height, border, format, type, pixels)
YAGL_IMPLEMENT_API_NORET3(glTexParameteri, GLenum, GLenum, GLint, target, pname, param)
YAGL_IMPLEMENT_API_NORET3(glTexParameterx, GLenum, GLenum, GLfixed, target, pname, param)
YAGL_IMPLEMENT_API_NORET3(glTexParameteriv, GLenum, GLenum, const GLint*, target, pname, params)
YAGL_IMPLEMENT_API_NORET3(glTexParameterxv, GLenum, GLenum, const GLfixed*, target, pname, params)
YAGL_IMPLEMENT_API_NORET9(glTexSubImage2D, GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*, target, level, xoffset, yoffset, width, height, format, type, pixels)
YAGL_IMPLEMENT_API_NORET3(glTranslatex, GLfixed, GLfixed, GLfixed, x, y, z)
YAGL_IMPLEMENT_API_NORET4(glVertexPointer, GLint, GLenum, GLsizei, const GLvoid*, size, type, stride, pointer)
YAGL_IMPLEMENT_API_NORET4(glViewport, GLint, GLint, GLsizei, GLsizei, x, y, width, height)

YAGL_API void glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image)
{
    YAGL_LOG_FUNC_ENTER_SPLIT2(glEGLImageTargetTexture2DOES, GLenum, GLeglImageOES, target, image);
    YAGL_LOG_FUNC_EXIT(NULL);
}
