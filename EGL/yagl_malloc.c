#include "yagl_malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *yagl_malloc(size_t size)
{
    void *tmp = malloc(size);
    if (!tmp) {
        fprintf( stderr,
                 "Critical error! Unable to allocate %u bytes!\n",
                 (unsigned int)size );
        exit(1);
        return 0;
    }
    return tmp;
}

void *yagl_malloc0(size_t size)
{
    void *tmp = yagl_malloc(size);
    memset(tmp, 0, size);
    return tmp;
}

void *yagl_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

void yagl_free(void* ptr)
{
    free(ptr);
}
