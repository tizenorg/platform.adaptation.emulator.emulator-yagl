#!/bin/bash

echo "Generating GLES marshalling code..."
./gen-yagl-calls.py ../GLES_common/yagl_gles_calls.in yagl_api_id_gles \
    YAGL_HOST_GLES_CALLS yagl_types.h GL/gl.h,yagl_host_gles_calls.h,yagl_transport_gl.h ../GLES_common/yagl_host_gles_calls \
    QEMU_YAGL_GLES_CALLS yagl_gles_api yagl_gles_calls.h,yagl_host_gles_calls.h,yagl_transport_gl.h yagl_gles_calls

echo "Generating EGL marshalling code..."
./gen-yagl-calls.py ../EGL/yagl_egl_calls.in yagl_api_id_egl \
    YAGL_HOST_EGL_CALLS yagl_types.h,EGL/egl.h yagl_host_egl_calls.h,yagl_transport_egl.h ../EGL/yagl_host_egl_calls \
    QEMU_YAGL_EGL_CALLS yagl_egl_api yagl_egl_calls.h,yagl_host_egl_calls.h,yagl_transport_egl.h yagl_egl_calls
