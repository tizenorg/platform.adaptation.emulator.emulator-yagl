#include "GLES2/gl2.h"
#include "yagl_gles2_shader.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"

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

void yagl_gles2_shader_source(struct yagl_gles2_shader *shader,
                              GLchar *source,
                              const GLchar *patched_source,
                              int patched_len)
{
    yagl_host_glShaderSource(shader->global_name,
                             patched_source,
                             patched_len + 1);

    yagl_free(shader->source);
    shader->source = source;
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
