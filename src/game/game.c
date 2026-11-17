#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include "game/game.h"
#include "core/settings.h"
#include "render/block_render.h"
#include "render/lighting.h"
#include "render/vegetation.h"
#include "ui/ui.h"
#include "ui/settings_menu.h"

#define SKY_PATH_MAX 64

static const char *MAP_PATH = "levels/world_map.txt";
static const char *TITLE_SCENE_PATH = "levels/title.txt";
static const char *LEVEL_PATHS[LEVEL_COUNT] = {
    "levels/level_1.txt",
    "levels/level_2.txt",
    "levels/level_3.txt",
    "levels/level_4.txt",
};
static const float TITLE_ORBIT_SPEED = 0.15f;
static const float TITLE_CAMERA_DISTANCE = 7.0f;

static bool start_level(const char *path, Level *level, Player *player, OrbitCamera *orbit, CrankAnim *crank)
{
    Level loaded;

    if (!level_load(&loaded, path)) {
        return false;
    }
    *level = loaded;
    player_place(player, level->start_x, level->start_y, level->start_z);
    orbit->target = level_center(level);
    block_render_reward_reset();
    crank->active = false;
    return true;
}

static bool enter_map(int entrance, Level *level, Player *player, OrbitCamera *orbit, CrankAnim *crank)
{
    if (!start_level(MAP_PATH, level, player, orbit, crank)) {
        return false;
    }
    GridPos spot = level->entrances[entrance];
    player_place(player, spot.x, spot.y, spot.z);
    return true;
}

static int entrance_under(const Player *player, const Level *map)
{
    if (!player_is_idle(player)) {
        return -1;
    }
    for (int i = 0; i < map->entrance_count; i++) {
        GridPos e = map->entrances[i];
        if (e.x == player->x && e.y == player->y && e.z == player->z) {
            return i;
        }
    }
    return -1;
}

static bool player_on_reward(const Player *player, const Level *level)
{
    BlockType type = level->cells[player->y][player->x][player->z].type;

    return type == BLOCK_STAR && player_is_idle(player);
}
static bool load_backdrop(Backdrop *backdrop, const char *level_path)
{
    char path[SKY_PATH_MAX];
    const char *dot = strrchr(level_path, '.');
    int stem = (dot != NULL) ? (int)(dot - level_path) : (int)strlen(level_path);

    snprintf(path, sizeof(path), "%.*s.sky", stem, level_path);
    return backdrop_load(backdrop, path);
}

static void update_world(Game *g)
{
    orbit_camera_update(&g->orbit);
    float camera_yaw = orbit_camera_snapped_yaw(&g->orbit);
    player_update(&g->player, &g->level, camera_yaw, !g->crank.active);
    if (g->player.crank_requested) {
        GridPos player_cell = { g->player.x, g->player.y, g->player.z };
        crank_start(&g->crank, &g->level, player_cell);
    }
    crank_update(&g->crank);
    player_set_crank(&g->player, g->crank.active, g->crank.progress);
    if (g->state == STATE_PLAYING && player_on_reward(&g->player, &g->level)) {
        g->state = STATE_WON;
        player_celebrate(&g->player, camera_yaw);
        block_render_reward_collect();
    }
    if (g->state == STATE_WON && block_render_reward_collected()) {
        g->level.cells[g->player.y][g->player.x][g->player.z].type = BLOCK_EMPTY;
    }
}

static void unload_assets(void)
{
    backdrop_assets_unload();
    vegetation_unload();
    block_render_unload();
    lighting_unload();
}

bool game_init(Game *g)
{
    lighting_load();
    block_render_load();
    vegetation_load();
    backdrop_assets_load();

    bool scenes_ok = level_load(&g->title_scene, TITLE_SCENE_PATH) && level_load(&g->level, MAP_PATH);
    if (!scenes_ok || g->level.entrance_count != LEVEL_COUNT) {
        TraceLog(LOG_ERROR, "MAP: %s and %s must load, and the map needs one entrance per level (%d)",
                 TITLE_SCENE_PATH, MAP_PATH, LEVEL_COUNT);
        unload_assets();
        return false;
    }
    g->player = player_load(g->level.start_x, g->level.start_y, g->level.start_z, DIR_SOUTH);
    g->orbit = orbit_camera_init(level_center(&g->level));
    load_backdrop(&g->title_sky, TITLE_SCENE_PATH);
    load_backdrop(&g->map_sky, MAP_PATH);
    for (int i = 0; i < LEVEL_COUNT; i++) {
        load_backdrop(&g->level_skies[i], LEVEL_PATHS[i]);
    }
    Level *t = &g->title_scene;
    g->title_player = player_load(t->start_x, t->start_y, t->start_z, DIR_SOUTH);
    g->title_orbit = orbit_camera_init(level_center(t));
    g->title_orbit.distance = TITLE_CAMERA_DISTANCE;
    g->state = STATE_TITLE;
    g->title_selected = TITLE_PLAY;
    g->settings_waiting = NOT_WAITING;
    return true;
}

static void update_title(Game *g, bool back, bool confirm)
{
    g->title_selected = ui_navigate(g->title_selected, TITLE_COUNT);
    g->title_orbit.yaw += TITLE_ORBIT_SPEED * GetFrameTime();
    if (confirm && g->title_selected == TITLE_PLAY) {
        g->state = STATE_MAP;
    } else if (confirm && g->title_selected == TITLE_SETTINGS) {
        g->state = STATE_SETTINGS;
    } else if (back || (confirm && g->title_selected == TITLE_QUIT)) {
        g->quit = true;
    }
}

static void update_map(Game *g, bool back, bool confirm)
{
    update_world(g);
    g->entrance = entrance_under(&g->player, &g->level);
    if (back) {
        g->state = STATE_TITLE;
    } else if (confirm && g->entrance >= 0 && ui_level_unlocked(g->completed, g->entrance)
               && start_level(LEVEL_PATHS[g->entrance], &g->level, &g->player, &g->orbit, &g->crank)) {
        g->level_index = g->entrance;
        g->state = STATE_PLAYING;
    }
}

static void update_level(Game *g, bool back, bool confirm)
{
    update_world(g);
    if (g->state == STATE_WON) {
        g->completed[g->level_index] = true;
    }
    bool leave = back || (g->state == STATE_WON && confirm);
    if (leave && enter_map(g->level_index, &g->level, &g->player, &g->orbit, &g->crank)) {
        g->state = STATE_MAP;
    } else if (IsKeyPressed(settings_key(ACTION_RESTART))
               && start_level(LEVEL_PATHS[g->level_index], &g->level, &g->player, &g->orbit, &g->crank)) {
        g->state = STATE_PLAYING;
    }
}

void game_update(Game *g)
{
    bool back = IsKeyPressed(KEY_ESCAPE);
    bool confirm = ui_confirm_pressed();

    g->entrance = -1;
    switch (g->state) {
    case STATE_TITLE:
        update_title(g, back, confirm);
        break;
    case STATE_SETTINGS:
        g->title_orbit.yaw += TITLE_ORBIT_SPEED * GetFrameTime();
        if (settings_menu_update(&g->settings_selected, &g->settings_waiting)) {
            g->state = STATE_TITLE;
        }
        break;
    case STATE_MAP:
        update_map(g, back, confirm);
        break;
    case STATE_PLAYING:
    case STATE_WON:
        update_level(g, back, confirm);
        break;
    }
    block_render_update();
    vegetation_update();
}

void game_unload(Game *g)
{
    player_unload(&g->player);
    player_unload(&g->title_player);
    unload_assets();
}
