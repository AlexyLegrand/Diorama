#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "world/level.h"
#include "core/camera.h"
#include "entities/player.h"
#include "entities/crank.h"
#include "render/backdrop.h"

#define LEVEL_COUNT 4

typedef enum {
    STATE_TITLE,
    STATE_SETTINGS,
    STATE_MAP,
    STATE_PLAYING,
    STATE_WON
} GameState;

typedef enum {
    TITLE_PLAY,
    TITLE_SETTINGS,
    TITLE_QUIT,
    TITLE_COUNT
} TitleItem;

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
    Backdrop level_skies[LEVEL_COUNT];
    bool completed[LEVEL_COUNT];
    int level_index;
    int entrance;
    int title_selected;
    int settings_selected;
    int settings_waiting;
    bool quit;
} Game;

bool game_init(Game *g);

void game_update(Game *g);

void game_draw(Game *g);

void game_unload(Game *g);

#endif
