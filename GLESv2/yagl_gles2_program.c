/*
 * YaGL
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Stanislav Vorobiov <s.vorobiov@samsung.com>
 * Jinhyung Jo <jinhyung.jo@samsung.com>
 * YeongKyoon Lee <yeongkyoon.lee@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * - S-Core Co., Ltd
 *
 */

#include "GLES2/gl2.h"
#include "yagl_gles2_program.h"
#include "yagl_gles2_shader.h"
#include "yagl_gles2_utils.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_transport.h"
#include "yagl_log.h"
#include "yagl_utils.h"
#include "yagl_host_gles_calls.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

/*
 * We don't want to include GLES3/gl3.h here
 */
#define GL_INTERLEAVED_ATTRIBS 0x8C8C

struct yagl_gles2_location_v
{
    GLchar *name;

    uint32_t global_location;
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

static void yagl_gles2_transform_feedback_info_copy(
    struct yagl_gles2_transform_feedback_info *from,
    struct yagl_gles2_transform_feedback_info *to)
{
    GLuint i;

    yagl_gles2_transform_feedback_info_reset(to);

    memcpy(to, from, sizeof(*from));

    if (from->num_varyings > 0) {
        to->varyings = yagl_malloc(to->num_varyings * sizeof(*to->varyings));

        for (i = 0; i < from->num_varyings; ++i) {
            memcpy(&to->varyings[i],
                   &from->varyings[i],
                   sizeof(from->varyings[0]));

            if (from->varyings[i].name) {
                to->varyings[i].name = yagl_malloc(from->varyings[i].name_size);
                memcpy(to->varyings[i].name,
                       from->varyings[i].name,
                       from->varyings[i].name_size);
            }
        }
    }
}

static void yagl_gles2_program_reset_cached(struct yagl_gles2_program *program)
{
    int i;
    struct yagl_gles2_location_l *location_l, *tmp_l;

    for (i = 0; i < (int)program->num_active_attribs; ++i) {
        yagl_free(program->active_attribs[i].name);
    }
    yagl_free(program->active_attribs);
    program->active_attribs = NULL;

    for (i = 0; i < (int)program->num_active_uniforms; ++i) {
        yagl_free(program->active_uniforms[i].name);
    }
    yagl_free(program->active_uniforms);
    program->active_uniforms = NULL;

    for (i = 0; i < (int)program->num_active_uniform_blocks; ++i) {
        yagl_free(program->active_uniform_blocks[i].name);
        yagl_free(program->active_uniform_blocks[i].active_uniform_indices);
    }
    yagl_free(program->active_uniform_blocks);
    program->active_uniform_blocks = NULL;

    yagl_list_for_each_safe(struct yagl_gles2_location_l,
                            location_l,
                            tmp_l,
                            &program->frag_data_locations, list) {
        yagl_list_remove(&location_l->list);
        free(location_l->name);
        yagl_free(location_l);
    }

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

static void yagl_gles2_program_destroy(struct yagl_ref *ref)
{
    struct yagl_gles2_program *program = (struct yagl_gles2_program*)ref;

    yagl_gles2_program_reset_cached(program);

    yagl_gles2_transform_feedback_info_reset(&program->linked_transform_feedback_info);
    yagl_gles2_transform_feedback_info_reset(&program->transform_feedback_info);

    if (program->gen_locations) {
        yagl_vector_cleanup(&program->uniform_locations_v);
    }

    yagl_gles2_shader_release(program->fragment_shader);
    yagl_gles2_shader_release(program->vertex_shader);

    yagl_host_glDeleteObjects(&program->global_name, 1);

    yagl_object_cleanup(&program->base);

    yagl_free(program);
}

int yagl_gles2_program_translate_location(struct yagl_gles2_program *program,
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

void yagl_gles2_transform_feedback_info_reset(
    struct yagl_gles2_transform_feedback_info *transform_feedback_info)
{
    GLuint i;

    for (i = 0; i < transform_feedback_info->num_varyings; ++i) {
        yagl_free(transform_feedback_info->varyings[i].name);
    }
    yagl_free(transform_feedback_info->varyings);

    memset(transform_feedback_info, 0, sizeof(*transform_feedback_info));
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
    yagl_list_init(&program->frag_data_locations);

    program->transform_feedback_info.buffer_mode = GL_INTERLEAVED_ATTRIBS;

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
    GLint params[8];

    memset(&params[0], 0, sizeof(params));

    yagl_gles2_program_reset_cached(program);

    yagl_host_glLinkProgram(program->global_name,
                            params,
                            sizeof(params)/sizeof(params[0]),
                            NULL);

    program->linked = params[0];

    program->link_status = params[0];
    program->info_log_length = params[1];
    program->num_active_attribs = params[2];
    program->max_active_attrib_bufsize = params[3];
    program->num_active_uniforms = params[4];
    program->max_active_uniform_bufsize = params[5];
    program->num_active_uniform_blocks = params[6];
    program->max_active_uniform_block_bufsize = params[7];

    if (program->num_active_attribs > 0) {
        program->active_attribs = yagl_malloc0(program->num_active_attribs *
                                               sizeof(program->active_attribs[0]));
    }

    if (program->num_active_uniforms) {
        program->active_uniforms = yagl_malloc0(program->num_active_uniforms *
                                                sizeof(program->active_uniforms[0]));
    }

    if (program->num_active_uniform_blocks) {
        program->active_uniform_blocks = yagl_malloc0(program->num_active_uniform_blocks *
                                                      sizeof(program->active_uniform_blocks[0]));
    }

    yagl_gles2_transform_feedback_info_copy(
        &program->transform_feedback_info,
        &program->linked_transform_feedback_info);
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

void yagl_gles2_program_get_active_uniform(struct yagl_gles2_program *program,
                                           GLuint index,
                                           GLsizei bufsize,
                                           GLsizei *length,
                                           GLint *size,
                                           GLenum *type,
                                           GLchar *name)
{
    struct yagl_gles2_uniform_variable *var;

    YAGL_LOG_FUNC_SET(yagl_gles2_program_get_active_uniform);

    if (index >= program->num_active_uniforms) {
        assert(0);
        return;
    }

    var = &program->active_uniforms[index];

    if (!var->generic_fetched) {
        yagl_free(var->name);
        var->name = yagl_malloc(program->max_active_uniform_bufsize);
        var->name[0] = '\0';

        yagl_host_glGetActiveUniform(program->global_name,
                                     index,
                                     &var->size,
                                     &var->type,
                                     var->name,
                                     program->max_active_uniform_bufsize,
                                     &var->name_size);

        var->name_fetched = 1;
        var->generic_fetched = 1;
    }

    yagl_gles2_set_name(var->name, var->name_size,
                        bufsize,
                        length,
                        name);

    if (size) {
        *size = var->size;
    }

    if (type) {
        *type = var->type;
    }

    YAGL_LOG_DEBUG("Got uniform variable at index %u: name = %s, size = %d, type = 0x%X",
                   index,
                   ((bufsize > 0) ? name : NULL),
                   var->size,
                   var->type);
}

void yagl_gles2_program_get_active_attrib(struct yagl_gles2_program *program,
                                          GLuint index,
                                          GLsizei bufsize,
                                          GLsizei *length,
                                          GLint *size,
                                          GLenum *type,
                                          GLchar *name)
{
    struct yagl_gles2_attrib_variable *var;

    YAGL_LOG_FUNC_SET(yagl_gles2_program_get_active_attrib);

    if (index >= program->num_active_attribs) {
        assert(0);
        return;
    }

    var = &program->active_attribs[index];

    if (!var->fetched) {
        yagl_free(var->name);
        var->name = yagl_malloc(program->max_active_attrib_bufsize);
        var->name[0] = '\0';

        yagl_host_glGetActiveAttrib(program->global_name,
                                    index,
                                    &var->size,
                                    &var->type,
                                    var->name,
                                    program->max_active_attrib_bufsize,
                                    &var->name_size);

        var->fetched = 1;
    }

    yagl_gles2_set_name(var->name, var->name_size,
                        bufsize,
                        length,
                        name);

    if (size) {
        *size = var->size;
    }

    if (type) {
        *type = var->type;
    }

    YAGL_LOG_DEBUG("Got attrib variable at index %u: name = %s, size = %d, type = 0x%X",
                   index,
                   ((bufsize > 0) ? name : NULL),
                   var->size,
                   var->type);
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
