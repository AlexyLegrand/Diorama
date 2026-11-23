#ifndef WORLD_H
#define WORLD_H

#include <stddef.h>

#define WORLD_DIR_MAX 32
#define WORLD_NAME_MAX 32
#define WORLD_PATH_MAX 96
#define WORLD_MAP (-1)

typedef struct {
    char dir[WORLD_DIR_MAX];
    char name[WORLD_NAME_MAX];
} WorldInfo;

int worlds_load(WorldInfo worlds[], int max, const char *path);

void world_level_path(const WorldInfo *world, int level, char *out, size_t size);

#endif
