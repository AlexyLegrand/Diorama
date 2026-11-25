#ifndef CRYSTAL_HUE_H
#define CRYSTAL_HUE_H

#include <stdbool.h>
#include "raylib.h"

bool crystal_hue_matches(Color color);

Color crystal_hue_shift(Color color, float hue);

#endif
