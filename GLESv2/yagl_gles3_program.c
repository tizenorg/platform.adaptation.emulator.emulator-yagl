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

static int yagl_gles3_get_uniform_block_params(struct yagl_gles2_uniform_block *block,
                                               GLenum pname,
                                               GLint *params)
{
    switch (pname) {
    case GL_UNIFORM_BLOCK_BINDING:
        *params = block->binding;
        return 1;
    case GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES:
        if (block->active_uniform_indices) {
            memcpy(params,
                   block->active_uniform_indices,
                   block->num_active_uniform_indices *
                   sizeof(block->active_uniform_indices[0]));
            return 1;
        }
        break;
    case GL_UNIFORM_BLOCK_DATA_SIZE:
        if (block->params_fetched) {
            *params = block->data_size;
            return 1;
        }
        break;
    case GL_UNIFORM_BLOCK_NAME_LENGTH:
        if (block->name_fetched || block->params_fetched) {
            *params = block->name_size;
            return 1;
        }
        break;
    case GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS:
        if (block->params_fetched) {
            *params = block->num_active_uniform_indices;
            return 1;
        }
        break;
    case GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER:
        if (block->params_fetched) {
            *params = block->referenced_by_vertex_shader;
            return 1;
        }
        break;
    case GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER:
        if (block->params_fetched) {
            *params = block->referenced_by_fragment_shader;
            return 1;
        }
        break;
    default:
        return 0;
    }
    return -1;
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

        YAGL_LOG_DEBUG("Program %u, got uniform parameters 0x%X for %d indices from cache",
                       program->global_name, pname, num_indices);

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

    YAGL_LOG_DEBUG("Program %u, got uniform parameters 0x%X for %d indices, %d from cache",
                   program->global_name, pname, num_indices,
                   (num_indices - num_fetch_indices));

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

        YAGL_LOG_DEBUG("Program %u, got uniform indices for %d names from cache",
                       program->global_name, num_names);

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

    YAGL_LOG_DEBUG("Program %u, got uniform indices for %d names, %d from cache",
                   program->global_name, num_names,
                   (num_names - num_fetch_names));
}

GLuint yagl_gles3_program_get_uniform_block_index(struct yagl_gles2_program *program,
                                                  const GLchar *block_name)
{
    int32_t block_name_size = strlen(block_name) + 1;
    GLuint i;
    struct yagl_gles2_uniform_block *block;

    YAGL_LOG_FUNC_SET(yagl_gles3_program_get_uniform_block_index);

    if (block_name_size > program->max_active_uniform_block_bufsize) {
        return GL_INVALID_INDEX;
    }

    for (i = 0; i < program->num_active_uniform_blocks; ++i) {
        block = &program->active_uniform_blocks[i];

        if (block->name_fetched && (strcmp(block->name, block_name) == 0)) {
            YAGL_LOG_DEBUG("Program %u, got uniform block index for %s = %u from cache",
                           program->global_name, block_name, i);

            return i;
        }
    }

    i = yagl_host_glGetUniformBlockIndex(program->global_name,
                                         block_name,
                                         block_name_size);

    if ((i != GL_INVALID_INDEX) && (i < program->num_active_uniform_blocks)) {
        block = &program->active_uniform_blocks[i];

        block->name_size = block_name_size;
        yagl_free(block->name);
        block->name = yagl_malloc(block_name_size);

        strcpy(block->name, block_name);

        block->name_fetched = 1;
    }

    YAGL_LOG_DEBUG("Program %u, got uniform block index for %s = %u",
                   program->global_name, block_name, i);

    return i;
}

void yagl_gles3_program_set_uniform_block_binding(struct yagl_gles2_program *program,
                                                  GLuint block_index,
                                                  GLuint block_binding)
{
    YAGL_LOG_FUNC_SET(yagl_gles3_program_set_uniform_block_binding);

    if (block_index >= program->num_active_uniform_blocks) {
        return;
    }

    yagl_host_glUniformBlockBinding(program->global_name,
                                    block_index,
                                    block_binding);

    program->active_uniform_blocks[block_index].binding = block_binding;

    YAGL_LOG_DEBUG("Program %u, setting uniform block binding %u to %u",
                   program->global_name, block_index, block_binding);
}

