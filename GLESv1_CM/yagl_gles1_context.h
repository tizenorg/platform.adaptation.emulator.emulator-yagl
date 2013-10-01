#ifndef _YAGL_GLES1_CONTEXT_H_
#define _YAGL_GLES1_CONTEXT_H_

#include "yagl_gles_context.h"

/*
 * GLES1 has arrays of vertices, normals, colors, texture coordinates and
 * point sizes. Every texture unit has its own texture coordinates array.
 */
typedef enum
{
    yagl_gles1_array_vertex = 0,
    yagl_gles1_array_color,
    yagl_gles1_array_normal,
    yagl_gles1_array_pointsize,
    yagl_gles1_array_texcoord,
} yagl_gles1_array_type;

struct yagl_gles1_context
{
    struct yagl_gles_context base;

    /*
     * From 'base.base.sg'.
     */
    struct yagl_sharegroup *sg;

    /* GL_OES_matrix_palette */
    int matrix_palette;

    int client_active_texture;

    int max_clip_planes;

    int max_lights;

    int max_tex_size;
};

struct yagl_client_context *yagl_gles1_context_create(struct yagl_sharegroup *sg);

#endif
