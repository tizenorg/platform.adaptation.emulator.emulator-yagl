#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "yagl_gles_array.h"
#include "yagl_gles_buffer.h"
#include "yagl_state.h"
#include "yagl_utils.h"
#include "yagl_host_gles_calls.h"
#include <string.h>
#include <assert.h>

/*
 * We can't include GL/glext.h here
 */
#define GL_HALF_FLOAT 0x140B
#define GL_INT_2_10_10_10_REV 0x8D9F

static __inline int yagl_get_el_size(GLenum type,
                                     int integer,
                                     int *el_size,
                                     GLenum *actual_type)
{
    if (actual_type) {
        *actual_type = type;
    }

    switch (type) {
    case GL_BYTE:
        *el_size = 1;
        return 1;
    case GL_UNSIGNED_BYTE:
        *el_size = 1;
        return 1;
    case GL_SHORT:
        *el_size = 2;
        return 1;
    case GL_UNSIGNED_SHORT:
        *el_size = 2;
        return 1;
    case GL_INT:
        *el_size = 4;
        return 1;
    case GL_UNSIGNED_INT:
        *el_size = 4;
        return 1;
    default:
        break;
    }

    if (integer) {
        return 0;
    }

    switch (type) {
    case GL_FLOAT:
        *el_size = 4;
        break;
    case GL_HALF_FLOAT_OES:
    case GL_HALF_FLOAT:
        *el_size = 2;
        if (actual_type) {
            *actual_type = GL_HALF_FLOAT;
        }
        break;
    case GL_FIXED:
        *el_size = 4;
        break;
    case GL_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV_EXT:
        *el_size = 4;
        break;
    default:
        return 0;
    }

    return 1;
}

static __inline void yagl_get_actual(GLenum type,
                                     GLsizei stride,
                                     GLint offset,
                                     int need_convert,
                                     GLenum *actual_type,
                                     GLsizei *actual_stride,
                                     GLint *actual_offset)
{
    *actual_type = type;
    *actual_stride = stride;

    if (actual_offset) {
        *actual_offset = offset;
    }

    if (!need_convert) {
        return;
    }

    switch (type) {
    case GL_BYTE:
        *actual_type = GL_SHORT;
        *actual_stride *= 2;
        if (actual_offset) {
            *actual_offset *= 2;
        }
        break;
    case GL_FIXED:
        *actual_type = GL_FLOAT;
        break;
    default:
        break;
    }
}

/*
 * GLES1 could use GL_FIXED data type, which is not supported by host OpenGL.
 * Also, GLES1 glTexCoordPointer and glVertexPointer could use GL_BYTE type
 * for data while host OpenGL doesn't support this type for these functions.
 * Conversion to host-acceptable data type is required for either of these
 * cases.
 * @{
 */

static void *yagl_gles_array_byte_to_short(struct yagl_gles_array *array,
                                           uint32_t first,
                                           uint32_t count)
{
    uint8_t *converted;
    uint32_t i, j;

    converted = yagl_get_tmp_buffer((first + count) * array->actual_stride);

    for (i = first; i < (first + count); ++i) {
        for (j = 0; j < array->size; ++j) {
            *((GLshort*)(converted + (i * array->actual_stride)) + j) =
                *((GLbyte*)(array->ptr + (i * array->stride)) + j);
        }
    }

    return converted;
}

static void *yagl_gles_array_fixed_to_float(struct yagl_gles_array *array,
                                            uint32_t first,
                                            uint32_t count)
{
    uint8_t *converted;
    uint32_t i, j;

    converted = yagl_get_tmp_buffer((first + count) * array->actual_stride);

    for (i = first; i < (first + count); ++i) {
        for (j = 0; j < array->size; ++j) {
            *((GLfloat*)(converted + (i * array->actual_stride)) + j) =
                yagl_fixed_to_float(*((GLfixed*)(array->ptr + (i * array->stride)) + j));
        }
    }

    return converted;
}

/*
 * @}
 */

static void yagl_gles_array_reset(struct yagl_gles_array *array)
{
    if (array->vbo) {
        yagl_gles_buffer_release(array->vbo);
        array->vbo = NULL;
        array->offset = 0;
        array->actual_offset = 0;
    } else {
        array->ptr = NULL;
    }
}

void yagl_gles_array_init(struct yagl_gles_array *array,
                          GLuint index,
                          yagl_gles_array_apply_func apply,
                          void *user_data)
{
    memset(array, 0, sizeof(*array));

    array->index = index;
    array->apply = apply;
    array->user_data = user_data;
}

