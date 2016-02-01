%{
#include <GL/gl.h>
#include "yagl_glsl_state.h"
#include "yagl_malloc.h"

#define YY_NO_INPUT

#define YY_INPUT(buf, result, max_size) \
    result = string_input(buf, max_size, yyscanner);

static yy_size_t string_input(char *buf, yy_size_t max_size, yyscan_t yyscanner);
%}

%option bison-bridge reentrant noyywrap
%option nounput
%option never-interactive
%option prefix="yagl_glsl_lexer_"
%option extra-type="struct yagl_glsl_state *"

%x COMMENT PP

WS [ \r\t\v\f]
PRECISION "lowp"|"mediump"|"highp"|"precision"[^;\n]+;?
CONTROL [()\[\]{},;?:/%*&|^!+\-=<>\.]
STRING [^ \r\t\v\f\n()\[\]{},;?:/%*&|^!+\-=<>\.]+

%%

"//"[^\n]* {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_comment(state, strlen(yytext));
}

"/*" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    BEGIN(COMMENT);
    yagl_glsl_state_new_comment(state, strlen(yytext));
}

^{WS}*#{WS}*version {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    BEGIN(PP);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_VERSION;
}

^{WS}*#{WS}*define {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    BEGIN(PP);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_DEFINE;
}

^{WS}*#{WS}*extension[^\n]* {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_EXTENSION;
}

[\n]+ {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_pending(state, yytext);
}

{WS}+ {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_pending(state, yytext);
}

"gl_MaxVertexUniformVectors" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_MAXVERTEXUNIFORMVECTORS;
}

"gl_MaxFragmentUniformVectors" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_MAXFRAGMENTUNIFORMVECTORS;
}

"gl_MaxVaryingVectors" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_MAXVARYINGVECTORS;
}

"gl_MaxVaryingFloats" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_MAXVARYINGFLOATS;
}

"attribute" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_ATTRIBUTE;
}

"varying" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_VARYING;
}

"texture1DProjLod" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURE1DPROJLOD;
}

"texture1DProj" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURE1DPROJ;
}

"texture1DLod" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURE1DLOD;
}

"texture1D" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURE1D;
}

"texture2DProjLod" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURE2DPROJLOD;
}

"texture2DLod" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURE2DLOD;
}

"texture2DProj" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURE2DPROJ;
}

"texture2D" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURE2D;
}

"texture3DProjLod" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURE3DPROJLOD;
}

"texture3DLod" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURE3DLOD;
}

"texture3DProj" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURE3DPROJ;
}

"texture3D" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURE3D;
}

"textureCubeLod" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURECUBELOD;
}

"textureCube" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_TEXTURECUBE;
}

"gl_FragColor" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_GLFRAGCOLOR;
}

"samplerExternalOES" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_SAMPLEREXTERNALOES;
}

{PRECISION} {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_PRECISION;
}

{CONTROL} {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_pending(state, yytext);
}

{STRING} {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_pending(state, yytext);
}

<PP>\n {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    BEGIN(INITIAL);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_EOL;
}

<PP>{WS}+ {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_pending(state, yytext);
}

<PP>[1-9][0-9]* {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_integer_token(state, yylval, strtol(yytext, NULL, 10));
    return TOK_INTEGER;
}

<PP>"es" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_ES;
}

<PP>{PRECISION} {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_PRECISION;
}

<PP>{CONTROL} {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_STRING;
}

<PP>{STRING} {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_str_token(state, yylval, yytext);
    return TOK_STRING;
}

<COMMENT>[^*\n]* {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_comment(state, strlen(yytext));
}

<COMMENT>[^*\n]*\n {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_comment(state, strlen(yytext) - 1);
    yagl_glsl_state_new_pending(state, "\n");
}

<COMMENT>"*"+[^*/\n]* {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_comment(state, strlen(yytext));
}

<COMMENT>"*"+[^*/\n]*\n {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    yagl_glsl_state_new_comment(state, strlen(yytext) - 1);
    yagl_glsl_state_new_pending(state, "\n");
}

<COMMENT>"*"+"/" {
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    BEGIN(INITIAL);
    yagl_glsl_state_new_comment(state, strlen(yytext));
}

%%

static yy_size_t string_input(char *buf, yy_size_t max_size, yyscan_t yyscanner)
{
    struct yagl_glsl_state *state = yagl_glsl_lexer_get_extra(yyscanner);
    int rem = state->source_len - state->source_pos;
    int num_read = (rem < max_size) ? rem : max_size;

    memcpy(buf, state->source + state->source_pos, num_read);

    state->source_pos += num_read;

    return num_read;
}

//extern int yagl_glsl_debug;

int yagl_glsl_state_init(struct yagl_glsl_state *state,
                         GLenum shader_type,
                         const char *source,
                         int source_len,
                         int es3_supported)
{
    yyscan_t scanner = NULL;

    memset(state, 0, sizeof(*state));

    //yagl_glsl_debug = 1;

    if (yagl_glsl_lexer_lex_init_extra(state, &scanner)) {
        return 0;
    }

    state->shader_type = shader_type;
    state->source = source;
    state->source_len = source_len;
    state->es3_supported = es3_supported;
    state->scanner = scanner;
    state->token_index = 1;

    yagl_glsl_state_set_version(state, 120, 0);

    yagl_vector_init(&state->header, sizeof(char), 0);
    yagl_vector_init(&state->output, sizeof(char), 0);
    yagl_list_init(&state->pending_list);
    yagl_vector_init(&state->pending, sizeof(char), 0);
    yagl_vector_init(&state->strings, sizeof(char*), 0);

    return 1;
}

void yagl_glsl_state_cleanup(struct yagl_glsl_state *state)
{
    yyscan_t scanner = state->scanner;
    int i;
    char **tmp;
    struct yagl_glsl_pending *it, *tmp_it;

    tmp = yagl_vector_data(&state->strings);

    for (i = 0; i < yagl_vector_size(&state->strings); ++i) {
        free(*(tmp + i));
    }

    yagl_vector_cleanup(&state->strings);

    yagl_vector_cleanup(&state->pending);

    yagl_list_for_each_safe(struct yagl_glsl_pending,
                            it,
                            tmp_it,
                            &state->pending_list, list) {
        yagl_list_remove(&it->list);
        yagl_free(it);
    }

    yagl_vector_cleanup(&state->output);

    yagl_vector_cleanup(&state->header);

    yagl_glsl_lexer_lex_destroy(scanner);
}
