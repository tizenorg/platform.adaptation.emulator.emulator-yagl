#include "GLES3/gl3.h"
#include "yagl_gles3_buffer_binding.h"
#include "yagl_gles_buffer.h"
#include "yagl_host_gles_calls.h"
#include <assert.h>

void yagl_gles3_buffer_binding_init(struct yagl_gles3_buffer_binding *buffer_binding,
                                    GLenum target,
                                    GLuint index)
{
    buffer_binding->target = target;
    buffer_binding->index = index;
    yagl_list_init(&buffer_binding->list);
}

void yagl_gles3_buffer_binding_reset(struct yagl_gles3_buffer_binding *buffer_binding)
{
    yagl_gles_buffer_release(buffer_binding->buffer);
    yagl_list_remove(&buffer_binding->list);
    buffer_binding->buffer = NULL;
    buffer_binding->entire = 0;
    buffer_binding->offset = 0;
    buffer_binding->size = 0;
}

void yagl_gles3_buffer_binding_set_base(struct yagl_gles3_buffer_binding *buffer_binding,
                                        struct yagl_gles_buffer *buffer)
{
    yagl_gles_buffer_acquire(buffer);
    yagl_gles3_buffer_binding_reset(buffer_binding);
    if (buffer) {
        buffer_binding->buffer = buffer;
        buffer_binding->entire = 1;
    }
}

void yagl_gles3_buffer_binding_set_range(struct yagl_gles3_buffer_binding *buffer_binding,
                                         struct yagl_gles_buffer *buffer,
                                         GLintptr offset,
                                         GLsizeiptr size)
{
    yagl_gles_buffer_acquire(buffer);
    yagl_gles3_buffer_binding_reset(buffer_binding);
    if (buffer) {
        buffer_binding->buffer = buffer;
        buffer_binding->offset = offset;
        buffer_binding->size = size;
    }
}

void yagl_gles3_buffer_binding_transfer_begin(struct yagl_gles3_buffer_binding *buffer_binding)
{
    GLint offset;
    GLsizei size;

    assert(buffer_binding->buffer);
    if (!buffer_binding->buffer) {
        return;
    }

    if (buffer_binding->buffer->size <= 0) {
        return;
    }

    if (yagl_gles_buffer_is_dirty(buffer_binding->buffer, 0, 0)) {
        yagl_gles_buffer_bind(buffer_binding->buffer, 0, 0,
                              buffer_binding->target);
        yagl_gles_buffer_transfer(buffer_binding->buffer, 0,
                                  buffer_binding->target, 0);
    }

    if (buffer_binding->entire) {
        yagl_host_glBindBufferBase(buffer_binding->target,
                                   buffer_binding->index,
                                   buffer_binding->buffer->default_part.global_name);
    } else {
        offset = buffer_binding->offset;
        size = buffer_binding->size;

        if (offset < 0) {
            size += offset;
            offset = 0;
        }

        if ((offset >= buffer_binding->buffer->size) || (size <= 0)) {
            return;
        }

        if ((offset + size) > buffer_binding->buffer->size) {
            size = buffer_binding->buffer->size - offset;
        }

        yagl_host_glBindBufferRange(buffer_binding->target,
                                    buffer_binding->index,
                                    buffer_binding->buffer->default_part.global_name,
                                    offset, size);
    }
}

void yagl_gles3_buffer_binding_transfer_end(struct yagl_gles3_buffer_binding *buffer_binding)
{
    yagl_host_glBindBufferBase(buffer_binding->target,
                               buffer_binding->index,
                               0);
}
