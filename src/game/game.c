#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "game/game.h"
#include "game/game_internal.h"
#include "core/settings.h"
#include "render/block_render.h"
#include "render/lighting.h"
#include "render/vegetation.h"
#include "render/title_logo.h"
#include "render/rocket.h"
#include "core/sfx.h"
#include "ui/ui.h"
#include "ui/settings_menu.h"

#define SKY_PATH_MAX 96

static const char *WORLDS_PATH = "levels/worlds.txt";
static const char *TITLE_SCENE_PATH = "levels/title.txt";
static const char *MENU_MUSIC_PATH = "assets/music/world/diorama_ambiance.wav";
static const char *WORLD_MUSIC_PATH = "assets/music/world/diorama_ambiance_version_longue.wav";
static const char *WIND_PATH = "assets/music/world/ambience_wind.mp3";
static const float WIND_VOLUME = 0.4f;
static const float ROCKET_BOARD_DISTANCE = 0.7f;
static const double ENTRANCE_SOUND_COOLDOWN = 0.5;
static const float FADE_OUT_DURATION = 1.0f;
static const float FADE_HOLD_DURATION = 0.4f;
static const float FADE_IN_DURATION = 1.2f;
static const float TITLE_YAW = 90.0f * DEG2RAD;
static const float TITLE_SWAY_ANGLE = 12.0f * DEG2RAD;
static const float TITLE_SWAY_SPEED = 0.25f;
static const float TITLE_PITCH = 22.0f * DEG2RAD;
static const float TITLE_TARGET_RAISE = 0.6f;
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
    orbit_camera_fit(orbit, (Vector3){ (float)level->width, (float)level->height, (float)level->depth });
    block_render_reward_reset();
    crank->active = false;
    return true;
}

static bool start_world_level(Game *g, int level)
{
    char path[WORLD_PATH_MAX];

    world_level_path(&g->worlds[g->world], level, path, sizeof(path));
    return start_level(path, &g->level, &g->player, &g->orbit, &g->crank);
}

static void place_rockets(Game *g)
{
    GridPos pads[2] = { g->level.portal_next, g->level.portal_prev };
    bool shown[2] = { game_rocket_next(g), game_rocket_prev(g) };

    for (int i = 0; i < 2; i++) {
        if (shown[i]) {
            g->level.cells[pads[i].y][pads[i].x][pads[i].z] = (Cell){ BLOCK_PROP, DIR_NORTH };
        }
    }
}

static bool enter_map(Game *g, int entrance)
{
    if (!start_world_level(g, WORLD_MAP)) {
        return false;
    }
    place_rockets(g);
    if (entrance < g->level.entrance_count) {
        GridPos spot = g->level.entrances[entrance];
        player_place(&g->player, spot.x, spot.y, spot.z);
    }
    return true;
}

static bool player_at(const Player *player, GridPos cell)
{
    return player_is_idle(player) && cell.x == player->x && cell.y == player->y && cell.z == player->z;
}

static int entrance_under(const Player *player, const Level *map)
{
    for (int i = 0; i < map->entrance_count; i++) {
        if (player_at(player, map->entrances[i])) {
            return i;
        }
    }
    return -1;
}

static bool beside(const Player *player, GridPos pad, Direction *toward)
{
    int dx = pad.x - player->x;
    int dz = pad.z - player->z;

    if (!player_is_idle(player) || pad.y != player->y || abs(dx) + abs(dz) != 1) {
        return false;
    }
    *toward = dx > 0 ? DIR_EAST : dx < 0 ? DIR_WEST : dz > 0 ? DIR_SOUTH : DIR_NORTH;
    return true;
}

static int portal_near(const Game *g, Direction *toward)
{
    const Level *map = &g->level;

    if (game_rocket_next(g) && beside(&g->player, map->portal_next, toward)) {
        return 1;
    }
    if (!game_rocket_next(g) && map->has_portal_next && player_at(&g->player, map->portal_next)) {
        return 1;
    }
    if (game_rocket_prev(g) && beside(&g->player, map->portal_prev, toward)) {
        return -1;
    }
    return 0;
}

