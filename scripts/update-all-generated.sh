#!/bin/bash

echo "Generating GLESv1_CM marshalling code..."
./gen-yagl-calls.py ../GLES_common/yagl_gles_calls.in,../GLESv1_CM/yagl_gles1_calls.in yagl_api_id_gles1 \
    YAGL_HOST_GLES1_CALLS GLES/gl.h yagl_host_gles1_calls.h,yagl_transport_gl1.h ../GLESv1_CM/yagl_host_gles1_calls \
    QEMU_YAGL_GLES1_CALLS yagl_gles1_api yagl_gles1_calls.h,yagl_host_gles1_calls.h,yagl_transport_gl1.h yagl_gles1_calls

echo "Generating GLESv2 marshalling code..."
./gen-yagl-calls.py ../GLES_common/yagl_gles_calls.in,../GLESv2/yagl_gles2_calls.in yagl_api_id_gles2 \
    YAGL_HOST_GLES2_CALLS GLES2/gl2.h yagl_host_gles2_calls.h,yagl_transport_gl2.h ../GLESv2/yagl_host_gles2_calls \
    QEMU_YAGL_GLES2_CALLS yagl_gles2_api yagl_gles2_calls.h,yagl_host_gles2_calls.h,yagl_transport_gl2.h yagl_gles2_calls

echo "Generating EGL marshalling code..."
./gen-yagl-calls.py ../EGL/yagl_egl_calls.in yagl_api_id_egl \
    YAGL_HOST_EGL_CALLS EGL/egl.h yagl_host_egl_calls.h,yagl_transport_egl.h ../EGL/yagl_host_egl_calls \
    QEMU_YAGL_EGL_CALLS yagl_egl_api yagl_egl_calls.h,yagl_host_egl_calls.h,yagl_transport_egl.h yagl_egl_calls
