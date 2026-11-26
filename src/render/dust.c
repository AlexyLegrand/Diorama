#include <math.h>
#include "render/dust.h"

#define DUST_MAX 48

static const float DUST_LIFETIME = 0.45f;
static const float DUST_START_RADIUS = 0.05f;
static const float DUST_END_RADIUS = 0.13f;
static const float DUST_SPREAD_SPEED = 0.6f;
static const float DUST_RISE_SPEED = 0.25f;
static const float DUST_HEIGHT = 0.06f;
static const unsigned char DUST_ALPHA = 150;
static const Color DUST_COLOR = { 235, 225, 205, 255 };

typedef struct {
    Vector3 position;
    Vector3 velocity;
    float age;
    bool alive;
} Puff;

static Puff puffs[DUST_MAX];

void dust_spawn(Vector3 feet, int count)
{
    for (int n = 0, i = 0; n < count && i < DUST_MAX; i++) {
        if (puffs[i].alive) {
            continue;
        }
        float angle = (float)GetRandomValue(0, 359) * DEG2RAD;
        puffs[i] = (Puff){
            .position = { feet.x, feet.y + DUST_HEIGHT, feet.z },
            .velocity = { cosf(angle) * DUST_SPREAD_SPEED, DUST_RISE_SPEED, sinf(angle) * DUST_SPREAD_SPEED },
            .age = 0.0f,
            .alive = true,
        };
        n++;
    }
}

void dust_update(float dt)
{
    for (int i = 0; i < DUST_MAX; i++) {
        if (!puffs[i].alive) {
            continue;
        }
        puffs[i].age += dt;
        puffs[i].alive = puffs[i].age < DUST_LIFETIME;
        puffs[i].position.x += puffs[i].velocity.x * dt;
        puffs[i].position.y += puffs[i].velocity.y * dt;
        puffs[i].position.z += puffs[i].velocity.z * dt;
    }
}

void dust_draw(void)
{
    for (int i = 0; i < DUST_MAX; i++) {
        if (!puffs[i].alive) {
            continue;
        }
        float t = puffs[i].age / DUST_LIFETIME;
        float radius = DUST_START_RADIUS + (DUST_END_RADIUS - DUST_START_RADIUS) * t;
        Color color = DUST_COLOR;
        color.a = (unsigned char)(DUST_ALPHA * (1.0f - t));
        DrawSphereEx(puffs[i].position, radius, 6, 8, color);
    }
}

void dust_clear(void)
{
    for (int i = 0; i < DUST_MAX; i++) {
        puffs[i].alive = false;
    }
}
