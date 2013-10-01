#include "GLES2/gl2.h"
#include "yagl_gles2_program.h"
#include "yagl_gles2_shader.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_transport.h"
#include "yagl_log.h"
#include "yagl_utils.h"
#include "yagl_host_gles_calls.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

struct yagl_gles2_location_l
{
    struct yagl_list list;

    int location;

    GLchar *name;
};

struct yagl_gles2_location_v
{
    GLchar *name;

    uint32_t global_location;
};

struct yagl_gles2_variable
{
    int enabled;
    GLchar *name;
    GLenum type;
    GLint size;
};

static pthread_once_t g_gen_locations_init = PTHREAD_ONCE_INIT;

static pthread_mutex_t g_gen_locations_mutex;
static uint32_t g_gen_locations_next = 0;

static void yagl_gen_locations_init()
{
    yagl_mutex_init(&g_gen_locations_mutex);
}

static uint32_t yagl_gen_location()
{
    uint32_t ret;

    pthread_once(&g_gen_locations_init, yagl_gen_locations_init);

    pthread_mutex_lock(&g_gen_locations_mutex);

    ret = g_gen_locations_next++;

    pthread_mutex_unlock(&g_gen_locations_mutex);

    return ret;
}

static void yagl_gles2_program_reset_cached(struct yagl_gles2_program *program)
{
    int i;
    struct yagl_gles2_variable *vars;
    struct yagl_gles2_location_l *location_l, *tmp_l;

    vars = yagl_vector_data(&program->active_attribs);
    for (i = 0; i < yagl_vector_size(&program->active_attribs); ++i) {
        yagl_free(vars[i].name);
    }
    yagl_vector_resize(&program->active_attribs, 0);

    vars = yagl_vector_data(&program->active_uniforms);
    for (i = 0; i < yagl_vector_size(&program->active_uniforms); ++i) {
        yagl_free(vars[i].name);
    }
    yagl_vector_resize(&program->active_uniforms, 0);

    yagl_list_for_each_safe(struct yagl_gles2_location_l,
                            location_l,
                            tmp_l,
                            &program->attrib_locations, list) {
        yagl_list_remove(&location_l->list);
        free(location_l->name);
        yagl_free(location_l);
    }

    if (program->gen_locations) {
        struct yagl_gles2_location_v *locations =
            yagl_vector_data(&program->uniform_locations_v);
        uint32_t *tmp = (uint32_t*)yagl_get_tmp_buffer(
            yagl_vector_size(&program->uniform_locations_v) * sizeof(uint32_t));

        for (i = 0; i < yagl_vector_size(&program->uniform_locations_v); ++i) {
            tmp[i] = locations[i].global_location;
            free(locations[i].name);
        }

        yagl_host_glDeleteUniformLocationsYAGL(tmp,
                                               yagl_vector_size(&program->uniform_locations_v));

        yagl_vector_resize(&program->uniform_locations_v, 0);
    } else {
        yagl_list_for_each_safe(struct yagl_gles2_location_l,
                                location_l,
                                tmp_l,
                                &program->uniform_locations_l, list) {
            yagl_list_remove(&location_l->list);
            free(location_l->name);
            yagl_free(location_l);
        }
    }
}

static __inline int yagl_gles2_program_translate_location(struct yagl_gles2_program *program,
                                                          GLint location,
                                                          uint32_t *global_location)
{
    struct yagl_gles2_location_v *locations;

    if (program->gen_locations) {
        if ((location < 0) ||
            (location >= yagl_vector_size(&program->uniform_locations_v))) {
            return 0;
        }

        locations = yagl_vector_data(&program->uniform_locations_v);

        *global_location = locations[location].global_location;
    } else {
        *global_location = location;
    }

    return 1;
}

typedef GLboolean (*get_active_variable_func)(GLuint /*program*/,
                                              GLuint /*index*/,
                                              GLint */*size*/,
                                              GLenum */*type*/,
                                              GLchar */*name*/,
                                              int32_t /*name_maxcount*/,
                                              int32_t */*name_count*/);

