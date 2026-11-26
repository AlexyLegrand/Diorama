#include <math.h>
#include <stddef.h>
#include "raylib.h"
#include "game/game.h"
#include "game/game_internal.h"
#include "core/settings.h"
#include "render/block_render.h"
#include "render/lighting.h"
#include "render/vegetation.h"
#include "render/sky.h"
#include "render/title_logo.h"
#include "render/rocket.h"
#include "render/collectible.h"
#include "render/dust.h"
#include "render/postfx.h"
#include "entities/player.h"
#include "ui/ui.h"
#include "core/i18n.h"
#include "ui/settings_menu.h"

static const int FPS_MARGIN = 20;
static const int FPS_WIDTH = 100;
static const float PAUSE_SHADE = 0.45f;
static const float TITLE_BLUR = 0.0f;
static const float GAME_BLUR = 1.0f;
static const float ROCKET_FLOOR_OFFSET = 0.5f;
static const float COLLECTIBLE_PHASE = 0.7f;
static const int LOADING_FONT_SIZE = 30;
static const int LOADING_MARGIN = 40;
static const float LOADING_TEXT_MIN_FADE = 0.6f;

static void draw_world(const Level *level, const Player *player, const CrankAnim *crank, bool show_player)
{
    GridPos crate_cell;
    Vector3 crate_position;
    Cell crate;

    if (player_moving_crate(player, &crate_cell, &crate_position, &crate)) {
        level_draw(level, &crate_cell, crank->active);
        draw_block_at(crate_position, crate);
    } else {
        level_draw(level, NULL, crank->active);
    }
    vegetation_draw(level);
    if (crank->active) {
        crank_draw_platform(crank);
    }
    if (show_player) {
        player_draw(player);
    }
}

static void draw_collectibles(const Game *g)
{
    double now = GetTime();

    for (int i = 0; i < g->level.collectible_count; i++) {
        GridPos cell = g->level.collectibles[i];
        Vector3 position = { (float)cell.x, (float)cell.y, (float)cell.z };
        if (game_collectible_taken(g, i)) {
            float elapsed = (float)(now - g->collect_times[i]);
            if (elapsed < collectible_collect_duration()) {
                collectible_draw(position, 0.0f, elapsed, false);
            }
            continue;
        }
        bool owned = progress_has_collectible(&g->progress, g->world, g->level_index, i);
        collectible_draw(position, (float)now + i * COLLECTIBLE_PHASE, -1.0f, owned);
    }
}

static void draw_rocket(const Game *g, GridPos pad)
{
    bool launching = g->launching && g->launch_pad.x == pad.x && g->launch_pad.y == pad.y
                     && g->launch_pad.z == pad.z;
    Vector3 base = { (float)pad.x, pad.y - ROCKET_FLOOR_OFFSET, (float)pad.z };

    if (launching && g->launch_done) {
        return;
    }
    rocket_draw(base, launching ? player_model_angle(g->launch_dir) : 0.0f,
                launching ? g->launch_time : 0.0f);
}

static void draw_scene(Game *g, bool in_menu, Camera3D camera)
{
    GameState scene = game_scene_state(g);
    const Backdrop *sky = in_menu ? &g->title_sky
                        : (scene == STATE_MAP ? &g->map_sky : &g->level_skies[g->level_index]);
    Vector3 scene_center = in_menu ? level_center(&g->title_scene) : level_center(&g->level);

    backdrop_draw_dome(scene_center);
    backdrop_draw(sky, scene_center, camera);
    if (in_menu) {
        player_update(&g->title_player, &g->title_scene, g->title_orbit.yaw, false);
        level_draw(&g->title_scene, NULL, false);
        vegetation_draw(&g->title_scene);
        player_draw(&g->title_player);
        if (g->state != STATE_SETTINGS) {
            title_logo_draw(scene_center);
        }
    } else {
        draw_world(&g->level, &g->player, &g->crank, !g->launch_done);
        draw_collectibles(g);
        dust_draw();
    }
    if (!in_menu && scene == STATE_MAP) {
        ui_draw_entrances(&g->level, game_completed(g));
        if (game_rocket_next(g)) {
            draw_rocket(g, g->level.portal_next);
        } else if (g->level.has_portal_next) {
            ui_draw_portal(g->level.portal_next, false);
        }
        if (game_rocket_prev(g)) {
            draw_rocket(g, g->level.portal_prev);
        }
    }
}

