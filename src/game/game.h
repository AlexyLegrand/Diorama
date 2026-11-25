#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "world/level.h"
#include "core/camera.h"
#include "entities/player.h"
#include "entities/crank.h"
#include "render/backdrop.h"
#include "core/progress.h"
#include "game/world.h"

typedef enum {
    STATE_TITLE,
    STATE_SETTINGS,
    STATE_MAP,
    STATE_PLAYING,
    STATE_WON,
    STATE_PAUSED,
    STATE_CONFIRM_NEW
} GameState;

typedef enum {
    TRANSITION_NONE,
    TRANSITION_OUT,
    TRANSITION_HOLD,
    TRANSITION_IN
} TransitionPhase;

typedef struct {
    GameState state;
    Level title_scene;
    Level level;
    Player player;
    Player title_player;
    OrbitCamera orbit;
    OrbitCamera title_orbit;
    CrankAnim crank;
    Backdrop title_sky;
    Backdrop map_sky;
    Backdrop level_skies[LEVEL_MAX_ENTRANCES];
    WorldInfo worlds[WORLD_MAX];
    int world_count;
    int world;
    int level_count;
    Progress progress;
    int level_index;
    int entrance;
    int portal;
    int menu_selected;
    GameState paused_from;
    GameState settings_return;
    int settings_selected;
    int settings_waiting;
    bool quit;
    Music menu_music;
    Music world_music;
    Music wind;
    bool was_on_spot;
    double entrance_sound_time;
    bool launching;
    float launch_time;
    Direction launch_dir;
    GridPos launch_pad;
    int launch_target;
    bool launch_done;
    unsigned int level_collected;
    double collect_times[LEVEL_MAX_COLLECTIBLES];
    int lives;
    TransitionPhase transition;
    float transition_time;
    GameState next_state;
    int next_level;
    int next_world;
} Game;

bool game_init(Game *g);

void game_update(Game *g);

void game_draw(Game *g);

void game_unload(Game *g);

float game_fade(const Game *g);

const bool *game_completed(const Game *g);

bool game_world_done(const Game *g);

bool game_in_title_scene(const Game *g);

float game_world_hue(const Game *g);

bool game_collectible_taken(const Game *g, int index);

void game_lose_life(Game *g);

bool game_rocket_next(const Game *g);

bool game_rocket_prev(const Game *g);

GameState game_scene_state(const Game *g);

#endif
