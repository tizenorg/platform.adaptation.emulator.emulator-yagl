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

#include "yagl_namespace.h"
#include "yagl_avl.h"
#include "yagl_object.h"
#include <assert.h>

static int yagl_namespace_entry_comparison_func(const void *avl_a,
                                                const void *avl_b,
                                                void *avl_param)
{
    const struct yagl_object *a = avl_a;
    const struct yagl_object *b = avl_b;

    if (a->local_name < b->local_name) {
        return -1;
    } else if (a->local_name > b->local_name) {
        return 1;
    } else {
        return 0;
    }
}

static void yagl_namespace_entry_destroy_func(void *avl_item, void *avl_param)
{
    struct yagl_object *item = avl_item;

    yagl_object_release(item);
}

void yagl_namespace_init(struct yagl_namespace *ns)
{
    ns->entries = yagl_avl_create(&yagl_namespace_entry_comparison_func,
                                  ns,
                                  NULL);
    assert(ns->entries);
    ns->next_local_name = 1;
}

void yagl_namespace_cleanup(struct yagl_namespace *ns)
{
    yagl_avl_destroy(ns->entries, &yagl_namespace_entry_destroy_func);
    ns->entries = NULL;
    ns->next_local_name = 0;
}

void yagl_namespace_add(struct yagl_namespace *ns,
                        struct yagl_object *obj)
{
    yagl_object_acquire(obj);

    do {
        if (!ns->next_local_name) {
            /*
             * 0 names are invalid.
             */

            ++ns->next_local_name;
        }

        obj->local_name = ns->next_local_name++;

        /*
         * Find a free local name.
         */
    } while (yagl_avl_insert(ns->entries, obj));
}

struct yagl_object *yagl_namespace_add_named(struct yagl_namespace *ns,
                                             yagl_object_name local_name,
                                             struct yagl_object *obj)
{
    struct yagl_object *dup_item;

    obj->local_name = local_name;

    dup_item = yagl_avl_insert(ns->entries, obj);

    if (dup_item) {
        obj->local_name = 0;

        yagl_namespace_entry_destroy_func(obj, ns->entries->avl_param);

        yagl_object_acquire(dup_item);

        return dup_item;
    } else {
        yagl_object_acquire(obj);

        return obj;
    }
}

void yagl_namespace_remove(struct yagl_namespace *ns,
                           yagl_object_name local_name)
{
    void *item;
    struct yagl_object dummy;

    dummy.local_name = local_name;

    item = yagl_avl_delete(ns->entries, &dummy);

    if (item) {
        yagl_namespace_entry_destroy_func(item, ns->entries->avl_param);
    }
}

struct yagl_object *yagl_namespace_acquire(struct yagl_namespace *ns,
                                           yagl_object_name local_name)
{
    struct yagl_object *item;
    struct yagl_object dummy;

    dummy.local_name = local_name;

    item = yagl_avl_find(ns->entries, &dummy);

    if (item) {
        yagl_object_acquire(item);
        return item;
    } else {
        return NULL;
    }
}
