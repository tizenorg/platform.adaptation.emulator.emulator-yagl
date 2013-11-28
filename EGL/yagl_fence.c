#include "yagl_fence.h"

void yagl_fence_init(struct yagl_fence *fence,
                     yagl_ref_destroy_func destroy_func,
                     struct yagl_display *dpy,
                     uint32_t seq)
{
    yagl_resource_init(&fence->res, destroy_func, 0);

    fence->dpy = dpy;
    fence->seq = seq;
}

void yagl_fence_cleanup(struct yagl_fence *fence)
{
    yagl_resource_cleanup(&fence->res);
}

void yagl_fence_acquire(struct yagl_fence *fence)
{
    if (fence) {
        yagl_resource_acquire(&fence->res);
    }
}

void yagl_fence_release(struct yagl_fence *fence)
{
    if (fence) {
        yagl_resource_release(&fence->res);
    }
}
