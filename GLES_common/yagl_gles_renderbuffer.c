#include "GL/gl.h"
#include "yagl_gles_renderbuffer.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"

static void yagl_gles_renderbuffer_destroy(struct yagl_ref *ref)
{
    struct yagl_gles_renderbuffer *rb = (struct yagl_gles_renderbuffer*)ref;

    yagl_host_glDeleteObjects(&rb->global_name, 1);

    yagl_object_cleanup(&rb->base);

    yagl_free(rb);
}

struct yagl_gles_renderbuffer *yagl_gles_renderbuffer_create(void)
{
    struct yagl_gles_renderbuffer *rb;

    rb = yagl_malloc0(sizeof(*rb));

    yagl_object_init(&rb->base, &yagl_gles_renderbuffer_destroy);

    rb->global_name = yagl_get_global_name();

    yagl_host_glGenRenderbuffers(&rb->global_name, 1);

    return rb;
}

void yagl_gles_renderbuffer_acquire(struct yagl_gles_renderbuffer *rb)
{
    if (rb) {
        yagl_object_acquire(&rb->base);
    }
}

void yagl_gles_renderbuffer_release(struct yagl_gles_renderbuffer *rb)
{
    if (rb) {
        yagl_object_release(&rb->base);
    }
}

void yagl_gles_renderbuffer_bind(struct yagl_gles_renderbuffer *rb,
                                 GLenum target)
{
    if (!rb) {
        yagl_host_glBindRenderbuffer(target, 0);
        return;
    }

    yagl_host_glBindRenderbuffer(target, rb->global_name);

    rb->was_bound = 1;
}

int yagl_gles_renderbuffer_was_bound(struct yagl_gles_renderbuffer *rb)
{
    return rb->was_bound;
}
