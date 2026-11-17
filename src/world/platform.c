#include "world/platform.h"

static bool collect_cells(const Level *level, PlatformMove *move)
{
    int min_x = level->width;
    int min_z = level->depth;
    int max_x = -1;
    int max_z = -1;

    move->count = 0;
    for (int y = 0; y < level->height; y++) {
        for (int x = 0; x < level->width; x++) {
            for (int z = 0; z < level->depth; z++) {
                if (level->cells[y][x][z].type != BLOCK_PLATFORM) {
                    continue;
                }
                if (move->count >= PLATFORM_MAX_CELLS) {
                    return false;
                }
                move->cells[move->count] = (GridPos){ x, y, z };
                move->count++;
                min_x = (x < min_x) ? x : min_x;
                min_z = (z < min_z) ? z : min_z;
                max_x = (x > max_x) ? x : max_x;
                max_z = (z > max_z) ? z : max_z;
            }
        }
    }
    move->pivot.x = (min_x + max_x) / 2;
    move->pivot.y = 0;
    move->pivot.z = (min_z + max_z) / 2;
    return move->count > 0;
}

static GridPos rotated(GridPos cell, GridPos pivot)
{
    int offset_x = cell.x - pivot.x;
    int offset_z = cell.z - pivot.z;
    GridPos result = { pivot.x - offset_z, cell.y, pivot.z + offset_x };

    return result;
}

static bool target_is_free(const Level *level, GridPos target)
{
    bool x_ok = target.x >= 0 && target.x < level->width;
    bool z_ok = target.z >= 0 && target.z < level->depth;
    if (!x_ok || !z_ok) {
        return false;
    }
    BlockType type = level->cells[target.y][target.x][target.z].type;
    return type == BLOCK_EMPTY || type == BLOCK_PLATFORM;
}

static bool player_rides(const PlatformMove *move, GridPos player)
{
    for (int i = 0; i < move->count; i++) {
        GridPos cell = move->cells[i];
        if (cell.x == player.x && cell.z == player.z && cell.y == player.y - 1) {
            return true;
        }
    }
    return false;
}

bool platform_rotate(Level *level, GridPos player, PlatformMove *move)
{
    if (!collect_cells(level, move) || player_rides(move, player)) {
        return false;
    }
    for (int i = 0; i < move->count; i++) {
        if (!target_is_free(level, rotated(move->cells[i], move->pivot))) {
            return false;
        }
    }
    Cell platform = level->cells[move->cells[0].y][move->cells[0].x][move->cells[0].z];
    for (int i = 0; i < move->count; i++) {
        GridPos cell = move->cells[i];
        level->cells[cell.y][cell.x][cell.z] = (Cell){ BLOCK_EMPTY, DIR_NORTH };
    }
    for (int i = 0; i < move->count; i++) {
        GridPos target = rotated(move->cells[i], move->pivot);
        level->cells[target.y][target.x][target.z] = platform;
    }
    return true;
}
