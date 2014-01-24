#define _GNU_SOURCE
#define _XOPEN_SOURCE 600
#include "yagl_state.h"
#include "yagl_log.h"
#include "yagl_ioctl.h"
#include "yagl_malloc.h"
#include "yagl_offscreen.h"
#include "yagl_onscreen.h"
#include "yagl_backend.h"
#include "yagl_transport.h"
#include "yagl_utils.h"
#include "yagl_display.h"
#include "yagl_context.h"
#include "yagl_fence.h"
#include "yagl_egl_state.h"
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define YAGL_DEFAULT_MAX_BUFF_SIZE 1048576
#define YAGL_DEFAULT_MAX_CALL_SIZE 1048576

#define YAGL_REG_BUFFPTR 0
#define YAGL_REG_TRIGGER 4
#define YAGL_REGS_SIZE   8

#define YAGL_USER_PTR(regs, index) ((regs) + ((index) * YAGL_REGS_SIZE))

struct yagl_state
{
    int fd;

    uint8_t *regs;
    uint32_t user_index;

    uint8_t *buff;
    uint32_t buff_size;

    struct yagl_transport *t;

    struct yagl_backend *backend;

    yagl_gl_version gl_version;

    uint8_t *tmp_buff;
    uint32_t tmp_buff_size;

    uint8_t *tmp_buff2;
    uint32_t tmp_buff2_size;

    uint8_t *tmp_buff3;
    uint32_t tmp_buff3_size;
    
    struct yagl_display *fence_dpy;
};

static pthread_key_t g_state_key;
static pthread_once_t g_state_key_init = PTHREAD_ONCE_INIT;

static pthread_mutex_t g_name_gen_mutex;
static uint32_t g_name_gen_next = 0;

static void *yagl_state_transport_resize(void *ops_data, uint32_t size)
{
    struct yagl_state *state = ops_data;
    void *buff = mmap(NULL,
                      size,
                      PROT_READ|PROT_WRITE,
                      MAP_SHARED,
                      state->fd,
                      sysconf(_SC_PAGE_SIZE));

    if (buff == MAP_FAILED) {
        fprintf(stderr, "Unable to resize YaGL buffer to %u: %s!\n",
                size,
                strerror(errno));
        return NULL;
    }

    munmap(state->buff, state->buff_size);

    state->buff = buff;
    state->buff_size = size;

    return buff;
}

static void yagl_state_transport_commit(void *ops_data, int sync)
{
    struct yagl_state *state = ops_data;
    volatile uint32_t *trigger =
        (uint32_t*)(YAGL_USER_PTR(state->regs, state->user_index) +
                    YAGL_REG_TRIGGER);

    *trigger = sync;
}

static uint32_t yagl_state_transport_flush(void *ops_data, struct yagl_egl_fence **fence)
{
    struct yagl_state *state = ops_data;
    struct yagl_fence *fence_obj = NULL;
    struct yagl_context *ctx = yagl_get_context();

    if ((!ctx || ctx->need_throttle) && !fence) {
        /*
         * Throttle already set up and caller
         * doesn't want a fence reference - no-op.
         */

        return 0;
    }

    if (fence && *fence) {
        fence_obj = (struct yagl_fence*)*fence;
    } else if (state->fence_dpy) {
        fence_obj = state->backend->create_fence(state->fence_dpy);
    }

    if (ctx && !ctx->need_throttle && (!fence || *fence)) {
        /*
         * Throttle haven't been set up yet and caller doesn't
         * want a fence reference or he passed his own fence -
         * set up throttle.
         *
         * Note that if the caller wants a fence reference then we
         * don't want to set up throttle - the caller will wait for
         * that fence anyway, so throttling is not necessary.
         */
        yagl_context_set_need_throttle(ctx, fence_obj);
    }

    if (!fence_obj) {
        if (fence) {
            *fence = NULL;
        }
        return 0;
    }

    if (fence) {
        *fence = &fence_obj->base;
        return fence_obj->seq;
    } else {
        uint32_t seq = fence_obj->seq;
        yagl_fence_release(fence_obj);
        return seq;
    }
}

static struct yagl_transport_ops yagl_state_transport_ops =
{
    .resize = yagl_state_transport_resize,
    .commit = yagl_state_transport_commit,
    .flush = yagl_state_transport_flush
};

