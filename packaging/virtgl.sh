 #!/bin/sh
 
echo -e "[${_G} Opengl-es acceleration module setting. ${C_}]"
if grep "yagl=1" /proc/cmdline ; then
        echo -e "[${_G} Emulator support gles hw acceleration. ${C_}]"
        echo -e "[${_G} Change permission of /dev/yagl. ${C_}]"
        chmod 666 /dev/yagl
        echo -e "[${_G} Apply to use hw gles library. ${C_}]"
               ln -s -f /usr/lib/yagl/libEGL.so.1.0 /usr/lib/libEGL.so
               ln -s -f /usr/lib/yagl/libEGL.so.1.0 /usr/lib/libEGL.so.1
               ln -s -f /usr/lib/yagl/libGLESv1_CM.so.1.0 /usr/lib/libGLESv1_CM.so
               ln -s -f /usr/lib/yagl/libGLESv1_CM.so.1.0 /usr/lib/libGLESv1_CM.so.1
               ln -s -f /usr/lib/yagl/libGLESv2.so.1.0 /usr/lib/libGLESv2.so
               ln -s -f /usr/lib/yagl/libGLESv2.so.1.0 /usr/lib/libGLESv2.so.1
               rm -f /usr/lib/st_GL.so
               rm -f /usr/lib/egl_gallium.so
               rm -f /usr/lib/libglapi.so*
               systemctl set-environment ELM_ENGINE=gl
elif grep "gles=1" /proc/cmdline ; then
        echo -e "[${_G} Emulator support gles hw acceleration. ${C_}]"
        echo -e "[${_G} Change permission of /dev/glmem. ${C_}]"
        chmod 666 /dev/glmem
        echo -e "[${_G} Apply to use hw gles library. ${C_}]"
               ln -s -f /usr/lib/host-gl/libEGL.so.1.0 /usr/lib/libEGL.so
               ln -s -f /usr/lib/host-gl/libEGL.so.1.0 /usr/lib/libEGL.so.1
               ln -s -f /usr/lib/host-gl/libGLESv1_CM.so.1.0 /usr/lib/libGLESv1_CM.so
               ln -s -f /usr/lib/host-gl/libGLESv1_CM.so.1.0 /usr/lib/libGLESv1_CM.so.1
               ln -s -f /usr/lib/host-gl/libGLESv2.so.1.0 /usr/lib/libGLESv2.so
               ln -s -f /usr/lib/host-gl/libGLESv2.so.1.0 /usr/lib/libGLESv2.so.1
               rm -f /usr/lib/st_GL.so
               rm -f /usr/lib/egl_gallium.so
               rm -f /usr/lib/libglapi.so*
else
        echo -e "[${_G} Emulator does not support gles hw acceleration. ${C_}]"
               echo -e "[${_G} Apply to use sw mesa gles library. ${C_}]"
               ln -s -f /usr/lib/mesa-gl/libEGL.so.1.0 /usr/lib/libEGL.so
               ln -s -f /usr/lib/mesa-gl/libEGL.so.1.0 /usr/lib/libEGL.so.1
               ln -s -f /usr/lib/mesa-gl/libGLESv1_CM.so.1.1.0 /usr/lib/libGLESv1_CM.so
               ln -s -f /usr/lib/mesa-gl/libGLESv1_CM.so.1.1.0 /usr/lib/libGLESv1_CM.so.1
               ln -s -f /usr/lib/mesa-gl/libGLESv2.so.2.0.0 /usr/lib/libGLESv2.so
               ln -s -f /usr/lib/mesa-gl/libGLESv2.so.2.0.0 /usr/lib/libGLESv2.so.1
               ln -s -f /usr/lib/mesa-gl/st_GL.so /usr/lib/st_GL.so
               ln -s -f /usr/lib/mesa-gl/egl_gallium.so /usr/lib/egl_gallium.so
               ln -s -f /usr/lib/mesa-gl/libglapi.so.0.0.0 /usr/lib/libglapi.so
               ln -s -f /usr/lib/mesa-gl/libglapi.so.0.0.0 /usr/lib/libglapi.so.0
               ln -s -f /usr/lib/mesa-gl/libglapi.so.0.0.0 /usr/lib/libglapi.so.0.0.0
fi
