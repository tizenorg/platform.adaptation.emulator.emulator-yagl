set(SOURCES
    yagl_context.c
    yagl_display.c
    yagl_egl_calls.c
    yagl_egl_state.c
    yagl_gles_context.c
    yagl_gles_image.c
    yagl_host_egl_calls.c
    yagl_image.c
    yagl_log.c
    yagl_malloc.c
    yagl_offscreen.c
    yagl_offscreen_image.c
    yagl_offscreen_surface.c
    yagl_onscreen.c
    yagl_onscreen_image.c
    yagl_onscreen_surface.c
    yagl_onscreen_utils.c
    yagl_ref.c
    yagl_render.c
    yagl_resource.c
    yagl_state.c
    yagl_surface.c
    yagl_utils.c
    yagl_native_platform.c
    yagl_native_display.c
    yagl_native_drawable.c
    yagl_native_image.c
)

set(LIBRARIES
    ${CMAKE_THREAD_LIBS_INIT}
    ${LIBDRM_LIBRARIES}
    ${LIBDRM_VIGS_LIBRARIES}
    dl
)

if (PLATFORM_X11)
    set(SOURCES ${SOURCES}
        x11/yagl_dri2.c
        x11/yagl_x11_platform.c
        x11/yagl_x11_display.c
        x11/yagl_x11_drawable.c
        x11/yagl_x11_image.c
    )
    set(LIBRARIES ${LIBRARIES}
        ${X11_LIBRARIES}
        ${XEXT_LIBRARIES}
        ${XFIXES_LIBRARIES}
    )
    add_definitions(-DYAGL_PLATFORM_X11)
endif ()

if (PLATFORM_GBM)
    set(SOURCES ${SOURCES}
        gbm/yagl_gbm_platform.c
        gbm/yagl_gbm_display.c
        gbm/yagl_gbm_window.c
        gbm/yagl_gbm_pixmap.c
    )
    set(LIBRARIES ${LIBRARIES}
        gbm-yagl
    )
    add_definitions(-DYAGL_PLATFORM_GBM)
endif ()

if (PLATFORM_WAYLAND)
    add_definitions(-DYAGL_PLATFORM_WAYLAND)
endif ()

include_directories(.)

add_library(EGL SHARED ${SOURCES})
set_target_properties(EGL PROPERTIES VERSION 1.0 SOVERSION 1)

target_link_libraries(EGL ${LIBRARIES})

install(
    TARGETS EGL
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
)
