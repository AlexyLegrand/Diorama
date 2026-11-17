#ifndef VEGETATION_H
#define VEGETATION_H

#include "world/level.h"

void vegetation_load(void);

void vegetation_unload(void);

void vegetation_update(void);

void vegetation_draw(const Level *level);

#endif