void yagl_gles_array_cleanup(struct yagl_gles_array *array)
{
    yagl_gles_array_reset(array);
}

void yagl_gles_array_enable(struct yagl_gles_array *array, int enable)
{
    array->enabled = enable;
}

int yagl_gles_array_update(struct yagl_gles_array *array,
                           GLint size,
                           GLenum type,
                           int need_convert,
                           GLboolean normalized,
                           GLsizei stride,
                           const GLvoid *ptr,
                           int integer)
{
    if (!yagl_get_el_size(type, integer, &array->el_size, &type)) {
        return 0;
    }

    yagl_gles_array_reset(array);

    yagl_get_actual(type, stride, 0, need_convert,
                    &array->actual_type,
                    &array->actual_stride,
                    NULL);

    yagl_get_el_size(array->actual_type, integer, &array->actual_el_size, NULL);

    array->size = size;
    array->type = type;
    array->need_convert = need_convert;
    array->normalized = normalized;
    array->stride = stride;

    if (!array->stride) {
        array->stride = array->size * array->el_size;
    }

    if (!array->actual_stride) {
        array->actual_stride = array->size * array->actual_el_size;
    }

    array->integer = integer;

    array->ptr = ptr;

    if (need_convert) {
        assert((type == GL_FIXED) || (type == GL_BYTE));
    }

    return 1;
}

int yagl_gles_array_update_vbo(struct yagl_gles_array *array,
                               GLint size,
                               GLenum type,
                               int need_convert,
                               GLboolean normalized,
                               GLsizei stride,
                               struct yagl_gles_buffer *vbo,
                               GLint offset,
                               int integer)
{
    if (!yagl_get_el_size(type, integer, &array->el_size, &type)) {
        return 0;
    }

    yagl_gles_buffer_acquire(vbo);

    yagl_gles_array_reset(array);

    yagl_get_actual(type, stride, offset, need_convert,
                    &array->actual_type,
                    &array->actual_stride,
                    &array->actual_offset);

    yagl_get_el_size(array->actual_type, integer, &array->actual_el_size, NULL);

    array->size = size;
    array->type = type;
    array->need_convert = need_convert;
    array->normalized = normalized;
    array->stride = stride;

    if (!array->stride) {
        array->stride = array->size * array->el_size;
    }

    if (!array->actual_stride) {
        array->actual_stride = array->size * array->actual_el_size;
    }

    array->integer = integer;

    array->vbo = vbo;
    array->offset = offset;

    if (need_convert) {
        assert(type == GL_FIXED || type == GL_BYTE);
    }

    yagl_gles_array_apply(array);

    return 1;
}

void yagl_gles_array_apply(struct yagl_gles_array *array)
{
    if (array->vbo) {
        yagl_gles_buffer_bind(array->vbo,
                              array->type,
                              array->need_convert,
                              GL_ARRAY_BUFFER);
        array->apply(array, 0, 0, NULL, array->user_data);
        yagl_host_glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void yagl_gles_array_set_divisor(struct yagl_gles_array *array, GLuint divisor)
{
    array->divisor = divisor;

    yagl_host_glVertexAttribDivisor(array->index, divisor);
}

void yagl_gles_array_transfer(struct yagl_gles_array *array,
                              uint32_t first,
                              uint32_t count,
                              GLsizei primcount)
{
    if (!array->enabled) {
        return;
    }

    if (array->vbo) {
        if (yagl_gles_buffer_is_cpu_dirty(array->vbo,
                                          array->type,
                                          array->need_convert)) {
            yagl_gles_buffer_bind(array->vbo,
                                  array->type,
                                  array->need_convert,
                                  GL_ARRAY_BUFFER);
            yagl_gles_buffer_transfer(array->vbo,
                                      array->type,
                                      GL_ARRAY_BUFFER,
                                      array->need_convert);
            yagl_host_glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    } else if (array->ptr) {
        const GLvoid *ptr = array->ptr;

        if ((array->divisor > 0) && (primcount >= 0)) {
            first = 0;
            count = ((primcount - 1) / array->divisor) + 1;
        }

        if (array->need_convert) {
            switch (array->type) {
            case GL_BYTE:
                ptr = yagl_gles_array_byte_to_short(array, first, count);
                break;
            case GL_FIXED:
                ptr = yagl_gles_array_fixed_to_float(array, first, count);
                break;
            }
        }

        array->apply(array, first, count, ptr, array->user_data);
    } else {
        /*
         * Transferring enabled array which has no pointer, typically we should
         * just crash.
         */

        assert(0);
        *((int*)NULL) = 0xDEADBEEF;
    }
}
