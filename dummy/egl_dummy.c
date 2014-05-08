/*
* Emulator YaGL dummy library for EGL
*
* Copyright (c) 2013 - 2014 Samsung Electronics Co., Ltd. All rights reserved.
*
* Contact : Jinhyung Jo <jinhyung.jo@samsung.conm>
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*/

#include "EGL/egl.h"

/* EGL Functions */
EGLAPI EGLint EGLAPIENTRY eglGetError(void) { return 0; }
EGLAPI EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id) { return EGL_NO_DISPLAY; }
EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy) { return EGL_FALSE; }
EGLAPI const char * EGLAPIENTRY eglQueryString(EGLDisplay dpy, EGLint name) { return 0; }
EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value) { return EGL_FALSE; }
EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list) { return EGL_NO_SURFACE; }
EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) { return EGL_NO_SURFACE; }
EGLAPI EGLSurface EGLAPIENTRY eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list) { return EGL_NO_SURFACE; }
EGLAPI EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay dpy, EGLSurface surface) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglBindAPI(EGLenum api) { return EGL_FALSE; }
EGLAPI EGLenum EGLAPIENTRY eglQueryAPI(void) { return 0; }
EGLAPI EGLBoolean EGLAPIENTRY eglWaitClient(void) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglReleaseThread(void) { return EGL_FALSE; }
EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list) { return EGL_NO_SURFACE; }
EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval) { return EGL_FALSE; }
EGLAPI EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) { return EGL_NO_CONTEXT; }
EGLAPI EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext ctx) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) { return EGL_FALSE; }
EGLAPI EGLContext EGLAPIENTRY eglGetCurrentContext(void) { return EGL_NO_CONTEXT; }
EGLAPI EGLSurface EGLAPIENTRY eglGetCurrentSurface(EGLint readdraw) { return EGL_NO_SURFACE; }
EGLAPI EGLDisplay EGLAPIENTRY eglGetCurrentDisplay(void) { return EGL_NO_DISPLAY; }
EGLAPI EGLBoolean EGLAPIENTRY eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglWaitGL(void) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglWaitNative(EGLint engine) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) { return EGL_FALSE; }
EGLAPI EGLBoolean EGLAPIENTRY eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target) { return EGL_FALSE; }
EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY eglGetProcAddress(const char *procname) { return ((__eglMustCastToProperFunctionPointerType)0); }
