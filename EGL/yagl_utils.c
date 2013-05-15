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

void yagl_recursive_mutex_init(pthread_mutex_t* mutex)
{
    pthread_mutexattr_t attrs;

    pthread_mutexattr_init(&attrs);
    pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_RECURSIVE);

    if (pthread_mutex_init(mutex, &attrs) != 0)
    {
        fprintf(stderr, "Critical error! Unable to init recursive mutex!\n");
        exit(1);
    }
}
