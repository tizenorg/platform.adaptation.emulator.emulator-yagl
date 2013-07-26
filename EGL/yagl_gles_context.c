#include "GL/gl.h"
#include "yagl_gles_context.h"
#include "yagl_egl_state.h"
#include "yagl_context.h"

struct yagl_gles_context *yagl_gles_context_get()
{
    struct yagl_context *ctx = yagl_get_context();

    if (ctx) {
        return yagl_context_get_gles_context(ctx);
    } else {
        return NULL;
    }
}
