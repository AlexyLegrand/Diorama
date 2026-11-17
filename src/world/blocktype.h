#ifndef BLOCKTYPE_H
#define BLOCKTYPE_H

#include "raylib.h"

typedef enum {
    BLOCK_EMPTY,
    BLOCK_GROUND,
    BLOCK_CRATE,
    BLOCK_STAIRS,
    BLOCK_LADDER,
    BLOCK_STAR,
    BLOCK_CRANK,
    BLOCK_PLATFORM,
    BLOCK_PROP,
    BLOCK_COUNT
} BlockType;

typedef enum {
    DIR_NORTH,
    DIR_EAST,
    DIR_SOUTH,
    DIR_WEST
} Direction;

typedef struct {
    BlockType type;
    Direction dir;
} Cell;

typedef struct {
    bool solid;
    bool walkable_on;
    Color color;
} BlockInfo;

extern const BlockInfo BLOCK_INFO[BLOCK_COUNT];

#endif
