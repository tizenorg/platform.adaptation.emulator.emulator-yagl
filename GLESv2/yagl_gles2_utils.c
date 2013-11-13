#include "GLES2/gl2.h"
#include "yagl_gles2_utils.h"
#include "yagl_malloc.h"
#include <string.h>

struct yagl_gles2_shader_strtok
{
    char *buffer;
    int buffersize;
    const char *prev;
};

static const char *g_delim = " \t\n\r()[]{},;?:/%*&|^!+-=<>";

static const char *yagl_gles2_shader_opengl_strtok(struct yagl_gles2_shader_strtok *st,
                                                   const char *s,
                                                   int *n)
{
    if (!s) {
        if (!*(st->prev) || !*n) {
            if (st->buffer) {
                yagl_free(st->buffer);
                st->buffer = 0;
                st->buffersize = -1;
            }
            st->prev = 0;
            return 0;
        }
        s = st->prev;
    } else {
        if (st->buffer) {
            yagl_free(st->buffer);
            st->buffer = 0;
            st->buffersize = -1;
        }
        st->prev = s;
    }
    for (; *n && strchr(g_delim, *s); s++, (*n)--) {
        if (*s == '/' && *n > 1) {
            if (s[1] == '/') {
                do {
                    s++, (*n)--;
                } while (*n > 1 && s[1] != '\n' && s[1] != '\r');
            } else if (s[1] == '*') {
                do {
                    s++, (*n)--;
                } while (*n > 2 && (s[1] != '*' || s[2] != '/'));
                s++, (*n)--;
            }
        }
    }
    const char *e = s;
    if (s > st->prev) {
        s = st->prev;
    } else {
        for (; *n && *e && !strchr(g_delim, *e); e++, (*n)--);
    }
    st->prev = e;
    if (st->buffersize < e - s) {
        st->buffersize = e - s;
        if (st->buffer) {
            yagl_free(st->buffer);
        }
        st->buffer = yagl_malloc(st->buffersize + 1);
    }
    /* never return comment fields so caller does not need to handle them */
    char *p = st->buffer;
    int m = e - s;
    while (m > 0) {
        if (*s == '/' && m > 1) {
            if (s[1] == '/') {
                do {
                    s++, m--;
                } while (m > 1 && s[1] != '\n' && s[1] != '\r');
                s++, m--;
                continue;
            } else if (s[1] == '*') {
                do {
                    s++, m--;
                } while (m > 2 && (s[1] != '*' || s[2] != '/'));
                s += 3, m -= 3;
                continue;
            }
        }
        *(p++) = *(s++), m--;
    }
    *p = 0;
    return st->buffer;
}

char *yagl_gles2_shader_patch(const char *source,
                              int length,
                              int *patched_len)
{
    /* DISCLAIMER: this is not a full-blown shader parser but a simple
     * implementation which tries to remove the OpenGL ES shader
     * "precision" statements and precision qualifiers "lowp", "mediump"
     * and "highp" from the specified shader source. It also replaces
     * OpenGL ES shading language built-in constants gl_MaxVertexUniformVectors,
     * gl_MaxFragmentUniformVectors and gl_MaxVaryingVectors with corresponding
     * values from OpenGL shading language. */

    struct yagl_gles2_shader_strtok st;
    char *sp;

    st.buffer = NULL;
    st.buffersize = -1;
    st.prev = NULL;

    if (!length) {
        length = strlen(source);
    }
    *patched_len = 0;
    int patched_size = length;
    char *patched = yagl_malloc(patched_size + 1);
    const char *p = yagl_gles2_shader_opengl_strtok(&st, source, &length);
    for (; p; p = yagl_gles2_shader_opengl_strtok(&st, 0, &length)) {
        if (!strcmp(p, "lowp") || !strcmp(p, "mediump") || !strcmp(p, "highp")) {
            continue;
        } else if (!strcmp(p, "precision")) {
            while ((p = yagl_gles2_shader_opengl_strtok(&st, 0, &length)) && !strchr(p, ';'));
        } else {
            if (!strcmp(p, "gl_MaxVertexUniformVectors")) {
                p = "(gl_MaxVertexUniformComponents / 4)";
            } else if (!strcmp(p, "gl_MaxFragmentUniformVectors")) {
                p = "(gl_MaxFragmentUniformComponents / 4)";
            } else if (!strcmp(p, "gl_MaxVaryingVectors")) {
                p = "(gl_MaxVaryingFloats / 4)";
            }
            int new_len = strlen(p);
            if (*patched_len + new_len > patched_size) {
                patched_size *= 2;
                patched = yagl_realloc(patched, patched_size + 1);
            }
            memcpy(patched + *patched_len, p, new_len);
            *patched_len += new_len;
        }
    }
    patched[*patched_len] = 0;
    /* check that we don't leave dummy preprocessor lines */
    for (sp = patched; *sp;) {
        for (; *sp == ' ' || *sp == '\t'; sp++);
        if (!strncmp(sp, "#define", 7)) {
            for (p = sp + 7; *p == ' ' || *p == '\t'; p++);
            if (*p == '\n' || *p == '\r' || *p == '/') {
                memset(sp, 0x20, 7);
            }
        }
        for (; *sp && *sp != '\n' && *sp != '\r'; sp++);
        for (; *sp == '\n' || *sp == '\r'; sp++);
    }

    yagl_free(st.buffer);

    return patched;
}

int yagl_gles2_shader_has_version(const char *source,
                                  int *is_es3)
{
    if (is_es3) {
        *is_es3 = 0;
    }

    /*
     * Skip whitespaces and newlines.
     */
    while ((*source == '\r') ||
           (*source == '\n') ||
           (*source == ' ') ||
           (*source == '\t')) {
        ++source;
    }

    if (*source != '#') {
        return 0;
    }

    ++source;

    /*
     * Skip whitespaces.
     */
    while ((*source == ' ') ||
           (*source == '\t')) {
        ++source;
    }

    if (strncmp(source, "version", 7) != 0) {
        return 0;
    }

    source += 7;

    /*
     * Skip whitespaces.
     */
    while ((*source == ' ') ||
           (*source == '\t')) {
        ++source;
    }

    /*
     * Skip number.
     */
    while ((*source >= '0') &&
           (*source <= '9')) {
        ++source;
    }

    if ((*source != ' ') &&
        (*source != '\t')) {
        /*
         * number + space not found, just assume it has some version, but
         * it's certainly not ES3.
         */
        return 1;
    }

    /*
     * Skip whitespaces.
     */
    while ((*source == ' ') ||
           (*source == '\t')) {
        ++source;
    }

    if (strncmp(source, "es", 2) != 0) {
        /*
         * "es" not found, just assume it has some version, but
         * it's certainly not ES3.
         */
        return 1;
    }

    source += 2;

    if (is_es3) {
        if ((*source == '\0') ||
            (*source == '\n') ||
            (*source == '\r') ||
            (*source == ' ') ||
            (*source == '\t')) {
            *is_es3 = 1;
        }
    }

    return 1;
}

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
