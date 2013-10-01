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
