#include <assert.h>
#include <stdio.h>
#include "raylib.h"
#include "world/level.h"
#include "core/settings.h"
#include "world/movement.h"
#include "world/platform.h"
#include "game/world.h"
#include "core/progress.h"
#include "core/i18n.h"
#include <string.h>

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
    assert(level_load(&level, "levels/world_1/level_1.txt"));
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
    assert(level_load(&level, "levels/world_1/level_2.txt"));
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
    assert(level_load(&level, "levels/world_1/level_3.txt"));
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
    assert(level_load(&trap, "levels/world_1/level_3.txt"));
    pos = walk(&trap, start_of(&trap), "NNNE");
    assert(trap.cells[0][2][1].type == BLOCK_CRATE);
    pos = walk(&trap, pos, "E");
    MovePath path;
    assert(!movement_plan(&trap, pos.x, pos.y, pos.z, DIR_EAST, &path));
}

static void test_level_4(void)
{
    Level level;
    assert(level_load(&level, "levels/world_1/level_4.txt"));
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
    assert(level_load(&map, "levels/world_1/map.txt"));
    assert(map.entrance_count == 4);
    Pos start = { map.start_x, map.start_y, map.start_z };
    Pos end = walk(&map, start, "W");
    GridPos first = map.entrances[0];
    assert(end.x == first.x && end.y == first.y && end.z == first.z);
}

static void test_world_1_portal(void)
{
    Level map;
    assert(level_load(&map, "levels/world_1/map.txt"));
    assert(map.has_portal_next && !map.has_portal_prev);
    GridPos last = map.entrances[map.entrance_count - 1];
    Pos pos = { last.x, last.y, last.z };
    pos = walk(&map, pos, "WWWWW");
    assert(pos.x == map.portal_next.x && pos.y == map.portal_next.y && pos.z == map.portal_next.z);
}

static void test_world_2_level_1(void)
{
    Level level;
    assert(level_load(&level, "levels/world_2/level_1.txt"));
    Pos pos = walk(&level, start_of(&level), "NNNNESSWSE");
    assert(level.cells[0][2][3].type == BLOCK_CRATE);
    pos = walk(&level, pos, "EE");
    assert(level.cells[0][4][3].type == BLOCK_CRATE);
    pos = walk(&level, pos, "EENNN");
    assert(pos.x == 5 && pos.y == 1 && pos.z == 0);
    assert(level.cells[pos.y][pos.x][pos.z].type == BLOCK_STAR);

    Level trap;
    assert(level_load(&trap, "levels/world_2/level_1.txt"));
    pos = walk(&trap, start_of(&trap), "NNNE");
    assert(trap.cells[0][2][1].type == BLOCK_CRATE);
    pos = walk(&trap, pos, "EESS");
    assert(trap.cells[1][3][4].type == BLOCK_CRATE);
    MovePath path;
    assert(!movement_plan(&trap, pos.x, pos.y, pos.z, DIR_EAST, &path));
}

static void test_world_2_level_2(void)
{
    Level level;
    assert(level_load(&level, "levels/world_2/level_2.txt"));
    Pos pos = walk(&level, start_of(&level), "NNN");
    PlatformMove move;
    GridPos player = { pos.x, pos.y, pos.z };
    assert(platform_rotate(&level, player, &move));
    pos = walk(&level, pos, "SEEEEE");
    assert(level.cells[0][6][2].type == BLOCK_CRATE);
    pos = walk(&level, pos, "ENN");
    assert(pos.x == 6 && pos.y == 1 && pos.z == 0);
    assert(level.cells[pos.y][pos.x][pos.z].type == BLOCK_STAR);

    Level trap;
    assert(level_load(&trap, "levels/world_2/level_2.txt"));
    pos = walk(&trap, start_of(&trap), "NNE");
    assert(trap.cells[0][2][2].type == BLOCK_CRATE);
    pos = walk(&trap, pos, "NN");
    player = (GridPos){ pos.x, pos.y, pos.z };
    assert(!platform_rotate(&trap, player, &move));
}

