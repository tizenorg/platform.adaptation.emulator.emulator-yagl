#ifndef _YAGL_FENCE_H_
#define _YAGL_FENCE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_resource.h"

struct yagl_display;

struct yagl_fence
{
    struct yagl_resource res;

    struct yagl_display *dpy;

    uint32_t seq;

    int (*wait)(struct yagl_fence */*fence*/);

    int (*signaled)(struct yagl_fence */*fence*/);
};

void yagl_fence_init(struct yagl_fence *fence,
                     yagl_ref_destroy_func destroy_func,
                     struct yagl_display *dpy,
                     uint32_t seq);

void yagl_fence_cleanup(struct yagl_fence *fence);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_fence_acquire(struct yagl_fence *fence);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_fence_release(struct yagl_fence *fence);

#endif
