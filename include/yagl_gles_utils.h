#ifndef _YAGL_GLES_UTILS_H_
#define _YAGL_GLES_UTILS_H_

GLint yagl_get_integer(GLenum pname);
void yagl_update_arrays(void);
int yagl_get_el_size(GLenum type, int *el_size);
void yagl_update_vbo(void);
int yagl_gles_get_stride(GLsizei alignment,
                         GLsizei width,
                         GLenum format,
                         GLenum type,
                         GLsizei *stride);

static inline int yagl_get_index_size(GLenum type, int *index_size)
{
    switch (type) {
    case GL_UNSIGNED_BYTE:
        *index_size = 1;
        break;
    case GL_UNSIGNED_SHORT:
        *index_size = 2;
        break;
    default:
        return 0;
    }
    return 1;
}

#endif
