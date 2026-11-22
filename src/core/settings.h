#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>

typedef enum {
    ACTION_UP,
    ACTION_DOWN,
    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_RESTART,
    ACTION_COUNT
} Action;

void settings_load(void);

void settings_save(void);

int settings_key(Action action);

void settings_set_key(Action action, int key);

void settings_reset_keys(void);

float settings_volume(void);

void settings_change_volume(float delta);

void settings_toggle_fullscreen(void);

bool settings_show_fps(void);

void settings_toggle_show_fps(void);

bool settings_mouse_enabled(void);

void settings_toggle_mouse(void);

const char *settings_action_name(Action action);

const char *settings_key_name(int key);

#endif
