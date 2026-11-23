#ifndef ROCKET_H
#define ROCKET_H

#include "raylib.h"

void rocket_load(void);

void rocket_unload(void);

float rocket_launch_duration(void);

void rocket_draw(Vector3 base, float angle, float launch_time);

#endif
