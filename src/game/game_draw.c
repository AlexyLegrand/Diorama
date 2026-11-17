#include <stddef.h>
#include "raylib.h"
#include "game/game.h"
#include "core/settings.h"
#include "render/block_render.h"
#include "render/lighting.h"
#include "render/vegetation.h"
#include "render/sky.h"
#include "ui/ui.h"
#include "ui/settings_menu.h"

static const int FPS_MARGIN = 20;
static const int FPS_WIDTH = 100;

static void draw_world(const Level *level, const Player *player, const CrankAnim *crank)
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
    player_draw(player);
}

static void draw_scene(Game *g, bool in_menu, Camera3D camera)
{
    const Backdrop *sky = in_menu ? &g->title_sky
                        : (g->state == STATE_MAP ? &g->map_sky : &g->level_skies[g->level_index]);
    Vector3 scene_center = in_menu ? level_center(&g->title_scene) : level_center(&g->level);

    backdrop_draw_dome(scene_center);
    backdrop_draw(sky, scene_center, camera);
    if (in_menu) {
        player_update(&g->title_player, &g->title_scene, g->title_orbit.yaw, false);
        level_draw(&g->title_scene, NULL, false);
        vegetation_draw(&g->title_scene);
        player_draw(&g->title_player);
    } else {
        draw_world(&g->level, &g->player, &g->crank);
    }
    if (g->state == STATE_MAP) {
        ui_draw_entrances(&g->level, g->completed);
    }
}

static void draw_overlay(const Game *g, Camera3D camera)
{
    if (g->state == STATE_TITLE) {
        const char *labels[TITLE_COUNT] = { "Jouer", "Parametres", "Quitter" };
        ui_draw_menu("Diorama", labels, TITLE_COUNT, g->title_selected,
                     "Haut/Bas : choisir   Entree : valider   Echap : quitter");
    } else if (g->state == STATE_SETTINGS) {
        settings_menu_draw(g->settings_selected, g->settings_waiting);
    } else if (g->state == STATE_MAP) {
        ui_draw_map_hud(camera, &g->level, g->completed, g->entrance);
    } else {
        hud_draw(g->state == STATE_WON, g->level_index);
    }
    if (settings_show_fps()) {
        DrawFPS(GetScreenWidth() - FPS_WIDTH, FPS_MARGIN);
    }
}

void game_draw(Game *g)
{
    bool in_menu = g->state == STATE_TITLE || g->state == STATE_SETTINGS;
    Camera3D camera = orbit_camera_to_raylib(in_menu ? &g->title_orbit : &g->orbit);

    lighting_update(camera.position);
    BeginDrawing();
    sky_draw_background();
    BeginMode3D(camera);
    draw_scene(g, in_menu, camera);
    EndMode3D();
    sky_draw_vignette();
    draw_overlay(g, camera);
    EndDrawing();
}

