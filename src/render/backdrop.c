#include <math.h>
#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include "rlgl.h"
#include "render/backdrop.h"
#include "render/lighting.h"

static const struct {
    const char *name;
    const char *path;
    bool lit;
} SKY_ASSETS[SKY_KIND_COUNT] = {
    [SKY_CLOUD_SMALL]     = { "cloud_small",     "assets/sky/cloud_small.glb",     true },
    [SKY_CLOUD_MEDIUM]    = { "cloud_medium",    "assets/sky/cloud_medium.glb",    true },
    [SKY_CLOUD_BIG]       = { "cloud_big",       "assets/sky/cloud_big.glb",       true },
    [SKY_CLOUD_SEA]       = { "cloud_sea",       "assets/sky/cloud_sea.glb",       true },
    [SKY_FLOATING_ISLAND] = { "floating_island", "assets/sky/floating_island.glb", true },
    [SKY_RAINBOW]         = { "rainbow",         "assets/sky/rainbow.glb",         false },
    [SKY_SUN]             = { "sun",             "assets/sky/sun.glb",             false },
    [SKY_BIRD]            = { "bird",            "assets/sky/bird.glb",            true },
};
static const char *DOME_PATH = "assets/sky/sky_dome.glb";

static const float ANIM_FPS = 60.0f;
static const float BOB_SPEED = 0.35f;
static const float BOB_HEIGHT = 0.25f;
static const float BIRD_CIRCLE_RADIUS = 3.0f;
static const float BIRD_CIRCLE_SPEED = 0.4f;
#define SKY_LINE_MAX 128

static Model models[SKY_KIND_COUNT];
static Model dome;
static ModelAnimation *bird_anims;
static int bird_anim_count;

void backdrop_assets_load(void)
{
    for (int i = 0; i < SKY_KIND_COUNT; i++) {
        models[i] = LoadModel(SKY_ASSETS[i].path);
        if (SKY_ASSETS[i].lit) {
            lighting_apply(&models[i]);
        }
    }
    dome = LoadModel(DOME_PATH);
    bird_anims = LoadModelAnimations(SKY_ASSETS[SKY_BIRD].path, &bird_anim_count);
}

void backdrop_assets_unload(void)
{
    UnloadModelAnimations(bird_anims, bird_anim_count);
    UnloadModel(dome);
    for (int i = 0; i < SKY_KIND_COUNT; i++) {
        UnloadModel(models[i]);
    }
}

static int kind_from_name(const char *name)
{
    for (int i = 0; i < SKY_KIND_COUNT; i++) {
        if (strcmp(SKY_ASSETS[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static bool parse_line(const char *line, SkyElement *element)
{
    char name[32];
    SkyElement e;

    if (sscanf(line, "%31s %f %f %f %f %f", name, &e.offset.x, &e.offset.y, &e.offset.z, &e.scale, &e.yaw) != 6) {
        return false;
    }
    int kind = kind_from_name(name);
    if (kind < 0 || e.scale <= 0.0f) {
        return false;
    }
    e.kind = (SkyKind)kind;
    *element = e;
    return true;
}

bool backdrop_load(Backdrop *backdrop, const char *path)
{
    backdrop->count = 0;
    if (!FileExists(path)) {
        return true;
    }
    char *text = LoadFileText(path);
    if (text == NULL) {
        return false;
    }
    bool ok = true;
    int line_number = 0;
    for (char *line = strtok(text, "\n"); line != NULL && ok; line = strtok(NULL, "\n")) {
        line_number++;
        char trimmed[SKY_LINE_MAX];
        if (sscanf(line, " %127[^\r]", trimmed) != 1 || trimmed[0] == '#') {
            continue;
        }
        if (backdrop->count >= BACKDROP_MAX_ELEMENTS || !parse_line(trimmed, &backdrop->elements[backdrop->count])) {
            TraceLog(LOG_WARNING, "BACKDROP: [%s] line %d is invalid or over the limit", path, line_number);
            ok = false;
            break;
        }
        backdrop->count++;
    }
    UnloadFileText(text);
    return ok;
}

void backdrop_draw_dome(Vector3 center)
{
    rlDisableDepthMask();
    rlDisableBackfaceCulling();
    DrawModel(dome, center, 1.0f, WHITE);
    rlDrawRenderBatchActive();
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}

static void draw_model(SkyKind kind, Vector3 position, float yaw, float scale)
{
    DrawModelEx(models[kind], position, (Vector3){ 0.0f, 1.0f, 0.0f }, yaw, (Vector3){ scale, scale, scale }, WHITE);
}

static void draw_bird(const SkyElement *e, Vector3 anchor, float time, int index)
{
    float angle = time * BIRD_CIRCLE_SPEED + (float)index;
    Vector3 position = {
        anchor.x + cosf(angle) * BIRD_CIRCLE_RADIUS * e->scale,
        anchor.y,
        anchor.z + sinf(angle) * BIRD_CIRCLE_RADIUS * e->scale,
    };
    float heading = atan2f(-sinf(angle), cosf(angle)) * RAD2DEG;

    draw_model(SKY_BIRD, position, heading, e->scale);
}

void backdrop_draw(const Backdrop *backdrop, Vector3 center, Camera3D camera)
{
    float time = (float)GetTime();

    if (bird_anim_count > 0) {
        int frame = (int)fmodf(time * ANIM_FPS, (float)bird_anims[0].keyframeCount);
        UpdateModelAnimation(models[SKY_BIRD], bird_anims[0], frame);
    }
    for (int i = 0; i < backdrop->count; i++) {
        const SkyElement *e = &backdrop->elements[i];
        Vector3 anchor = { center.x + e->offset.x, center.y + e->offset.y, center.z + e->offset.z };

        if (e->kind == SKY_BIRD) {
            draw_bird(e, anchor, time, i);
            continue;
        }
        if (e->kind == SKY_SUN) {
            float yaw = atan2f(camera.position.x - anchor.x, camera.position.z - anchor.z) * RAD2DEG;
            draw_model(SKY_SUN, anchor, yaw, e->scale);
            continue;
        }
        if (e->kind != SKY_CLOUD_SEA) {
            anchor.y += sinf(time * BOB_SPEED + (float)i) * BOB_HEIGHT;
        }
        draw_model(e->kind, anchor, e->yaw, e->scale);
    }
}
