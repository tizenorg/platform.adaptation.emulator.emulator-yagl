#include "GLES/gl.h"
#include "GLES/glext.h"
#include "yagl_state.h"
#include "yagl_utils.h"
#include "yagl_malloc.h"
#include "yagl_log.h"
#include "yagl_gles_buffer.h"
#include "yagl_gles_validate.h"
#include "yagl_host_gles_calls.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/*
 * We can't include GL/glext.h here
 */
#define GL_BUFFER_ACCESS_FLAGS 0x911F
#define GL_BUFFER_MAP_OFFSET 0x9121
#define GL_BUFFER_MAP_LENGTH 0x9120

typedef void (*yagl_gles_buffer_transfer_func)(struct yagl_gles_buffer */*buffer*/,
                                               GLenum /*target*/,
                                               int /*start*/,
                                               int /*size*/);

static void yagl_gles_buffer_transfer_default(struct yagl_gles_buffer *buffer,
                                              GLenum target,
                                              int start,
                                              int size)
{
    if ((start == 0) && (size == buffer->size)) {
        yagl_host_glBufferData(target,
                               buffer->data,
                               size,
                               buffer->usage);
    } else {
        yagl_host_glBufferSubData(target,
                                  start,
                                  buffer->data + start,
                                  size);
    }
}

static void yagl_gles_buffer_transfer_fixed(struct yagl_gles_buffer *buffer,
                                            GLenum target,
                                            int start,
                                            int size)
{
    GLfixed *data = buffer->data + start;
    GLfloat *converted;
    int i;

    assert(sizeof(GLfixed) == sizeof(GLfloat));

    converted = (GLfloat*)yagl_get_tmp_buffer(size);

    for (i = 0; i < (size / sizeof(GLfloat)); ++i) {
        converted[i] = yagl_fixed_to_float(data[i]);
    }

    if ((start == 0) && (size == buffer->size)) {
        yagl_host_glBufferData(target,
                               converted,
                               size,
                               buffer->usage);
    } else {
        yagl_host_glBufferSubData(target,
                                  start,
                                  converted,
                                  size);
    }
}

static void yagl_gles_buffer_transfer_byte(struct yagl_gles_buffer *buffer,
                                           GLenum target,
                                           int start,
                                           int size)
{
    GLbyte *data = buffer->data + start;
    GLshort *converted;
    int i;

    converted = (GLshort*)yagl_get_tmp_buffer(size * sizeof(GLshort));

    for (i = 0; i < size; ++i) {
        converted[i] = data[i];
    }

    if ((start == 0) && (size == buffer->size)) {
        yagl_host_glBufferData(target,
                               converted,
                               size * sizeof(GLshort),
                               buffer->usage);
    } else {
        yagl_host_glBufferSubData(target,
                                  start * sizeof(GLshort),
                                  converted,
                                  size * sizeof(GLshort));
    }
}

static void yagl_gles_buffer_transfer_internal(struct yagl_gles_buffer *buffer,
                                               struct yagl_range_list *range_list,
                                               GLenum target,
                                               yagl_gles_buffer_transfer_func transfer_func)
{
    int num_ranges = yagl_range_list_size(range_list);
    int i, start, size;

    if (num_ranges <= 0) {
        return;
    }

    if (num_ranges == 1) {
        yagl_range_list_get(range_list,
                            0,
                            &start,
                            &size);
        if (size == 0) {
            /*
             * Buffer clear.
             */
            assert(start == 0);
            yagl_host_glBufferData(target,
                                   NULL,
                                   0,
                                   buffer->usage);
            yagl_range_list_clear(range_list);
            return;
        } else if ((start == 0) && (size == buffer->size)) {
            /*
             * Buffer full update.
             */
            transfer_func(buffer, target, 0, size);
            yagl_range_list_clear(range_list);
            return;
        }
    }

    /*
     * Buffer partial updates.
     */

    for (i = 0; i < num_ranges; ++i) {
        yagl_range_list_get(range_list,
                            i,
                            &start,
                            &size);
        transfer_func(buffer, target, start, size);
    }
    yagl_range_list_clear(range_list);
}

