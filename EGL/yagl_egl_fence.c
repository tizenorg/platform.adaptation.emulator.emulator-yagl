#include "yagl_egl_fence.h"
#include "yagl_egl_state.h"
#include "yagl_fence.h"
#include "yagl_context.h"
#include "yagl_backend.h"

int yagl_egl_fence_supported(void)
{
    return yagl_get_backend()->fence_supported;
}

void yagl_egl_fence_acquire(struct yagl_egl_fence *egl_fence)
{
    if (egl_fence) {
        yagl_resource_acquire(&egl_fence->res);
    }
}

void yagl_egl_fence_release(struct yagl_egl_fence *egl_fence)
{
    if (egl_fence) {
        yagl_resource_release(&egl_fence->res);
    }
}

struct yagl_egl_fence *yagl_create_egl_fence(void)
{
    struct yagl_context *ctx = yagl_get_context();
    struct yagl_fence *fence;

    if (!ctx) {
        return NULL;
    }

    fence = yagl_get_backend()->create_fence(ctx->dpy);

    if (!fence) {
        return NULL;
    }

    return &fence->base;
}
