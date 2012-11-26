#include "yagl_utils.h"
#include <stdio.h>
#include <stdlib.h>

void yagl_mutex_init(pthread_mutex_t* mutex)
{
    if (pthread_mutex_init(mutex, NULL) != 0)
    {
        fprintf(stderr, "Critical error! Unable to init mutex!\n");
        exit(1);
    }
}
