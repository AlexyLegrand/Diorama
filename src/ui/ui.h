#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "raylib.h"
#include "world/level.h"

float ui_scale(void);

int ui_px(int pixels);

void ui_load_font(void);

void ui_unload_font(void);

void ui_text(const char *text, int x, int y, int size, Color color);

int ui_measure(const char *text, int size);

typedef enum {
    MENU_ANCHOR_CENTER,
    MENU_ANCHOR_BOTTOM
} MenuAnchor;

void ui_set_menu_anchor(MenuAnchor anchor);

bool ui_menu_is_row(void);

int ui_navigate(int selected, int count);

bool ui_confirm_pressed(void);

int ui_menu_hovered(int count);

bool ui_menu_clicked(int count);

void ui_draw_menu(const char *title, const char *const labels[], int count, int selected, const char *hint);

bool ui_level_unlocked(const bool completed[], int index);

void ui_draw_entrances(const Level *map, const bool completed[]);

void ui_draw_portal(GridPos cell, bool open);

void ui_draw_map_hud(Camera3D camera, const Level *map, const bool completed[], int entrance_here,
                     const char *world_name, const char *portal_prompt);

void hud_draw(const char *world_name, int level_index);

#endif
