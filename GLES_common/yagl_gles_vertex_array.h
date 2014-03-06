#ifndef _YAGL_GLES_VERTEX_ARRAY_H_
#define _YAGL_GLES_VERTEX_ARRAY_H_

#include "yagl_gles_types.h"
#include "yagl_object.h"

struct yagl_gles_array;
struct yagl_gles_buffer;

struct yagl_gles_vertex_array
{
    struct yagl_object base;

    yagl_object_name global_name;

    /*
     * GLES arrays, the number of arrays is different depending on
     * GLES version, 'num_arrays' holds that number.
     */
    struct yagl_gles_array *arrays;
    int num_arrays;

    struct yagl_gles_buffer *ebo;

    int was_bound;
};

struct yagl_gles_vertex_array
    *yagl_gles_vertex_array_create(int fake,
                                   struct yagl_gles_array *arrays,
                                   int num_arrays);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_vertex_array_acquire(struct yagl_gles_vertex_array *va);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_vertex_array_release(struct yagl_gles_vertex_array *va);

void yagl_gles_vertex_array_bind(struct yagl_gles_vertex_array *va);

int yagl_gles_vertex_array_was_bound(struct yagl_gles_vertex_array *va);

#endif
