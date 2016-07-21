#!/bin/sh

echo -e "[${_G} OpenGL ES acceleration module setting. ${C_}]"
if [ -e /dev/yagl ] ; then
    echo -e "[${_G} Emulator supports gles hw acceleration. ${C_}]"
    ln -s -f libEGL.so.1.0 /usr/lib/driver/libEGL.so.1
    ln -s -f libGLESv1_CM.so.1.0 /usr/lib/driver/libGLESv1_CM.so.1
    ln -s -f libGLESv2.so.2.0 /usr/lib/driver/libGLESv2.so.2
else
    echo -e "[${_G} Emulator does not support gles hw acceleration. ${C_}]"
    ln -s -f libEGL_dummy.so /usr/lib/driver/libEGL.so.1
    ln -s -f libGLESv1_dummy.so /usr/lib/driver/libGLESv1_CM.so.1
    ln -s -f libGLESv2_dummy.so /usr/lib/driver/libGLESv2.so.2
fi
