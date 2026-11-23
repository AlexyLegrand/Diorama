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
static const char *LOADING_TEXT = "Chargement...";
static const int LOADING_FONT_SIZE = 30;
static const int LOADING_MARGIN = 40;
static const float LOADING_TEXT_MIN_FADE = 0.6f;

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
        ui_draw_entrances(&g->level, game_completed(g));
        bool next_open = g->world + 1 < g->world_count && game_world_done(g);
        if (g->level.has_portal_next) {
            ui_draw_portal(g->level.portal_next, next_open);
        }
        if (g->level.has_portal_prev) {
            ui_draw_portal(g->level.portal_prev, g->world > 0);
        }
    }
}

static const char *portal_prompt(const Game *g)
{
    if (g->portal < 0 && g->world > 0) {
        return TextFormat("Retour au monde %s  -  Entree", g->worlds[g->world - 1].name);
    }
    if (g->portal <= 0) {
        return NULL;
    }
    if (g->world + 1 >= g->world_count) {
        return "Monde suivant : bientot !";
    }
    if (!game_world_done(g)) {
        return "Monde suivant verrouille  -  terminez tous les niveaux";
    }
    return TextFormat("Monde %s  -  Entree : voyager", g->worlds[g->world + 1].name);
}

static void draw_overlay(const Game *g, Camera3D camera)
{
    if (g->state == STATE_TITLE) {
        const char *labels[TITLE_COUNT] = { "Jouer", "Parametres", "Quitter" };
        ui_draw_menu("Diorama", labels, TITLE_COUNT, g->title_selected,
                     settings_mouse_enabled() ? "Haut/Bas : choisir   Entree / clic : valider   Echap : quitter"
                                              : "Haut/Bas : choisir   Entree : valider   Echap : quitter");
    } else if (g->state == STATE_SETTINGS) {
        settings_menu_draw(g->settings_selected, g->settings_waiting);
    } else if (g->state == STATE_MAP) {
        ui_draw_map_hud(camera, &g->level, game_completed(g), g->entrance,
                        g->worlds[g->world].name, portal_prompt(g));
    } else {
        hud_draw(g->state == STATE_WON, g->worlds[g->world].name, g->level_index);
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
        int width = MeasureText(LOADING_TEXT, LOADING_FONT_SIZE);
        DrawText(LOADING_TEXT, GetScreenWidth() - width - LOADING_MARGIN,
                 GetScreenHeight() - LOADING_FONT_SIZE - LOADING_MARGIN, LOADING_FONT_SIZE, Fade(RAYWHITE, fade));
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
    draw_transition(g);
    EndDrawing();
}