static void place_beside(Game *g, GridPos pad)
{
    static const int DX[4] = { 0, 1, 0, -1 };
    static const int DZ[4] = { 1, 0, -1, 0 };
    const Level *map = &g->level;

    for (int i = 0; i < 4; i++) {
        int x = pad.x + DX[i];
        int z = pad.z + DZ[i];
        bool inside = x >= 0 && x < map->width && z >= 0 && z < map->depth && pad.y > 0;
        if (inside && map->cells[pad.y][x][z].type == BLOCK_EMPTY
            && BLOCK_INFO[map->cells[pad.y - 1][x][z].type].walkable_on) {
            player_place(&g->player, x, pad.y, z);
            return;
        }
    }
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
    if (g->state != STATE_WON) {
        orbit_camera_update(&g->orbit);
    }
    float camera_yaw = orbit_camera_snapped_yaw(&g->orbit);
    player_update(&g->player, &g->level, camera_yaw, !g->crank.active);
    if (g->player.crank_requested) {
        GridPos player_cell = { g->player.x, g->player.y, g->player.z };
        if (crank_start(&g->crank, &g->level, player_cell)) {
            sfx_play(SFX_CRANK);
            sfx_play(SFX_CRANK_TURN);
        } else {
            sfx_play(SFX_BUMP);
        }
    }
    bool was_cranking = g->crank.active;
    crank_update(&g->crank);
    if (was_cranking && !g->crank.active) {
        sfx_play(SFX_PLATFORM_STOP);
    }
    player_set_crank(&g->player, g->crank.active, g->crank.progress);
    if (g->state == STATE_PLAYING && player_on_reward(&g->player, &g->level)) {
        g->state = STATE_WON;
        player_celebrate(&g->player, camera_yaw);
        block_render_reward_collect();
        sfx_play(SFX_VICTORY);
        sfx_play(SFX_SEED_COLLECT);
    }
    if (g->state == STATE_WON && block_render_reward_collected()) {
        g->level.cells[g->player.y][g->player.x][g->player.z].type = BLOCK_EMPTY;
    }
}

static void unload_assets(void)
{
    title_logo_unload();
    ui_unload_font();
    rocket_unload();
    sfx_unload();
    backdrop_assets_unload();
    vegetation_unload();
    block_render_unload();
    lighting_unload();
}

static void unload_music(Music *music)
{
    if (IsMusicValid(*music)) {
        UnloadMusicStream(*music);
    }
    *music = (Music){0};
}

static void load_world_scenery(Game *g)
{
    const WorldInfo *world = &g->worlds[g->world];
    char path[WORLD_PATH_MAX];

    world_level_path(world, WORLD_MAP, path, sizeof(path));
    load_backdrop(&g->map_sky, path);
    for (int i = 0; i < g->level_count; i++) {
        world_level_path(world, i, path, sizeof(path));
        load_backdrop(&g->level_skies[i], path);
    }
}

bool load_world(Game *g, int index)
{
    int previous = g->world;

    g->world = index;
    if (!start_world_level(g, WORLD_MAP) || g->level.entrance_count == 0) {
        TraceLog(LOG_ERROR, "WORLDS: map of world '%s' is missing or has no level entrance", g->worlds[index].dir);
        g->world = previous;
        return false;
    }
    g->level_count = g->level.entrance_count;
    place_rockets(g);
    load_world_scenery(g);
    g->progress.world = index;
    progress_save(&g->progress);
    return true;
}

static int saved_world(const Game *g)
{
    int world = g->progress.world;

    return (world >= 0 && world < g->world_count) ? world : 0;
}

