#include "GLES3/gl3.h"
#include "yagl_gles3_buffer_binding.h"
#include "yagl_gles_buffer.h"

void yagl_gles3_buffer_binding_init(struct yagl_gles3_buffer_binding *buffer_binding,
                                    GLuint index)
{
    buffer_binding->index = index;
    yagl_list_init(&buffer_binding->list);
}

void yagl_gles3_buffer_binding_reset(struct yagl_gles3_buffer_binding *buffer_binding)
{
    yagl_gles_buffer_release(buffer_binding->buffer);
    yagl_list_remove(&buffer_binding->list);
    buffer_binding->buffer = NULL;
    buffer_binding->target = 0;
    buffer_binding->entire = 0;
    buffer_binding->offset = 0;
    buffer_binding->size = 0;
}

void yagl_gles3_buffer_binding_set_base(struct yagl_gles3_buffer_binding *buffer_binding,
                                        struct yagl_gles_buffer *buffer,
                                        GLenum target)
{
    yagl_gles_buffer_acquire(buffer);
    yagl_gles3_buffer_binding_reset(buffer_binding);
    if (buffer) {
        buffer_binding->buffer = buffer;
        buffer_binding->target = target;
        buffer_binding->entire = 1;
    }
}

void yagl_gles3_buffer_binding_set_range(struct yagl_gles3_buffer_binding *buffer_binding,
                                         struct yagl_gles_buffer *buffer,
                                         GLenum target,
                                         GLintptr offset,
                                         GLsizeiptr size)
{
    yagl_gles_buffer_acquire(buffer);
    yagl_gles3_buffer_binding_reset(buffer_binding);
    if (buffer) {
        buffer_binding->buffer = buffer;
        buffer_binding->target = target;
        buffer_binding->offset = offset;
        buffer_binding->size = size;
    }
}
