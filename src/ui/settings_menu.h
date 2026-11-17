#ifndef SETTINGS_MENU_H
#define SETTINGS_MENU_H

#include <stdbool.h>

#define NOT_WAITING (-1)

bool settings_menu_update(int *selected, int *waiting);

void settings_menu_draw(int selected, int waiting);

#endif
