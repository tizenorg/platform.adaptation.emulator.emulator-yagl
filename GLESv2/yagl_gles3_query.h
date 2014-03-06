#ifndef _YAGL_GLES3_QUERY_H_
#define _YAGL_GLES3_QUERY_H_

#include "yagl_types.h"
#include "yagl_object.h"

struct yagl_gles3_query
{
    struct yagl_object base;

    yagl_object_name global_name;

    int active;

    int result_available;
    GLuint result;

    int was_active;
};

struct yagl_gles3_query *yagl_gles3_query_create(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles3_query_acquire(struct yagl_gles3_query *query);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles3_query_release(struct yagl_gles3_query *query);

void yagl_gles3_query_begin(struct yagl_gles3_query *query,
                            GLenum target);

void yagl_gles3_query_end(struct yagl_gles3_query *query,
                          GLenum target);

int yagl_gles3_query_is_result_available(struct yagl_gles3_query *query);

GLuint yagl_gles3_query_get_result(struct yagl_gles3_query *query);

int yagl_gles3_query_was_active(struct yagl_gles3_query *query);

#endif
