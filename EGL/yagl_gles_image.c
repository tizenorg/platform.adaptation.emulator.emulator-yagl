#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "yagl_gles_image.h"
#include "yagl_egl_state.h"
#include "yagl_display.h"
#include "yagl_context.h"
#include "yagl_image.h"

struct yagl_gles_image *yagl_gles_image_acquire(GLeglImageOES image)
{
    struct yagl_context *ctx = yagl_get_context();

    if (ctx) {
        struct yagl_image *egl_image = yagl_display_image_acquire(ctx->dpy,
            (EGLImageKHR)image);
        return (egl_image ? egl_image->gles_image : NULL);
    } else {
        return NULL;
    }
}

void yagl_gles_image_release(struct yagl_gles_image *image)
{
    if (image) {
        struct yagl_image *egl_image = image->opaque;
        yagl_image_release(egl_image);
    }
}