static int yagl_gles2_program_get_active_variable(GLuint program,
                                                  get_active_variable_func func,
                                                  struct yagl_vector *active_variables,
                                                  GLuint index,
                                                  GLsizei bufsize,
                                                  GLsizei *length,
                                                  GLint *size,
                                                  GLenum *type,
                                                  GLchar *name)
{
    struct yagl_gles2_variable *var = NULL;
    GLsizei tmp_length = 0;
    GLint tmp_size;
    GLenum tmp_type;
    GLuint num_active_variables = (GLuint)yagl_vector_size(active_variables);

    YAGL_LOG_FUNC_SET(yagl_gles2_program_get_active_variable);

    if (index < num_active_variables) {
        var = yagl_vector_data(active_variables);
        var += index;
    }

    if (var && var->enabled) {
        tmp_length = strlen(var->name) + 1;
        tmp_length = (bufsize <= tmp_length) ? bufsize : tmp_length;

        if (tmp_length > 0) {
            strncpy(name, var->name, tmp_length);
            name[tmp_length - 1] = '\0';
            --tmp_length;
        }

        tmp_size = var->size;
        tmp_type = var->type;
    } else {
        /*
         * We allocate a new buffer here which is 1 byte
         * bigger than the one provided by 'name'.
         * This is in order to detect if we've actually got
         * the whole name (which is almost always the case)
         */

        GLchar *tmp_name = yagl_malloc(bufsize + 1);

        if (!func(program,
                  index,
                  &tmp_size,
                  &tmp_type,
                  tmp_name,
                  bufsize + 1,
                  &tmp_length)) {
            /*
             * Error, do nothing.
             */
            yagl_free(tmp_name);
            YAGL_LOG_ERROR("Cannot get variable at index %u", index);
            return 0;
        }

        if (tmp_length <= bufsize) {
            /*
             * Number of bytes returned can fit into 'bufsize', this
             * means that we've also fetched whole variable name, we
             * can cache it.
             */

            GLuint new_size;

            assert(tmp_length > 0);

            strncpy(name, tmp_name, tmp_length);

            new_size = num_active_variables;

            if (index >= num_active_variables) {
                new_size = index + 1;
            }

            yagl_vector_resize(active_variables, new_size);

            var = yagl_vector_data(active_variables);

            memset(var + num_active_variables, 0,
                   (new_size - num_active_variables) * sizeof(*var));

            var += index;

            var->enabled = 1;
            var->name = tmp_name;
            var->type = tmp_type;
            var->size = tmp_size;
        } else {
            YAGL_LOG_WARN("Cannot cache variable, whole name not fetched");

            /*
             * Number of bytes returned is bufsize + 1, it's
             * not guaranteed that we've got whole variable name,
             * don't cache.
             */
            if (bufsize > 0) {
                assert(tmp_length > 1);

                strncpy(name, tmp_name, bufsize);
                name[bufsize - 1] = '\0';

                /*
                 * Don't count that extra byte.
                 */
                --tmp_length;
            }

            yagl_free(tmp_name);
        }

        /*
         * 'tmp_length' is currently in bytes, but we need chars.
         */
        --tmp_length;
    }

    if (length) {
        *length = tmp_length;
    }

    if (size) {
        *size = tmp_size;
    }

    if (type) {
        *type = tmp_type;
    }

    YAGL_LOG_DEBUG("Got variable at index %u: name = %s, size = %d, type = 0x%X",
                   index,
                   ((bufsize > 0) ? name : NULL),
                   tmp_size,
                   tmp_type);

    return 1;
}

static void yagl_gles2_program_destroy(struct yagl_ref *ref)
{
    struct yagl_gles2_program *program = (struct yagl_gles2_program*)ref;

    yagl_gles2_program_reset_cached(program);

    yagl_vector_cleanup(&program->active_attribs);
    yagl_vector_cleanup(&program->active_uniforms);

    if (program->gen_locations) {
        yagl_vector_cleanup(&program->uniform_locations_v);
    }

    yagl_gles2_shader_release(program->fragment_shader);
    yagl_gles2_shader_release(program->vertex_shader);

    yagl_host_glDeleteObjects(&program->global_name, 1);

    yagl_object_cleanup(&program->base);

    yagl_free(program);
}

struct yagl_gles2_program *yagl_gles2_program_create(int gen_locations)
{
    struct yagl_gles2_program *program;

    program = yagl_malloc0(sizeof(*program));

    yagl_object_init(&program->base, &yagl_gles2_program_destroy);

    program->is_shader = 0;
    program->gen_locations = gen_locations;
    program->global_name = yagl_get_global_name();

    if (gen_locations) {
        yagl_vector_init(&program->uniform_locations_v,
                         sizeof(struct yagl_gles2_location_v),
                         0);
    } else {
        yagl_list_init(&program->uniform_locations_l);
    }

    yagl_list_init(&program->attrib_locations);

    yagl_vector_init(&program->active_uniforms,
                     sizeof(struct yagl_gles2_variable),
                     0);
    yagl_vector_init(&program->active_attribs,
                     sizeof(struct yagl_gles2_variable),
                     0);

    yagl_host_glCreateProgram(program->global_name);

    return program;
}

