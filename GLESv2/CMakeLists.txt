set(SOURCES
    yagl_glsl_lexer.l
    yagl_glsl_parser.y
    yagl_glsl_state.c
    yagl_gles2_context.c
    yagl_gles2_interface.c
    yagl_gles2_program.c
    yagl_gles2_shader.c
    yagl_gles2_validate.c
    yagl_gles2_calls.c
    yagl_gles2_utils.c
    yagl_texcompress.c
    yagl_texcompress_etc2.c
    yagl_gles3_context.c
    yagl_gles3_calls.c
    yagl_gles3_buffer_binding.c
    yagl_gles3_program.c
    yagl_gles3_validate.c
    yagl_gles3_transform_feedback.c
    yagl_gles3_query.c
    yagl_gles3_sync.c
    yagl_gles3_pixel_formats.c
)

include_directories(${YAGL_SOURCE_DIR}/GLES_common)
include_directories(.)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

flex_target(GLESv2 ${CMAKE_CURRENT_SOURCE_DIR}/yagl_glsl_lexer.l ${CMAKE_CURRENT_BINARY_DIR}/yagl_glsl_lexer.c)
bison_target(GLESv2 ${CMAKE_CURRENT_SOURCE_DIR}/yagl_glsl_parser.y ${CMAKE_CURRENT_BINARY_DIR}/yagl_glsl_parser.c)

add_library(GLESv2 SHARED ${SOURCES} ${FLEX_GLESv2_OUTPUTS} ${BISON_GLESv2_OUTPUTS})
set_target_properties(GLESv2 PROPERTIES VERSION 2.0 SOVERSION 2)

target_link_libraries(GLESv2 "-Wl,-whole-archive" GLES_common "-Wl,-no-whole-archive")

install(
    TARGETS GLESv2
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}/driver
)
