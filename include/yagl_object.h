#ifndef _YAGL_OBJECT_H_
#define _YAGL_OBJECT_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_ref.h"

/*
 * This represents OpenGL client object such as buffer, texture, etc.
 * An object can have a "local name", i.e. name that is visible to an
 * OpenGL user, that name is unique only within a particular namespace
 * inside a particular context (or sharegroup to be more precise).
 * Object implementation will typically also have one or more
 * "global name"s, which are unique per-process, these names are
 * used for target <-> host interactions, they're not visible to
 * OpenGL user, they're implementation details.
 * It's possible to have an object without a local name, such object
 * won't be visible to OpenGL user, but can be used internally.
 *
 * In OpenGL one can often spot the following scenario:
 * N = glGenXXX(...) - creates object, returns its name N
 * glBindXXX(N, ...) - binds name N
 * glDeleteXXX(N, ...) - frees name N
 * glGetIntegerv(GL_XXX_BINDING, ...) - must return N though it's free
 * Thus, local name for an object must be available even after object
 * is removed from a namespace.
 */

struct yagl_object
{
    struct yagl_ref ref;

    yagl_object_name local_name;
};

/*
 * For implementations.
 * @{
 */

YAGL_API void yagl_object_init(struct yagl_object *obj,
                               yagl_ref_destroy_func destroy);
YAGL_API void yagl_object_cleanup(struct yagl_object *obj);

/*
 * @}
 */

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_object_acquire(struct yagl_object *obj);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_object_release(struct yagl_object *obj);

#endif
