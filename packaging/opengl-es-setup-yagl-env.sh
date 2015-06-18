 #!/bin/sh

echo -e "[${_G} Opengl-es acceleration module setting. ${C_}]"
if [ -e /dev/yagl ] ; then
    echo -e "[${_G} Emulator support gles hw acceleration. ${C_}]"
    echo -e "[${_G} Change permission of /dev/yagl. ${C_}]"
    chown root:video /dev/dri/card0
    chown root:video /dev/yagl
    chmod 660 /dev/dri/card0
    chmod 660 /dev/yagl
    chsmack -a "*" /dev/dri/card0
    chsmack -a "*" /dev/yagl
    echo -e "[${_G} Apply to use hw gles library. ${C_}]"
    ln -s -f /usr/lib/yagl/libEGL.so.1.0 /usr/lib/libEGL.so
    ln -s -f /usr/lib/yagl/libEGL.so.1.0 /usr/lib/libEGL.so.1
    ln -s -f /usr/lib/yagl/libGLESv1_CM.so.1.0 /usr/lib/libGLESv1_CM.so
    ln -s -f /usr/lib/yagl/libGLESv1_CM.so.1.0 /usr/lib/libGLESv1_CM.so.1
    ln -s -f /usr/lib/yagl/libGLESv2.so.1.0 /usr/lib/libGLESv2.so
    ln -s -f /usr/lib/yagl/libGLESv2.so.1.0 /usr/lib/libGLESv2.so.1
elif grep "yagl=1" /proc/cmdline ; then
    echo -e "[${_G} Emulator support gles hw acceleration. ${C_}]"
    echo -e "[${_G} Change permission of /dev/yagl. ${C_}]"
    chown root:video /dev/dri/card0
    chown root:video /dev/yagl
    chmod 660 /dev/dri/card0
    chmod 660 /dev/yagl
    chsmack -a "*" /dev/dri/card0
    chsmack -a "*" /dev/yagl
    echo -e "[${_G} Apply to use hw gles library. ${C_}]"
    ln -s -f /usr/lib/yagl/libEGL.so.1.0 /usr/lib/libEGL.so
    ln -s -f /usr/lib/yagl/libEGL.so.1.0 /usr/lib/libEGL.so.1
    ln -s -f /usr/lib/yagl/libGLESv1_CM.so.1.0 /usr/lib/libGLESv1_CM.so
    ln -s -f /usr/lib/yagl/libGLESv1_CM.so.1.0 /usr/lib/libGLESv1_CM.so.1
    ln -s -f /usr/lib/yagl/libGLESv2.so.1.0 /usr/lib/libGLESv2.so
    ln -s -f /usr/lib/yagl/libGLESv2.so.1.0 /usr/lib/libGLESv2.so.1
else
    echo -e "[${_G} Emulator does not support gles hw acceleration. ${C_}]"
    echo -e "[${_G} Apply to use gles stub library. ${C_}]"
    ln -s -f /usr/lib/dummy-gl/libEGL_dummy.so /usr/lib/libEGL.so
    ln -s -f /usr/lib/dummy-gl/libEGL_dummy.so /usr/lib/libEGL.so.1
    ln -s -f /usr/lib/dummy-gl/libGLESv1_dummy.so /usr/lib/libGLESv1_CM.so
    ln -s -f /usr/lib/dummy-gl/libGLESv1_dummy.so /usr/lib/libGLESv1_CM.so.1
    ln -s -f /usr/lib/dummy-gl/libGLESv2_dummy.so /usr/lib/libGLESv2.so
    ln -s -f /usr/lib/dummy-gl/libGLESv2_dummy.so /usr/lib/libGLESv2.so.1
fi
