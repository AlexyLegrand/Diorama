#ifndef PROGRESS_H
#define PROGRESS_H

#include <stdbool.h>
#include "world/level.h"

#define WORLD_MAX 8

typedef struct {
    int version;
    int world;
    bool completed[WORLD_MAX][LEVEL_MAX_ENTRANCES];
} Progress;

void progress_load(Progress *progress);

void progress_save(const Progress *progress);

#endif
