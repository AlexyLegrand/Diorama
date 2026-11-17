#include "render/block_render.h"
#include "render/lighting.h"
#include "raymath.h"
#include <string.h>
#include <stddef.h>

#define MODEL_FACING_OFFSET 90.0f
#define REWARD_MODEL_PATH   "assets/reward/golden_seed.glb"
#define CRANK_MODEL_PATH    "assets/crank/crank.glb"
#define CRANK_SCALE         0.7f
#define ANIM_FPS            60.0f

static Model block_models[BLOCK_COUNT];
static ModelAnimation *reward_anims;
static int reward_anim_count;
static int reward_idle_index = -1;
static int reward_collect_index = -1;
static float reward_frame;
static bool reward_collecting;
static bool reward_collect_done;
static ModelAnimation *crank_anims;
static int crank_anim_count;

static const char *BLOCK_MODEL_PATHS[BLOCK_COUNT] = {
    [BLOCK_GROUND] = "assets/blocks/block.glb",
    [BLOCK_CRATE]  = "assets/push/crate_big.glb",
    [BLOCK_STAIRS] = "assets/blocks/stairs.glb",
    [BLOCK_LADDER] = "assets/blocks/ladder.glb",
    [BLOCK_STAR]   = REWARD_MODEL_PATH,
    [BLOCK_CRANK]  = CRANK_MODEL_PATH,
};

static float direction_to_rotation_angle(Direction dir)
{
    static const float angles[] = {
        [DIR_EAST]  = 0.0f,
        [DIR_NORTH] = 90.0f,
        [DIR_WEST]  = 180.0f,
        [DIR_SOUTH] = 270.0f,
    };

    return angles[dir];
}

