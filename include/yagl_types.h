#ifndef _YAGL_TYPES_H_
#define _YAGL_TYPES_H_

#include <stdint.h>
#include <stddef.h>

#if defined(__i386) || defined(_M_IX86)
#define YAGL_LITTLE_ENDIAN
#elif defined(__x86_64) || defined(_M_X64) || defined(_M_IA64)
#define YAGL_LITTLE_ENDIAN
#elif defined(__arm__)
#define YAGL_LITTLE_ENDIAN
#else
#error Unknown architecture
#endif

#if defined(__x86_64) || defined(_M_X64) || defined(_M_IA64) || defined(__LP64__)
#define YAGL_64
#else
#define YAGL_32
#endif

#if !defined(YAGL_64) && !defined(YAGL_32)
#error 32 or 64 bit mode must be set
#endif

typedef enum
{
    yagl_render_type_offscreen = 1,
    yagl_render_type_onscreen = 2,
} yagl_render_type;

typedef enum
{
    yagl_api_id_egl = 1,
    yagl_api_id_gles = 2,
} yagl_api_id;

typedef enum
{
    yagl_client_api_ogl = (1 << 0),
    yagl_client_api_gles1 = (1 << 1),
    yagl_client_api_gles2 = (1 << 2),
    yagl_client_api_gles3 = (1 << 3),
    yagl_client_api_ovg = (1 << 4)
} yagl_client_api;

typedef enum
{
    /* OpenGL 2.1 or OpenGL >= 3.1 compatibility. */
    yagl_gl_2 = 0,
    /* OpenGL >= 3.1 core. */
    yagl_gl_3_1 = 1,
    /* OpenGL >= 3.1 core, GL_ARB_ES3_compatibility support. */
    yagl_gl_3_1_es3 = 2,
    /* OpenGL >= 3.2 core, no GL_ARB_ES3_compatibility support. */
    yagl_gl_3_2 = 3
} yagl_gl_version;

typedef uint32_t yagl_host_handle;
typedef uint32_t yagl_winsys_id;
typedef uint32_t yagl_object_name;

#define yagl_offsetof(type, member) ((size_t)&((type*)0)->member)

#define yagl_containerof(ptr, type, member) ((type*)((char*)(ptr) - yagl_offsetof(type, member)))

#endif
