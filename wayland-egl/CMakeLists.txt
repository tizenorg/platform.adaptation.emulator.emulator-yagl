set(SOURCES
    yagl_wayland_egl.c
)

add_library(wayland-egl-yagl SHARED ${SOURCES})
set_target_properties(wayland-egl-yagl PROPERTIES VERSION 1.0.0 SOVERSION 1)
set_target_properties(wayland-egl-yagl PROPERTIES OUTPUT_NAME wayland-egl)

target_link_libraries(wayland-egl-yagl ${CMAKE_THREAD_LIBS_INIT})

install(
    TARGETS wayland-egl-yagl
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
)
