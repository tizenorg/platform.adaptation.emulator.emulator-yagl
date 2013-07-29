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
    yagl_api_id_gles1 = 2,
    yagl_api_id_gles2 = 3,
} yagl_api_id;

typedef uint32_t yagl_host_handle;
typedef uint32_t yagl_winsys_id;

#define yagl_offsetof(type, member) ((size_t)&((type*)0)->member)

#define yagl_containerof(ptr, type, member) ((type*)((char*)(ptr) - yagl_offsetof(type, member)))

#endif
