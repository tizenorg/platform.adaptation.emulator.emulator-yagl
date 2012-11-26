#ifndef _YAGL_CONTEXT_H_
#define _YAGL_CONTEXT_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_resource.h"
#include <EGL/egl.h>

struct yagl_display;
struct yagl_gles_context;

struct yagl_context
{
    struct yagl_resource res;

    struct yagl_display *dpy;

    EGLenum api;

    EGLint version;

    struct yagl_gles_context *gles_ctx;

    EGLSurface draw_sfc;
    EGLSurface read_sfc;
};

struct yagl_context
    *yagl_context_create(yagl_host_handle handle,
                         struct yagl_display *dpy,
                         EGLenum api,
                         EGLint version);

void yagl_context_update(struct yagl_context *ctx,
                         EGLSurface draw_sfc,
                         EGLSurface read_sfc);

struct yagl_gles_context
    *yagl_context_get_gles_context(struct yagl_context *ctx);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_context_acquire(struct yagl_context *ctx);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_context_release(struct yagl_context *ctx);

#endif
