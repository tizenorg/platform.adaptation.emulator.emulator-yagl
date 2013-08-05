function(wp_codegen_target _TARGET _INPUT _OUT_DIR)
    file(MAKE_DIRECTORY ${_OUT_DIR})
    add_custom_command(OUTPUT ${_OUT_DIR}/${_TARGET}-server-protocol.h
                       COMMAND ${CMAKE_FIND_ROOT_PATH}${WAYLAND_CLIENT_PREFIX}/bin/wayland-scanner server-header < ${_INPUT} > ${_OUT_DIR}/${_TARGET}-server-protocol.h
                       DEPENDS ${_INPUT}
                       VERBATIM)
    add_custom_command(OUTPUT ${_OUT_DIR}/${_TARGET}-client-protocol.h
                       COMMAND ${CMAKE_FIND_ROOT_PATH}${WAYLAND_CLIENT_PREFIX}/bin/wayland-scanner client-header < ${_INPUT} > ${_OUT_DIR}/${_TARGET}-client-protocol.h
                       DEPENDS ${_INPUT}
                       VERBATIM)
    add_custom_command(OUTPUT ${_OUT_DIR}/${_TARGET}-protocol.c
                       COMMAND ${CMAKE_FIND_ROOT_PATH}${WAYLAND_CLIENT_PREFIX}/bin/wayland-scanner code < ${_INPUT} > ${_OUT_DIR}/${_TARGET}-protocol.c
                       DEPENDS ${_INPUT}
                       VERBATIM)
endfunction ()
