#ifndef _YAGL_GLES_ARRAY_H_
#define _YAGL_GLES_ARRAY_H_

#include "yagl_types.h"

/*
 * This structure represents GLES array, such arrays are
 * typically set up by calls to 'glColorPointer', 'glVertexPointer',
 * 'glVertexAttribPointer', etc.
 *
 * Note that we can't call host 'glVertexAttribPointer' right away
 * when target 'glVertexAttribPointer' call is made. We can't just
 * transfer the data from target to host and feed it to
 * host 'glVertexAttribPointer' since the data that pointer points to
 * may be changed later, thus, we must do the transfer right before host
 * OpenGL will attempt to use it. Host OpenGL will attempt to use
 * the array pointer on 'glDrawArrays' and 'glDrawElements', thus, we must
 * do the transfer right before those calls. For this to work, we'll
 * store 'apply' function together with an array, this function will be
 * responsible for calling host 'glVertexAttribPointer', 'glColorPointer'
 * or whatever.
 */

struct yagl_gles_array;
struct yagl_gles_buffer;

/*
 * Calls the corresponding host array function.
 */
typedef void (*yagl_gles_array_apply_func)(struct yagl_gles_array */*array*/,
                                           uint32_t /*first*/,
                                           uint32_t /*count*/,
                                           const GLvoid */*ptr*/,
                                           void */*user_data*/);

struct yagl_gles_array
{
    GLuint index;
    GLint size;
    GLenum type;
    GLenum actual_type;
    int el_size;
    int actual_el_size;
    GLboolean normalized;
    GLsizei stride;
    GLsizei actual_stride;
    GLuint divisor;

    /*
     * Specifies if array data needs to be converted before transferring to
     * host OpenGL occurs.
     * This could be used with GL_FIXED or GL_BYTE types, then data is
     * converted to GL_FLOAT or GL_SHORT respectively. Setting this flag
     * when yagl_gles_array::type is not either GL_FIXED or GL_FLOAT is
     * programming error.
     */
    int need_convert;

    /*
     * Is array enabled by 'glEnableClientState'/'glEnableVertexAttribArray'.
     */
    int enabled;

    struct yagl_gles_buffer *vbo;

    union
    {
        const GLvoid *ptr;
        struct
        {
            GLint offset;
            GLint actual_offset;
        };
    };

    yagl_gles_array_apply_func apply;
    void *user_data;
};

void yagl_gles_array_init(struct yagl_gles_array *array,
                          GLuint index,
                          yagl_gles_array_apply_func apply,
                          void *user_data);

void yagl_gles_array_cleanup(struct yagl_gles_array *array);

void yagl_gles_array_enable(struct yagl_gles_array *array, int enable);

int yagl_gles_array_update(struct yagl_gles_array *array,
                           GLint size,
                           GLenum type,
                           int need_convert,
                           GLboolean normalized,
                           GLsizei stride,
                           const GLvoid *ptr);

int yagl_gles_array_update_vbo(struct yagl_gles_array *array,
                               GLint size,
                               GLenum type,
                               int need_convert,
                               GLboolean normalized,
                               GLsizei stride,
                               struct yagl_gles_buffer *vbo,
                               GLint offset);

void yagl_gles_array_set_divisor(struct yagl_gles_array *array, GLuint divisor);

void yagl_gles_array_transfer(struct yagl_gles_array *array,
                              uint32_t first,
                              uint32_t count,
                              GLsizei primcount);

#endif
