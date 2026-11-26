#ifndef DUST_H
#define DUST_H

#include "raylib.h"

void dust_spawn(Vector3 feet, int count);

void dust_update(float dt);

void dust_draw(void);

void dust_clear(void);

#endif
