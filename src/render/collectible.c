#include <string.h>
#include "render/collectible.h"
#include "render/crystal_hue.h"
#include "render/lighting.h"

#define MATERIAL_MAX 16

static const char *MODEL_PATH = "assets/collectibles/crystal_lotus.glb";
static const char *IDLE_ANIM = "Idle";
static const char *COLLECT_ANIM = "Collect";
static const float ANIM_FPS = 60.0f;
static const float FLOOR_OFFSET = 0.5f;
static const float GHOST_ALPHA = 0.35f;
static const float MODEL_SCALE = 1.6f;

static Model model;
static ModelAnimation *anims;
static int anim_count;
static int idle_index = -1;
static int collect_index = -1;
static Color original_colors[MATERIAL_MAX];
static bool crystal_material[MATERIAL_MAX];

static int find_anim(const char *name)
{
    for (int i = 0; i < anim_count; i++) {
        if (strcmp(anims[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void collectible_load(void)
{
    model = LoadModel(MODEL_PATH);
    if (!IsModelValid(model)) {
        return;
    }
    lighting_apply(&model);
    anims = LoadModelAnimations(MODEL_PATH, &anim_count);
    idle_index = find_anim(IDLE_ANIM);
    collect_index = find_anim(COLLECT_ANIM);
    for (int i = 0; i < model.materialCount && i < MATERIAL_MAX; i++) {
        original_colors[i] = model.materials[i].maps[MATERIAL_MAP_DIFFUSE].color;
        crystal_material[i] = crystal_hue_matches(original_colors[i]);
    }
}

void collectible_unload(void)
{
    if (anims != NULL) {
        UnloadModelAnimations(anims, anim_count);
    }
    if (IsModelValid(model)) {
        UnloadModel(model);
    }
}

void collectible_set_hue(float hue)
{
    for (int i = 0; i < model.materialCount && i < MATERIAL_MAX; i++) {
        if (crystal_material[i]) {
            model.materials[i].maps[MATERIAL_MAP_DIFFUSE].color = crystal_hue_shift(original_colors[i], hue);
        }
    }
}

float collectible_collect_duration(void)
{
    return collect_index < 0 ? 0.0f : (float)anims[collect_index].keyframeCount / ANIM_FPS;
}

static void pose(int index, float time, bool loops)
{
    if (index < 0) {
        return;
    }
    int count = anims[index].keyframeCount;
    int frame = (int)(time * ANIM_FPS);
    frame = loops ? frame % count : (frame < count - 2 ? frame : count - 2);
    UpdateModelAnimation(model, anims[index], frame);
}

void collectible_draw(Vector3 cell, float idle_time, float collect_time, bool ghost)
{
    if (!IsModelValid(model)) {
        return;
    }
    if (collect_time >= 0.0f) {
        pose(collect_index, collect_time, false);
    } else {
        pose(idle_index, idle_time, true);
    }
    Vector3 base = { cell.x, cell.y - FLOOR_OFFSET, cell.z };
    DrawModel(model, base, MODEL_SCALE, ghost ? Fade(WHITE, GHOST_ALPHA) : WHITE);
}
