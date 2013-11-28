/*
 * Generated by gen-yagl-calls.py, do not modify!
 */
#ifndef _YAGL_HOST_EGL_CALLS_H_
#define _YAGL_HOST_EGL_CALLS_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "EGL/egl.h"

/*
 * eglGetDisplay wrapper. id = 1
 */
yagl_host_handle yagl_host_eglGetDisplay(uint32_t display_id, EGLint *error);

/*
 * eglInitialize wrapper. id = 2
 */
EGLBoolean yagl_host_eglInitialize(yagl_host_handle dpy, EGLint *major, EGLint *minor, EGLint *error);

/*
 * eglTerminate wrapper. id = 3
 */
EGLBoolean yagl_host_eglTerminate(yagl_host_handle dpy, EGLint *error);

/*
 * eglGetConfigs wrapper. id = 4
 */
EGLBoolean yagl_host_eglGetConfigs(yagl_host_handle dpy, yagl_host_handle *configs, int32_t configs_maxcount, int32_t *configs_count, EGLint *error);

/*
 * eglChooseConfig wrapper. id = 5
 */
EGLBoolean yagl_host_eglChooseConfig(yagl_host_handle dpy, const EGLint *attrib_list, int32_t attrib_list_count, yagl_host_handle *configs, int32_t configs_maxcount, int32_t *configs_count, EGLint *error);

/*
 * eglGetConfigAttrib wrapper. id = 6
 */
EGLBoolean yagl_host_eglGetConfigAttrib(yagl_host_handle dpy, yagl_host_handle config, EGLint attribute, EGLint *value, EGLint *error);

/*
 * eglDestroySurface wrapper. id = 7
 */
EGLBoolean yagl_host_eglDestroySurface(yagl_host_handle dpy, yagl_host_handle surface, EGLint *error);

/*
 * eglQuerySurface wrapper. id = 8
 */
EGLBoolean yagl_host_eglQuerySurface(yagl_host_handle dpy, yagl_host_handle surface, EGLint attribute, EGLint *value, EGLint *error);

/*
 * eglBindAPI wrapper. id = 9
 */
void yagl_host_eglBindAPI(EGLenum api);

/*
 * eglWaitClient wrapper. id = 10
 */
void yagl_host_eglWaitClient();

/*
 * eglReleaseThread wrapper. id = 11
 */
EGLBoolean yagl_host_eglReleaseThread(EGLint *error);

/*
 * eglSurfaceAttrib wrapper. id = 12
 */
EGLBoolean yagl_host_eglSurfaceAttrib(yagl_host_handle dpy, yagl_host_handle surface, EGLint attribute, EGLint value, EGLint *error);

/*
 * eglCreateContext wrapper. id = 13
 */
yagl_host_handle yagl_host_eglCreateContext(yagl_host_handle dpy, yagl_host_handle config, yagl_host_handle share_context, const EGLint *attrib_list, int32_t attrib_list_count, EGLint *error);

/*
 * eglDestroyContext wrapper. id = 14
 */
EGLBoolean yagl_host_eglDestroyContext(yagl_host_handle dpy, yagl_host_handle ctx, EGLint *error);

/*
 * eglMakeCurrent wrapper. id = 15
 */
void yagl_host_eglMakeCurrent(yagl_host_handle dpy, yagl_host_handle draw, yagl_host_handle read, yagl_host_handle ctx);

/*
 * eglQueryContext wrapper. id = 16
 */
EGLBoolean yagl_host_eglQueryContext(yagl_host_handle dpy, yagl_host_handle ctx, EGLint attribute, EGLint *value, EGLint *error);

/*
 * eglSwapBuffers wrapper. id = 17
 */
void yagl_host_eglSwapBuffers(yagl_host_handle dpy, yagl_host_handle surface);

/*
 * eglCopyBuffers wrapper. id = 18
 */
void yagl_host_eglCopyBuffers(yagl_host_handle dpy, yagl_host_handle surface);

/*
 * eglCreateWindowSurfaceOffscreenYAGL wrapper. id = 19
 */
yagl_host_handle yagl_host_eglCreateWindowSurfaceOffscreenYAGL(yagl_host_handle dpy, yagl_host_handle config, uint32_t width, uint32_t height, uint32_t bpp, void *pixels, const EGLint *attrib_list, int32_t attrib_list_count, EGLint *error);

/*
 * eglCreatePbufferSurfaceOffscreenYAGL wrapper. id = 20
 */
yagl_host_handle yagl_host_eglCreatePbufferSurfaceOffscreenYAGL(yagl_host_handle dpy, yagl_host_handle config, uint32_t width, uint32_t height, uint32_t bpp, void *pixels, const EGLint *attrib_list, int32_t attrib_list_count, EGLint *error);

/*
 * eglCreatePixmapSurfaceOffscreenYAGL wrapper. id = 21
 */
yagl_host_handle yagl_host_eglCreatePixmapSurfaceOffscreenYAGL(yagl_host_handle dpy, yagl_host_handle config, uint32_t width, uint32_t height, uint32_t bpp, void *pixels, const EGLint *attrib_list, int32_t attrib_list_count, EGLint *error);

/*
 * eglResizeOffscreenSurfaceYAGL wrapper. id = 22
 */
EGLBoolean yagl_host_eglResizeOffscreenSurfaceYAGL(yagl_host_handle dpy, yagl_host_handle surface, uint32_t width, uint32_t height, uint32_t bpp, void *pixels, EGLint *error);

/*
 * eglCreateWindowSurfaceOnscreenYAGL wrapper. id = 23
 */
yagl_host_handle yagl_host_eglCreateWindowSurfaceOnscreenYAGL(yagl_host_handle dpy, yagl_host_handle config, yagl_winsys_id win, const EGLint *attrib_list, int32_t attrib_list_count, EGLint *error);

/*
 * eglCreatePbufferSurfaceOnscreenYAGL wrapper. id = 24
 */
yagl_host_handle yagl_host_eglCreatePbufferSurfaceOnscreenYAGL(yagl_host_handle dpy, yagl_host_handle config, yagl_winsys_id buffer, const EGLint *attrib_list, int32_t attrib_list_count, EGLint *error);

/*
 * eglCreatePixmapSurfaceOnscreenYAGL wrapper. id = 25
 */
yagl_host_handle yagl_host_eglCreatePixmapSurfaceOnscreenYAGL(yagl_host_handle dpy, yagl_host_handle config, yagl_winsys_id pixmap, const EGLint *attrib_list, int32_t attrib_list_count, EGLint *error);

/*
 * eglInvalidateOnscreenSurfaceYAGL wrapper. id = 26
 */
void yagl_host_eglInvalidateOnscreenSurfaceYAGL(yagl_host_handle dpy, yagl_host_handle surface, yagl_winsys_id buffer);

/*
 * eglCreateImageYAGL wrapper. id = 27
 */
EGLBoolean yagl_host_eglCreateImageYAGL(uint32_t texture, yagl_host_handle dpy, yagl_winsys_id buffer, EGLint *error);

#endif
