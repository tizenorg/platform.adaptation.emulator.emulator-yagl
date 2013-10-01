#ifndef _YAGL_GLES_RENDERBUFFER_H_
#define _YAGL_GLES_RENDERBUFFER_H_

#include "yagl_types.h"
#include "yagl_object.h"

#define YAGL_NS_RENDERBUFFER 3

struct yagl_gles_renderbuffer
{
    struct yagl_object base;

    yagl_object_name global_name;

    int was_bound;
};

struct yagl_gles_renderbuffer *yagl_gles_renderbuffer_create(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_renderbuffer_acquire(struct yagl_gles_renderbuffer *rb);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_renderbuffer_release(struct yagl_gles_renderbuffer *rb);

/*
 * Assumes that 'target' is valid.
 */
void yagl_gles_renderbuffer_bind(struct yagl_gles_renderbuffer *rb,
                                 GLenum target);

int yagl_gles_renderbuffer_was_bound(struct yagl_gles_renderbuffer *rb);

#endif
