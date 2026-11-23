#ifndef LEVEL_H
#define LEVEL_H

#include <stdbool.h>
#include "world/blocktype.h"

#define LEVEL_MAX_WIDTH  8
#define LEVEL_MAX_HEIGHT 6
#define LEVEL_MAX_DEPTH  8
#define LEVEL_MAX_ENTRANCES 9
#define LEVEL_MAX_DECOR 128

typedef enum {
    VEG_TREE_ROUND,
    VEG_TREE_PINE,
    VEG_BUSH,
    VEG_FERN,
    VEG_GRASS,
    VEG_FLOWERS,
    VEG_MUSHROOMS,
    VEG_IVY,
    VEG_VINE,
    VEG_COUNT
} Vegetation;

typedef struct {
    int x;
    int y;
    int z;
} GridPos;

typedef struct {
    Vegetation kind;
    GridPos pos;
} Decor;

typedef struct {
    int width;
    int height;
    int depth;
    int start_x;
    int start_y;
    int start_z;
    int entrance_count;
    GridPos entrances[LEVEL_MAX_ENTRANCES];
    bool has_portal_next;
    bool has_portal_prev;
    GridPos portal_next;
    GridPos portal_prev;
    int decor_count;
    Decor decor[LEVEL_MAX_DECOR];
    Cell cells[LEVEL_MAX_HEIGHT][LEVEL_MAX_WIDTH][LEVEL_MAX_DEPTH];
} Level;

bool level_load(Level *level, const char *path);

Vector3 level_center(const Level *level);

void level_draw(const Level *level, const GridPos *hidden, bool hide_platforms);

#endif