static void yagl_state_free(void *ptr)
{
    struct yagl_state *state = ptr;

    YAGL_LOG_FUNC_ENTER(yagl_state_free, "%p", ptr);

    if (!ptr) {
        return;
    }

    state->backend->destroy(state->backend);

    yagl_transport_destroy(state->t);

    munmap(state->buff, state->buff_size);
    munmap(state->regs, sysconf(_SC_PAGE_SIZE));

    close(state->fd);

    yagl_free(state->tmp_buff);
    yagl_free(state->tmp_buff2);
    yagl_free(state->tmp_buff3);

    yagl_free(state);

    YAGL_LOG_FUNC_EXIT(NULL);
}

static void yagl_state_key_init()
{
    pthread_key_create(&g_state_key, yagl_state_free);

    yagl_mutex_init(&g_name_gen_mutex);
}

static void yagl_state_atfork()
{
    struct yagl_state *state;

    /*
     * We're currently in forked child, this means that someone
     * issued some GL calls and then forked!
     * The only thing we can do here is resetting the state
     * and make the next GL call behave as the first one.
     */

    YAGL_LOG_FUNC_SET(yagl_state_atfork);

    YAGL_LOG_WARN("Forking after GL calls!");

    pthread_once(&g_state_key_init, yagl_state_key_init);

    state = (struct yagl_state*)pthread_getspecific(g_state_key);

    if (state) {
        /*
         * 'fork' was called by a GL thread. Close
         * parent resources here.
         */

        yagl_display_atfork();

        munmap(state->buff, state->buff_size);
        munmap(state->regs, sysconf(_SC_PAGE_SIZE));

        close(state->fd);

        yagl_free(state->tmp_buff);
        yagl_free(state->tmp_buff2);
        yagl_free(state->tmp_buff3);

        yagl_free(state);
    } else {
        /*
         * 'fork' was called by a non-GL thread. No-op.
         */
    }

    pthread_setspecific(g_state_key, NULL);
}

static struct yagl_state *yagl_get_state()
{
    struct yagl_state *state;
    unsigned int version = 0;
    struct yagl_user_info user_info = { 0, 0 };
    char *tmp;
    int max_buff_size, max_call_size;

    pthread_once(&g_state_key_init, yagl_state_key_init);

    state = (struct yagl_state*)pthread_getspecific(g_state_key);

    if (state) {
        return state;
    }

    YAGL_LOG_FUNC_ENTER(yagl_state_init, NULL);

    state = yagl_malloc0(sizeof(struct yagl_state));

    state->fd = open("/dev/yagl", O_RDWR | O_SYNC | O_CLOEXEC);

    if (state->fd == -1) {
        fprintf(stderr, "Critical error! Unable to open YaGL kernel device: %s!\n",
                strerror(errno));
        exit(1);
    }

    if (ioctl(state->fd, YAGL_IOC_GET_VERSION, &version) == -1) {
        fprintf(stderr, "Critical error! Unable to get YaGL version: %s!\n",
                strerror(errno));
        exit(1);
    }

    if (version != YAGL_VERSION) {
        fprintf(stderr,
                "Critical error! YaGL version mismatch: version is %u, but %u is expected!\n",
                version,
                YAGL_VERSION );
        exit(1);
    }

    if (ioctl(state->fd, YAGL_IOC_GET_USER_INFO, &user_info) == -1) {
        fprintf(stderr, "Critical error! Unable to get YaGL user info: %s!\n",
                strerror(errno));
        exit(1);
    }

    if ((user_info.index < 0) ||
        (user_info.index >= (sysconf(_SC_PAGE_SIZE) / YAGL_REGS_SIZE))) {
        fprintf(stderr, "Critical error! Bad user index: %d!\n",
                user_info.index);
        exit(1);
    }

    state->user_index = user_info.index;

    switch (user_info.render_type) {
    case yagl_render_type_offscreen:
    case yagl_render_type_onscreen:
        break;
    default:
        fprintf(stderr, "Critical error! Bad render type reported by kernel: %d!\n",
                user_info.render_type);
        exit(1);
    }

    switch (user_info.gl_version) {
    case yagl_gl_2:
    case yagl_gl_3_1:
    case yagl_gl_3_1_es3:
    case yagl_gl_3_2:
        break;
    default:
        fprintf(stderr, "Critical error! Bad host OpenGL version reported by kernel: %d!\n",
                user_info.gl_version);
        exit(1);
    }

