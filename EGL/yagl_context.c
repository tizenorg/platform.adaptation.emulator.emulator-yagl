#include "GL/gl.h"
#include "yagl_context.h"
#include "yagl_malloc.h"
#include "yagl_display.h"
#include "yagl_gles_context.h"

static void yagl_context_destroy(struct yagl_ref *ref)
{
    struct yagl_context *ctx = (struct yagl_context*)ref;

    if (ctx->gles_ctx) {
        yagl_free(ctx->gles_ctx->arrays);
        ctx->gles_ctx->arrays = NULL;

        yagl_free(ctx->gles_ctx->extensions);
        ctx->gles_ctx->extensions = NULL;

        yagl_free(ctx->gles_ctx);
        ctx->gles_ctx = NULL;
    }

    yagl_resource_cleanup(&ctx->res);

    yagl_free(ctx);
}

struct yagl_context
    *yagl_context_create(yagl_host_handle handle,
                         struct yagl_display *dpy,
                         EGLenum api,
                         EGLint version)
{
    struct yagl_context *ctx;

    ctx = yagl_malloc0(sizeof(*ctx));

    yagl_resource_init(&ctx->res, &yagl_context_destroy, handle);

    ctx->dpy = dpy;

    ctx->api = api;

    ctx->version = version;

    return ctx;
}

struct yagl_gles_context
    *yagl_context_get_gles_context(struct yagl_context *ctx)
{
    if (!ctx->gles_ctx) {
        ctx->gles_ctx = yagl_malloc0(sizeof(*ctx->gles_ctx));
    }

    return ctx->gles_ctx;
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