void yagl_gles3_program_get_active_uniform_block_name(struct yagl_gles2_program *program,
                                                      GLuint block_index,
                                                      GLsizei bufsize,
                                                      GLsizei *length,
                                                      GLchar *block_name)
{
    struct yagl_gles2_uniform_block *block;
    GLsizei tmp_length;

    YAGL_LOG_FUNC_SET(yagl_gles3_program_get_active_uniform_block_name);

    if (block_index >= program->num_active_uniform_blocks) {
        return;
    }

    block = &program->active_uniform_blocks[block_index];

    if (!block->name_fetched) {
        yagl_free(block->name);
        block->name = yagl_malloc(program->max_active_uniform_block_bufsize);
        block->name[0] = '\0';

        yagl_host_glGetActiveUniformBlockName(program->global_name,
                                              block_index,
                                              block->name,
                                              program->max_active_uniform_block_bufsize,
                                              &block->name_size);

        block->name_fetched = 1;
    }

    tmp_length = (bufsize <= block->name_size) ? bufsize : block->name_size;

    if (tmp_length > 0) {
        strncpy(block_name, block->name, tmp_length);
        block_name[tmp_length - 1] = '\0';
        --tmp_length;
    }

    if (length) {
        *length = tmp_length;
    }

    YAGL_LOG_DEBUG("Program %u, got active uniform block name at index %u = %s",
                   program->global_name,
                   block_index,
                   ((bufsize > 0) ? block_name : NULL));
}

int yagl_gles3_program_get_active_uniform_blockiv(struct yagl_gles2_program *program,
                                                  GLuint block_index,
                                                  GLenum pname,
                                                  GLint *params)
{
    struct yagl_gles2_uniform_block *block;
    int res;
    GLint *fetch_params;

    YAGL_LOG_FUNC_SET(yagl_gles3_program_get_active_uniform_blockiv);

    if (block_index >= program->num_active_uniform_blocks) {
        return 0;
    }

    block = &program->active_uniform_blocks[block_index];

    res = yagl_gles3_get_uniform_block_params(block,
                                              pname,
                                              params);

    if (res >= 0) {
        if (res > 0) {
            YAGL_LOG_DEBUG("Program %u, got active uniform block %u pname 0x%X from cache",
                           program->global_name,
                           block_index,
                           pname);
        }

        return res;
    }

    if (!block->params_fetched) {
        fetch_params = (GLint*)yagl_get_tmp_buffer(
            sizeof(fetch_params[0]) * 5);

        yagl_host_glGetActiveUniformBlockiv(program->global_name,
                                            block_index,
                                            0,
                                            fetch_params,
                                            5,
                                            NULL);

        block->data_size = fetch_params[0];
        block->name_size = fetch_params[1];
        block->num_active_uniform_indices = fetch_params[2];
        block->referenced_by_vertex_shader = fetch_params[3];
        block->referenced_by_fragment_shader = fetch_params[4];

        block->params_fetched = 1;
    }

    if (pname == GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES) {
        yagl_free(block->active_uniform_indices);

        block->active_uniform_indices = (GLuint*)yagl_get_tmp_buffer(
            block->num_active_uniform_indices *
            sizeof(block->active_uniform_indices[0]));

        yagl_host_glGetActiveUniformBlockiv(program->global_name,
                                            block_index,
                                            pname,
                                            (GLint*)block->active_uniform_indices,
                                            block->num_active_uniform_indices,
                                            NULL);
    }

    yagl_gles3_get_uniform_block_params(block, pname, params);

    YAGL_LOG_DEBUG("Program %u, got active uniform block %u pname 0x%X",
                   program->global_name,
                   block_index,
                   pname);

    return 1;
}
