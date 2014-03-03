%{
#include <GL/gl.h>
#include "yagl_glsl_state.h"
#include <stdio.h>
#include <string.h>

#undef yyerror

extern int yagl_glsl_lexer_lex(YYSTYPE *yylval, void *scanner);

static void yyerror(struct yagl_glsl_state *state, const char *msg)
{
    yagl_glsl_state_set_error(state, msg);
}

static int yagl_glsl_lex(union YYSTYPE *val, struct yagl_glsl_state *state)
{
    return yagl_glsl_lexer_lex(val, state->scanner);
}
%}

%pure-parser
%name-prefix="yagl_glsl_"
%lex-param {struct yagl_glsl_state *state}
%parse-param {struct yagl_glsl_state *state}
%debug

%union
{
    struct
    {
        int index;
        int value;
    } integer;

    struct
    {
        int index;
        const char *value;
    } str;
}

%token <str> TOK_EOL
%token <str> TOK_VERSION
%token <str> TOK_DEFINE
%token <str> TOK_STRING
%token <str> TOK_GLEXT
%token <integer> TOK_INTEGER
%token <str> TOK_ES
%token <str> TOK_PRECISION
%token <str> TOK_MAXVERTEXUNIFORMVECTORS
%token <str> TOK_MAXFRAGMENTUNIFORMVECTORS
%token <str> TOK_MAXVARYINGVECTORS
%token <str> TOK_MAXVARYINGFLOATS
%token <str> TOK_ATTRIBUTE
%token <str> TOK_VARYING
%token <str> TOK_TEXTURE
%token <str> TOK_TEXTUREPROJ
%token <str> TOK_TEXTURELOD
%token <str> TOK_TEXTUREPROJLOD
%token <str> TOK_GLFRAGCOLOR

%%

start
:
{
}
| version expressions
{
}
| expressions
{
}
| version
{
}
;

version
: TOK_VERSION TOK_INTEGER TOK_ES TOK_EOL
{
    char s[100];
    int version = $2.value;

    yagl_glsl_state_set_version(state, $2.value, 1);

    state->have_version = 1;

    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);
    yagl_glsl_state_flush_pending(state, $2.index);

    switch (state->patch_version) {
    case yagl_glsl_120:
        version = 120;
        break;
    case yagl_glsl_140:
        version = 140;
        break;
    case yagl_glsl_150:
        version = 150;
        break;
    default:
        break;
    }

    sprintf(s, "%d", version);
    yagl_glsl_state_append_output(state, s);
    yagl_glsl_state_flush_pending(state, $3.index);
    if (state->patch_version == yagl_glsl_asis) {
        yagl_glsl_state_append_output(state, $3.value);
    }
    yagl_glsl_state_flush_pending(state, $4.index);
    yagl_glsl_state_append_output(state, $4.value);

    yagl_glsl_state_output_to_header(state);
}
| TOK_VERSION TOK_INTEGER TOK_EOL
{
    char s[100];
    int version = $2.value;

    yagl_glsl_state_set_version(state, $2.value, 0);

    state->have_version = 1;

    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);
    yagl_glsl_state_flush_pending(state, $2.index);

    switch (state->patch_version) {
    case yagl_glsl_120:
        version = 120;
        break;
    case yagl_glsl_140:
        version = 140;
        break;
    case yagl_glsl_150:
        version = 150;
        break;
    default:
        break;
    }

    sprintf(s, "%d", version);
    yagl_glsl_state_append_output(state, s);
    yagl_glsl_state_flush_pending(state, $3.index);
    yagl_glsl_state_append_output(state, $3.value);

    yagl_glsl_state_output_to_header(state);
}
;

expressions
: expression
{
}
| expressions expression
{
}
;

