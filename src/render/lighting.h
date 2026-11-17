#ifndef LIGHTING_H
#define LIGHTING_H

#include "raylib.h"

void lighting_load(void);

void lighting_unload(void);

void lighting_apply(Model *model);

void lighting_update(Vector3 camera_position);

#endif
