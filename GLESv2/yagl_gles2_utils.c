#include "GLES2/gl2.h"
#include "yagl_gles2_utils.h"
#include <string.h>

void yagl_gles2_set_name(const GLchar *from, GLint from_size,
                         GLint bufsize,
                         GLint *length,
                         GLchar *name)
{
    GLint tmp_length;

    if (bufsize < 0) {
        bufsize = 0;
    }

    tmp_length = (bufsize <= from_size) ? bufsize : from_size;

    if (tmp_length > 0) {
        strncpy(name, from, tmp_length);
        name[tmp_length - 1] = '\0';
        --tmp_length;
    } else if (bufsize > 0) {
        name[0] = '\0';
    }

    if (length) {
        *length = tmp_length;
    }
}
