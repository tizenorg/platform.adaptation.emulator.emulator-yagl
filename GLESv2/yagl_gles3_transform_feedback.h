#ifndef _YAGL_GLES3_TRANSFORM_FEEDBACK_H_
#define _YAGL_GLES3_TRANSFORM_FEEDBACK_H_

#include "yagl_types.h"
#include "yagl_object.h"

struct yagl_gles3_buffer_binding;
struct yagl_gles_buffer;

struct yagl_gles3_transform_feedback
{
    struct yagl_object base;

    yagl_object_name global_name;

    struct yagl_gles3_buffer_binding *buffer_bindings;
    GLuint num_buffer_bindings;

    int active;
    int paused;

    GLuint num_active_buffer_bindings;

    int was_bound;
};

struct yagl_gles3_transform_feedback
    *yagl_gles3_transform_feedback_create(int fake,
                                          GLuint num_buffer_bindings);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles3_transform_feedback_acquire(struct yagl_gles3_transform_feedback *tf);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles3_transform_feedback_release(struct yagl_gles3_transform_feedback *tf);

int yagl_gles3_transform_feedback_bind_buffer_base(struct yagl_gles3_transform_feedback *tf,
                                                   GLuint index,
                                                   struct yagl_gles_buffer *buffer);

int yagl_gles3_transform_feedback_bind_buffer_range(struct yagl_gles3_transform_feedback *tf,
                                                    GLuint index,
                                                    GLintptr offset,
                                                    GLsizeiptr size,
                                                    struct yagl_gles_buffer *buffer);

void yagl_gles3_transform_feedback_unbind_buffer(struct yagl_gles3_transform_feedback *tf,
                                                 yagl_object_name buffer_local_name);

void yagl_gles3_transform_feedback_bind(struct yagl_gles3_transform_feedback *tf,
                                        GLenum target);

int yagl_gles3_transform_feedback_was_bound(struct yagl_gles3_transform_feedback *tf);

void yagl_gles3_transform_feedback_begin(struct yagl_gles3_transform_feedback *tf,
                                         GLenum primitive_mode,
                                         GLuint num_active_buffer_bindings);

void yagl_gles3_transform_feedback_pause(struct yagl_gles3_transform_feedback *tf);

void yagl_gles3_transform_feedback_resume(struct yagl_gles3_transform_feedback *tf);

void yagl_gles3_transform_feedback_post_draw(struct yagl_gles3_transform_feedback *tf);

void yagl_gles3_transform_feedback_end(struct yagl_gles3_transform_feedback *tf);

#endif
