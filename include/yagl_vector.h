#ifndef _YAGL_VECTOR_H_
#define _YAGL_VECTOR_H_

#include "yagl_export.h"
#include "yagl_types.h"

struct yagl_vector
{
    void *data;
    int elem_size;
    int size;
    int capacity;
};

YAGL_API void yagl_vector_init(struct yagl_vector *v,
                               int elem_size,
                               int initial_capacity);

/*
 * Cleans up the vector, you must call 'yagl_vector_init' again to be able
 * to use it.
 */
YAGL_API void yagl_vector_cleanup(struct yagl_vector *v);

/*
 * Detaches the buffer from vector. vector will automatically clean up,
 * you must call 'yagl_vector_init' again to be able to use it.
 */
YAGL_API void *yagl_vector_detach(struct yagl_vector *v);

YAGL_API int yagl_vector_size(struct yagl_vector *v);

YAGL_API int yagl_vector_capacity(struct yagl_vector *v);

YAGL_API void yagl_vector_push_back(struct yagl_vector *v, const void *elem);

YAGL_API void yagl_vector_resize(struct yagl_vector *v, int new_size);

YAGL_API void *yagl_vector_data(struct yagl_vector *v);

#endif
