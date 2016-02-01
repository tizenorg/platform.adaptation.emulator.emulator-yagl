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

#ifndef _YAGL_SHAREGROUP_H_
#define _YAGL_SHAREGROUP_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_ref.h"
#include "yagl_namespace.h"

#define YAGL_NUM_NAMESPACES 6

#define YAGL_NUM_TEXTURE_TARGETS 5

struct yagl_sharegroup
{
    struct yagl_ref ref;

    struct yagl_namespace namespaces[YAGL_NUM_NAMESPACES];

    struct yagl_object *texture_zero[YAGL_NUM_TEXTURE_TARGETS];
};

YAGL_API struct yagl_sharegroup *yagl_sharegroup_create(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_sharegroup_acquire(struct yagl_sharegroup *sg);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_sharegroup_release(struct yagl_sharegroup *sg);

/*
 * Adds an object to share group and sets its local name,
 * this acquires 'obj', so the
 * caller should release 'obj' if he doesn't want to use it and wants
 * it to belong to this share group alone.
 */
YAGL_API void yagl_sharegroup_add(struct yagl_sharegroup *sg,
                                  int ns,
                                  struct yagl_object *obj);

/*
 * Same as the above, but adds an object with local_name.
 * If an object with such local name already exists then 'obj' will be
 * released and the existing object will be acquired and returned.
 * Otherwise, 'obj' is acquired and returned.
 * Typical use-case for this function is:
 *
 * yagl_object *obj;
 * obj = yagl_sharegroup_acquire_object(sg, ns, local_name);
 * if (!obj) {
 *     obj = yagl_xxx_create(...);
 *     obj = yagl_sharegroup_add_named(sg, ns, local_name, obj);
 * }
 * // use 'obj'.
 * yagl_object_release(obj);
 */
YAGL_API struct yagl_object *yagl_sharegroup_add_named(struct yagl_sharegroup *sg,
                                                       int ns,
                                                       yagl_object_name local_name,
                                                       struct yagl_object *obj);

/*
 * Removes an object from the share group, this also releases the
 * object.
 */
YAGL_API void yagl_sharegroup_remove(struct yagl_sharegroup *sg,
                                     int ns,
                                     yagl_object_name local_name);

/*
 * Acquires an object by its local name. Be sure to release the object when
 * you're done.
 */
YAGL_API struct yagl_object *yagl_sharegroup_acquire_object(struct yagl_sharegroup *sg,
                                                            int ns,
                                                            yagl_object_name local_name);

#endif
