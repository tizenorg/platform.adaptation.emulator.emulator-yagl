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

#ifndef _YAGL_LIST_H_
#define _YAGL_LIST_H_

#include "yagl_types.h"

struct yagl_list
{
    struct yagl_list* prev;
    struct yagl_list* next;
};

/*
 * Private interface.
 */

static __inline void __yagl_list_add( struct yagl_list* nw,
                                      struct yagl_list* prev,
                                      struct yagl_list* next )
{
    next->prev = nw;
    nw->next = next;
    nw->prev = prev;
    prev->next = nw;
}

static __inline void __yagl_list_remove( struct yagl_list* prev,
                                         struct yagl_list* next )
{
    next->prev = prev;
    prev->next = next;
}

/*
 * Public interface.
 */

#define YAGL_DECLARE_LIST(name) struct yagl_list name = { &(name), &(name) }

static __inline void yagl_list_init(struct yagl_list* list)
{
    list->next = list;
    list->prev = list;
}

static __inline void yagl_list_add(struct yagl_list* head, struct yagl_list* nw)
{
    __yagl_list_add(nw, head, head->next);
}

static __inline void yagl_list_add_tail(struct yagl_list* head, struct yagl_list* nw)
{
    __yagl_list_add(nw, head->prev, head);
}

static __inline void yagl_list_remove(struct yagl_list* entry)
{
    __yagl_list_remove(entry->prev, entry->next);
    yagl_list_init(entry);
}

static __inline int yagl_list_empty(const struct yagl_list* head)
{
    return ( (head->next == head) && (head->prev == head) );
}

#define yagl_list_first(container_type, iter, head, member) iter = yagl_containerof((head)->next, container_type, member)

#define yagl_list_last(container_type, iter, head, member) iter = yagl_containerof((head)->prev, container_type, member)

/*
 * Iterate over list in direct and reverse order.
 */

#define yagl_list_for_each(container_type, iter, head, member) \
    for ( iter = yagl_containerof((head)->next, container_type, member); \
          &iter->member != (head); \
          iter = yagl_containerof(iter->member.next, container_type, member) )

#define yagl_list_for_each_reverse(container_type, iter, head, member) \
    for ( iter = yagl_containerof((head)->prev, container_type, member); \
          &iter->member != (head); \
          iter = yagl_containerof(iter->member.prev, container_type, member) )

/*
 * Iterate over list in direct and reverse order, safe to list entries removal.
 */

#define yagl_list_for_each_safe(container_type, iter, tmp_iter, head, member) \
    for ( iter = yagl_containerof((head)->next, container_type, member), \
          tmp_iter = yagl_containerof(iter->member.next, container_type, member); \
          &iter->member != (head); \
          iter = tmp_iter, tmp_iter = yagl_containerof(tmp_iter->member.next, container_type, member) )

#define yagl_list_for_each_safe_reverse(container_type, iter, tmp_iter, head, member) \
    for ( iter = yagl_containerof((head)->prev, container_type, member), \
          tmp_iter = yagl_containerof(iter->member.prev, container_type, member); \
          &iter->member != (head); \
          iter = tmp_iter, tmp_iter = yagl_containerof(tmp_iter->member.prev, container_type, member) )

#endif
