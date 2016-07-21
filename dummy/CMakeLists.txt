include_directories(.)

add_library(EGL_dummy SHARED egl_dummy.c)
add_library(GLESv1_dummy SHARED gles1_dummy.c)
add_library(GLESv2_dummy SHARED gles2_dummy.c)

install(
    TARGETS EGL_dummy GLESv1_dummy GLESv2_dummy
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}/driver
)