static void test_world_2_level_3(void)
{
    Level level;
    assert(level_load(&level, "levels/world_2/level_3.txt"));
    assert(level.width == 12 && level.height == 10 && level.depth == 12);
    Pos pos = walk(&level, start_of(&level), "EEEEEEEEEESSSSSSSSSSS");
    assert(pos.x == 10 && pos.y == 1 && pos.z == 11);
    pos = walk(&level, pos, "E");
    assert(pos.x == 11 && pos.y == 9 && pos.z == 11);
    assert(level.cells[pos.y][pos.x][pos.z].type == BLOCK_STAR);

    Level push;
    assert(level_load(&push, "levels/world_2/level_3.txt"));
    pos = walk(&push, start_of(&push), "SSEEEEEEEEEE");
    assert(pos.x == 10 && pos.z == 2);
    assert(push.cells[1][11][2].type == BLOCK_CRATE);
}

static bool has_free_neighbor(const Level *map, GridPos pad)
{
    static const int DX[4] = { 0, 1, 0, -1 };
    static const int DZ[4] = { 1, 0, -1, 0 };

    for (int i = 0; i < 4; i++) {
        int x = pad.x + DX[i];
        int z = pad.z + DZ[i];
        bool inside = x >= 0 && x < map->width && z >= 0 && z < map->depth && pad.y > 0;
        if (inside && map->cells[pad.y][x][z].type == BLOCK_EMPTY
            && BLOCK_INFO[map->cells[pad.y - 1][x][z].type].walkable_on) {
            return true;
        }
    }
    return false;
}

static void test_worlds_are_complete(void)
{
    WorldInfo worlds[WORLD_MAX];
    int count = worlds_load(worlds, WORLD_MAX, "levels/worlds.txt");
    assert(count >= 2);
    for (int w = 0; w < count; w++) {
        char path[WORLD_PATH_MAX];
        Level map;
        world_level_path(&worlds[w], WORLD_MAP, path, sizeof(path));
        assert(level_load(&map, path));
        assert(map.entrance_count > 0);
        assert(map.has_portal_prev == (w > 0));
        assert(map.has_portal_next == (w + 1 < count));
        assert(!map.has_portal_next || has_free_neighbor(&map, map.portal_next));
        assert(!map.has_portal_prev || has_free_neighbor(&map, map.portal_prev));
        for (int i = 0; i < map.entrance_count; i++) {
            Level level;
            world_level_path(&worlds[w], i, path, sizeof(path));
            if (!level_load(&level, path)) {
                fprintf(stderr, "missing level %s\n", path);
            }
            assert(level_load(&level, path));
        }
    }
}

static void test_level_size_limits(void)
{
    Level level;
    assert(level_load(&level, "tests/levels/max_size.txt"));
    assert(level.width == LEVEL_MAX_WIDTH && level.height == LEVEL_MAX_HEIGHT && level.depth == LEVEL_MAX_DEPTH);
    Pos pos = walk(&level, start_of(&level), "EEEEEEEEEEESSSSSSSSSSS");
    assert(pos.x == LEVEL_MAX_WIDTH - 1 && pos.z == LEVEL_MAX_DEPTH - 1);
    assert(!level_load(&level, "tests/levels/too_wide.txt"));
    assert(!level_load(&level, "tests/levels/too_high.txt"));
    assert(!level_load(&level, "tests/levels/too_deep.txt"));
}

static void format_signature(const char *text, char *out)
{
    int n = 0;

    for (const char *c = text; *c != '\0'; c++) {
        if (c[0] == '%' && c[1] != '\0') {
            out[n++] = c[1];
            c++;
        }
    }
    out[n] = '\0';
}

static void test_translations_are_complete(void)
{
    char signatures[TXT_COUNT][16];

    for (int lang = 0; lang < LANG_COUNT; lang++) {
        if (lang > 0) {
            settings_toggle_language();
        }
        for (int id = 0; id < TXT_COUNT; id++) {
            const char *text = tr((TextId)id);
            char signature[16];
            assert(text != NULL && text[0] != '\0');
            format_signature(text, signature);
            if (lang == 0) {
                strcpy(signatures[id], signature);
            } else if (strcmp(signatures[id], signature) != 0) {
                fprintf(stderr, "text %d: format %s vs %s\n", id, signatures[id], signature);
                assert(false);
            }
        }
    }
    settings_toggle_language();
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
    test_world_1_portal();
    test_world_2_level_1();
    test_world_2_level_2();
    test_world_2_level_3();
    test_worlds_are_complete();
    test_level_size_limits();
    test_translations_are_complete();
    test_key_rebind_swaps_conflict();
    printf("movement tests OK\n");
    return 0;
}