static int find_bone(Model model, const char *name)
{
    for (int i = 0; i < model.skeleton.boneCount; i++) {
        if (strcmp(model.skeleton.bones[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static void rebuild_wheel_spin(void)
{
    int wheel = find_bone(block_models[BLOCK_CRANK], "wheel");
    if (crank_anim_count == 0 || wheel == -1) {
        return;
    }
    ModelAnimation turn = crank_anims[0];
    Quaternion rest = turn.keyframePoses[0][wheel].rotation;
    Vector3 spin_axis = { 0.0f, 1.0f, 0.0f };
    float last_frame = (float)(turn.keyframeCount - 1);

    for (int frame = 0; frame < turn.keyframeCount; frame++) {
        float t = (float)frame / last_frame;
        float eased = t * t * (3.0f - 2.0f * t);
        float angle = -2.0f * PI * eased;
        Quaternion spin = QuaternionFromAxisAngle(spin_axis, angle);
        turn.keyframePoses[frame][wheel].rotation = QuaternionMultiply(rest, spin);
    }
}

void block_render_load(void)
{
    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (BLOCK_MODEL_PATHS[i] != NULL) {
            block_models[i] = LoadModel(BLOCK_MODEL_PATHS[i]);
            lighting_apply(&block_models[i]);
        }
    }
    crank_anims = LoadModelAnimations(CRANK_MODEL_PATH, &crank_anim_count);
    rebuild_wheel_spin();
    block_render_set_crank_progress(0.0f);
    reward_anims = LoadModelAnimations(REWARD_MODEL_PATH, &reward_anim_count);
    for (int i = 0; i < reward_anim_count; i++) {
        if (strcmp(reward_anims[i].name, "Idle") == 0) {
            reward_idle_index = i;
        }
        if (strcmp(reward_anims[i].name, "Collect") == 0) {
            reward_collect_index = i;
        }
    }
}

void block_render_update(void)
{
    int index = reward_collecting ? reward_collect_index : reward_idle_index;
    if (index == -1) {
        reward_collect_done = reward_collecting;
        return;
    }
    ModelAnimation anim = reward_anims[index];
    float last_frame = (float)(anim.keyframeCount - 1);
    reward_frame += GetFrameTime() * ANIM_FPS;
    if (reward_collecting && reward_frame >= last_frame) {
        reward_frame = last_frame;
        reward_collect_done = true;
    }
    while (reward_frame > last_frame) {
        reward_frame -= last_frame;
    }
    UpdateModelAnimation(block_models[BLOCK_STAR], anim, reward_frame);
}

void block_render_reward_collect(void)
{
    reward_collecting = true;
    reward_collect_done = false;
    reward_frame = 0.0f;
}

bool block_render_reward_collected(void)
{
    return reward_collect_done;
}

void block_render_reward_reset(void)
{
    reward_collecting = false;
    reward_collect_done = false;
    reward_frame = 0.0f;
}

void block_render_unload(void)
{
    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (BLOCK_MODEL_PATHS[i] != NULL) {
            UnloadModel(block_models[i]);
        }
    }
    UnloadModelAnimations(reward_anims, reward_anim_count);
    UnloadModelAnimations(crank_anims, crank_anim_count);
}

static void draw_reward_shadow(Vector3 center)
{
    Vector3 position;
    position.x = center.x;
    position.y = center.y - 0.49f;
    position.z = center.z;
    Color shadow_color = Fade(BLACK, 0.25f);

    DrawCylinder(position, 0.22f, 0.22f, 0.005f, 16, shadow_color);
}

static void draw_model(Model model, Vector3 center, float angle, float size, Color tint)
{
    Vector3 base;
    base.x = center.x;
    base.y = center.y - 0.5f;
    base.z = center.z;

    Vector3 axis;
    axis.x = 0.0f;
    axis.y = 1.0f;
    axis.z = 0.0f;

    Vector3 scale;
    scale.x = size;
    scale.y = size;
    scale.z = size;

    DrawModelEx(model, base, axis, angle, scale, tint);
}

void block_render_set_crank_progress(float progress)
{
    if (crank_anim_count == 0) {
        return;
    }
    ModelAnimation turn = crank_anims[0];
    float frame = progress * (float)(turn.keyframeCount - 1);
    UpdateModelAnimation(block_models[BLOCK_CRANK], turn, frame);
}

static float crank_model_angle(Direction wheel_side)
{
    static const float angles[] = {
        [DIR_NORTH] = 0.0f,
        [DIR_WEST]  = 90.0f,
        [DIR_SOUTH] = 180.0f,
        [DIR_EAST]  = 270.0f,
    };

    return angles[wheel_side];
}

void draw_block(int x, int y, int z, Cell cell)
{
    Vector3 center = { .x = x, .y = y, .z = z };

    draw_block_at(center, cell);
}

void draw_block_at(Vector3 center, Cell cell)
{
    float oriented_angle = direction_to_rotation_angle(cell.dir) + MODEL_FACING_OFFSET;

    switch (cell.type) {
    case BLOCK_GROUND:
        draw_model(block_models[cell.type], center, 0.0f, 1.0f, WHITE);
        break;
    case BLOCK_CRATE:
        draw_model(block_models[cell.type], center, 0.0f, 1.0f, WHITE);
        break;
    case BLOCK_STAIRS:
    case BLOCK_LADDER:
        draw_model(block_models[cell.type], center, oriented_angle, 1.0f, WHITE);
        break;
    case BLOCK_STAR:
        draw_reward_shadow(center);
        draw_model(block_models[cell.type], center, 0.0f, 1.0f, WHITE);
        break;
    case BLOCK_PLATFORM:
        draw_model(block_models[BLOCK_GROUND], center, 0.0f, 1.0f, BLOCK_INFO[cell.type].color);
        break;
    case BLOCK_CRANK:
        draw_model(block_models[cell.type], center, crank_model_angle(cell.dir), CRANK_SCALE, WHITE);
        break;
    case BLOCK_EMPTY:
    case BLOCK_PROP:
    case BLOCK_COUNT:
        break;
    }
}