int yagl_gles2_program_attach_shader(struct yagl_gles2_program *program,
                                     struct yagl_gles2_shader *shader)
{
    switch (shader->type) {
    case GL_VERTEX_SHADER:
        if (program->vertex_shader) {
            return 0;
        }
        yagl_gles2_shader_acquire(shader);
        program->vertex_shader = shader;
        break;
    case GL_FRAGMENT_SHADER:
        if (program->fragment_shader) {
            return 0;
        }
        yagl_gles2_shader_acquire(shader);
        program->fragment_shader = shader;
        break;
    default:
        return 0;
    }

    yagl_host_glAttachShader(program->global_name,
                             shader->global_name);

    return 1;
}

int yagl_gles2_program_detach_shader(struct yagl_gles2_program *program,
                                     struct yagl_gles2_shader *shader)
{
    if (program->vertex_shader == shader) {
        yagl_gles2_shader_release(program->vertex_shader);
        program->vertex_shader = NULL;
    } else if (program->fragment_shader == shader) {
        yagl_gles2_shader_release(program->fragment_shader);
        program->fragment_shader = NULL;
    } else {
        return 0;
    }

    yagl_host_glDetachShader(program->global_name,
                             shader->global_name);

    return 1;
}

void yagl_gles2_program_link(struct yagl_gles2_program *program)
{
    yagl_gles2_program_reset_cached(program);

    yagl_host_glLinkProgram(program->global_name);

    program->linked = 1;
}

int yagl_gles2_program_get_uniform_location(struct yagl_gles2_program *program,
                                            const GLchar *name)
{
    int ret = 0;

    if (program->gen_locations) {
        struct yagl_gles2_location_v *locations =
            yagl_vector_data(&program->uniform_locations_v);
        int i;
        struct yagl_gles2_location_v location;

        for (i = 0; i < yagl_vector_size(&program->uniform_locations_v); ++i) {
            if (strcmp(locations[i].name, name) == 0) {
                return i;
            }
        }

        location.name = strdup(name);
        location.global_location = yagl_gen_location();

        yagl_vector_push_back(&program->uniform_locations_v, &location);

        yagl_host_glGenUniformLocationYAGL(location.global_location,
                                           program->global_name,
                                           name,
                                           yagl_transport_string_count(name));

        ret = yagl_vector_size(&program->uniform_locations_v) - 1;
    } else {
        struct yagl_gles2_location_l *location;

        yagl_list_for_each(struct yagl_gles2_location_l,
                           location,
                           &program->uniform_locations_l, list) {
            if (strcmp(location->name, name) == 0) {
                return location->location;
            }
        }

        ret = yagl_host_glGetUniformLocation(program->global_name,
                                             name,
                                             yagl_transport_string_count(name));

        location = yagl_malloc(sizeof(*location));

        yagl_list_init(&location->list);
        location->name = strdup(name);
        location->location = ret;

        yagl_list_add_tail(&program->uniform_locations_l, &location->list);
    }

    return ret;
}

int yagl_gles2_program_get_attrib_location(struct yagl_gles2_program *program,
                                           const GLchar *name)
{
    struct yagl_gles2_location_l *location;
    int ret;

    yagl_list_for_each(struct yagl_gles2_location_l,
                       location,
                       &program->attrib_locations, list) {
        if (strcmp(location->name, name) == 0) {
            return location->location;
        }
    }

    ret = yagl_host_glGetAttribLocation(program->global_name,
                                        name,
                                        yagl_transport_string_count(name));

    location = yagl_malloc(sizeof(*location));

    yagl_list_init(&location->list);
    location->name = strdup(name);
    location->location = ret;

    yagl_list_add_tail(&program->attrib_locations, &location->list);

    return ret;
}

int yagl_gles2_program_get_active_uniform(struct yagl_gles2_program *program,
                                          GLuint index,
                                          GLsizei bufsize,
                                          GLsizei *length,
                                          GLint *size,
                                          GLenum *type,
                                          GLchar *name)
{
    return yagl_gles2_program_get_active_variable(program->global_name,
                                                  &yagl_host_glGetActiveUniform,
                                                  &program->active_uniforms,
                                                  index,
                                                  bufsize,
                                                  length,
                                                  size,
                                                  type,
                                                  name);
}

int yagl_gles2_program_get_active_attrib(struct yagl_gles2_program *program,
                                         GLuint index,
                                         GLsizei bufsize,
                                         GLsizei *length,
                                         GLint *size,
                                         GLenum *type,
                                         GLchar *name)
{
    return yagl_gles2_program_get_active_variable(program->global_name,
                                                  &yagl_host_glGetActiveAttrib,
                                                  &program->active_attribs,
                                                  index,
                                                  bufsize,
                                                  length,
                                                  size,
                                                  type,
                                                  name);
}

