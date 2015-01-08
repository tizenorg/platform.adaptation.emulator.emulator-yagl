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

#include "GLES3/gl3.h"
#include "yagl_gles3_program.h"
#include "yagl_gles2_utils.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_transport.h"
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

    yagl_gles2_set_name(block->name, block->name_size,
                        bufsize,
                        length,
                        block_name);

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

        block->active_uniform_indices = yagl_malloc(
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

void yagl_gles3_program_set_transform_feedback_varyings(struct yagl_gles2_program *program,
                                                        const GLchar *const *varyings,
                                                        GLuint num_varyings,
                                                        GLenum buffer_mode)
{
    GLchar *varyings_str = NULL;
    GLint varyings_size = 0;

    struct yagl_gles2_transform_feedback_info *tfi = &program->transform_feedback_info;

    yagl_gles2_transform_feedback_info_reset(tfi);

    if (num_varyings > 0) {
        GLuint i;
        GLchar *tmp;

        tfi->varyings = yagl_malloc0(num_varyings * sizeof(*tfi->varyings));

        for (i = 0; i < num_varyings; ++i) {
            tfi->varyings[i].name_size = strlen(varyings[i]) + 1;
            tfi->varyings[i].name = yagl_malloc(tfi->varyings[i].name_size);
            memcpy(tfi->varyings[i].name, varyings[i], tfi->varyings[i].name_size);

            if (tfi->varyings[i].name_size > tfi->max_varying_bufsize) {
                tfi->max_varying_bufsize = tfi->varyings[i].name_size;
            }

            varyings_size += tfi->varyings[i].name_size;
        }

        varyings_str = tmp = (GLchar*)yagl_get_tmp_buffer(varyings_size);

        for (i = 0; i < num_varyings; ++i) {
            memcpy(tmp, varyings[i], tfi->varyings[i].name_size);
            tmp += tfi->varyings[i].name_size;
        }
    }

    tfi->num_varyings = num_varyings;
    tfi->buffer_mode = buffer_mode;

    yagl_host_glTransformFeedbackVaryings(program->global_name,
                                          varyings_str,
                                          varyings_size,
                                          buffer_mode);
}

void yagl_gles3_program_get_transform_feedback_varying(struct yagl_gles2_program *program,
                                                       GLuint index,
                                                       GLsizei bufsize,
                                                       GLsizei *length,
                                                       GLsizei *size,
                                                       GLenum *type,
                                                       GLchar *name)
{
    struct yagl_gles2_transform_feedback_info *tfi = &program->linked_transform_feedback_info;
    struct yagl_gles2_transform_feedback_varying *varying;

    if (index >= tfi->num_varyings) {
        return;
    }

    varying = &tfi->varyings[index];

    if (!tfi->fetched) {
        GLuint i;
        GLsizei *sizes;
        GLenum *types;

        sizes = (GLsizei*)yagl_get_tmp_buffer(tfi->num_varyings *
                                              sizeof(sizes[0]));
        types = (GLenum*)yagl_get_tmp_buffer2(tfi->num_varyings *
                                              sizeof(types[0]));

        yagl_host_glGetTransformFeedbackVaryings(program->global_name,
                                                 sizes,
                                                 tfi->num_varyings,
                                                 NULL,
                                                 types,
                                                 tfi->num_varyings,
                                                 NULL);

        for (i = 0; i < tfi->num_varyings; ++i) {
            tfi->varyings[i].size = sizes[i];
            tfi->varyings[i].type = types[i];

            if ((sizes[i] == 0) && (types[i] == 0)) {
                yagl_free(tfi->varyings[i].name);
                tfi->varyings[i].name = NULL;
                tfi->varyings[i].name_size = 0;
            }
        }

        tfi->fetched = 1;
    }

    yagl_gles2_set_name(varying->name, varying->name_size,
                        bufsize,
                        length,
                        name);

    if (size) {
        *size = varying->size;
    }

    if (type) {
        *type = varying->type;
    }
}

int yagl_gles3_program_get_uniformuiv(struct yagl_gles2_program *program,
                                      GLint location,
                                      GLuint *params)
{
    uint32_t global_location;
    GLuint tmp[100]; // This fits all cases.
    int32_t num = 0;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glGetUniformuiv(program->gen_locations,
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

int yagl_gles3_program_uniform1ui(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLuint v0)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform1ui(program->gen_locations, global_location, v0);

    return 1;
}

int yagl_gles3_program_uniform2ui(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLuint v0,
                                  GLuint v1)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform2ui(program->gen_locations, global_location, v0, v1);

    return 1;
}

int yagl_gles3_program_uniform3ui(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLuint v0,
                                  GLuint v1,
                                  GLuint v2)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform3ui(program->gen_locations, global_location, v0, v1, v2);

    return 1;
}

