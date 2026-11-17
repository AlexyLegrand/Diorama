#include "world/movement.h"
#include <math.h>

static const int DIR_DX[4] = {
    [DIR_NORTH] = 0,
    [DIR_EAST]  = 1,
    [DIR_SOUTH] = 0,
    [DIR_WEST]  = -1,
};

static const int DIR_DZ[4] = {
    [DIR_NORTH] = -1,
    [DIR_EAST]  = 0,
    [DIR_SOUTH] = 1,
    [DIR_WEST]  = 0,
};

static Direction opposite(Direction dir)
{
    return (dir + 2) % 4;
}

static bool in_bounds(const Level *level, int x, int y, int z)
{
    bool x_ok = x >= 0 && x < level->width;
    bool y_ok = y >= 0 && y < level->height;
    bool z_ok = z >= 0 && z < level->depth;

    return x_ok && y_ok && z_ok;
}

static bool is_passable(const Level *level, int x, int y, int z)
{
    if (!in_bounds(level, x, y, z)) {
        return false;
    }
    BlockType type = level->cells[y][x][z].type;
    return type == BLOCK_EMPTY || type == BLOCK_STAR || type == BLOCK_LADDER;
}

static bool can_stand(const Level *level, int x, int y, int z)
{
    if (!is_passable(level, x, y, z) || y == 0) {
        return false;
    }
    BlockType below = level->cells[y - 1][x][z].type;
    return BLOCK_INFO[below].walkable_on;
}

static bool is_cell(const Level *level, int x, int y, int z, BlockType type, Direction dir)
{
    if (!in_bounds(level, x, y, z)) {
        return false;
    }
    Cell cell = level->cells[y][x][z];
    return cell.type == type && cell.dir == dir;
}

static void path_add(MovePath *path, float x, float y, float z, bool climbing, Direction facing)
{
    PathPoint *point = &path->points[path->count];

    point->position.x = x;
    point->position.y = y;
    point->position.z = z;
    point->climbing = climbing;
    point->facing = facing;
    path->count++;
}

static void path_end(MovePath *path, int x, int y, int z)
{
    path->end_x = x;
    path->end_y = y;
    path->end_z = z;
}

static bool plan_ladder_up(const Level *level, int x, int y, int z, Direction dir, MovePath *path)
{
    int dx = DIR_DX[dir];
    int dz = DIR_DZ[dir];
    int top = y;

    if (!is_cell(level, x, y, z, BLOCK_LADDER, dir)) {
        return false;
    }
    while (is_cell(level, x, top + 1, z, BLOCK_LADDER, dir)) {
        top++;
    }
    int exit_y = top + 1;
    if (!is_passable(level, x, exit_y, z) || !can_stand(level, x + dx, exit_y, z + dz)) {
        return false;
    }
    path_add(path, x, exit_y, z, true, dir);
    path_add(path, x + dx, exit_y, z + dz, false, dir);
    path_end(path, x + dx, exit_y, z + dz);
    return true;
}

static bool plan_stairs_up(const Level *level, int x, int y, int z, Direction dir, MovePath *path)
{
    int dx = DIR_DX[dir];
    int dz = DIR_DZ[dir];
    int end_x = x + 2 * dx;
    int end_z = z + 2 * dz;

    if (!is_cell(level, x + dx, y, z + dz, BLOCK_STAIRS, dir) || !can_stand(level, end_x, y + 1, end_z)) {
        return false;
    }
    path_add(path, x + 0.5f * dx, y, z + 0.5f * dz, false, dir);
    path_add(path, x + 1.5f * dx, y + 1, z + 1.5f * dz, false, dir);
    path_add(path, end_x, y + 1, end_z, false, dir);
    path_end(path, end_x, y + 1, end_z);
    return true;
}

static bool plan_flat(const Level *level, int x, int y, int z, Direction dir, MovePath *path)
{
    int end_x = x + DIR_DX[dir];
    int end_z = z + DIR_DZ[dir];

    if (!can_stand(level, end_x, y, end_z)) {
        return false;
    }
    path_add(path, end_x, y, end_z, false, dir);
    path_end(path, end_x, y, end_z);
    return true;
}

