#ifndef _YAGL_CONTEXT_H_
#define _YAGL_CONTEXT_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_resource.h"
#include "EGL/egl.h"

struct yagl_display;
struct yagl_fence;
struct yagl_client_context;

struct yagl_context
{
    struct yagl_resource res;

    struct yagl_display *dpy;

    struct yagl_client_context *client_ctx;

    int need_throttle;
    struct yagl_fence *throttle_fence;

    int client_ctx_prepared;

    pthread_mutex_t mtx;

    int current;
};

struct yagl_context
    *yagl_context_create(yagl_host_handle handle,
                         struct yagl_display *dpy,
                         struct yagl_client_context *client_ctx);

void yagl_context_set_need_throttle(struct yagl_context *ctx,
                                    struct yagl_fence *throttle_fence);

void yagl_context_throttle(struct yagl_context *ctx);

int yagl_context_mark_current(struct yagl_context *ctx, int current);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_context_acquire(struct yagl_context *ctx);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_context_release(struct yagl_context *ctx);

#endif
