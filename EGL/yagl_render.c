#include "yagl_render.h"
#include "yagl_egl_state.h"
#include "yagl_surface.h"

void yagl_render_invalidate()
{
    struct yagl_surface *draw_sfc = yagl_get_draw_surface();
    struct yagl_surface *read_sfc = yagl_get_read_surface();

    if (draw_sfc) {
        draw_sfc->invalidate(draw_sfc);
    }

    if (read_sfc && (draw_sfc != read_sfc)) {
        read_sfc->invalidate(read_sfc);
    }
}

void yagl_render_finish()
{
    struct yagl_surface *draw_sfc = yagl_get_draw_surface();

    if (draw_sfc) {
        draw_sfc->finish(draw_sfc);
    }
}