static bool plan_stairs_down(const Level *level, int x, int y, int z, Direction dir, MovePath *path)
{
    int dx = DIR_DX[dir];
    int dz = DIR_DZ[dir];
    int end_x = x + 2 * dx;
    int end_z = z + 2 * dz;
    bool front_is_free = is_passable(level, x + dx, y, z + dz);
    bool stairs_below = is_cell(level, x + dx, y - 1, z + dz, BLOCK_STAIRS, opposite(dir));

    if (!front_is_free || !stairs_below || !can_stand(level, end_x, y - 1, end_z)) {
        return false;
    }
    path_add(path, x + 0.5f * dx, y, z + 0.5f * dz, false, dir);
    path_add(path, x + 1.5f * dx, y - 1, z + 1.5f * dz, false, dir);
    path_add(path, end_x, y - 1, end_z, false, dir);
    path_end(path, end_x, y - 1, end_z);
    return true;
}

static bool plan_ladder_down(const Level *level, int x, int y, int z, Direction dir, MovePath *path)
{
    int next_x = x + DIR_DX[dir];
    int next_z = z + DIR_DZ[dir];
    Direction wall_side = opposite(dir);
    int bottom = y - 1;

    if (!is_passable(level, next_x, y, next_z)) {
        return false;
    }
    if (!is_cell(level, next_x, bottom, next_z, BLOCK_LADDER, wall_side)) {
        return false;
    }
    while (is_cell(level, next_x, bottom - 1, next_z, BLOCK_LADDER, wall_side)) {
        bottom--;
    }
    if (!can_stand(level, next_x, bottom, next_z)) {
        return false;
    }
    path_add(path, next_x, y, next_z, false, dir);
    path_add(path, next_x, bottom, next_z, true, wall_side);
    path_end(path, next_x, bottom, next_z);
    return true;
}

static bool plan_push(const Level *level, int x, int y, int z, Direction dir, MovePath *path)
{
    int crate_x = x + DIR_DX[dir];
    int crate_z = z + DIR_DZ[dir];
    int side_x = crate_x + DIR_DX[dir];
    int side_z = crate_z + DIR_DZ[dir];

    if (!is_cell(level, crate_x, y, crate_z, BLOCK_CRATE, DIR_NORTH)) {
        return false;
    }
    if (!in_bounds(level, side_x, y, side_z) || level->cells[y][side_x][side_z].type != BLOCK_EMPTY) {
        return false;
    }
    int landing_y = y;
    while (landing_y > 0 && level->cells[landing_y - 1][side_x][side_z].type == BLOCK_EMPTY) {
        landing_y--;
    }
    bool player_floor_ok = y > 0 && BLOCK_INFO[level->cells[y - 1][crate_x][crate_z].type].walkable_on;
    if (!player_floor_ok) {
        return false;
    }
    path_add(path, crate_x, y, crate_z, false, dir);
    path_end(path, crate_x, y, crate_z);
    path->push = true;
    path->crate_from = (GridPos){ crate_x, y, crate_z };
    path->crate_side = (GridPos){ side_x, y, side_z };
    path->crate_to = (GridPos){ side_x, landing_y, side_z };
    return true;
}

void movement_apply(Level *level, const MovePath *path)
{
    if (!path->push) {
        return;
    }
    GridPos from = path->crate_from;
    GridPos to = path->crate_to;
    level->cells[to.y][to.x][to.z] = level->cells[from.y][from.x][from.z];
    level->cells[from.y][from.x][from.z] = (Cell){ BLOCK_EMPTY, DIR_NORTH };
}

bool movement_plan(const Level *level, int x, int y, int z, Direction dir, MovePath *path)
{
    path->count = 0;
    path->push = false;
    if (plan_push(level, x, y, z, dir, path)) {
        return true;
    }
    if (plan_ladder_up(level, x, y, z, dir, path)) {
        return true;
    }
    if (plan_stairs_up(level, x, y, z, dir, path)) {
        return true;
    }
    if (plan_flat(level, x, y, z, dir, path)) {
        return true;
    }
    if (plan_stairs_down(level, x, y, z, dir, path)) {
        return true;
    }
    return plan_ladder_down(level, x, y, z, dir, path);
}

Direction movement_input_direction(float camera_yaw, int input_x, int input_y)
{
    const float tie_bias = 0.01f;
    float forward_x = -cosf(camera_yaw);
    float forward_z = -sinf(camera_yaw);
    float right_x = sinf(camera_yaw);
    float right_z = -cosf(camera_yaw);
    float move_x = forward_x * input_y + right_x * input_x;
    float move_z = forward_z * input_y + right_z * input_x;
    float angle = atan2f(move_z, move_x) + tie_bias;
    int quarter = (int)floorf(angle / (PI / 2.0f) + 0.5f);
    int index = ((quarter % 4) + 4) % 4;

    static const Direction QUARTER_TO_DIR[4] = { DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_NORTH };
    return QUARTER_TO_DIR[index];
}
