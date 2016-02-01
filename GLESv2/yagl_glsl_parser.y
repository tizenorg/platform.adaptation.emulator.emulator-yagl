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
%token <str> TOK_EXTENSION
%token <str> TOK_STRING
%token <integer> TOK_INTEGER
%token <str> TOK_ES
%token <str> TOK_PRECISION
%token <str> TOK_MAXVERTEXUNIFORMVECTORS
%token <str> TOK_MAXFRAGMENTUNIFORMVECTORS
%token <str> TOK_MAXVARYINGVECTORS
%token <str> TOK_MAXVARYINGFLOATS
%token <str> TOK_ATTRIBUTE
%token <str> TOK_VARYING
%token <str> TOK_TEXTURE1D
%token <str> TOK_TEXTURE1DPROJ
%token <str> TOK_TEXTURE1DLOD
%token <str> TOK_TEXTURE1DPROJLOD
%token <str> TOK_TEXTURE2D
%token <str> TOK_TEXTURE2DPROJ
%token <str> TOK_TEXTURE2DLOD
%token <str> TOK_TEXTURE2DPROJLOD
%token <str> TOK_TEXTURE3D
%token <str> TOK_TEXTURE3DPROJ
%token <str> TOK_TEXTURE3DLOD
%token <str> TOK_TEXTURE3DPROJLOD
%token <str> TOK_TEXTURECUBE
%token <str> TOK_TEXTURECUBELOD
%token <str> TOK_GLFRAGCOLOR
%token <str> TOK_SAMPLEREXTERNALOES

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

    snprintf(s, 100, "%d", version);
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

    snprintf(s, 100, "%d", version);
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
| TOK_EXTENSION
{
    /*
     * Always strip #extension constructs. The reasons are:
     *
     * 1. Pretty useless, host GLSL already includes
     *    most of the extensions as core features. Extension naming
     *    on real hardware and host OpenGL is
     *    different, so #extension constructs can fail.
     *
     *    e.g. GLSL ES shader can contain something like this:
     *
     *    #ifdef GL_ARB_draw_instanced
     *    #extension GL_ARB_draw_instanced : require
     *    #endif
     *
     *    On real hardware GL_ARB_draw_instanced is not defined, so the code
     *    inside won't compile. In emulated environment when host GPU
     *    is used GL_ARB_draw_instanced will be defined and #extension inside
     *    will be compiled when it shouldn't.
     *
     *    Another e.g. GLSL ES 2.0 shader can contain:
     *
     *    #extension GL_OES_texture_3D : enable
     *
     *    Useless, host GLSL already has 3D textures and they're
     *    called GL_EXT_texture_3D, so this code can either be ignored
     *    or fail.
     *
     * 2. Some host OpenGL drivers like mesa only allow #extension constructs
     *    after #version. Many real hardware allow #extension anywhere. Also,
     *    when we patch shaders we need to add some of our own #extension
     *    constructs, functions, variables right after #version. If such shader
     *    contains #extension constructs of its own it'll fail to compile with
     *    mesa.
     */

    yagl_glsl_state_flush_pending(state, $1.index);
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
| TOK_TEXTURE1D
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texture1d_declared) {
            if (state->shader_type == GL_FRAGMENT_SHADER) {
                yagl_glsl_state_append_header(state, "vec4 texture1D(sampler1D sampler, float coord, float bias) {\n");
                yagl_glsl_state_append_header(state, "    return texture(sampler, coord, bias);\n");
                yagl_glsl_state_append_header(state, "}\n");
            }
            yagl_glsl_state_append_header(state, "vec4 texture1D(sampler1D sampler, float coord) {\n");
            yagl_glsl_state_append_header(state, "    return texture(sampler, coord);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texture1d_declared = 1;
        }
    }
}
| TOK_TEXTURE1DPROJ
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texture1dproj_declared) {
            if (state->shader_type == GL_FRAGMENT_SHADER) {
                yagl_glsl_state_append_header(state, "vec4 texture1DProj(sampler1D sampler, vec2 coord, float bias) {\n");
                yagl_glsl_state_append_header(state, "    return textureProj(sampler, coord, bias);\n");
                yagl_glsl_state_append_header(state, "}\n");
                yagl_glsl_state_append_header(state, "vec4 texture1DProj(sampler1D sampler, vec4 coord, float bias) {\n");
                yagl_glsl_state_append_header(state, "    return textureProj(sampler, coord, bias);\n");
                yagl_glsl_state_append_header(state, "}\n");
            }
            yagl_glsl_state_append_header(state, "vec4 texture1DProj(sampler1D sampler, vec2 coord) {\n");
            yagl_glsl_state_append_header(state, "    return textureProj(sampler, coord);\n");
            yagl_glsl_state_append_header(state, "}\n");
            yagl_glsl_state_append_header(state, "vec4 texture1DProj(sampler1D sampler, vec4 coord) {\n");
            yagl_glsl_state_append_header(state, "    return textureProj(sampler, coord);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texture1dproj_declared = 1;
        }
    }
}
| TOK_TEXTURE1DLOD
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texture1dlod_declared) {
            yagl_glsl_state_append_header(state, "vec4 texture1DLod(sampler1D sampler, float coord, float lod) {\n");
            yagl_glsl_state_append_header(state, "    return textureLod(sampler, coord, lod);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texture1dlod_declared = 1;
        }
    }
}
| TOK_TEXTURE1DPROJLOD
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texture1dprojlod_declared) {
            yagl_glsl_state_append_header(state, "vec4 texture1DProjLod(sampler1D sampler, vec2 coord, float lod) {\n");
            yagl_glsl_state_append_header(state, "    return textureProjLod(sampler, coord, lod);\n");
            yagl_glsl_state_append_header(state, "}\n");
            yagl_glsl_state_append_header(state, "vec4 texture1DProjLod(sampler1D sampler, vec4 coord, float lod) {\n");
            yagl_glsl_state_append_header(state, "    return textureProjLod(sampler, coord, lod);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texture1dprojlod_declared = 1;
        }
    }
}
| TOK_TEXTURE2D
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texture2d_declared) {
            if (state->shader_type == GL_FRAGMENT_SHADER) {
                yagl_glsl_state_append_header(state, "vec4 texture2D(sampler2D sampler, vec2 coord, float bias) {\n");
                yagl_glsl_state_append_header(state, "    return texture(sampler, coord, bias);\n");
                yagl_glsl_state_append_header(state, "}\n");
            }
            yagl_glsl_state_append_header(state, "vec4 texture2D(sampler2D sampler, vec2 coord) {\n");
            yagl_glsl_state_append_header(state, "    return texture(sampler, coord);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texture2d_declared = 1;
        }
    }
}
| TOK_TEXTURE2DPROJ
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texture2dproj_declared) {
            if (state->shader_type == GL_FRAGMENT_SHADER) {
                yagl_glsl_state_append_header(state, "vec4 texture2DProj(sampler2D sampler, vec3 coord, float bias) {\n");
                yagl_glsl_state_append_header(state, "    return textureProj(sampler, coord, bias);\n");
                yagl_glsl_state_append_header(state, "}\n");
                yagl_glsl_state_append_header(state, "vec4 texture2DProj(sampler2D sampler, vec4 coord, float bias) {\n");
                yagl_glsl_state_append_header(state, "    return textureProj(sampler, coord, bias);\n");
                yagl_glsl_state_append_header(state, "}\n");
            }
            yagl_glsl_state_append_header(state, "vec4 texture2DProj(sampler2D sampler, vec3 coord) {\n");
            yagl_glsl_state_append_header(state, "    return textureProj(sampler, coord);\n");
            yagl_glsl_state_append_header(state, "}\n");
            yagl_glsl_state_append_header(state, "vec4 texture2DProj(sampler2D sampler, vec4 coord) {\n");
            yagl_glsl_state_append_header(state, "    return textureProj(sampler, coord);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texture2dproj_declared = 1;
        }
    }
}
| TOK_TEXTURE2DLOD
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texture2dlod_declared) {
            yagl_glsl_state_append_header(state, "vec4 texture2DLod(sampler2D sampler, vec2 coord, float lod) {\n");
            yagl_glsl_state_append_header(state, "    return textureLod(sampler, coord, lod);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texture2dlod_declared = 1;
        }
    }
}
| TOK_TEXTURE2DPROJLOD
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texture2dprojlod_declared) {
            yagl_glsl_state_append_header(state, "vec4 texture2DProjLod(sampler2D sampler, vec3 coord, float lod) {\n");
            yagl_glsl_state_append_header(state, "    return textureProjLod(sampler, coord, lod);\n");
            yagl_glsl_state_append_header(state, "}\n");
            yagl_glsl_state_append_header(state, "vec4 texture2DProjLod(sampler2D sampler, vec4 coord, float lod) {\n");
            yagl_glsl_state_append_header(state, "    return textureProjLod(sampler, coord, lod);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texture2dprojlod_declared = 1;
        }
    }
}
| TOK_TEXTURE3D
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texture3d_declared) {
            if (state->shader_type == GL_FRAGMENT_SHADER) {
                yagl_glsl_state_append_header(state, "vec4 texture3D(sampler3D sampler, vec3 coord, float bias) {\n");
                yagl_glsl_state_append_header(state, "    return texture(sampler, coord, bias);\n");
                yagl_glsl_state_append_header(state, "}\n");
            }
            yagl_glsl_state_append_header(state, "vec4 texture3D(sampler3D sampler, vec3 coord) {\n");
            yagl_glsl_state_append_header(state, "    return texture(sampler, coord);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texture3d_declared = 1;
        }
    }
}
| TOK_TEXTURE3DPROJ
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texture3dproj_declared) {
            if (state->shader_type == GL_FRAGMENT_SHADER) {
                yagl_glsl_state_append_header(state, "vec4 texture3DProj(sampler3D sampler, vec4 coord, float bias) {\n");
                yagl_glsl_state_append_header(state, "    return textureProj(sampler, coord, bias);\n");
                yagl_glsl_state_append_header(state, "}\n");
            }
            yagl_glsl_state_append_header(state, "vec4 texture3DProj(sampler3D sampler, vec4 coord) {\n");
            yagl_glsl_state_append_header(state, "    return textureProj(sampler, coord);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texture3dproj_declared = 1;
        }
    }
}
| TOK_TEXTURE3DLOD
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texture3dlod_declared) {
            yagl_glsl_state_append_header(state, "vec4 texture3DLod(sampler3D sampler, vec3 coord, float lod) {\n");
            yagl_glsl_state_append_header(state, "    return textureLod(sampler, coord, lod);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texture3dlod_declared = 1;
        }
    }
}
| TOK_TEXTURE3DPROJLOD
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texture3dprojlod_declared) {
            yagl_glsl_state_append_header(state, "vec4 texture3DProjLod(sampler3D sampler, vec4 coord, float lod) {\n");
            yagl_glsl_state_append_header(state, "    return textureProjLod(sampler, coord, lod);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texture3dprojlod_declared = 1;
        }
    }
}
| TOK_TEXTURECUBE
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texturecube_declared) {
            if (state->shader_type == GL_FRAGMENT_SHADER) {
                yagl_glsl_state_append_header(state, "vec4 textureCube(samplerCube sampler, vec3 coord, float bias) {\n");
                yagl_glsl_state_append_header(state, "    return texture(sampler, coord, bias);\n");
                yagl_glsl_state_append_header(state, "}\n");
            }
            yagl_glsl_state_append_header(state, "vec4 textureCube(samplerCube sampler, vec3 coord) {\n");
            yagl_glsl_state_append_header(state, "    return texture(sampler, coord);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texturecube_declared = 1;
        }
    }
}
| TOK_TEXTURECUBELOD
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, $1.value);

    if (state->patch_gl2) {
        if (!state->texturecubelod_declared) {
            yagl_glsl_state_append_header(state, "vec4 textureCubeLod(samplerCube sampler, vec3 coord, float lod) {\n");
            yagl_glsl_state_append_header(state, "    return textureLod(sampler, coord, lod);\n");
            yagl_glsl_state_append_header(state, "}\n");
            state->texturecubelod_declared = 1;
        }
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
| TOK_SAMPLEREXTERNALOES
{
    yagl_glsl_state_flush_pending(state, $1.index);
    yagl_glsl_state_append_output(state, "sampler2D");
}
;
