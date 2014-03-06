#ifndef _YAGL_GLES_TEXTURE_UNIT_H_
#define _YAGL_GLES_TEXTURE_UNIT_H_

#include "yagl_gles_types.h"
#include "yagl_sharegroup.h"

struct yagl_gles_texture;
struct yagl_gles_sampler;

struct yagl_gles_texture_target_state
{
    struct yagl_gles_texture *texture;

    /*
     * For GLESv1 only. In GLESv2 2D texture and cubemap textures cannot be
     * enabled/disabled. Currently not used.
     */
    int enabled;

    struct yagl_gles_texture *texture_zero;
};

struct yagl_gles_texture_unit
{
    struct yagl_gles_texture_target_state target_states[YAGL_NUM_TEXTURE_TARGETS];

    struct yagl_gles_sampler *sampler;
};

void yagl_gles_texture_unit_init(struct yagl_gles_texture_unit *texture_unit,
                                 struct yagl_sharegroup *sg);

void yagl_gles_texture_unit_cleanup(struct yagl_gles_texture_unit *texture_unit);

#endif
