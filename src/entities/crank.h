#ifndef CRANK_H
#define CRANK_H

#include <stdbool.h>
#include "world/platform.h"

typedef struct {
    bool active;
    float progress;
    PlatformMove move;
} CrankAnim;

bool crank_start(CrankAnim *anim, Level *level, GridPos player);

void crank_update(CrankAnim *anim);

void crank_draw_platform(const CrankAnim *anim);

#endif