static void sway_title_camera(Game *g)
{
    g->title_orbit.yaw = TITLE_YAW + sinf((float)GetTime() * TITLE_SWAY_SPEED) * TITLE_SWAY_ANGLE;
}

bool game_init(Game *g)
{
    lighting_load();
    block_render_load();
    title_logo_load();
    ui_load_font();
    rocket_load();
    sfx_load();
    vegetation_load();
    backdrop_assets_load();
    progress_load(&g->progress);
    g->world_count = worlds_load(g->worlds, WORLD_MAX, WORLDS_PATH);
    if (g->world_count == 0 || !level_load(&g->title_scene, TITLE_SCENE_PATH)) {
        TraceLog(LOG_ERROR, "GAME: %s and %s must load", WORLDS_PATH, TITLE_SCENE_PATH);
        unload_assets();
        return false;
    }
    Level *t = &g->title_scene;
    g->player = player_load(t->start_x, t->start_y, t->start_z, DIR_SOUTH);
    g->orbit = orbit_camera_init(level_center(t));
    if (!load_world(g, saved_world(g)) && !load_world(g, 0)) {
        player_unload(&g->player);
        unload_assets();
        return false;
    }
    load_backdrop(&g->title_sky, TITLE_SCENE_PATH);
    g->title_player = player_load(t->start_x, t->start_y, t->start_z, DIR_SOUTH);
    g->title_orbit = orbit_camera_init(level_center(t));
    g->title_orbit.distance = TITLE_CAMERA_DISTANCE;
    g->title_orbit.pitch = TITLE_PITCH;
    g->title_orbit.target.y += TITLE_TARGET_RAISE;
    sway_title_camera(g);
    g->state = STATE_TITLE;
    g->menu_selected = 0;
    g->settings_waiting = NOT_WAITING;
    g->menu_music = LoadMusicStream(MENU_MUSIC_PATH);
    g->world_music = LoadMusicStream(WORLD_MUSIC_PATH);
    g->wind = LoadMusicStream(WIND_PATH);
    g->entrance_sound_time = -ENTRANCE_SOUND_COOLDOWN;
    return true;
}

const bool *game_completed(const Game *g)
{
    return g->progress.completed[g->world];
}

bool game_world_done(const Game *g)
{
    for (int i = 0; i < g->level_count; i++) {
        if (!g->progress.completed[g->world][i]) {
            return false;
        }
    }
    return true;
}

void begin_transition(Game *g, GameState next_state, int next_level)
{
    g->transition = TRANSITION_OUT;
    g->transition_time = 0.0f;
    g->next_state = next_state;
    g->next_level = next_level;
    g->next_world = g->world;
    sfx_play(SFX_UI_TRANSITION);
}

static void begin_world_transition(Game *g, int next_world)
{
    begin_transition(g, STATE_MAP, -1);
    g->next_world = next_world;
}

static void apply_world_change(Game *g)
{
    g->launching = false;
    g->launch_done = false;
    bool forward = g->next_world > g->world;

    if (!load_world(g, g->next_world)) {
        return;
    }
    if (forward && g->level.has_portal_prev) {
        place_beside(g, g->level.portal_prev);
    } else if (!forward && g->level.has_portal_next) {
        place_beside(g, g->level.portal_next);
    }
    g->state = STATE_MAP;
}

static void apply_transition(Game *g)
{
    if (g->next_world != g->world) {
        apply_world_change(g);
    } else if (g->next_state == STATE_PLAYING) {
        if (start_world_level(g, g->next_level)) {
            g->level_index = g->next_level;
            g->state = STATE_PLAYING;
        }
    } else if (g->next_state == STATE_MAP && g->state != STATE_TITLE) {
        if (enter_map(g, g->level_index)) {
            g->state = STATE_MAP;
        }
    } else {
        g->state = g->next_state;
    }
}

