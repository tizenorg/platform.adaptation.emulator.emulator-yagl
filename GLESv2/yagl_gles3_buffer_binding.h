#ifndef _YAGL_GLES3_BUFFER_BINDING_H_
#define _YAGL_GLES3_BUFFER_BINDING_H_

#include "yagl_gles_types.h"

struct yagl_gles_buffer;

struct yagl_gles3_buffer_binding
{
    struct yagl_gles_buffer *buffer;

    int entire;

    GLintptr offset;
    GLsizeiptr size;
};

void yagl_gles3_buffer_binding_reset(struct yagl_gles3_buffer_binding *buffer_binding);

void yagl_gles3_buffer_binding_set_base(struct yagl_gles3_buffer_binding *buffer_binding,
                                        struct yagl_gles_buffer *buffer);

void yagl_gles3_buffer_binding_set_range(struct yagl_gles3_buffer_binding *buffer_binding,
                                         struct yagl_gles_buffer *buffer,
                                         GLintptr offset,
                                         GLsizeiptr size);

#endif