static void yagl_gles_buffer_transfer_from_gpu(struct yagl_gles_buffer *buffer)
{
    int i, num_ranges = yagl_range_list_size(&buffer->gpu_dirty_list);
    GLuint* ranges;
    int current = 0;

    YAGL_LOG_FUNC_SET(yagl_gles_buffer_transfer_from_gpu);

    if (num_ranges <= 0) {
        return;
    }

    ranges = (GLuint*)yagl_get_tmp_buffer2(num_ranges * 2);

    for (i = 0; i < num_ranges; ++i) {
        int start, size;

        yagl_range_list_get(&buffer->gpu_dirty_list,
                            i,
                            &start,
                            &size);

        ranges[(i * 2) + 0] = start - current;
        ranges[(i * 2) + 1] = size;

        current = start + size;
    }

    YAGL_LOG_DEBUG("transferring %d ranges, max %d bytes",
                   num_ranges, (int32_t)(current - ranges[0]));

    yagl_host_glMapBuffer(buffer->default_part.global_name,
                          ranges, num_ranges * 2,
                          buffer->data + ranges[0],
                          current - ranges[0],
                          NULL);

    yagl_range_list_clear(&buffer->gpu_dirty_list);
}

static void yagl_gles_buffer_destroy(struct yagl_ref *ref)
{
    GLuint buffer_names[3];
    struct yagl_gles_buffer *buffer = (struct yagl_gles_buffer*)ref;

    buffer_names[0] = buffer->default_part.global_name;
    buffer_names[1] = buffer->fixed_part.global_name;
    buffer_names[2] = buffer->byte_part.global_name;

    yagl_host_glDeleteObjects(buffer_names,
                              sizeof(buffer_names)/sizeof(buffer_names[0]));

    yagl_range_list_cleanup(&buffer->gpu_dirty_list);

    yagl_range_list_cleanup(&buffer->default_part.range_list);
    yagl_range_list_cleanup(&buffer->fixed_part.range_list);
    yagl_range_list_cleanup(&buffer->byte_part.range_list);

    yagl_free(buffer->data);

    yagl_object_cleanup(&buffer->base);

    yagl_free(buffer);
}

struct yagl_gles_buffer *yagl_gles_buffer_create(void)
{
    GLuint buffer_names[3];
    struct yagl_gles_buffer *buffer;

    buffer = yagl_malloc0(sizeof(*buffer));

    yagl_object_init(&buffer->base, &yagl_gles_buffer_destroy);

    buffer_names[0] = buffer->default_part.global_name = yagl_get_global_name();
    buffer_names[1] = buffer->fixed_part.global_name = yagl_get_global_name();
    buffer_names[2] = buffer->byte_part.global_name = yagl_get_global_name();

    yagl_host_glGenBuffers(buffer_names,
                           sizeof(buffer_names)/sizeof(buffer_names[0]));

    yagl_range_list_init(&buffer->default_part.range_list);
    yagl_range_list_init(&buffer->fixed_part.range_list);
    yagl_range_list_init(&buffer->byte_part.range_list);

    yagl_range_list_init(&buffer->gpu_dirty_list);

    return buffer;
}

void yagl_gles_buffer_acquire(struct yagl_gles_buffer *buffer)
{
    if (buffer) {
        yagl_object_acquire(&buffer->base);
    }
}

void yagl_gles_buffer_release(struct yagl_gles_buffer *buffer)
{
    if (buffer) {
        yagl_object_release(&buffer->base);
    }
}