    state->regs = mmap(NULL,
                       sysconf(_SC_PAGE_SIZE),
                       PROT_READ|PROT_WRITE,
                       MAP_SHARED,
                       state->fd,
                       0);

    if (state->regs == MAP_FAILED) {
        fprintf(stderr, "Critical error! Unable to map YaGL regs memory: %s!\n",
                strerror(errno));
        exit(1);
    }

    tmp = getenv("YAGL_MAX_BUFF_SIZE");
    max_buff_size = tmp ? atoi(tmp) : 0;

    tmp = getenv("YAGL_MAX_CALL_SIZE");
    max_call_size = tmp ? atoi(tmp) : 0;

    if (max_buff_size <= 0) {
        max_buff_size = YAGL_DEFAULT_MAX_BUFF_SIZE;
    }

    if (max_call_size <= 0) {
        max_call_size = YAGL_DEFAULT_MAX_CALL_SIZE;
    }

    state->t = yagl_transport_create(&yagl_state_transport_ops,
                                     state,
                                     max_buff_size,
                                     max_call_size);

    if (!state->t) {
        fprintf(stderr, "Critical error! Unable to create YaGL transport!\n");
        exit(1);
    }

    switch (user_info.render_type) {
    case yagl_render_type_offscreen:
        state->backend = yagl_offscreen_create();
        break;
    case yagl_render_type_onscreen:
        state->backend = yagl_onscreen_create();
        break;
    default:
        break;
    }

    state->gl_version = user_info.gl_version;

    pthread_setspecific(g_state_key, state);

    pthread_atfork(NULL, NULL, &yagl_state_atfork);

    YAGL_LOG_FUNC_EXIT("%p", state);

    return state;
}

struct yagl_transport *yagl_get_transport()
{
    return yagl_get_state()->t;
}

uint8_t *yagl_get_tmp_buffer(uint32_t size)
{
    struct yagl_state *state = yagl_get_state();

    if (size <= state->tmp_buff_size) {
        return state->tmp_buff;
    }

    yagl_free(state->tmp_buff);

    state->tmp_buff_size = size;
    state->tmp_buff = yagl_malloc(state->tmp_buff_size);

    return state->tmp_buff;
}

uint8_t *yagl_get_tmp_buffer2(uint32_t size)
{
    struct yagl_state *state = yagl_get_state();

    if (size <= state->tmp_buff2_size) {
        return state->tmp_buff2;
    }

    yagl_free(state->tmp_buff2);

    state->tmp_buff2_size = size;
    state->tmp_buff2 = yagl_malloc(state->tmp_buff2_size);

    return state->tmp_buff2;
}

uint8_t *yagl_get_tmp_buffer3(uint32_t size)
{
    struct yagl_state *state = yagl_get_state();

    if (size <= state->tmp_buff3_size) {
        return state->tmp_buff3;
    }

    yagl_free(state->tmp_buff3);

    state->tmp_buff3_size = size;
    state->tmp_buff3 = yagl_malloc(state->tmp_buff3_size);

    return state->tmp_buff3;
}

yagl_object_name yagl_get_global_name()
{
    uint32_t ret;

    pthread_once(&g_state_key_init, yagl_state_key_init);

    pthread_mutex_lock(&g_name_gen_mutex);

    if (!g_name_gen_next) {
        /*
         * 0 handles are invalid.
         */

        ++g_name_gen_next;
    }

    ret = g_name_gen_next++;

    pthread_mutex_unlock(&g_name_gen_mutex);

    return ret;
}

yagl_gl_version yagl_get_host_gl_version()
{
    return yagl_get_state()->gl_version;
}

struct yagl_backend *yagl_get_backend()
{
    return yagl_get_state()->backend;
}

int yagl_mlock(void *ptr, uint32_t size)
{
    struct yagl_mlock_arg arg;

    arg.address = (unsigned long)ptr;
    arg.size = size;

    return ioctl(yagl_get_state()->fd, YAGL_IOC_MLOCK, &arg);
}

int yagl_munlock(void *ptr)
{
    unsigned long arg = (unsigned long)ptr;

    return ioctl(yagl_get_state()->fd, YAGL_IOC_MUNLOCK, &arg);
}

void yagl_set_fence_display(struct yagl_display *fence_dpy)
{
    struct yagl_state *state = yagl_get_state();

    if (!state->fence_dpy) {
        state->fence_dpy = fence_dpy;
    }
}
