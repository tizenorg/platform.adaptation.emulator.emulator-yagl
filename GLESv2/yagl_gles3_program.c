#include "GLES3/gl3.h"
#include "yagl_gles3_program.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_log.h"
#include "yagl_host_gles_calls.h"
#include <string.h>

static int yagl_gles3_get_uniform_param(struct yagl_gles2_uniform_variable *var,
                                        GLenum pname,
                                        GLint *param)
{
    switch (pname) {
    case GL_UNIFORM_TYPE:
        if (var->generic_fetched ||
            var->extended_fetched) {
            *param = var->type;
            return 1;
        }
        break;
    case GL_UNIFORM_SIZE:
        if (var->generic_fetched ||
            var->extended_fetched) {
            *param = var->size;
            return 1;
        }
        break;
    case GL_UNIFORM_NAME_LENGTH:
        if (var->name_fetched ||
            var->generic_fetched ||
            var->extended_fetched) {
            *param = var->name_size;
            return 1;
        }
        break;
    case GL_UNIFORM_BLOCK_INDEX:
        if (var->extended_fetched) {
            *param = var->block_index;
            return 1;
        }
        break;
    case GL_UNIFORM_OFFSET:
        if (var->extended_fetched) {
            *param = var->block_offset;
            return 1;
        }
        break;
    case GL_UNIFORM_ARRAY_STRIDE:
        if (var->extended_fetched) {
            *param = var->array_stride;
            return 1;
        }
        break;
    case GL_UNIFORM_MATRIX_STRIDE:
        if (var->extended_fetched) {
            *param = var->matrix_stride;
            return 1;
        }
        break;
    case GL_UNIFORM_IS_ROW_MAJOR:
        if (var->extended_fetched) {
            *param = var->is_row_major;
            return 1;
        }
        break;
    default:
        break;
    }
    return 0;
}

int yagl_gles3_program_get_active_uniformsiv(struct yagl_gles2_program *program,
                                             const GLuint *indices,
                                             int num_indices,
                                             GLenum pname,
                                             GLint *params)
{
    int i;
    GLuint *fetch_indices_positions;
    int num_fetch_indices = 0;
    struct yagl_gles2_uniform_variable *var;
    GLint *fetch_params;

    YAGL_LOG_FUNC_SET(yagl_gles3_program_get_active_uniformsiv);

    fetch_indices_positions = (GLuint*)yagl_get_tmp_buffer(
        num_indices * sizeof(fetch_indices_positions[0]) * 2);

    for (i = 0; i < num_indices; ++i) {
        if (indices[i] >= program->num_active_uniforms) {
            return 0;
        }

        var = &program->active_uniforms[indices[i]];

        if (!yagl_gles3_get_uniform_param(var, pname, &params[i])) {
            fetch_indices_positions[num_fetch_indices] = indices[i];
            fetch_indices_positions[num_indices + num_fetch_indices] = i;
            ++num_fetch_indices;
        }
    }

    if (num_fetch_indices == 0) {
        /*
         * Everything read from cache, return.
         */

        YAGL_LOG_DEBUG("Got uniform parameters 0x%X for %d indices from cache",
                       pname, num_indices);

        return 1;
    }

    fetch_params = (GLint*)yagl_get_tmp_buffer2(
        num_fetch_indices * sizeof(fetch_params[0]) * 8);

    yagl_host_glGetActiveUniformsiv(program->global_name,
                                    fetch_indices_positions,
                                    num_fetch_indices,
                                    fetch_params,
                                    num_fetch_indices * 8, NULL);

    for (i = 0; i < num_fetch_indices; ++i) {
        var = &program->active_uniforms[fetch_indices_positions[i]];

        var->type = fetch_params[i + (num_fetch_indices * 0)];
        var->size = fetch_params[i + (num_fetch_indices * 1)];
        var->name_size = fetch_params[i + (num_fetch_indices * 2)];
        var->block_index = fetch_params[i + (num_fetch_indices * 3)];
        var->block_offset = fetch_params[i + (num_fetch_indices * 4)];
        var->array_stride = fetch_params[i + (num_fetch_indices * 5)];
        var->matrix_stride = fetch_params[i + (num_fetch_indices * 6)];
        var->is_row_major = fetch_params[i + (num_fetch_indices * 7)];

        var->extended_fetched = 1;

        yagl_gles3_get_uniform_param(var,
             pname,
             &params[fetch_indices_positions[num_indices + i]]);
    }

    YAGL_LOG_DEBUG("Got uniform parameters 0x%X for %d indices, %d from cache",
                   pname, num_indices, (num_indices - num_fetch_indices));

    return 1;
}

void yagl_gles3_program_get_uniform_indices(struct yagl_gles2_program *program,
                                            const GLchar *const *names,
                                            int num_names,
                                            GLuint *indices)
{
    struct yagl_gles2_uniform_variable *var;
    GLchar *fetch_names;
    int num_fetch_names = 0;
    int *fetch_positions;
    GLuint *fetch_indices;
    int i, j;

    YAGL_LOG_FUNC_SET(yagl_gles3_program_get_uniform_indices);

    fetch_names = (GLchar*)yagl_get_tmp_buffer(
        program->max_active_uniform_bufsize * num_names);

    fetch_positions = (int*)yagl_get_tmp_buffer2(
        num_names * sizeof(fetch_positions[0]));

    for (i = 0; i < num_names; ++i) {
        int found = 0;

        if ((strlen(names[i]) + 1) > program->max_active_uniform_bufsize) {
            indices[i] = GL_INVALID_INDEX;
            continue;
        }

        for (j = 0; j < program->num_active_uniforms; ++j) {
            var = &program->active_uniforms[j];

            if (var->name_fetched && (strcmp(var->name, names[i]) == 0)) {
                indices[i] = j;
                found = 1;
                break;
            }
        }

        if (!found) {
            strcpy(fetch_names +
                   (program->max_active_uniform_bufsize * num_fetch_names),
                   names[i]);
            fetch_positions[num_fetch_names] = i;
            ++num_fetch_names;
        }
    }

    if (num_fetch_names == 0) {
        /*
         * Everything read from cache, return.
         */

        YAGL_LOG_DEBUG("Got uniform indices for %d names from cache",
                       num_names);

        return;
    }

    fetch_indices = (GLuint*)yagl_get_tmp_buffer3(
        num_fetch_names * sizeof(fetch_indices[0]));

    yagl_host_glGetUniformIndices(program->global_name,
        fetch_names,
        program->max_active_uniform_bufsize * num_fetch_names,
        fetch_indices,
        num_fetch_names,
        NULL);

    for (i = 0; i < num_fetch_names; ++i) {
        indices[fetch_positions[i]] = fetch_indices[i];

        if ((fetch_indices[i] == GL_INVALID_INDEX) ||
            (fetch_indices[i] >= program->num_active_uniforms)) {
            continue;
        }

        var = &program->active_uniforms[fetch_indices[i]];

        var->name_size = strlen(names[fetch_positions[i]]) + 1;
        yagl_free(var->name);
        var->name = yagl_malloc(var->name_size);

        strcpy(var->name, names[fetch_positions[i]]);

        var->name_fetched = 1;
    }

    YAGL_LOG_DEBUG("Got uniform indices for %d names, %d from cache",
                   num_names, (num_names - num_fetch_names));
}
