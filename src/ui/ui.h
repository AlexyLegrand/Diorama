#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "raylib.h"
#include "world/level.h"

int ui_navigate(int selected, int count);

bool ui_confirm_pressed(void);

void ui_draw_menu(const char *title, const char *const labels[], int count, int selected, const char *hint);

bool ui_level_unlocked(const bool completed[], int index);

void ui_draw_entrances(const Level *map, const bool completed[]);

void ui_draw_map_hud(Camera3D camera, const Level *map, const bool completed[], int entrance_here);

void hud_draw(bool won, int level_index);

#endif
