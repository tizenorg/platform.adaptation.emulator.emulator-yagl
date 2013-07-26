cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0014 NEW)

option(CROSSCOMPILING "Enable cross-compiling")

if (CROSSCOMPILING)
    set(CMAKE_SYSTEM_NAME "Linux")
    if (NOT CMAKE_FIND_ROOT_PATH_MODE_PROGRAM)
        set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    endif ()
    if (NOT CMAKE_FIND_ROOT_PATH_MODE_LIBRARY)
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    endif ()
    if (NOT CMAKE_FIND_ROOT_PATH_MODE_INCLUDE)
        set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    endif ()
endif ()

project(YAGL C)

# USER SETTINGS

if (CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

option(PLATFORM_X11 "Enable X11 platform" TRUE)
option(PLATFORM_GBM "Enable GBM platform" TRUE)
option(PLATFORM_WAYLAND "Enable wayland platform" TRUE)

set(INSTALL_LIB_DIR "lib" CACHE STRING "Where to install library files")

# END USER SETTINGS

if ((NOT PLATFORM_X11) AND (NOT PLATFORM_GBM) AND (NOT PLATFORM_WAYLAND))
    message(FATAL_ERROR "At least one platform must be supported")
endif ()

#out dir

set(YAGL_OUT_DIR ${YAGL_BINARY_DIR}/out)

#set includes/libs

set(YAGL_INCLUDE_DIR "${YAGL_SOURCE_DIR}/include")
include_directories(${YAGL_INCLUDE_DIR})
include_directories(${CMAKE_FIND_ROOT_PATH}/usr/include)

#find packages

find_package(Threads)
find_package(PkgConfig)

pkg_check_modules(LIBDRM REQUIRED libdrm)
add_definitions(${LIBDRM_CFLAGS})
include_directories(${LIBDRM_INCLUDE_DIRS})
link_directories(${LIBDRM_LIBRARY_DIRS})

pkg_check_modules(LIBDRM_VIGS REQUIRED libdrm_vigs)
add_definitions(${LIBDRM_VIGS_CFLAGS})
include_directories(${LIBDRM_VIGS_INCLUDE_DIRS})
link_directories(${LIBDRM_VIGS_LIBRARY_DIRS})

if (PLATFORM_X11)
    pkg_check_modules(X11 REQUIRED x11)
    pkg_check_modules(XEXT REQUIRED xext)
    pkg_check_modules(XFIXES REQUIRED xfixes)

    add_definitions(${X11_CFLAGS})
    include_directories(${X11_INCLUDE_DIRS})
    link_directories(${X11_LIBRARY_DIRS})

    add_definitions(${XEXT_CFLAGS})
    include_directories(${XEXT_INCLUDE_DIRS})
    link_directories(${XEXT_LIBRARY_DIRS})

    add_definitions(${XFIXES_CFLAGS})
    include_directories(${XFIXES_INCLUDE_DIRS})
    link_directories(${XFIXES_LIBRARY_DIRS})
endif ()

#fix output directories

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${YAGL_OUT_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${YAGL_OUT_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${YAGL_OUT_DIR}/lib)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -D_REENTRANT -fPIC -DPIC")
foreach (CONFIGURATION RELEASE MINSIZEREL RELWITHDEBINFO)
    string(REPLACE "-O3" "-O2" CMAKE_C_FLAGS_${CONFIGURATION} "${CMAKE_C_FLAGS_${CONFIGURATION}}")
endforeach ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined -Wl,--exclude-libs,ALL")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined -Wl,--exclude-libs,ALL")

message(STATUS "Build type:       " ${CMAKE_BUILD_TYPE})
if (PLATFORM_X11)
message(STATUS "X11 platform:     yes")
else ()
message(STATUS "X11 platform:     no")
endif ()

if (PLATFORM_GBM)
message(STATUS "GBM platform:     yes")
else ()
message(STATUS "GBM platform:     no")
endif ()

if (PLATFORM_WAYLAND)
message(STATUS "Wayland platform: yes")
else ()
message(STATUS "Wayland platform: no")
endif ()

message(STATUS "Install prefix:   " ${CMAKE_INSTALL_PREFIX})
message(STATUS "Install lib dir:  " ${INSTALL_LIB_DIR})

add_subdirectory(EGL)
add_subdirectory(GLESv1_CM)
add_subdirectory(GLESv2)
