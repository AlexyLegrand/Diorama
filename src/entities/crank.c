#include "entities/crank.h"
#include "render/block_render.h"
#include "rlgl.h"

#define CRANK_DURATION (74.0f / 60.0f)

bool crank_start(CrankAnim *anim, Level *level, GridPos player)
{
    if (anim->active || !platform_rotate(level, player, &anim->move)) {
        return false;
    }
    anim->active = true;
    anim->progress = 0.0f;
    return true;
}

void crank_update(CrankAnim *anim)
{
    if (!anim->active) {
        return;
    }
    anim->progress += GetFrameTime() / CRANK_DURATION;
    if (anim->progress >= 1.0f) {
        anim->progress = 1.0f;
        anim->active = false;
    }
    block_render_set_crank_progress(anim->progress);
}

void crank_draw_platform(const CrankAnim *anim)
{
    float eased = anim->progress * anim->progress * (3.0f - 2.0f * anim->progress);
    float angle = -90.0f * eased;
    GridPos pivot = anim->move.pivot;
    Cell platform = { BLOCK_PLATFORM, DIR_NORTH };

    for (int i = 0; i < anim->move.count; i++) {
        GridPos cell = anim->move.cells[i];
        Vector3 offset;
        offset.x = (float)(cell.x - pivot.x);
        offset.y = 0.0f;
        offset.z = (float)(cell.z - pivot.z);

        rlPushMatrix();
        rlTranslatef((float)pivot.x, (float)cell.y, (float)pivot.z);
        rlRotatef(angle, 0.0f, 1.0f, 0.0f);
        draw_block_at(offset, platform);
        rlPopMatrix();
    }
}
