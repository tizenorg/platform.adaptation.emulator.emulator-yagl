#ifndef _YAGL_GLES_BUFFER_H_
#define _YAGL_GLES_BUFFER_H_

#include "yagl_types.h"
#include "yagl_object.h"
#include "yagl_range_list.h"

 /*
  * VBO implementation is somewhat tricky because
  * we must correctly handle GL_FIXED data type, which must be
  * converted to GL_FLOAT, and GL_BYTE data type, which (in some cases)
  * must be converted to GL_SHORT. Buffer objects may be bound to
  * multiple contexts, so there may be several VBOs using the
  * same buffer object, but having different data types. We handle this
  * by having three 'yagl_gles_buffer_part' objects: one for GL_FIXED type,
  * one for GL_BYTE type and the other for all other types.
  * Each 'yagl_gles_buffer_part' object has a separate global buffer name
  * and a range list that consists of 'glBufferData' and 'glBufferSubData'
  * ranges which haven't been processed yet. When a VBO is used we walk
  * the appropriate range list, make conversions, clear it and call
  * 'glBufferData' or 'glBufferSubData' as many times as needed.
  */

#define YAGL_NS_BUFFER 0

struct yagl_gles_buffer_part
{
    yagl_object_name global_name;

    struct yagl_range_list range_list;
};

struct yagl_gles_buffer
{
    struct yagl_object base;

    struct yagl_gles_buffer_part default_part;
    struct yagl_gles_buffer_part fixed_part;
    struct yagl_gles_buffer_part byte_part;

    GLint size;
    void *data;
    GLenum usage;

    void *map_pointer;
    GLbitfield map_access;
    GLintptr map_offset;
    GLsizeiptr map_length;

    struct yagl_range_list gpu_dirty_list;

    int was_bound;

    int cached_minmax_idx;
    GLenum cached_type;
    GLint cached_offset;
    GLint cached_count;
    uint32_t cached_min_idx;
    uint32_t cached_max_idx;
};

struct yagl_gles_buffer *yagl_gles_buffer_create(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_buffer_acquire(struct yagl_gles_buffer *buffer);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_buffer_release(struct yagl_gles_buffer *buffer);

void yagl_gles_buffer_set_data(struct yagl_gles_buffer *buffer,
                               GLint size,
                               const void *data,
                               GLenum usage);

int yagl_gles_buffer_update_data(struct yagl_gles_buffer *buffer,
                                 GLint offset,
                                 GLint size,
                                 const void *data);

int yagl_gles_buffer_get_minmax_index(struct yagl_gles_buffer *buffer,
                                      GLenum type,
                                      GLint offset,
                                      GLint count,
                                      uint32_t *min_idx,
                                      uint32_t *max_idx);

void yagl_gles_buffer_bind(struct yagl_gles_buffer *buffer,
                           GLenum type,
                           int need_convert,
                           GLenum target);

void yagl_gles_buffer_transfer(struct yagl_gles_buffer *buffer,
                               GLenum type,
                               GLenum target,
                               int need_convert);

int yagl_gles_buffer_get_parameter(struct yagl_gles_buffer *buffer,
                                   GLenum pname,
                                   GLint *param);

/*
 * Assumes that 'access' has already been validated.
 */
int yagl_gles_buffer_map(struct yagl_gles_buffer *buffer,
                         GLintptr offset,
                         GLsizeiptr length,
                         GLbitfield access);

int yagl_gles_buffer_mapped(struct yagl_gles_buffer *buffer);

int yagl_gles_buffer_flush_mapped_range(struct yagl_gles_buffer *buffer,
                                        GLintptr offset,
                                        GLsizeiptr length);

void yagl_gles_buffer_unmap(struct yagl_gles_buffer *buffer);

void yagl_gles_buffer_set_bound(struct yagl_gles_buffer *buffer);

int yagl_gles_buffer_was_bound(struct yagl_gles_buffer *buffer);

int yagl_gles_buffer_is_cpu_dirty(struct yagl_gles_buffer *buffer,
                                  GLenum type,
                                  int need_convert);

void yagl_gles_buffer_set_gpu_dirty(struct yagl_gles_buffer *buffer,
                                    GLint offset,
                                    GLint size);

#endif
