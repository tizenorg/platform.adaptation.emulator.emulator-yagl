/*
 * Generated by gen-yagl-calls.sh, do not modify!
 */
#ifndef _YAGL_HOST_EGL_CALLS_H_
#define _YAGL_HOST_EGL_CALLS_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include <EGL/egl.h>

/*
 * eglGetError wrapper. id = 1
 */
int yagl_host_eglGetError(EGLint* retval);

/*
 * eglGetDisplay wrapper. id = 2
 */
int yagl_host_eglGetDisplay(yagl_host_handle* retval, Display* display_id);

/*
 * eglInitialize wrapper. id = 3
 */
int yagl_host_eglInitialize(EGLBoolean* retval, yagl_host_handle dpy, EGLint* major, EGLint* minor);

/*
 * eglTerminate wrapper. id = 4
 */
int yagl_host_eglTerminate(EGLBoolean* retval, yagl_host_handle dpy);

/*
 * eglGetConfigs wrapper. id = 5
 */
int yagl_host_eglGetConfigs(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle* configs, EGLint config_size, EGLint* num_config);

/*
 * eglChooseConfig wrapper. id = 6
 */
int yagl_host_eglChooseConfig(EGLBoolean* retval, yagl_host_handle dpy, const EGLint* attrib_list, yagl_host_handle* configs, EGLint config_size, EGLint* num_config);

/*
 * eglGetConfigAttrib wrapper. id = 7
 */
int yagl_host_eglGetConfigAttrib(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle config, EGLint attribute, EGLint* value);

/*
 * eglDestroySurface wrapper. id = 8
 */
int yagl_host_eglDestroySurface(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle surface);

/*
 * eglQuerySurface wrapper. id = 9
 */
int yagl_host_eglQuerySurface(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle surface, EGLint attribute, EGLint* value);

/*
 * eglBindAPI wrapper. id = 10
 */
int yagl_host_eglBindAPI(EGLBoolean* retval, EGLenum api);

/*
 * eglWaitClient wrapper. id = 11
 */
int yagl_host_eglWaitClient(EGLBoolean* retval);

/*
 * eglReleaseThread wrapper. id = 12
 */
int yagl_host_eglReleaseThread(EGLBoolean* retval);

/*
 * eglCreatePbufferFromClientBuffer wrapper. id = 13
 */
int yagl_host_eglCreatePbufferFromClientBuffer(yagl_host_handle* retval, yagl_host_handle dpy, EGLenum buftype, yagl_host_handle buffer, yagl_host_handle config, const EGLint* attrib_list);

/*
 * eglSurfaceAttrib wrapper. id = 14
 */
int yagl_host_eglSurfaceAttrib(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle surface, EGLint attribute, EGLint value);

/*
 * eglBindTexImage wrapper. id = 15
 */
int yagl_host_eglBindTexImage(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle surface, EGLint buffer);

/*
 * eglReleaseTexImage wrapper. id = 16
 */
int yagl_host_eglReleaseTexImage(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle surface, EGLint buffer);

/*
 * eglCreateContext wrapper. id = 17
 */
int yagl_host_eglCreateContext(yagl_host_handle* retval, yagl_host_handle dpy, yagl_host_handle config, yagl_host_handle share_context, const EGLint* attrib_list);

/*
 * eglDestroyContext wrapper. id = 18
 */
int yagl_host_eglDestroyContext(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle ctx);

/*
 * eglMakeCurrent wrapper. id = 19
 */
int yagl_host_eglMakeCurrent(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle draw, yagl_host_handle read, yagl_host_handle ctx);

/*
 * eglQueryContext wrapper. id = 20
 */
int yagl_host_eglQueryContext(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle ctx, EGLint attribute, EGLint* value);

/*
 * eglSwapBuffers wrapper. id = 21
 */
int yagl_host_eglSwapBuffers(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle surface);

/*
 * eglCopyBuffers wrapper. id = 22
 */
int yagl_host_eglCopyBuffers(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle surface);

/*
 * eglCreateImageKHR wrapper. id = 23
 */
int yagl_host_eglCreateImageKHR(yagl_host_handle* retval, yagl_host_handle dpy, yagl_host_handle ctx, EGLenum target, yagl_winsys_id buffer, const EGLint* attrib_list);

/*
 * eglDestroyImageKHR wrapper. id = 24
 */
int yagl_host_eglDestroyImageKHR(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle image);

/*
 * eglCreateWindowSurfaceOffscreenYAGL wrapper. id = 25
 */
int yagl_host_eglCreateWindowSurfaceOffscreenYAGL(yagl_host_handle* retval, yagl_host_handle dpy, yagl_host_handle config, uint32_t width, uint32_t height, uint32_t bpp, void* pixels, const EGLint* attrib_list);

/*
 * eglCreatePbufferSurfaceOffscreenYAGL wrapper. id = 26
 */
int yagl_host_eglCreatePbufferSurfaceOffscreenYAGL(yagl_host_handle* retval, yagl_host_handle dpy, yagl_host_handle config, uint32_t width, uint32_t height, uint32_t bpp, void* pixels, const EGLint* attrib_list);

/*
 * eglCreatePixmapSurfaceOffscreenYAGL wrapper. id = 27
 */
int yagl_host_eglCreatePixmapSurfaceOffscreenYAGL(yagl_host_handle* retval, yagl_host_handle dpy, yagl_host_handle config, uint32_t width, uint32_t height, uint32_t bpp, void* pixels, const EGLint* attrib_list);

/*
 * eglResizeOffscreenSurfaceYAGL wrapper. id = 28
 */
int yagl_host_eglResizeOffscreenSurfaceYAGL(EGLBoolean* retval, yagl_host_handle dpy, yagl_host_handle surface, uint32_t width, uint32_t height, uint32_t bpp, void* pixels);

/*
 * eglUpdateOffscreenImageYAGL wrapper. id = 29
 */
int yagl_host_eglUpdateOffscreenImageYAGL(yagl_host_handle dpy, yagl_host_handle image, uint32_t width, uint32_t height, uint32_t bpp, const void* pixels);

/*
 * eglCreateWindowSurfaceOnscreenYAGL wrapper. id = 30
 */
int yagl_host_eglCreateWindowSurfaceOnscreenYAGL(yagl_host_handle* retval, yagl_host_handle dpy, yagl_host_handle config, yagl_winsys_id win, const EGLint* attrib_list);

/*
 * eglCreatePbufferSurfaceOnscreenYAGL wrapper. id = 31
 */
int yagl_host_eglCreatePbufferSurfaceOnscreenYAGL(yagl_host_handle* retval, yagl_host_handle dpy, yagl_host_handle config, yagl_winsys_id buffer, const EGLint* attrib_list);

/*
 * eglCreatePixmapSurfaceOnscreenYAGL wrapper. id = 32
 */
int yagl_host_eglCreatePixmapSurfaceOnscreenYAGL(yagl_host_handle* retval, yagl_host_handle dpy, yagl_host_handle config, yagl_winsys_id pixmap, const EGLint* attrib_list);

/*
 * eglInvalidateOnscreenSurfaceYAGL wrapper. id = 33
 */
int yagl_host_eglInvalidateOnscreenSurfaceYAGL(yagl_host_handle dpy, yagl_host_handle surface, yagl_winsys_id buffer);

#endif
