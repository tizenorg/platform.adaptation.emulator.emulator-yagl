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

#include "yagl_sharegroup.h"
#include "yagl_object.h"
#include "yagl_malloc.h"

static void yagl_sharegroup_destroy(struct yagl_ref *ref)
{
    struct yagl_sharegroup *sg = (struct yagl_sharegroup*)ref;
    int i;

    for (i = 0; i < YAGL_NUM_NAMESPACES; ++i) {
        yagl_namespace_cleanup(&sg->namespaces[i]);
    }

    for (i = 0; i < YAGL_NUM_TEXTURE_TARGETS; ++i) {
        yagl_object_release(sg->texture_zero[i]);
    }

    yagl_ref_cleanup(&sg->ref);

    yagl_free(sg);
}

struct yagl_sharegroup *yagl_sharegroup_create(void)
{
    struct yagl_sharegroup *sg = yagl_malloc0(sizeof(struct yagl_sharegroup));
    int i;

    yagl_ref_init(&sg->ref, &yagl_sharegroup_destroy);

    for (i = 0; i < YAGL_NUM_NAMESPACES; ++i) {
        yagl_namespace_init(&sg->namespaces[i]);
    }

    return sg;
}

void yagl_sharegroup_acquire(struct yagl_sharegroup *sg)
{
    if (sg) {
        yagl_ref_acquire(&sg->ref);
    }
}

void yagl_sharegroup_release(struct yagl_sharegroup *sg)
{
    if (sg) {
        yagl_ref_release(&sg->ref);
    }
}

void yagl_sharegroup_add(struct yagl_sharegroup *sg,
                         int ns,
                         struct yagl_object *obj)
{
    yagl_namespace_add(&sg->namespaces[ns], obj);
}

struct yagl_object *yagl_sharegroup_add_named(struct yagl_sharegroup *sg,
                                              int ns,
                                              yagl_object_name local_name,
                                              struct yagl_object *obj)
{
    return yagl_namespace_add_named(&sg->namespaces[ns], local_name, obj);
}

void yagl_sharegroup_remove(struct yagl_sharegroup *sg,
                            int ns,
                            yagl_object_name local_name)
{
    yagl_namespace_remove(&sg->namespaces[ns], local_name);
}

struct yagl_object *yagl_sharegroup_acquire_object(struct yagl_sharegroup *sg,
                                                   int ns,
                                                   yagl_object_name local_name)
{
    return yagl_namespace_acquire(&sg->namespaces[ns], local_name);
}
