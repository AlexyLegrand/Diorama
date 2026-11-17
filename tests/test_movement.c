#include <assert.h>
#include <stdio.h>
#include "raylib.h"
#include "world/level.h"
#include "core/settings.h"
#include "world/movement.h"
#include "world/platform.h"

typedef struct {
    int x;
    int y;
    int z;
} Pos;

static Pos walk(Level *level, Pos pos, const char *moves)
{
    for (int i = 0; moves[i] != '\0'; i++) {
        Direction dir = DIR_NORTH;
        if (moves[i] == 'E') {
            dir = DIR_EAST;
        } else if (moves[i] == 'S') {
            dir = DIR_SOUTH;
        } else if (moves[i] == 'W') {
            dir = DIR_WEST;
        }
        MovePath path;
        bool ok = movement_plan(level, pos.x, pos.y, pos.z, dir, &path);
        if (!ok) {
            fprintf(stderr, "blocked at %d,%d,%d move %c\n", pos.x, pos.y, pos.z, moves[i]);
        }
        assert(ok);
        movement_apply(level, &path);
        pos.x = path.end_x;
        pos.y = path.end_y;
        pos.z = path.end_z;
    }
    return pos;
}

static Pos start_of(const Level *level)
{
    Pos pos = { level->start_x, level->start_y, level->start_z };
    return pos;
}

static void test_level_1(void)
{
    Level level;
    assert(level_load(&level, "levels/level_1.txt"));
    Pos pos = walk(&level, start_of(&level), "WWNNNEE");
    assert(pos.x == 2 && pos.y == 1 && pos.z == 1);
    pos = walk(&level, pos, "S");
    assert(pos.x == 2 && pos.y == 3 && pos.z == 2);
    assert(level.cells[pos.y][pos.x][pos.z].type == BLOCK_STAR);
    pos = walk(&level, pos, "N");
    assert(pos.x == 2 && pos.y == 1 && pos.z == 1);
    MovePath path;
    assert(!movement_plan(&level, 2, 1, 4, DIR_NORTH, &path) || path.end_y == 1);
}

static void test_level_2(void)
{
    Level level;
    assert(level_load(&level, "levels/level_2.txt"));
    Pos pos = walk(&level, start_of(&level), "EN");
    assert(pos.x == 3 && pos.y == 2 && pos.z == 2);
    pos = walk(&level, pos, "NWSE");
    assert(pos.x == 2 && pos.y == 3 && pos.z == 2);
    assert(level.cells[pos.y][pos.x][pos.z].type == BLOCK_STAR);
    pos = walk(&level, start_of(&level), "ENS");
    assert(pos.x == 3 && pos.y == 1 && pos.z == 4);
}

static void test_level_3(void)
{
    Level level;
    assert(level_load(&level, "levels/level_3.txt"));
    Pos pos = walk(&level, start_of(&level), "NNNNES");
    assert(pos.x == 1 && pos.y == 1 && pos.z == 1);
    assert(level.cells[1][1][2].type == BLOCK_CRATE);
    pos = walk(&level, pos, "WSE");
    assert(level.cells[0][2][2].type == BLOCK_CRATE);
    assert(level.cells[1][1][2].type == BLOCK_EMPTY);
    pos = walk(&level, pos, "EEEES");
    assert(pos.x == 5 && pos.y == 1 && pos.z == 3);
    assert(level.cells[pos.y][pos.x][pos.z].type == BLOCK_STAR);

    Level trap;
    assert(level_load(&trap, "levels/level_3.txt"));
    pos = walk(&trap, start_of(&trap), "NNNE");
    assert(trap.cells[0][2][1].type == BLOCK_CRATE);
    pos = walk(&trap, pos, "E");
    MovePath path;
    assert(!movement_plan(&trap, pos.x, pos.y, pos.z, DIR_EAST, &path));
}

static void test_level_4(void)
{
    Level level;
    assert(level_load(&level, "levels/level_4.txt"));
    Pos pos = walk(&level, start_of(&level), "NNN");
    assert(pos.x == 0 && pos.y == 1 && pos.z == 1);
    MovePath path;
    assert(!movement_plan(&level, pos.x, pos.y, pos.z, DIR_NORTH, &path));
    assert(!movement_plan(&level, 1, 1, 2, DIR_EAST, &path));

    PlatformMove move;
    GridPos player = { pos.x, pos.y, pos.z };
    assert(platform_rotate(&level, player, &move));
    assert(level.cells[0][2][2].type == BLOCK_PLATFORM);
    assert(level.cells[0][4][2].type == BLOCK_PLATFORM);
    assert(level.cells[0][3][1].type == BLOCK_EMPTY);
    pos = walk(&level, pos, "SEEEEEES");
    assert(pos.x == 6 && pos.y == 1 && pos.z == 3);
    assert(level.cells[pos.y][pos.x][pos.z].type == BLOCK_STAR);

    GridPos rider = { 3, 1, 2 };
    assert(!platform_rotate(&level, rider, &move));
}

static void test_input_direction(void)
{
    float yaw_south = 90.0f * DEG2RAD;
    assert(movement_input_direction(yaw_south, 0, 1) == DIR_NORTH);
    assert(movement_input_direction(yaw_south, 1, 0) == DIR_EAST);
    assert(movement_input_direction(yaw_south, 0, -1) == DIR_SOUTH);
    assert(movement_input_direction(yaw_south, -1, 0) == DIR_WEST);
    float yaw_diagonal = 45.0f * DEG2RAD;
    Direction up = movement_input_direction(yaw_diagonal, 0, 1);
    Direction right = movement_input_direction(yaw_diagonal, 1, 0);
    Direction down = movement_input_direction(yaw_diagonal, 0, -1);
    Direction left = movement_input_direction(yaw_diagonal, -1, 0);
    assert(up != right && up != down && up != left);
    assert(right != down && right != left && down != left);
}

static void test_world_map(void)
{
    Level map;
    assert(level_load(&map, "levels/world_map.txt"));
    assert(map.entrance_count == 4);
    Pos start = { map.start_x, map.start_y, map.start_z };
    Pos end = walk(&map, start, "W");
    GridPos first = map.entrances[0];
    assert(end.x == first.x && end.y == first.y && end.z == first.z);
}

static void test_key_rebind_swaps_conflict(void)
{
    settings_reset_keys();
    int old_up = settings_key(ACTION_UP);
    int down = settings_key(ACTION_DOWN);
    settings_set_key(ACTION_UP, down);
    assert(settings_key(ACTION_UP) == down);
    assert(settings_key(ACTION_DOWN) == old_up);
    settings_reset_keys();
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    test_level_1();
    test_level_2();
    test_level_3();
    test_level_4();
    test_input_direction();
    test_world_map();
    test_key_rebind_swaps_conflict();
    printf("movement tests OK\n");
    return 0;
}
