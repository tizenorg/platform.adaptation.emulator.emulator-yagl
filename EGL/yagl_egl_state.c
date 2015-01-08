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

#include "yagl_egl_state.h"
#include "yagl_context.h"
#include "yagl_surface.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_client_interface.h"
#include "yagl_client_context.h"
#include <pthread.h>
#include <dlfcn.h>

struct yagl_egl_state
{
    EGLint error;

    EGLenum api;

    struct yagl_context *ctx;
    struct yagl_surface *draw_sfc;
    struct yagl_surface *read_sfc;

    struct yagl_client_interface *gles1_iface;
    struct yagl_client_interface *gles2_iface;
};

static pthread_key_t g_state_key;
static pthread_once_t g_state_key_init = PTHREAD_ONCE_INIT;

void *yagl_get_gles1_sym(const char *name)
{
    void *handle;
    void *sym = dlsym(NULL, name);

    if (sym) {
        return sym;
    }

    handle = dlopen("libGLESv1_CM.so.1", RTLD_NOW|RTLD_GLOBAL);
    if (!handle) {
        handle = dlopen("libGLESv1_CM.so", RTLD_NOW|RTLD_GLOBAL);
    }

    if (handle) {
        return dlsym(handle, name);
    }

    return NULL;
}

void *yagl_get_gles2_sym(const char *name)
{
    void *handle;
    void *sym = dlsym(NULL, name);

    if (sym) {
        return sym;
    }

    handle = dlopen("libGLESv2.so.1", RTLD_NOW|RTLD_GLOBAL);
    if (!handle) {
        handle = dlopen("libGLESv2.so", RTLD_NOW|RTLD_GLOBAL);
    }

    if (handle) {
        return dlsym(handle, name);
    }

    return NULL;
}

static void yagl_egl_state_free(void* ptr)
{
    struct yagl_egl_state *state = ptr;

    YAGL_LOG_FUNC_ENTER(yagl_egl_state_free, "%p", ptr);

    if (!ptr) {
        return;
    }

    yagl_surface_release(state->read_sfc);
    yagl_surface_release(state->draw_sfc);
    yagl_context_release(state->ctx);

    yagl_free(state);

    YAGL_LOG_FUNC_EXIT(NULL);
}

static void yagl_egl_state_key_init()
{
    pthread_key_create(&g_state_key, yagl_egl_state_free);
}

static void yagl_egl_state_atfork()
{
    struct yagl_egl_state *state;

    /*
     * See yagl_state.c:yagl_state_atfork.
     */

    pthread_once(&g_state_key_init, yagl_egl_state_key_init);

    state = (struct yagl_egl_state*)pthread_getspecific(g_state_key);

    if (state) {
        yagl_free(state);
    }

    pthread_setspecific(g_state_key, NULL);
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

    pthread_atfork(NULL, NULL, &yagl_egl_state_atfork);

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

struct yagl_surface *yagl_get_draw_surface()
{
    struct yagl_egl_state *state = yagl_egl_get_state();

    return state->draw_sfc;
}

struct yagl_surface *yagl_get_read_surface()
{
    struct yagl_egl_state *state = yagl_egl_get_state();

    return state->read_sfc;
}

int yagl_set_context(struct yagl_context *ctx,
                     struct yagl_surface *draw_sfc,
                     struct yagl_surface *read_sfc)
{
    struct yagl_egl_state *state = yagl_egl_get_state();
    int ctx_marked = 0, draw_sfc_marked = 0;

    if (ctx && (state->ctx != ctx)) {
        if (!yagl_context_mark_current(ctx, 1)) {
            goto fail;
        }
        ctx_marked = 1;
    }

    if (draw_sfc && (state->draw_sfc != draw_sfc)) {
        if (!yagl_surface_mark_current(draw_sfc, 1)) {
            goto fail;
        }
        draw_sfc_marked = 1;
    }

    if (read_sfc &&
        (state->read_sfc != read_sfc) &&
        (read_sfc != draw_sfc) &&
        !yagl_surface_mark_current(read_sfc, 1)) {
        goto fail;
    }

    if (state->ctx && (state->ctx != ctx)) {
        yagl_context_mark_current(state->ctx, 0);
    }

    if (state->draw_sfc && (state->draw_sfc != draw_sfc)) {
        yagl_surface_mark_current(state->draw_sfc, 0);
    }

    if (state->read_sfc &&
        (state->read_sfc != read_sfc) &&
        (state->read_sfc != state->draw_sfc)) {
        yagl_surface_mark_current(state->read_sfc, 0);
    }

    yagl_context_acquire(ctx);
    yagl_surface_acquire(draw_sfc);
    yagl_surface_acquire(read_sfc);

    yagl_surface_release(state->read_sfc);
    yagl_surface_release(state->draw_sfc);
    yagl_context_release(state->ctx);

    state->ctx = ctx;
    state->read_sfc = read_sfc;
    state->draw_sfc = draw_sfc;

    return 1;

fail:
    if (ctx_marked) {
        yagl_context_mark_current(ctx, 0);
    }
    if (draw_sfc_marked) {
        yagl_surface_mark_current(draw_sfc, 0);
    }

    return 0;
}

void yagl_reset_state()
{
    struct yagl_egl_state *state = yagl_egl_get_state();

    yagl_set_context(NULL, NULL, NULL);

    state->error = EGL_SUCCESS;
    state->api = EGL_OPENGL_ES_API;
}

struct yagl_client_context *yagl_get_client_context(void)
{
    struct yagl_egl_state *state = yagl_egl_get_state();

    return state->ctx ? state->ctx->client_ctx : NULL;
}

struct yagl_client_interface *yagl_get_client_interface(yagl_client_api client_api)
{
    struct yagl_egl_state *state = yagl_egl_get_state();

    switch (client_api) {
    case yagl_client_api_gles1:
        if (!state->gles1_iface) {
            state->gles1_iface = yagl_get_gles1_sym("yagl_gles1_interface");
        }
        return state->gles1_iface;
    case yagl_client_api_gles2:
    case yagl_client_api_gles3:
        if (!state->gles2_iface) {
            state->gles2_iface = yagl_get_gles1_sym("yagl_gles2_interface");
        }
        return state->gles2_iface;
    default:
        return NULL;
    }
}

struct yagl_client_interface *yagl_get_any_client_interface()
{
    struct yagl_client_interface *iface;

    iface = yagl_get_client_interface(yagl_client_api_gles2);
    if (!iface) {
        iface = yagl_get_client_interface(yagl_client_api_gles1);
    }

    return iface;
}
