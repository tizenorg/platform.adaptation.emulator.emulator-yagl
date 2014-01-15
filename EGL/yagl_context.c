#include "GL/gl.h"
#include "yagl_context.h"
#include "yagl_malloc.h"
#include "yagl_utils.h"
#include "yagl_display.h"
#include "yagl_fence.h"
#include "yagl_transport.h"
#include "yagl_state.h"
#include "yagl_client_context.h"

static void yagl_context_destroy(struct yagl_ref *ref)
{
    struct yagl_context *ctx = (struct yagl_context*)ref;

    yagl_fence_release(ctx->throttle_fence);

    pthread_mutex_destroy(&ctx->mtx);

    ctx->client_ctx->destroy(ctx->client_ctx);

    yagl_resource_cleanup(&ctx->res);

    yagl_free(ctx);
}

struct yagl_context
    *yagl_context_create(yagl_host_handle handle,
                         struct yagl_display *dpy,
                         struct yagl_client_context *client_ctx)
{
    struct yagl_context *ctx;

    ctx = yagl_malloc0(sizeof(*ctx));

    yagl_resource_init(&ctx->res, &yagl_context_destroy, handle);

    ctx->dpy = dpy;
    ctx->client_ctx = client_ctx;

    yagl_mutex_init(&ctx->mtx);

    return ctx;
}

void yagl_context_set_need_throttle(struct yagl_context *ctx,
                                    struct yagl_fence *throttle_fence)
{
    ctx->need_throttle = 1;
    yagl_fence_acquire(throttle_fence);
    yagl_fence_release(ctx->throttle_fence);
    ctx->throttle_fence = throttle_fence;
}

void yagl_context_throttle(struct yagl_context *ctx)
{
    if (!ctx->need_throttle) {
        return;
    }

    if (ctx->throttle_fence) {
        ctx->throttle_fence->wait(ctx->throttle_fence);
        yagl_fence_release(ctx->throttle_fence);
        ctx->throttle_fence = NULL;
    } else {
        yagl_transport_wait(yagl_get_transport());
    }

    ctx->need_throttle = 0;
}

int yagl_context_mark_current(struct yagl_context *ctx, int current)
{
    int ret = 1;

    pthread_mutex_lock(&ctx->mtx);

    if ((!current ^ !ctx->current) != 0) {
        ctx->current = current;
    } else {
        ret = 0;
    }

    pthread_mutex_unlock(&ctx->mtx);

    return ret;
}

void yagl_context_acquire(struct yagl_context *ctx)
{
    if (ctx) {
        yagl_resource_acquire(&ctx->res);
    }
}

void yagl_context_release(struct yagl_context *ctx)
{
    if (ctx) {
        yagl_resource_release(&ctx->res);
    }
}
