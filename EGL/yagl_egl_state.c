#include "yagl_egl_state.h"
#include "yagl_context.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include <pthread.h>

struct yagl_egl_state
{
    EGLint error;

    EGLenum api;

    struct yagl_context *ctx;
};

static pthread_key_t g_state_key;
static pthread_once_t g_state_key_init = PTHREAD_ONCE_INIT;

static void yagl_egl_state_free(void* ptr)
{
    struct yagl_egl_state *state = ptr;

    YAGL_LOG_FUNC_ENTER(yagl_egl_state_free, "%p", ptr);

    if (!ptr) {
        return;
    }

    yagl_context_release(state->ctx);

    yagl_free(state);

    YAGL_LOG_FUNC_EXIT(NULL);
}

static void yagl_egl_state_key_init()
{
    pthread_key_create(&g_state_key, yagl_egl_state_free);
}

static void yagl_egl_state_init()
{
    struct yagl_egl_state *state;

    pthread_once(&g_state_key_init, yagl_egl_state_key_init);

    if (pthread_getspecific(g_state_key)) {
        return;
    }

    YAGL_LOG_FUNC_ENTER(yagl_egl_state_init, NULL);

    state = yagl_malloc0(sizeof(struct yagl_egl_state));

    state->error = EGL_SUCCESS;
    state->api = EGL_OPENGL_ES_API;

    pthread_setspecific(g_state_key, state);

    YAGL_LOG_FUNC_EXIT("%p", state);
}

static struct yagl_egl_state *yagl_egl_get_state()
{
    yagl_egl_state_init();

    return (struct yagl_egl_state*)pthread_getspecific(g_state_key);
}

EGLint yagl_get_error()
{
    struct yagl_egl_state *state = yagl_egl_get_state();

    EGLint error = state->error;

    state->error = EGL_SUCCESS;

    return error;
}

void yagl_set_error(EGLint error)
{
    struct yagl_egl_state *state = yagl_egl_get_state();

    if (state->error == EGL_SUCCESS) {
        state->error = error;
    }
}

EGLenum yagl_get_api()
{
    struct yagl_egl_state *state = yagl_egl_get_state();

    return state->api;
}

void yagl_set_api(EGLenum api)
{
    struct yagl_egl_state *state = yagl_egl_get_state();

    state->api = api;
}

struct yagl_context *yagl_get_context()
{
    struct yagl_egl_state *state = yagl_egl_get_state();

    return state->ctx;
}

void yagl_set_context(struct yagl_context *ctx)
{
    struct yagl_egl_state *state = yagl_egl_get_state();

    yagl_context_acquire(ctx);

    yagl_context_release(state->ctx);

    state->ctx = ctx;
}

void yagl_reset_state()
{
    struct yagl_egl_state *state = yagl_egl_get_state();

    yagl_set_context(NULL);

    state->error = EGL_SUCCESS;
    state->api = EGL_OPENGL_ES_API;
}
