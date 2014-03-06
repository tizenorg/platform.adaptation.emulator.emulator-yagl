#include "GL/gl.h"
#include "yagl_gles_vertex_array.h"
#include "yagl_gles_array.h"
#include "yagl_gles_buffer.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"

static void yagl_gles_vertex_array_destroy(struct yagl_ref *ref)
{
    struct yagl_gles_vertex_array *va = (struct yagl_gles_vertex_array*)ref;
    int i;

    yagl_gles_buffer_release(va->ebo);

    for (i = 0; i < va->num_arrays; ++i) {
        yagl_gles_array_cleanup(&va->arrays[i]);
    }
    yagl_free(va->arrays);

    if (va->global_name) {
        yagl_host_glDeleteObjects(&va->global_name, 1);
    }

    yagl_object_cleanup(&va->base);

    yagl_free(va);
}

struct yagl_gles_vertex_array
    *yagl_gles_vertex_array_create(int fake,
                                   struct yagl_gles_array *arrays,
                                   int num_arrays)
{
    struct yagl_gles_vertex_array *va;

    va = yagl_malloc0(sizeof(*va));

    yagl_object_init(&va->base, &yagl_gles_vertex_array_destroy);

    va->arrays = arrays;
    va->num_arrays = num_arrays;

    if (!fake) {
        va->global_name = yagl_get_global_name();

        yagl_host_glGenVertexArrays(&va->global_name, 1);
    }

    return va;
}

void yagl_gles_vertex_array_acquire(struct yagl_gles_vertex_array *va)
{
    if (va) {
        yagl_object_acquire(&va->base);
    }
}

void yagl_gles_vertex_array_release(struct yagl_gles_vertex_array *va)
{
    if (va) {
        yagl_object_release(&va->base);
    }
}

void yagl_gles_vertex_array_bind(struct yagl_gles_vertex_array *va)
{
    yagl_host_glBindVertexArray(va->global_name);

    va->was_bound = 1;
}

int yagl_gles_vertex_array_was_bound(struct yagl_gles_vertex_array *va)
{
    return va->was_bound;
}