expression
: TOK_EOL
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);
}
| TOK_DEFINE TOK_PRECISION
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);
    yagl_glsl_state_flush_pending(state, $2.index);
    yagl_glsl_state_append_output(state, $2.value);
}
| TOK_DEFINE
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);
}
| TOK_PRECISION
{
    yagl_glsl_state_flush_pending(state, $1.index);
    if (!state->patch_precision) {
        yagl_glsl_state_append_output(state, $1.value);
    }
}
| TOK_ES
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);
}
| TOK_INTEGER
{
    char s[100];

    yagl_glsl_state_flush_pending(state, $1.index);
    sprintf(s, "%d", $1.value);
    yagl_glsl_state_append_output(state, s);
}
| TOK_STRING
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);
}
| TOK_MAXVERTEXUNIFORMVECTORS
{
    yagl_glsl_state_flush_pending(state, $1.index);
    if (state->patch_builtins) {
        yagl_glsl_state_append_output(state, "(gl_MaxVertexUniformComponents / 4)");
    } else {
        yagl_glsl_state_append_output(state, $1.value);
    }
}
| TOK_MAXFRAGMENTUNIFORMVECTORS
{
    yagl_glsl_state_flush_pending(state, $1.index);
    if (state->patch_builtins) {
        yagl_glsl_state_append_output(state, "(gl_MaxFragmentUniformComponents / 4)");
    } else {
        yagl_glsl_state_append_output(state, $1.value);
    }
}
| TOK_MAXVARYINGVECTORS
{
    yagl_glsl_state_flush_pending(state, $1.index);
    if (state->patch_builtins) {
        if (state->patch_max_varying_floats) {
            /*
             * gl_MaxVaryingComponents must be used instead of
             * gl_MaxVaryingFloats in OpenGL 3.1, but it's deprecated in
             * OpenGL 3.2, thus, we just use a constant.
             */
            yagl_glsl_state_append_output(state, "(64 / 4)");
        } else {
            yagl_glsl_state_append_output(state, "(gl_MaxVaryingFloats / 4)");
        }
    } else {
        yagl_glsl_state_append_output(state, $1.value);
    }
}
| TOK_MAXVARYINGFLOATS
{
    yagl_glsl_state_flush_pending(state, $1.index);
    if (state->patch_max_varying_floats) {
        /*
         * See 'TOK_MAXVARYINGVECTORS' case.
         */
        yagl_glsl_state_append_output(state, "64");
    } else {
        yagl_glsl_state_append_output(state, $1.value);
    }
}
| TOK_GLEXT
{
    int i, found = 0;

    /*
     * GLSL ES shader can contain something like this:
     *
     * #ifdef GL_ARB_draw_instanced
     * #extension GL_ARB_draw_instanced : require
     * #endif
     *
     * On real hardware GL_ARB_draw_instanced is not defined, so the code
     * inside won't compile. In emulated environment when host GPU
     * is used GL_ARB_draw_instanced will be defined and code inside
     * will be compiled when it shouldn't.
     *
     * A workaround for this is to find all "GL_" tokens and unless
     * this is some supported OpenGL ES extension, replace it with
     * something else that's not defined in host preprocessor for sure.
     */

    for (i = 0; i < state->num_extensions; ++i) {
        if (strcmp(state->extensions[i], $1.value) == 0) {
            found = 1;
            break;
        }
    }

    yagl_glsl_state_flush_pending(state, $1.index);

    if (found) {
        yagl_glsl_state_append_output(state, $1.value);
    } else {
        /*
         * Replace "GL_" with "ND_" to make
         * sure it's not defined in preprocessor.
         */

        yagl_glsl_state_append_output(state, "ND_");
        yagl_glsl_state_append_output(state, $1.value + 3);
    }
}
| TOK_ATTRIBUTE
{
    yagl_glsl_state_flush_pending(state, $1.index);

    if (state->patch_gl2) {
        if (state->shader_type == GL_VERTEX_SHADER) {
            yagl_glsl_state_append_output(state, "in");
        }
    } else {
        yagl_glsl_state_append_output(state, $1.value);
    }
}
| TOK_VARYING
{
    yagl_glsl_state_flush_pending(state, $1.index);

    if (state->patch_gl2) {
        if (state->shader_type == GL_VERTEX_SHADER) {
            yagl_glsl_state_append_output(state, "out");
        } else {
            yagl_glsl_state_append_output(state, "in");
        }
    } else {
        yagl_glsl_state_append_output(state, $1.value);
    }
}
| TOK_TEXTURE
{
    yagl_glsl_state_flush_pending(state, $1.index);

    if (state->patch_gl2) {
        yagl_glsl_state_append_output(state, "texture");
    } else {
        yagl_glsl_state_append_output(state, $1.value);
    }
}
| TOK_TEXTUREPROJ
{
    yagl_glsl_state_flush_pending(state, $1.index);

    if (state->patch_gl2) {
        yagl_glsl_state_append_output(state, "textureProj");
    } else {
        yagl_glsl_state_append_output(state, $1.value);
    }
}
| TOK_TEXTURELOD
{
    yagl_glsl_state_flush_pending(state, $1.index);

    if (state->patch_gl2) {
        yagl_glsl_state_append_output(state, "textureLod");
    } else {
        yagl_glsl_state_append_output(state, $1.value);
    }
}
| TOK_TEXTUREPROJLOD
{
    yagl_glsl_state_flush_pending(state, $1.index);

    if (state->patch_gl2) {
        yagl_glsl_state_append_output(state, "textureProjLod");
    } else {
        yagl_glsl_state_append_output(state, $1.value);
    }
}
| TOK_GLFRAGCOLOR
{
    yagl_glsl_state_flush_pending(state, $1.index);

    if (state->patch_gl2) {
        if (!state->frag_color_declared) {
            yagl_glsl_state_append_header(state, "out vec4 GL_FragColor;\n");
            state->frag_color_declared = 1;
        }
        yagl_glsl_state_append_output(state, "GL_FragColor");
    } else {
        yagl_glsl_state_append_output(state, $1.value);
    }
}
;
