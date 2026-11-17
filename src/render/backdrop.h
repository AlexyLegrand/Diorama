#ifndef BACKDROP_H
#define BACKDROP_H

#include <stdbool.h>
#include "raylib.h"

#define BACKDROP_MAX_ELEMENTS 32

typedef enum {
    SKY_CLOUD_SMALL,
    SKY_CLOUD_MEDIUM,
    SKY_CLOUD_BIG,
    SKY_CLOUD_SEA,
    SKY_FLOATING_ISLAND,
    SKY_RAINBOW,
    SKY_SUN,
    SKY_BIRD,
    SKY_KIND_COUNT
} SkyKind;

typedef struct {
    SkyKind kind;
    Vector3 offset;
    float scale;
    float yaw;
} SkyElement;

typedef struct {
    int count;
    SkyElement elements[BACKDROP_MAX_ELEMENTS];
} Backdrop;

void backdrop_assets_load(void);

void backdrop_assets_unload(void);

bool backdrop_load(Backdrop *backdrop, const char *path);

void backdrop_draw_dome(Vector3 center);

void backdrop_draw(const Backdrop *backdrop, Vector3 center, Camera3D camera);

#endif
