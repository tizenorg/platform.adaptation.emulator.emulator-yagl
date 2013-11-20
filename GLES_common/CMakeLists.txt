set(SOURCES
    yagl_gles_vertex_array.c
    yagl_gles_array.c
    yagl_gles_buffer.c
    yagl_gles_texture.c
    yagl_gles_texture_unit.c
    yagl_gles_framebuffer.c
    yagl_gles_renderbuffer.c
    yagl_gles_image.c
    yagl_gles_context.c
    yagl_gles_validate.c
    yagl_gles_calls.c
    yagl_gles_utils.c
    yagl_host_gles_calls.c
)

include_directories(.)

add_library(GLES_common STATIC ${SOURCES})

target_link_libraries(GLES_common EGL)
