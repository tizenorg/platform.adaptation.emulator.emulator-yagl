include (WPCodegenTarget)

set(SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-drm-server-protocol.h
    ${YAGL_OUT_DIR}/include/wayland-drm-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-drm-protocol.c
    wayland-drm.c
)

wp_codegen_target(wayland-drm
                  ${CMAKE_CURRENT_SOURCE_DIR}/wayland-drm.xml
                  ${CMAKE_CURRENT_BINARY_DIR}
                  ${YAGL_OUT_DIR}/include
                  ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(.)

add_library(wayland-drm STATIC ${SOURCES})

target_link_libraries(wayland-drm ${CMAKE_THREAD_LIBS_INIT} ${WAYLAND_SERVER_LIBRARIES})
