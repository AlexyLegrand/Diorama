#include <string.h>
#include "render/rocket.h"
#include "render/lighting.h"

static const char *ROCKET_MODEL_PATH = "assets/rocket/rocket.glb";
static const char *LAUNCH_ANIM_NAME = "Launch";
static const float ROCKET_SCALE = 0.7f;
static const float ANIM_FPS = 60.0f;

static Model model;
static ModelAnimation *anims;
static int anim_count;
static int launch_index = -1;

void rocket_load(void)
{
    model = LoadModel(ROCKET_MODEL_PATH);
    if (!IsModelValid(model)) {
        return;
    }
    lighting_apply(&model);
    anims = LoadModelAnimations(ROCKET_MODEL_PATH, &anim_count);
    for (int i = 0; i < anim_count; i++) {
        if (strcmp(anims[i].name, LAUNCH_ANIM_NAME) == 0) {
            launch_index = i;
        }
    }
}

void rocket_unload(void)
{
    if (anims != NULL) {
        UnloadModelAnimations(anims, anim_count);
    }
    if (IsModelValid(model)) {
        UnloadModel(model);
    }
}

float rocket_launch_duration(void)
{
    return launch_index < 0 ? 0.0f : (float)anims[launch_index].keyframeCount / ANIM_FPS;
}

void rocket_draw(Vector3 base, float angle, float launch_time)
{
    if (!IsModelValid(model)) {
        return;
    }
    if (launch_index >= 0) {
        int last = anims[launch_index].keyframeCount - 2;
        int frame = (int)(launch_time * ANIM_FPS);
        UpdateModelAnimation(model, anims[launch_index], frame < last ? frame : last);
    }
    Vector3 scale = { ROCKET_SCALE, ROCKET_SCALE, ROCKET_SCALE };
    DrawModelEx(model, base, (Vector3){ 0.0f, 1.0f, 0.0f }, angle, scale, WHITE);
}
