#ifndef PROGRESS_H
#define PROGRESS_H

#include <stdbool.h>
#include "world/level.h"

#define WORLD_MAX 8

typedef struct {
    int version;
    int world;
    bool completed[WORLD_MAX][LEVEL_MAX_ENTRANCES];
    int seeds;
    int collectibles;
    unsigned int collected[WORLD_MAX][LEVEL_MAX_ENTRANCES];
} Progress;

void progress_load(Progress *progress);

void progress_save(const Progress *progress);

bool progress_is_new(const Progress *progress);

void progress_reset(Progress *progress);

void progress_complete_level(Progress *progress, int world, int level);

bool progress_has_collectible(const Progress *progress, int world, int level, int index);

void progress_collect(Progress *progress, int world, int level, int index);

#endif
