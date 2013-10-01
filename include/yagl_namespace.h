#ifndef _YAGL_NAMESPACE_H_
#define _YAGL_NAMESPACE_H_

#include "yagl_export.h"
#include "yagl_types.h"

struct yagl_object;
struct yagl_avl_table;

struct yagl_namespace
{
    struct yagl_avl_table *entries;

    yagl_object_name next_local_name;
};

YAGL_API void yagl_namespace_init(struct yagl_namespace *ns);

YAGL_API void yagl_namespace_cleanup(struct yagl_namespace *ns);

/*
 * Adds an object to namespace and sets its local name,
 * this acquires 'obj', so the
 * caller should release 'obj' if he doesn't want to use it and wants
 * it to belong to this namespace alone.
 */
YAGL_API void yagl_namespace_add(struct yagl_namespace *ns,
                                 struct yagl_object *obj);

/*
 * Same as the above, but adds an object with local_name.
 * If an object with such local name already exists then 'obj' will be
 * released and the existing object will be acquired and returned.
 * Otherwise, 'obj' is acquired and returned.
 * Typical use-case for this function is:
 *
 * yagl_object *obj;
 * obj = yagl_namespace_acquire(ns, local_name);
 * if (!obj) {
 *     obj = yagl_xxx_create(...);
 *     obj = yagl_namespace_add_named(ns, local_name, obj);
 * }
 * // use 'obj'.
 * yagl_object_release(obj);
 */
YAGL_API struct yagl_object *yagl_namespace_add_named(struct yagl_namespace *ns,
                                                      yagl_object_name local_name,
                                                      struct yagl_object *obj);

/*
 * Removes an object from the namespace, this also releases the
 * object.
 */
YAGL_API void yagl_namespace_remove(struct yagl_namespace *ns,
                                    yagl_object_name local_name);

/*
 * Acquires an object by its local name. Be sure to release the object when
 * you're done.
 */
YAGL_API struct yagl_object *yagl_namespace_acquire(struct yagl_namespace *ns,
                                                    yagl_object_name local_name);

#endif