void yagl_gles_buffer_set_data(struct yagl_gles_buffer *buffer,
                               GLint size,
                               const void *data,
                               GLenum usage)
{
    if (size > 0) {
        if (size > buffer->size) {
            yagl_free(buffer->data);
            buffer->data = yagl_malloc(size);
        }
        buffer->size = size;
        if (data) {
            memcpy(buffer->data, data, buffer->size);
        }
    } else {
        yagl_free(buffer->data);
        buffer->data = NULL;
        buffer->size = 0;
    }

    buffer->usage = usage;

    buffer->map_pointer = NULL;
    buffer->map_access = 0;
    buffer->map_offset = 0;
    buffer->map_length = 0;

    yagl_range_list_clear(&buffer->default_part.range_list);
    yagl_range_list_clear(&buffer->fixed_part.range_list);
    yagl_range_list_clear(&buffer->byte_part.range_list);

    /*
     * Full data update, we don't care about GPU dirty
     * regions anymore, just discard them.
     */
    yagl_range_list_clear(&buffer->gpu_dirty_list);

    yagl_range_list_add(&buffer->default_part.range_list, 0, buffer->size);
    yagl_range_list_add(&buffer->fixed_part.range_list, 0, buffer->size);
    yagl_range_list_add(&buffer->byte_part.range_list, 0, buffer->size);

    buffer->cached_minmax_idx = 0;
}

int yagl_gles_buffer_update_data(struct yagl_gles_buffer *buffer,
                                 GLint offset,
                                 GLint size,
                                 const void *data)
{
    if ((offset < 0) || (size < 0) || ((offset + size) > buffer->size)) {
        return 0;
    }

    if (size == 0) {
        return 1;
    }

    /*
     * We're partially updating data, the region being updated might
     * intersect with GPU dirty region, thus, if we issue
     * 'update_from_gpu' later we might screw things up, so do
     * this now.
     */
    yagl_gles_buffer_transfer_from_gpu(buffer);

    memcpy(buffer->data + offset, data, size);

    yagl_range_list_add(&buffer->default_part.range_list, offset, size);
    yagl_range_list_add(&buffer->fixed_part.range_list, offset, size);
    yagl_range_list_add(&buffer->byte_part.range_list, offset, size);

    buffer->cached_minmax_idx = 0;

    return 1;
}

