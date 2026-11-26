#ifndef POSTFX_H
#define POSTFX_H

#include "raylib.h"

void postfx_load(void);

void postfx_unload(void);

void postfx_set_blur(float strength);

void postfx_begin(void);

void postfx_end(void);

#endif
