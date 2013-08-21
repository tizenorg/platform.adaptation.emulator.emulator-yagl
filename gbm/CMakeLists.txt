set(SOURCES
    yagl_gbm.c
)

set(LIBRARIES
    ${CMAKE_THREAD_LIBS_INIT}
    ${LIBDRM_LIBRARIES}
    ${LIBDRM_VIGS_LIBRARIES}
)

if (PLATFORM_WAYLAND)
    set(LIBRARIES ${LIBRARIES}
        wayland-drm
    )
    add_definitions(-DYAGL_PLATFORM_WAYLAND)
endif ()

add_library(gbm-yagl SHARED ${SOURCES})
set_target_properties(gbm-yagl PROPERTIES VERSION 1.0.0 SOVERSION 1)
set_target_properties(gbm-yagl PROPERTIES OUTPUT_NAME gbm)

target_link_libraries(gbm-yagl ${LIBRARIES})

install(
    TARGETS gbm-yagl
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
)