int yagl_gles_buffer_get_minmax_index(struct yagl_gles_buffer *buffer,
                                      GLenum type,
                                      GLint offset,
                                      GLint count,
                                      uint32_t *min_idx,
                                      uint32_t *max_idx)
{
    int index_size, i;
    uint32_t idx = 0;

    *min_idx = UINT32_MAX;
    *max_idx = 0;

    if (!yagl_gles_get_index_size(type, &index_size)) {
        return 0;
    }

    if ((offset < 0) || (count <= 0) || ((offset + (count * index_size)) > buffer->size)) {
        return 0;
    }

    if (buffer->cached_minmax_idx &&
        (buffer->cached_type == type) &&
        (buffer->cached_offset == offset) &&
        (buffer->cached_count == count)) {
        *min_idx = buffer->cached_min_idx;
        *max_idx = buffer->cached_max_idx;
        return 1;
    }

    yagl_gles_buffer_transfer_from_gpu(buffer);

    switch (type) {
    case GL_UNSIGNED_BYTE:
        for (i = 0; i < count; ++i) {
            idx = *(uint8_t*)(buffer->data + offset + (i * index_size));
            if (idx < *min_idx) {
                *min_idx = idx;
            }
            if (idx > *max_idx) {
                *max_idx = idx;
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        for (i = 0; i < count; ++i) {
            idx = *(uint16_t*)(buffer->data + offset + (i * index_size));
            if (idx < *min_idx) {
                *min_idx = idx;
            }
            if (idx > *max_idx) {
                *max_idx = idx;
            }
        }
        break;
    case GL_UNSIGNED_INT:
        for (i = 0; i < count; ++i) {
            idx = *(uint32_t*)(buffer->data + offset + (i * index_size));
            if (idx < *min_idx) {
                *min_idx = idx;
            }
            if (idx > *max_idx) {
                *max_idx = idx;
            }
        }
        break;
    default:
        assert(0);
        break;
    }

    buffer->cached_minmax_idx = 1;
    buffer->cached_type = type;
    buffer->cached_offset = offset;
    buffer->cached_count = count;
    buffer->cached_min_idx = *min_idx;
    buffer->cached_max_idx = *max_idx;

    return 1;
}

void yagl_gles_buffer_bind(struct yagl_gles_buffer *buffer,
                           GLenum type,
                           int need_convert,
                           GLenum target)
{
    struct yagl_gles_buffer_part *bufpart = &buffer->default_part;

    if (need_convert) {
        switch (type) {
        case GL_BYTE:
            bufpart = &buffer->byte_part;
            break;
        case GL_FIXED:
            bufpart = &buffer->fixed_part;
            break;
        }
    }

    yagl_host_glBindBuffer(target, bufpart->global_name);
}

void yagl_gles_buffer_transfer(struct yagl_gles_buffer *buffer,
                               GLenum type,
                               GLenum target,
                               int need_convert)
{
    if (need_convert) {
        switch (type) {
        case GL_BYTE:
            yagl_gles_buffer_transfer_internal(buffer,
                                               &buffer->byte_part.range_list,
                                               target,
                                               &yagl_gles_buffer_transfer_byte);
            break;
        case GL_FIXED:
            yagl_gles_buffer_transfer_internal(buffer,
                                               &buffer->fixed_part.range_list,
                                               target,
                                               &yagl_gles_buffer_transfer_fixed);
            break;
        }
    } else {
        yagl_gles_buffer_transfer_internal(buffer,
                                           &buffer->default_part.range_list,
                                           target,
                                           &yagl_gles_buffer_transfer_default);
    }
}

int yagl_gles_buffer_get_parameter(struct yagl_gles_buffer *buffer,
                                   GLenum pname,
                                   GLint *param)
{
    switch (pname) {
    case GL_BUFFER_SIZE:
        *param = buffer->size;
        break;
    case GL_BUFFER_USAGE:
        *param = buffer->usage;
        break;
    case GL_BUFFER_ACCESS_OES:
        *param = GL_WRITE_ONLY_OES;
        break;
    case GL_BUFFER_MAPPED_OES:
        *param = buffer->map_pointer != NULL;
        break;
    case GL_BUFFER_ACCESS_FLAGS:
        *param = buffer->map_access;
        break;
    case GL_BUFFER_MAP_OFFSET:
        *param = buffer->map_offset;
        break;
    case GL_BUFFER_MAP_LENGTH:
        *param = buffer->map_length;
        break;
    default:
        return 0;
    }

    return 1;
}

int yagl_gles_buffer_map(struct yagl_gles_buffer *buffer,
                         GLintptr offset,
                         GLsizeiptr length,
                         GLbitfield access)
{
    if (buffer->map_pointer) {
        return 0;
    }

    if ((offset < 0) || (length <= 0) || ((offset + length) > buffer->size)) {
        return 0;
    }

    if (((access & GL_MAP_INVALIDATE_BUFFER_BIT_EXT) != 0) ||
        (((access & GL_MAP_INVALIDATE_RANGE_BIT_EXT) != 0) &&
         (offset == 0) &&
         (length == buffer->size))) {
        yagl_range_list_clear(&buffer->default_part.range_list);
        yagl_range_list_clear(&buffer->fixed_part.range_list);
        yagl_range_list_clear(&buffer->byte_part.range_list);
    } else {
        yagl_gles_buffer_transfer_from_gpu(buffer);
    }

    buffer->map_pointer = buffer->data + offset;
    buffer->map_access = access;
    buffer->map_offset = offset;
    buffer->map_length = length;

    return 1;
}

int yagl_gles_buffer_mapped(struct yagl_gles_buffer *buffer)
{
    return buffer->map_pointer != NULL;
}

int yagl_gles_buffer_flush_mapped_range(struct yagl_gles_buffer *buffer,
                                        GLintptr offset,
                                        GLsizeiptr length)
{
    if (!buffer->map_pointer) {
        return 0;
    }

    if ((offset < 0) || (length < 0) || ((offset + length) > buffer->map_length)) {
        return 0;
    }

    if ((buffer->map_access & GL_MAP_FLUSH_EXPLICIT_BIT_EXT) == 0) {
        return 0;
    }

    if (length == 0) {
        return 1;
    }

    yagl_range_list_add(&buffer->default_part.range_list, buffer->map_offset + offset, length);
    yagl_range_list_add(&buffer->fixed_part.range_list, buffer->map_offset + offset, length);
    yagl_range_list_add(&buffer->byte_part.range_list, buffer->map_offset + offset, length);

    buffer->cached_minmax_idx = 0;

    return 1;
}

void yagl_gles_buffer_unmap(struct yagl_gles_buffer *buffer)
{
    if (!buffer->map_pointer) {
        return;
    }

    if (((buffer->map_access & GL_MAP_WRITE_BIT_EXT) != 0) &&
        ((buffer->map_access & GL_MAP_FLUSH_EXPLICIT_BIT_EXT) == 0)) {
        yagl_range_list_add(&buffer->default_part.range_list, buffer->map_offset, buffer->map_length);
        yagl_range_list_add(&buffer->fixed_part.range_list, buffer->map_offset, buffer->map_length);
        yagl_range_list_add(&buffer->byte_part.range_list, buffer->map_offset, buffer->map_length);

        buffer->cached_minmax_idx = 0;
    }

    buffer->map_pointer = NULL;
    buffer->map_access = 0;
    buffer->map_offset = 0;
    buffer->map_length = 0;
}

void yagl_gles_buffer_set_bound(struct yagl_gles_buffer *buffer)
{
    buffer->was_bound = 1;
}

int yagl_gles_buffer_was_bound(struct yagl_gles_buffer *buffer)
{
    return buffer->was_bound;
}

int yagl_gles_buffer_is_cpu_dirty(struct yagl_gles_buffer *buffer,
                                  GLenum type,
                                  int need_convert)
{
    struct yagl_gles_buffer_part *bufpart = &buffer->default_part;

    if (need_convert) {
        switch (type) {
        case GL_BYTE:
            bufpart = &buffer->byte_part;
            break;
        case GL_FIXED:
            bufpart = &buffer->fixed_part;
            break;
        }
    }

    return yagl_range_list_size(&bufpart->range_list) > 0;
}

void yagl_gles_buffer_set_gpu_dirty(struct yagl_gles_buffer *buffer,
                                    GLint offset,
                                    GLint size)
{
    if ((offset < 0) || (size < 0) || ((offset + size) > buffer->size)) {
        assert(0);
        return;
    }

    if (size == 0) {
        return;
    }

    yagl_range_list_add(&buffer->gpu_dirty_list, offset, size);

    buffer->cached_minmax_idx = 0;
}

int yagl_gles_buffer_copy_gpu(struct yagl_gles_buffer *from_buffer,
                              GLenum from_target,
                              struct yagl_gles_buffer *to_buffer,
                              GLenum to_target,
                              GLint from_offset,
                              GLint to_offset,
                              GLint size)
{
    if ((from_offset < 0) || (to_offset < 0) || (size < 0) ||
        ((from_offset + size) > from_buffer->size) ||
        ((to_offset + size) > to_buffer->size)) {
        return 0;
    }

    if ((from_buffer == to_buffer) &&
        (abs(to_offset - from_offset) < size)) {
        return 0;
    }

    if (size != 0) {
        yagl_host_glCopyBufferSubData(from_target, to_target,
                                      from_offset, to_offset, size);
    }

    return 1;
}
