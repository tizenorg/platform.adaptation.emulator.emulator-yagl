#include "GL/gl.h"
#include "GL/glext.h"
#include "yagl_gles_utils.h"
#include "yagl_host_gles_calls.h"

void yagl_gles_reset_unpack(const struct yagl_gles_pixelstore* ps)
{
    if (ps->alignment != 1) {
        yagl_host_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    if (ps->row_length > 0) {
        yagl_host_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }

    if (ps->image_height > 0) {
        yagl_host_glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    }
}

void yagl_gles_set_unpack(const struct yagl_gles_pixelstore* ps)
{
    if (ps->alignment != 1) {
        yagl_host_glPixelStorei(GL_UNPACK_ALIGNMENT, ps->alignment);
    }

    if (ps->row_length > 0) {
        yagl_host_glPixelStorei(GL_UNPACK_ROW_LENGTH, ps->row_length);
    }

    if (ps->image_height > 0) {
        yagl_host_glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, ps->image_height);
    }
}
