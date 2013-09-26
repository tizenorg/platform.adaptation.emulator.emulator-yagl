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
