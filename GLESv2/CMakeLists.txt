set(SOURCES
    yagl_gles2_context.c
    yagl_gles2_interface.c
    yagl_gles2_program.c
    yagl_gles2_shader.c
    yagl_gles2_validate.c
    yagl_gles2_calls.c
    yagl_gles2_utils.c
    yagl_gles3_context.c
    yagl_gles3_calls.c
    yagl_gles3_buffer_binding.c
    yagl_gles3_program.c
    yagl_gles3_validate.c
    yagl_gles3_transform_feedback.c
    yagl_gles3_query.c
    dummy.c
)

include_directories(${YAGL_SOURCE_DIR}/GLES_common)
include_directories(.)
link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

add_library(GLESv2 SHARED ${SOURCES})
set_target_properties(GLESv2 PROPERTIES VERSION 2.0 SOVERSION 2)

target_link_libraries(GLESv2 "-Wl,-whole-archive" GLES_common "-Wl,-no-whole-archive")

install(
    TARGETS GLESv2
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
)
