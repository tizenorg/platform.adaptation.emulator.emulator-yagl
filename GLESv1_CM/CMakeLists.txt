set(SOURCES
    yagl_gles1_context.c
    yagl_gles1_interface.c
    yagl_gles1_validate.c
    yagl_gles1_calls.c
)

include_directories(${YAGL_SOURCE_DIR}/GLES_common)
include_directories(.)
link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

add_library(GLESv1_CM SHARED ${SOURCES})
set_target_properties(GLESv1_CM PROPERTIES VERSION 1.0 SOVERSION 1)

target_link_libraries(GLESv1_CM "-Wl,-whole-archive" GLES_common "-Wl,-no-whole-archive" EGL)

install(
    TARGETS GLESv1_CM
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
)
