#define _GNU_SOURCE
#define _XOPEN_SOURCE 600
#include "yagl_state.h"
#include "yagl_log.h"
#include "yagl_version.h"
#include "yagl_malloc.h"
#include "yagl_marshal.h"
#include "yagl_mem.h"
#include "yagl_offscreen.h"
#include "yagl_onscreen.h"
#include "yagl_backend.h"
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define YAGL_DATA_SIZE 0x80000

#define YAGL_MAX_RETRIES 100

#define YAGL_REG_BUFFPTR 0
#define YAGL_REG_TRIGGER 4
#define YAGL_REGS_SIZE   8

#define YAGL_USER_PTR(regs_base, index) ((regs_base) + ((index) * YAGL_REGS_SIZE))

struct yagl_state
{
    int fd;
    uint8_t* regs_base;
    uint8_t* marshal_base;
    uint8_t* data_base;
    uint32_t user_index;

    /*
     * Batch processing.
     * @{
     */

    /*
     * This is the current marshal pointer, whenever
     * there's a risk of overflowing marshal memory area
     * marshal buffer is flushed and this pointer is
     * set back to 'marshal_base'.
     */
    uint8_t *batch_marshal;

    /*
     * Batch data is written by 'yagl_batch_put', whenever
     * batch data memory gets overflowed 'batch_data_overflow'
     * is set and further calls to 'yagl_batch_put'
     * return the incoming pointer without copying it.
     */
    uint8_t *batch_data;

    /*
     * Batch data overflow indicator.
     */
    int batch_data_overflow;

    /*
     * Number of retries in a row.
     */
    uint32_t retry_count;

    /*
     * @}
     */

    struct yagl_backend *backend;
};

static pthread_key_t g_state_key;
static pthread_once_t g_state_key_init = PTHREAD_ONCE_INIT;

static void yagl_state_free(void* ptr)
{
    struct yagl_state* state = ptr;

    YAGL_LOG_FUNC_ENTER(yagl_state_free, "%p", ptr);

    if (!ptr)
    {
        return;
    }

    state->backend->destroy(state->backend);

    munlock(state->data_base, YAGL_DATA_SIZE);

    yagl_free(state->data_base);

    munmap(state->regs_base, sysconf(_SC_PAGE_SIZE));
    munmap(state->marshal_base, YAGL_MARSHAL_SIZE);

    close(state->fd);

    yagl_free(state);

    YAGL_LOG_FUNC_EXIT(NULL);
}

static void yagl_state_key_init()
{
    pthread_key_create(&g_state_key, yagl_state_free);
}

static struct yagl_state* yagl_get_state()
{
    struct yagl_state* state;
    long int page_size;
    unsigned int version = 0;
    uint8_t *tmp;
    yagl_render_type render_type;

    pthread_once(&g_state_key_init, yagl_state_key_init);

    state = (struct yagl_state*)pthread_getspecific(g_state_key);

    if (state)
    {
        return state;
    }

    YAGL_LOG_FUNC_ENTER(yagl_state_init, NULL);

    state = yagl_malloc0(sizeof(struct yagl_state));

    state->fd = open("/dev/yagl", O_RDWR|O_SYNC|O_CLOEXEC);

    if (state->fd == -1)
    {
        fprintf(stderr, "Critical error! Unable to open YaGL kernel device: %s!\n", strerror(errno));
        exit(1);
    }

    if (ioctl(state->fd, YAGL_IOC_GET_VERSION, &version) == -1)
    {
        fprintf(stderr, "Critical error! Unable to get YaGL version: %s!\n", strerror(errno));
        exit(1);
    }

    if (version != YAGL_VERSION)
    {
        fprintf( stderr,
                 "Critical error! YaGL version mismatch: version is %u, but %u is expected!\n",
                 version,
                 YAGL_VERSION );
        exit(1);
    }

    page_size = sysconf(_SC_PAGE_SIZE);

    state->regs_base = mmap( NULL,
                             page_size,
                             PROT_READ|PROT_WRITE,
                             MAP_SHARED,
                             state->fd,
                             0 );

    if (state->regs_base == MAP_FAILED)
    {
        fprintf(stderr, "Critical error! Unable to map YaGL regs memory: %s!\n", strerror(errno));
        exit(1);
    }

    state->marshal_base = mmap( NULL,
                                YAGL_MARSHAL_SIZE,
                                PROT_READ|PROT_WRITE,
                                MAP_SHARED,
                                state->fd,
                                page_size );

