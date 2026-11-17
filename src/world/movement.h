#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <stdbool.h>
#include "world/level.h"

#define PATH_MAX_POINTS 4

typedef struct {
    Vector3 position;
    bool climbing;
    Direction facing;
} PathPoint;

typedef struct {
    PathPoint points[PATH_MAX_POINTS];
    int count;
    int end_x;
    int end_y;
    int end_z;
    bool push;
    GridPos crate_from;
    GridPos crate_side;
    GridPos crate_to;
} MovePath;

bool movement_plan(const Level *level, int x, int y, int z, Direction dir, MovePath *path);

void movement_apply(Level *level, const MovePath *path);

Direction movement_input_direction(float camera_yaw, int input_x, int input_y);

#endif