int yagl_gles3_program_uniform4ui(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLuint v0,
                                  GLuint v1,
                                  GLuint v2,
                                  GLuint v3)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform4ui(program->gen_locations, global_location, v0, v1, v2, v3);

    return 1;
}

int yagl_gles3_program_uniform1uiv(struct yagl_gles2_program *program,
                                   GLint location,
                                   GLsizei count,
                                   const GLuint *v)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform1uiv(program->gen_locations, global_location, v, count);

    return 1;
}

int yagl_gles3_program_uniform2uiv(struct yagl_gles2_program *program,
                                   GLint location,
                                   GLsizei count,
                                   const GLuint *v)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform2uiv(program->gen_locations, global_location, v, 2 * count);

    return 1;
}

int yagl_gles3_program_uniform3uiv(struct yagl_gles2_program *program,
                                   GLint location,
                                   GLsizei count,
                                   const GLuint *v)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform3uiv(program->gen_locations, global_location, v, 3 * count);

    return 1;
}

int yagl_gles3_program_uniform4uiv(struct yagl_gles2_program *program,
                                   GLint location,
                                   GLsizei count,
                                   const GLuint *v)
{
    uint32_t global_location;

    if (!yagl_gles2_program_translate_location(program,
                                               location,
                                               &global_location)) {
        return 0;
    }

    yagl_host_glUniform4uiv(program->gen_locations, global_location, v, 4 * count);

    return 1;
}

int yagl_gles3_program_uniform_matrix2x3fv(struct yagl_gles2_program *program,
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

    yagl_host_glUniformMatrix2x3fv(program->gen_locations, global_location, transpose, value, 2 * 3 * count);

    return 1;
}

int yagl_gles3_program_uniform_matrix2x4fv(struct yagl_gles2_program *program,
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

    yagl_host_glUniformMatrix2x4fv(program->gen_locations, global_location, transpose, value, 2 * 4 * count);

    return 1;
}

int yagl_gles3_program_uniform_matrix3x2fv(struct yagl_gles2_program *program,
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

    yagl_host_glUniformMatrix3x2fv(program->gen_locations, global_location, transpose, value, 3 * 2 * count);

    return 1;
}

int yagl_gles3_program_uniform_matrix3x4fv(struct yagl_gles2_program *program,
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

    yagl_host_glUniformMatrix3x4fv(program->gen_locations, global_location, transpose, value, 3 * 4 * count);

    return 1;
}

int yagl_gles3_program_uniform_matrix4x2fv(struct yagl_gles2_program *program,
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

    yagl_host_glUniformMatrix4x2fv(program->gen_locations, global_location, transpose, value, 4 * 2 * count);

    return 1;
}

int yagl_gles3_program_uniform_matrix4x3fv(struct yagl_gles2_program *program,
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

    yagl_host_glUniformMatrix4x3fv(program->gen_locations, global_location, transpose, value, 4 * 3 * count);

    return 1;
}

int yagl_gles3_program_get_frag_data_location(struct yagl_gles2_program *program,
                                              const GLchar *name)
{
    struct yagl_gles2_location_l *location;
    int ret;

    yagl_list_for_each(struct yagl_gles2_location_l,
                       location,
                       &program->frag_data_locations, list) {
        if (strcmp(location->name, name) == 0) {
            return location->location;
        }
    }

    ret = yagl_host_glGetFragDataLocation(program->global_name,
                                          name,
                                          yagl_transport_string_count(name));

    location = yagl_malloc(sizeof(*location));

    yagl_list_init(&location->list);
    location->name = strdup(name);
    location->location = ret;

    yagl_list_add_tail(&program->frag_data_locations, &location->list);

    return ret;
}