    if (state->marshal_base == MAP_FAILED)
    {
        fprintf(stderr, "Critical error! Unable to map YaGL marshal memory: %s!\n", strerror(errno));
        exit(1);
    }

    tmp = state->marshal_base;

    state->user_index = yagl_marshal_get_uint32(&tmp);

    if (state->user_index > (uint32_t)(page_size / YAGL_REGS_SIZE))
    {
        fprintf(stderr, "Critical error! Bad user index: %d!\n", state->user_index);
        exit(1);
    }

    render_type = yagl_marshal_get_render_type(&tmp);

    state->data_base = yagl_malloc(YAGL_DATA_SIZE);

    /*
     * 'mlock', so the host would never get a page fault on reading buffered
     * GL call.
     */

    if (mlock(state->data_base, YAGL_DATA_SIZE) == -1)
    {
        fprintf(stderr, "Critical error! Unable to lock YaGL data memory: %s!\n", strerror(errno));
        exit(1);
    }

    /*
     * Probe in immediately.
     */

    yagl_mem_probe_read(state->data_base, YAGL_DATA_SIZE);

    state->batch_marshal = state->marshal_base;
    state->batch_data = state->data_base;
    state->batch_data_overflow = 0;

    switch (render_type) {
    case yagl_render_type_offscreen:
        state->backend = yagl_offscreen_create();
        break;
    case yagl_render_type_onscreen:
        state->backend = yagl_onscreen_create();
        break;
    default:
        fprintf(stderr, "Critical error! Bad render type reported by kernel: %d!\n", render_type);
        exit(1);
    }

    pthread_setspecific(g_state_key, state);

    YAGL_LOG_FUNC_EXIT("%p", state);

    return state;
}

uint8_t *yagl_batch_get_marshal()
{
    return yagl_get_state()->batch_marshal;
}

int yagl_batch_update_marshal(uint8_t *marshal)
{
    struct yagl_state *state = yagl_get_state();

    state->batch_marshal = marshal;

    if (((state->batch_marshal + YAGL_MARSHAL_MAX_REQUEST) >
         (state->marshal_base + YAGL_MARSHAL_SIZE)) ||
        state->batch_data_overflow)  {
        /*
         * If marshal buffer is full or data buffer is full then
         * force sync.
         */
        return yagl_batch_sync();
    }

    return 1;
}

const void *yagl_batch_put(const void *data, int len)
{
    uint8_t *tmp;
    struct yagl_state *state = yagl_get_state();

    if (len < 0) {
        len = 0;
    }

    if (!data || state->batch_data_overflow) {
        /*
         * If we can't copy the data to batch data memory we should at least
         * fault it in.
         */
        yagl_mem_probe_read(data, len);

        return data;
    }

    if ((state->batch_data + len) > (state->data_base + YAGL_DATA_SIZE)) {
        state->batch_data_overflow = 1;

        /*
         * If we can't copy the data to batch data memory we should at least
         * fault it in.
         */
        yagl_mem_probe_read(data, len);

        return data;
    }

    memcpy(state->batch_data, data, len);

    tmp = state->batch_data;

    state->batch_data += len;

    return tmp;
}

int yagl_batch_sync()
{
    struct yagl_state *state = yagl_get_state();
    volatile uint32_t *trigger =
        (uint32_t*)(YAGL_USER_PTR(state->regs_base, state->user_index) +
                    YAGL_REG_TRIGGER);
    yagl_call_result res;

    if (state->batch_marshal == state->marshal_base) {
        /*
         * No calls.
         */

        return 1;
    }

    /*
     * Insert batch terminator.
     */
    yagl_marshal_put_uint32(&state->batch_marshal, 0);

    *trigger = 1;

    state->batch_marshal = state->marshal_base;

    res = yagl_marshal_get_call_result(&state->batch_marshal);

    state->batch_marshal = state->marshal_base;
    state->batch_data = state->data_base;
    state->batch_data_overflow = 0;

    switch (res) {
    case yagl_call_result_retry:
        if (++state->retry_count >= YAGL_MAX_RETRIES) {
            fprintf(stderr,
                    "Critical error! Max retry count %u reached!\n",
                    state->retry_count);
            exit(1);
        }
        return 0;
    case yagl_call_result_ok:
        state->retry_count = 0;
        return 1;
    case yagl_call_result_fail:
        fprintf(stderr, "Critical error! Bad call!\n");
        exit(1);
        break;
    default:
        fprintf(stderr, "Critical error! Bad call result - %d!\n", res);
        exit(1);
        break;
    }
}

struct yagl_backend *yagl_get_backend()
{
    return yagl_get_state()->backend;
}
