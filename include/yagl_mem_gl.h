#ifndef _YAGL_MEM_GL_H_
#define _YAGL_MEM_GL_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_mem.h"
#include <string.h>

#define yagl_mem_probe_read_GLsizei(value) yagl_mem_probe_read_int32(value)
#define yagl_mem_probe_write_GLsizei(value) yagl_mem_probe_write_int32(value)
#define yagl_mem_probe_read_GLint(value) yagl_mem_probe_read_int32(value)
#define yagl_mem_probe_write_GLint(value) yagl_mem_probe_write_int32(value)
#define yagl_mem_probe_read_GLenum(value) yagl_mem_probe_read_uint32(value)
#define yagl_mem_probe_write_GLenum(value) yagl_mem_probe_write_uint32(value)
#define yagl_mem_probe_read_GLboolean(value) yagl_mem_probe_read_uint8(value)
#define yagl_mem_probe_write_GLboolean(value) yagl_mem_probe_write_uint8(value)
#define yagl_mem_probe_read_GLfloat(value) yagl_mem_probe_read_float(value)
#define yagl_mem_probe_write_GLfloat(value) yagl_mem_probe_write_float(value)

static __inline void yagl_mem_probe_read_GLchars(const GLchar *value)
{
    if (value) {
        volatile size_t tmp = strlen(value);
    }
}

#endif
