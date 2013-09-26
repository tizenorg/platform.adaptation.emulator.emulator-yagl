set(SOURCES
    yagl_gles2_calls.c
    yagl_host_gles2_calls.c
    ${YAGL_SOURCE_DIR}/GLES_common/yagl_gles_calls.c
    ${YAGL_SOURCE_DIR}/GLES_common/yagl_gles_utils.c
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(.)

set(HOST_GLES_CALLS_HEADER yagl_host_gles2_calls.h)
set(GLEXT_HEADER GLES2/gl2ext.h)
configure_file(${YAGL_SOURCE_DIR}/GLES_common/yagl_host_gles_calls.h.in ${CMAKE_CURRENT_BINARY_DIR}/yagl_host_gles_calls.h @ONLY)

add_library(GLESv2 SHARED ${SOURCES})
set_target_properties(GLESv2 PROPERTIES VERSION 2.0 SOVERSION 2)

target_link_libraries(GLESv2 EGL)

install(
    TARGETS GLESv2
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
)