static const char *portal_prompt(const Game *g)
{
    if (g->launching) {
        return NULL;
    }
    if (g->portal < 0 && g->world > 0) {
        return TextFormat(tr(TXT_PORTAL_BACK), world_name(&g->worlds[g->world - 1]));
    }
    if (g->portal <= 0) {
        return NULL;
    }
    if (g->world + 1 >= g->world_count) {
        return tr(TXT_PORTAL_SOON);
    }
    if (!game_world_done(g)) {
        return tr(TXT_PORTAL_LOCKED);
    }
    return TextFormat(tr(TXT_PORTAL_NEXT), world_name(&g->worlds[g->world + 1]));
}

static void draw_overlay(const Game *g, Camera3D camera)
{
    GameState scene = game_scene_state(g);
    bool in_title = game_in_title_scene(g);

    if (!in_title && scene == STATE_MAP) {
        ui_draw_map_hud(camera, &g->level, game_completed(g), g->entrance,
                        world_name(&g->worlds[g->world]), portal_prompt(g));
        ui_draw_map_counters(g->progress.seeds, g->progress.collectibles);
    } else if (!in_title) {
        bool owned[LEVEL_MAX_COLLECTIBLES];
        for (int i = 0; i < g->level.collectible_count; i++) {
            owned[i] = progress_has_collectible(&g->progress, g->world, g->level_index, i);
        }
        hud_draw(world_name(&g->worlds[g->world]), g->level_index);
        ui_draw_lives(g->lives);
        ui_draw_collectible_slots(g->level.collectible_count, owned);
    }
    if (!in_title && scene != g->state) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, PAUSE_SHADE));
    }
    if (g->state == STATE_SETTINGS) {
        settings_menu_draw(g->settings_selected, g->settings_waiting);
    } else if (game_menu_active(g)) {
        game_menu_draw(g);
    }
    if (settings_show_fps()) {
        DrawFPS(GetScreenWidth() - FPS_WIDTH, FPS_MARGIN);
    }
}

static void draw_transition(const Game *g)
{
    float fade = game_fade(g);

    if (fade <= 0.0f) {
        return;
    }
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, fade));
    if (fade >= LOADING_TEXT_MIN_FADE) {
        int width = ui_measure(tr(TXT_LOADING), ui_px(LOADING_FONT_SIZE));
        ui_text(tr(TXT_LOADING), GetScreenWidth() - width - ui_px(LOADING_MARGIN),
                 GetScreenHeight() - ui_px(LOADING_FONT_SIZE) - ui_px(LOADING_MARGIN), ui_px(LOADING_FONT_SIZE), Fade(RAYWHITE, fade));
    }
}

void game_draw(Game *g)
{
    bool in_menu = game_in_title_scene(g);
    Camera3D camera = orbit_camera_to_raylib(in_menu ? &g->title_orbit : &g->orbit);

    lighting_update(camera.position);
    BeginDrawing();
    postfx_set_blur(in_menu ? TITLE_BLUR : GAME_BLUR);
    postfx_begin();
    sky_draw_background();
    BeginMode3D(camera);
    draw_scene(g, in_menu, camera);
    EndMode3D();
    sky_draw_vignette();
    postfx_end();
    game_apply_menu_anchor(g);
    draw_overlay(g, camera);
    draw_transition(g);
    EndDrawing();
}

