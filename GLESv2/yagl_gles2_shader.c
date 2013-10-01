#include "GLES2/gl2.h"
#include "yagl_gles2_shader.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"
#include <string.h>

struct yagl_gles2_shader_strtok
{
    char *buffer;
    int buffersize;
    const char *prev;
};

static void yagl_gles2_shader_destroy(struct yagl_ref *ref)
{
    struct yagl_gles2_shader *shader = (struct yagl_gles2_shader*)ref;

    yagl_free(shader->source);

    yagl_host_glDeleteObjects(&shader->global_name, 1);

    yagl_object_cleanup(&shader->base);

    yagl_free(shader);
}

struct yagl_gles2_shader *yagl_gles2_shader_create(GLenum type)
{
    struct yagl_gles2_shader *shader;

    shader = yagl_malloc0(sizeof(*shader));

    yagl_object_init(&shader->base, &yagl_gles2_shader_destroy);

    shader->is_shader = 1;
    shader->global_name = yagl_get_global_name();
    shader->type = type;

    yagl_host_glCreateShader(shader->global_name, type);

    return shader;
}

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

static char *yagl_gles2_shader_patch(const char *source,
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

void yagl_gles2_shader_source(struct yagl_gles2_shader *shader,
                              GLchar *string)
{
    GLint patched_len = 0;
    GLchar *patched_string = yagl_gles2_shader_patch(string,
                                                     strlen(string),
                                                     &patched_len);

    /*
     * On some GPUs (like Ivybridge Desktop) it's necessary to add
     * "#version" directive as the first line of the shader, otherwise
     * some of the features might not be available to the shader.
     *
     * For example, on Ivybridge Desktop, if we don't add the "#version"
     * line to the fragment shader then "gl_PointCoord"
     * won't be available.
     */

    if (strstr(patched_string, "#version") == NULL) {
        patched_len += sizeof("#version 120\n\n") - 1;
        char *tmp = yagl_malloc(patched_len + 1);
        strcpy(tmp, "#version 120\n\n");
        strcat(tmp, patched_string);
        yagl_free(patched_string);
        patched_string = tmp;
    }

    yagl_host_glShaderSource(shader->global_name,
                             patched_string,
                             patched_len + 1);

    yagl_free(patched_string);

    yagl_free(shader->source);
    shader->source = string;
}

void yagl_gles2_shader_acquire(struct yagl_gles2_shader *shader)
{
    if (shader) {
        yagl_object_acquire(&shader->base);
    }
}

void yagl_gles2_shader_release(struct yagl_gles2_shader *shader)
{
    if (shader) {
        yagl_object_release(&shader->base);
    }
}
