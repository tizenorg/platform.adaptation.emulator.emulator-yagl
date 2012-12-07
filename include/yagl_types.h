#ifndef _YAGL_TYPES_H_
#define _YAGL_TYPES_H_

#include "yagl_platform.h"
#include <stdint.h>
#include <stddef.h>

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

#define yagl_offsetof(type, member) ((size_t)&((type*)0)->member)

#define yagl_containerof(ptr, type, member) ((type*)((char*)(ptr) - yagl_offsetof(type, member)))

#endif