static void update_transition(Game *g)
{
    static const float durations[] = {
        [TRANSITION_OUT] = FADE_OUT_DURATION,
        [TRANSITION_HOLD] = FADE_HOLD_DURATION,
        [TRANSITION_IN] = FADE_IN_DURATION,
    };

    g->transition_time += GetFrameTime();
    if (g->transition_time < durations[g->transition]) {
        return;
    }
    g->transition_time = 0.0f;
    if (g->transition == TRANSITION_OUT) {
        apply_transition(g);
        g->transition = TRANSITION_HOLD;
    } else if (g->transition == TRANSITION_HOLD) {
        g->transition = TRANSITION_IN;
    } else {
        g->transition = TRANSITION_NONE;
    }
}

static float smoothstep(float t)
{
    t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;
    return t * t * (3.0f - 2.0f * t);
}

float game_fade(const Game *g)
{
    switch (g->transition) {
    case TRANSITION_OUT:
        return smoothstep(g->transition_time / FADE_OUT_DURATION);
    case TRANSITION_HOLD:
        return 1.0f;
    case TRANSITION_IN:
        return 1.0f - smoothstep(g->transition_time / FADE_IN_DURATION);
    default:
        return 0.0f;
    }
}

static void start_rocket_launch(Game *g, GridPos pad, Direction dir, int target_world)
{
    static const float FORWARD_X[4] = { [DIR_NORTH] = 0.0f, [DIR_EAST] = 1.0f, [DIR_SOUTH] = 0.0f, [DIR_WEST] = -1.0f };
    static const float FORWARD_Z[4] = { [DIR_NORTH] = -1.0f, [DIR_EAST] = 0.0f, [DIR_SOUTH] = 1.0f, [DIR_WEST] = 0.0f };
    Vector3 start = {
        pad.x - FORWARD_X[dir] * ROCKET_BOARD_DISTANCE,
        (float)pad.y,
        pad.z - FORWARD_Z[dir] * ROCKET_BOARD_DISTANCE,
    };

    player_board_rocket(&g->player, start, dir);
    sfx_play(SFX_ROCKET_LAUNCH);
    g->launching = true;
    g->launch_done = false;
    g->launch_time = 0.0f;
    g->launch_dir = dir;
    g->launch_pad = pad;
    g->launch_target = target_world;
}

static void update_rocket_launch(Game *g)
{
    update_world(g);
    g->launch_time += GetFrameTime();
    float duration = fmaxf(rocket_launch_duration(), player_anim_duration(&g->player, ANIM_ROCKET_LAUNCH));
    if (g->launch_time >= duration && g->transition == TRANSITION_NONE) {
        g->launch_done = true;
        begin_world_transition(g, g->launch_target);
    }
}

static void update_map(Game *g, bool back, bool confirm)
{
    if (g->launching) {
        update_rocket_launch(g);
        return;
    }
    update_world(g);
    g->entrance = entrance_under(&g->player, &g->level);
    Direction toward = DIR_NORTH;
    g->portal = portal_near(g, &toward);
    bool on_spot = g->entrance >= 0 || g->portal != 0;
    bool cooled_down = GetTime() - g->entrance_sound_time >= ENTRANCE_SOUND_COOLDOWN;
    if (on_spot && !g->was_on_spot && cooled_down) {
        sfx_play(SFX_UI_ENTRANCE);
        g->entrance_sound_time = GetTime();
    }
    g->was_on_spot = on_spot;
    bool next_open = g->world + 1 < g->world_count && game_world_done(g);
    if (back) {
        game_menu_open_pause(g);
    } else if (!confirm) {
        return;
    } else if (g->entrance >= 0 && ui_level_unlocked(game_completed(g), g->entrance)) {
        begin_transition(g, STATE_PLAYING, g->entrance);
    } else if (g->portal > 0 && next_open) {
        start_rocket_launch(g, g->level.portal_next, toward, g->world + 1);
    } else if (g->portal < 0) {
        start_rocket_launch(g, g->level.portal_prev, toward, g->world - 1);
    } else if (g->entrance >= 0 || g->portal > 0) {
        sfx_play(SFX_UI_LOCKED);
    }
}

