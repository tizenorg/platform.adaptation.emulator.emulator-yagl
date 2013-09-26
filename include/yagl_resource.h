#ifndef _YAGL_RESOURCE_H_
#define _YAGL_RESOURCE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_ref.h"
#include "yagl_list.h"

struct yagl_resource
{
    struct yagl_ref ref;

    struct yagl_list list;

    yagl_host_handle handle;
};

/*
 * For implementations.
 * @{
 */

YAGL_API void yagl_resource_init(struct yagl_resource *res,
                                 yagl_ref_destroy_func destroy,
                                 yagl_host_handle handle);
YAGL_API void yagl_resource_cleanup(struct yagl_resource *res);

/*
 * @}
 */

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_resource_acquire(struct yagl_resource *res);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_resource_release(struct yagl_resource *res);

#endif
