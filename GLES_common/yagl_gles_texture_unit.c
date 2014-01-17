#include "GLES/gl.h"
#include "yagl_gles_texture_unit.h"
#include "yagl_gles_texture.h"
#include "yagl_gles_sampler.h"
#include <string.h>

void yagl_gles_texture_unit_init(struct yagl_gles_texture_unit *texture_unit)
{
    memset(texture_unit, 0, sizeof(*texture_unit));
}

void yagl_gles_texture_unit_cleanup(struct yagl_gles_texture_unit *texture_unit)
{
    int i;

    for (i = 0; i < YAGL_NUM_GLES_TEXTURE_TARGETS; ++i) {
        yagl_gles_texture_release(texture_unit->target_states[i].texture);
        texture_unit->target_states[i].texture = NULL;
    }

    yagl_gles_sampler_release(texture_unit->sampler);
}