static void update_level(Game *g, bool back, bool confirm)
{
    update_world(g);
    if (g->state == STATE_WON && !g->progress.completed[g->world][g->level_index]) {
        g->progress.completed[g->world][g->level_index] = true;
        progress_save(&g->progress);
        if (game_world_done(g) && g->world + 1 < g->world_count) {
            sfx_play(SFX_UI_UNLOCK);
        }
    }
    if (g->state == STATE_WON) {
        game_menu_update(g, back, confirm);
    } else if (back) {
        game_menu_open_pause(g);
    } else if (IsKeyPressed(settings_key(ACTION_RESTART))) {
        restart_level(g);
    }
}

void restart_level(Game *g)
{
    if (start_world_level(g, g->level_index)) {
        g->state = STATE_PLAYING;
        sfx_play(SFX_UI_RESTART);
    }
}

bool game_rocket_next(const Game *g)
{
    return g->level.has_portal_next && g->world + 1 < g->world_count && game_world_done(g);
}

bool game_rocket_prev(const Game *g)
{
    return g->level.has_portal_prev && g->world > 0;
}

bool game_in_title_scene(const Game *g)
{
    bool title_settings = g->state == STATE_SETTINGS && g->settings_return == STATE_TITLE;

    return g->state == STATE_TITLE || g->state == STATE_CONFIRM_NEW || title_settings;
}

GameState game_scene_state(const Game *g)
{
    if (g->state == STATE_PAUSED || (g->state == STATE_SETTINGS && g->settings_return == STATE_PAUSED)) {
        return g->paused_from;
    }
    return g->state;
}

static void play_music_while(Music music, bool active, float volume)
{
    if (!IsMusicValid(music)) {
        return;
    }
    if (active && !IsMusicStreamPlaying(music)) {
        PlayMusicStream(music);
    } else if (!active && IsMusicStreamPlaying(music)) {
        StopMusicStream(music);
    }
    SetMusicVolume(music, volume);
    UpdateMusicStream(music);
}

static void update_music(Game *g)
{
    float fade = 1.0f - game_fade(g);
    float volume = fade * settings_volume(VOLUME_MUSIC);
    float ambience = fade * settings_volume(VOLUME_SFX) * WIND_VOLUME;
    bool in_menu = game_in_title_scene(g);
    bool in_world = !in_menu && g->state == game_scene_state(g);

    play_music_while(g->menu_music, in_menu, volume);
    play_music_while(g->world_music, !in_menu, volume);
    play_music_while(g->wind, !in_menu, ambience);
    sfx_loop(SFX_LOOP_CRATE, in_world && player_pushing(&g->player));
    sfx_loop(SFX_LOOP_PLATFORM, in_world && g->crank.active);
}

void game_update(Game *g)
{
    bool back = IsKeyPressed(KEY_ESCAPE);
    bool confirm = ui_confirm_pressed();

    g->entrance = -1;
    g->portal = 0;
    if (g->transition != TRANSITION_NONE) {
        update_transition(g);
        back = false;
        confirm = false;
    }
    if (game_in_title_scene(g)) {
        sway_title_camera(g);
    }
    switch (g->state) {
    case STATE_TITLE:
    case STATE_CONFIRM_NEW:
    case STATE_PAUSED:
        game_menu_update(g, back, confirm);
        break;
    case STATE_SETTINGS:
        if (settings_menu_update(&g->settings_selected, &g->settings_waiting)) {
            g->state = g->settings_return;
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
    update_music(g);
}

void game_unload(Game *g)
{
    player_unload(&g->player);
    player_unload(&g->title_player);
    unload_music(&g->menu_music);
    unload_music(&g->world_music);
    unload_music(&g->wind);
    unload_assets();
}
