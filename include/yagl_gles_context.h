#ifndef _YAGL_GLES_CONTEXT_H_
#define _YAGL_GLES_CONTEXT_H_

#include "yagl_export.h"
#include "yagl_types.h"

struct yagl_gles_array
{
    int enabled;

    GLuint vbo;

    GLsizei stride;

    GLvoid *ptr;
};

struct yagl_gles_context
{
    GLsizei pack_alignment;

    GLsizei unpack_alignment;

    GLuint ebo;
    int ebo_valid;

    GLuint vbo;
    int vbo_valid;

    struct yagl_gles_array *arrays;
    GLuint num_arrays;

    GLchar *extensions;
};

YAGL_API struct yagl_gles_context *yagl_gles_context_get();

#endif