int yagl_gles2_program_get_uniformfv(struct yagl_gles2_program *program,
                                     GLint location,
                                     GLfloat *params)
{
    uint32_t global_location;
    GLfloat tmp[100]; // This fits all cases.
    int32_t num = 0;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glGetUniformfv(program->gen_locations,
                             program->global_name,
                             global_location,
                             tmp,
                             sizeof(tmp)/sizeof(tmp[0]),
                             &num);

    if (params) {
        memcpy(params, tmp, num * sizeof(tmp[0]));
    }

    return 1;
}

int yagl_gles2_program_get_uniformiv(struct yagl_gles2_program *program,
                                     GLint location,
                                     GLint *params)
{
    uint32_t global_location;
    GLint tmp[100]; // This fits all cases.
    int32_t num = 0;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glGetUniformiv(program->gen_locations,
                             program->global_name,
                             global_location,
                             tmp,
                             sizeof(tmp)/sizeof(tmp[0]),
                             &num);

    if (params) {
        memcpy(params, tmp, num * sizeof(tmp[0]));
    }

    return 1;
}

int yagl_gles2_program_uniform1f(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLfloat x)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform1f(program->gen_locations, global_location, x);

    return 1;
}

int yagl_gles2_program_uniform1fv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLfloat *v)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform1fv(program->gen_locations, global_location, v, count);

    return 1;
}

int yagl_gles2_program_uniform1i(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLint x)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform1i(program->gen_locations, global_location, x);

    return 1;
}

int yagl_gles2_program_uniform1iv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLint *v)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform1iv(program->gen_locations, global_location, v, count);

    return 1;
}

int yagl_gles2_program_uniform2f(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLfloat x,
                                 GLfloat y)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform2f(program->gen_locations, global_location, x, y);

    return 1;
}

int yagl_gles2_program_uniform2fv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLfloat *v)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform2fv(program->gen_locations, global_location, v, 2 * count);

    return 1;
}

int yagl_gles2_program_uniform2i(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLint x,
                                 GLint y)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform2i(program->gen_locations, global_location, x, y);

    return 1;
}

int yagl_gles2_program_uniform2iv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLint *v)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform2iv(program->gen_locations, global_location, v, 2 * count);

    return 1;
}

int yagl_gles2_program_uniform3f(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLfloat x,
                                 GLfloat y,
                                 GLfloat z)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform3f(program->gen_locations, global_location, x, y, z);

    return 1;
}

int yagl_gles2_program_uniform3fv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLfloat *v)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform3fv(program->gen_locations, global_location, v, 3 * count);

    return 1;
}

int yagl_gles2_program_uniform3i(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLint x,
                                 GLint y,
                                 GLint z)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform3i(program->gen_locations, global_location, x, y, z);

    return 1;
}

int yagl_gles2_program_uniform3iv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLint *v)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform3iv(program->gen_locations, global_location, v, 3 * count);

    return 1;
}

int yagl_gles2_program_uniform4f(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLfloat x,
                                 GLfloat y,
                                 GLfloat z,
                                 GLfloat w)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform4f(program->gen_locations, global_location, x, y, z, w);

    return 1;
}

int yagl_gles2_program_uniform4fv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLfloat *v)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform4fv(program->gen_locations, global_location, v, 4 * count);

    return 1;
}

int yagl_gles2_program_uniform4i(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLint x,
                                 GLint y,
                                 GLint z,
                                 GLint w)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform4i(program->gen_locations, global_location, x, y, z, w);

    return 1;
}

int yagl_gles2_program_uniform4iv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLint *v)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform4iv(program->gen_locations, global_location, v, 4 * count);

    return 1;
}

int yagl_gles2_program_uniform_matrix2fv(struct yagl_gles2_program *program,
                                         GLint location,
                                         GLsizei count,
                                         GLboolean transpose,
                                         const GLfloat *value)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniformMatrix2fv(program->gen_locations, global_location, transpose, value, 2 * 2 * count);

    return 1;
}

int yagl_gles2_program_uniform_matrix3fv(struct yagl_gles2_program *program,
                                         GLint location,
                                         GLsizei count,
                                         GLboolean transpose,
                                         const GLfloat *value)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniformMatrix3fv(program->gen_locations, global_location, transpose, value, 3 * 3 * count);

    return 1;
}

int yagl_gles2_program_uniform_matrix4fv(struct yagl_gles2_program *program,
                                         GLint location,
                                         GLsizei count,
                                         GLboolean transpose,
                                         const GLfloat *value)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniformMatrix4fv(program->gen_locations, global_location, transpose, value, 4 * 4 * count);

    return 1;
}

void yagl_gles2_program_acquire(struct yagl_gles2_program *program)
{
    if (program) {
        yagl_object_acquire(&program->base);
    }
}

void yagl_gles2_program_release(struct yagl_gles2_program *program)
{
    if (program) {
        yagl_object_release(&program->base);
    }
}
