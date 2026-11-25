#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#include "game/game.h"

void begin_transition(Game *g, GameState next_state, int next_level);

bool load_world(Game *g, int index);

void restart_level(Game *g);

bool game_menu_active(const Game *g);

void game_menu_update(Game *g, bool back, bool confirm);

void game_menu_open_pause(Game *g);

void game_menu_draw(const Game *g);

void game_apply_menu_anchor(const Game *g);

#endif
