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

#include <GL/gl.h>
#include "yagl_glsl_state.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include <string.h>
#include <stdio.h>

static void yagl_glsl_state_flush_version(struct yagl_glsl_state *state)
{
    int have_version = state->have_version;
    int have_extensions = state->have_extensions;
    int have_samplerexternaloes = state->have_samplerexternaloes;

    if (have_version && have_extensions && have_samplerexternaloes) {
        return;
    }

    state->have_version = 1;
    state->have_extensions = 1;
    state->have_samplerexternaloes = 1;

    if (!have_version) {
        switch (state->patch_version) {
        case yagl_glsl_140:
            yagl_glsl_state_append_header(state, "#version 140\n\n");
            break;
        case yagl_glsl_150:
            yagl_glsl_state_append_header(state, "#version 150\n\n");
            break;
        default:
            yagl_glsl_state_append_header(state, "#version 120\n\n");
            break;
        }
    }

    if (!have_extensions) {
        switch (state->patch_version) {
        case yagl_glsl_150:
            yagl_glsl_state_append_header(state, "#ifdef GL_ARB_explicit_attrib_location\n");
            yagl_glsl_state_append_header(state, "#extension GL_ARB_explicit_attrib_location : enable\n");
            yagl_glsl_state_append_header(state, "#endif\n");
            break;
        default:
            break;
        }
    }

    if(!have_samplerexternaloes) {
        yagl_glsl_state_append_header(state, "#define samplerExternalOES sampler2D\n\n");
    }
}

static void yagl_glsl_state_move_pending(struct yagl_glsl_state *state)
{
    struct yagl_glsl_pending *p;
    int pending_size = yagl_vector_size(&state->pending);
    char c = '\0';

    if (pending_size <= 0) {
        return;
    }

    yagl_vector_push_back(&state->pending, &c);

    p = yagl_malloc(sizeof(*p) + pending_size + 1);

    yagl_list_init(&p->list);

    p->token_index = state->token_index;

    memcpy((p + 1), yagl_vector_data(&state->pending), pending_size + 1);

    yagl_list_add_tail(&state->pending_list, &p->list);

    yagl_vector_resize(&state->pending, 0);
}

void yagl_glsl_state_set_version(struct yagl_glsl_state *state,
                                 int version,
                                 int is_es)
{
    if (((version == 100) || (version == 110) || (version == 120)) && !is_es) {
        /*
         * GLSL ES 2.
         */

        state->patch_precision = 1;
        state->patch_builtins = 1;

        switch (yagl_get_host_gl_version()) {
        case yagl_gl_2:
            state->patch_version = yagl_glsl_120;
            state->patch_gl2 = 0;
            state->patch_max_varying_floats = 0;
            break;
        default:
            state->patch_version = yagl_glsl_140;
            state->patch_gl2 = 1;
            state->patch_max_varying_floats = 1;
            break;
        }
    } else if ((version == 300) && is_es && state->es3_supported) {
        /*
         * GLSL ES 3.
         */

        state->patch_gl2 = 0;

        switch (yagl_get_host_gl_version()) {
        case yagl_gl_3_1_es3:
            /*
             * GL_ARB_ES3_compatibility includes full ES 3.00 shader
             * support, no patching is required.
             */

            state->patch_version = yagl_glsl_asis;
            state->patch_precision = 0;
            state->patch_builtins = 0;
            state->patch_max_varying_floats = 0;
            break;
        default:
            state->patch_version = yagl_glsl_150;
            state->patch_precision = 1;
            state->patch_builtins = 1;
            state->patch_max_varying_floats = 1;
            break;
        }
    } else {
        /*
         * Unknown.
         */

        state->patch_version = yagl_glsl_asis;
        state->patch_precision = 0;
        state->patch_builtins = 0;
        state->patch_gl2 = 0;
        state->patch_max_varying_floats = 0;
    }
}

void yagl_glsl_state_set_error(struct yagl_glsl_state *state,
                               const char *msg)
{
    YAGL_LOG_FUNC_SET(yagl_glsl_state_set_error);

    state->have_error = 1;

    YAGL_LOG_WARN("%s", msg);
}

void yagl_glsl_state_new_pending(struct yagl_glsl_state *state,
                                 const char *str)
{
    int size = yagl_vector_size(&state->pending);
    int str_len = strlen(str);

    yagl_vector_resize(&state->pending, size + str_len);
    memcpy(yagl_vector_data(&state->pending) + size, str, str_len);
}

void yagl_glsl_state_new_comment(struct yagl_glsl_state *state,
                                 int len)
{
    int size = yagl_vector_size(&state->pending);

    yagl_vector_resize(&state->pending, size + len);
    memset(yagl_vector_data(&state->pending) + size, ' ', len);
}

void yagl_glsl_state_new_str_token(struct yagl_glsl_state *state,
                                   YYSTYPE *yylval,
                                   const char *value)
{
    char *tmp;

    yagl_glsl_state_move_pending(state);

    tmp = strdup(value);

    yagl_vector_push_back(&state->strings, &tmp);

    yylval->str.index = state->token_index;
    yylval->str.value = tmp;

    ++state->token_index;
}

void yagl_glsl_state_new_integer_token(struct yagl_glsl_state *state,
                                       YYSTYPE *yylval,
                                       int value)
{
    yagl_glsl_state_move_pending(state);

    yylval->integer.index = state->token_index;
    yylval->integer.value = value;

    ++state->token_index;
}

void yagl_glsl_state_flush_pending(struct yagl_glsl_state *state,
                                   int token_index)
{
    struct yagl_glsl_pending *it, *tmp_it;

    yagl_list_for_each_safe(struct yagl_glsl_pending,
                            it,
                            tmp_it,
                            &state->pending_list, list) {
        if (it->token_index > token_index) {
            break;
        }

        yagl_glsl_state_append_output(state, (const char*)(it + 1));

        yagl_list_remove(&it->list);
        yagl_free(it);
    }
}

void yagl_glsl_state_append_header(struct yagl_glsl_state *state,
                                   const char *str)
{
    int size, str_len;

    yagl_glsl_state_flush_version(state);

    size = yagl_vector_size(&state->header);
    str_len = strlen(str);

    yagl_vector_resize(&state->header, size + str_len);
    memcpy(yagl_vector_data(&state->header) + size, str, str_len);
}

void yagl_glsl_state_append_output(struct yagl_glsl_state *state,
                                   const char *str)
{
    int size, str_len;

    size = yagl_vector_size(&state->output);
    str_len = strlen(str);

    yagl_vector_resize(&state->output, size + str_len);
    memcpy(yagl_vector_data(&state->output) + size, str, str_len);
}

void yagl_glsl_state_output_to_header(struct yagl_glsl_state *state)
{
    int header_size, output_size;

    header_size = yagl_vector_size(&state->header);
    output_size = yagl_vector_size(&state->output);

    yagl_vector_resize(&state->header, header_size + output_size);
    memcpy(yagl_vector_data(&state->header) + header_size,
           yagl_vector_data(&state->output),
           output_size);

    yagl_vector_resize(&state->output, 0);
}

char *yagl_glsl_state_get_output(struct yagl_glsl_state *state, int *output_len)
{
    char c = '\0';

    yagl_glsl_state_move_pending(state);
    yagl_glsl_state_flush_pending(state, state->token_index);
    yagl_glsl_state_flush_version(state);

    yagl_glsl_state_output_to_header(state);

    *output_len = yagl_vector_size(&state->header);

    yagl_vector_push_back(&state->header, &c);

    return yagl_vector_detach(&state->header);
}
