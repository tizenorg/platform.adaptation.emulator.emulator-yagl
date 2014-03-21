#ifndef _YAGL_GLES3_SYNC_H_
#define _YAGL_GLES3_SYNC_H_

#include "yagl_types.h"
#include "yagl_object.h"

#define YAGL_NS_SYNC 5

struct yagl_egl_fence;

struct yagl_gles3_sync
{
    struct yagl_object base;

    struct yagl_egl_fence *egl_fence;

    int signaled;
};

struct yagl_gles3_sync *yagl_gles3_sync_create(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles3_sync_acquire(struct yagl_gles3_sync *sync);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles3_sync_release(struct yagl_gles3_sync *sync);

#endif
