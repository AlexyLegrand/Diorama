#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include "raylib.h"

void collectible_load(void);

void collectible_unload(void);

void collectible_set_hue(float hue);

float collectible_collect_duration(void);

void collectible_draw(Vector3 cell, float idle_time, float collect_time, bool ghost);

#endif
