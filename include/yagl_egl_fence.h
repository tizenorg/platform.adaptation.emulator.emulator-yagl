#ifndef _YAGL_EGL_FENCE_H_
#define _YAGL_EGL_FENCE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_resource.h"

struct yagl_egl_fence
{
    struct yagl_resource res;

    int (*wait)(struct yagl_egl_fence */*egl_fence*/);

    int (*signaled)(struct yagl_egl_fence */*egl_fence*/);
};

YAGL_API int yagl_egl_fence_supported(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_egl_fence_acquire(struct yagl_egl_fence *egl_fence);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_egl_fence_release(struct yagl_egl_fence *egl_fence);

YAGL_API struct yagl_egl_fence *yagl_create_egl_fence(void);

#endif
