#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include "world/level.h"

#define PLATFORM_MAX_CELLS 32

typedef struct {
    GridPos cells[PLATFORM_MAX_CELLS];
    int count;
    GridPos pivot;
} PlatformMove;

bool platform_rotate(Level *level, GridPos player, PlatformMove *move);

#endif
