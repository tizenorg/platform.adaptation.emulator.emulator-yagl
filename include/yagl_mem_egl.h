#ifndef _YAGL_MEM_EGL_H_
#define _YAGL_MEM_EGL_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_mem.h"
#include "EGL/egl.h"

#define yagl_mem_probe_read_EGLint(value) yagl_mem_probe_read_int32(value)
#define yagl_mem_probe_write_EGLint(value) yagl_mem_probe_write_int32(value)

static __inline void yagl_mem_probe_read_attrib_list(const EGLint *value)
{
    if (value) {
        int i = 0;
        volatile EGLint tmp = value[i];
        while (tmp != EGL_NONE) {
            i += 2;
            tmp = value[i];
        }
    }
}

#endif
