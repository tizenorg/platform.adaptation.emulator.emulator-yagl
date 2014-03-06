#include "GL/gl.h"
#include "GL/glext.h"
#include "yagl_gles_texture_unit.h"
#include "yagl_gles_texture.h"
#include "yagl_gles_sampler.h"
#include <string.h>

static const GLenum target_map[] =
{
    GL_TEXTURE_2D,
    GL_TEXTURE_2D_ARRAY,
    GL_TEXTURE_3D,
    GL_TEXTURE_CUBE_MAP
};

void yagl_gles_texture_unit_init(struct yagl_gles_texture_unit *texture_unit,
                                 struct yagl_sharegroup *sg)
{
    memset(texture_unit, 0, sizeof(*texture_unit));

    int i;

    for (i = 0; i < YAGL_NUM_TEXTURE_TARGETS; ++i) {
        if (sg->texture_zero[i]) {
            yagl_object_acquire(sg->texture_zero[i]);
            texture_unit->target_states[i].texture_zero = (struct yagl_gles_texture*)sg->texture_zero[i];
        } else {
            texture_unit->target_states[i].texture_zero = yagl_gles_texture_create();
            sg->texture_zero[i] = &texture_unit->target_states[i].texture_zero->base;
            yagl_object_acquire(sg->texture_zero[i]);
        }

        yagl_gles_texture_bind(texture_unit->target_states[i].texture_zero,
                               target_map[i]);

        yagl_gles_texture_acquire(texture_unit->target_states[i].texture_zero);
        texture_unit->target_states[i].texture =
            texture_unit->target_states[i].texture_zero;
    }
}

void yagl_gles_texture_unit_cleanup(struct yagl_gles_texture_unit *texture_unit)
{
    int i;

    for (i = 0; i < YAGL_NUM_TEXTURE_TARGETS; ++i) {
        yagl_gles_texture_release(texture_unit->target_states[i].texture);
        yagl_gles_texture_release(texture_unit->target_states[i].texture_zero);
    }

    yagl_gles_sampler_release(texture_unit->sampler);
}
