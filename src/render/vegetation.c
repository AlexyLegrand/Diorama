#include <math.h>
#include "raylib.h"
#include "render/vegetation.h"
#include "render/lighting.h"

static const char *VEGETATION_PATHS[VEG_COUNT] = {
    [VEG_TREE_ROUND] = "assets/vegetation/tree_round.glb",
    [VEG_TREE_PINE]  = "assets/vegetation/tree_pine.glb",
    [VEG_BUSH]       = "assets/vegetation/bush.glb",
    [VEG_FERN]       = "assets/vegetation/fern.glb",
    [VEG_GRASS]      = "assets/vegetation/grass_tuft.glb",
    [VEG_FLOWERS]    = "assets/vegetation/flowers.glb",
    [VEG_MUSHROOMS]  = "assets/vegetation/mushrooms.glb",
    [VEG_IVY]        = "assets/vegetation/ivy_wall.glb",
    [VEG_VINE]       = "assets/vegetation/vine_hanging.glb",
};

static const float SHADOW_RADIUS[VEG_COUNT] = {
    [VEG_TREE_ROUND] = 0.42f,
    [VEG_TREE_PINE]  = 0.40f,
    [VEG_BUSH]       = 0.36f,
    [VEG_FERN]       = 0.32f,
    [VEG_GRASS]      = 0.12f,
    [VEG_FLOWERS]    = 0.18f,
    [VEG_MUSHROOMS]  = 0.16f,
};
static const float SHADOW_LIFT = 0.01f;
static const float SHADOW_THICKNESS = 0.005f;
static const float SHADOW_ALPHA = 0.28f;
static const int SHADOW_SLICES = 16;

static const float ANIM_FPS = 60.0f;
static const float HALF_CELL = 0.5f;
static const float QUARTER_TURN = 90.0f;
static const unsigned int PHASE_SEED_X = 73856093u;
static const unsigned int PHASE_SEED_Y = 19349663u;
static const unsigned int PHASE_SEED_Z = 83492791u;

static Model models[VEG_COUNT];
static ModelAnimation *anims[VEG_COUNT];
static int anim_counts[VEG_COUNT];

void vegetation_load(void)
{
    for (int i = 0; i < VEG_COUNT; i++) {
        models[i] = LoadModel(VEGETATION_PATHS[i]);
        lighting_apply(&models[i]);
        anims[i] = LoadModelAnimations(VEGETATION_PATHS[i], &anim_counts[i]);
    }
}

void vegetation_unload(void)
{
    for (int i = 0; i < VEG_COUNT; i++) {
        UnloadModelAnimations(anims[i], anim_counts[i]);
        UnloadModel(models[i]);
    }
}

static unsigned int cell_hash(GridPos p)
{
    return ((unsigned int)p.x * PHASE_SEED_X) ^ ((unsigned int)p.y * PHASE_SEED_Y) ^ ((unsigned int)p.z * PHASE_SEED_Z);
}

static bool is_wall_plant(Vegetation kind)
{
    return kind == VEG_IVY || kind == VEG_VINE;
}

static bool side_is_open(const Level *level, GridPos p, int dx, int dz)
{
    int x = p.x + dx;
    int z = p.z + dz;
    bool inside = x >= 0 && x < level->width && z >= 0 && z < level->depth;

    return !inside || level->cells[p.y][x][z].type == BLOCK_EMPTY;
}

void vegetation_update(void)
{
    for (int i = 0; i < VEG_COUNT; i++) {
        if (anim_counts[i] == 0) {
            continue;
        }
        ModelAnimation sway = anims[i][0];
        int frame = (int)fmodf((float)GetTime() * ANIM_FPS, (float)sway.keyframeCount);
        UpdateModelAnimation(models[i], sway, frame);
    }
}

static void draw_shadow(Vegetation kind, GridPos p)
{
    float radius = SHADOW_RADIUS[kind];
    if (radius <= 0.0f) {
        return;
    }
    Vector3 floor = { (float)p.x, (float)p.y - HALF_CELL + SHADOW_LIFT, (float)p.z };
    DrawCylinder(floor, radius, radius, SHADOW_THICKNESS, SHADOW_SLICES, Fade(BLACK, SHADOW_ALPHA));
}

static void draw_at(Vegetation kind, GridPos p, float angle)
{
    Vector3 base = { (float)p.x, (float)p.y - HALF_CELL, (float)p.z };

    DrawModelEx(models[kind], base, (Vector3){ 0.0f, 1.0f, 0.0f }, angle, (Vector3){ 1.0f, 1.0f, 1.0f }, WHITE);
}

static void draw_wall_plant(const Level *level, Vegetation kind, GridPos p)
{
    static const struct { int dx; int dz; float angle; } SIDES[] = {
        {  0,  1, 0.0f },
        {  1,  0, 90.0f },
        {  0, -1, 180.0f },
        { -1,  0, 270.0f },
    };

    for (int i = 0; i < 4; i++) {
        if (side_is_open(level, p, SIDES[i].dx, SIDES[i].dz)) {
            draw_at(kind, p, SIDES[i].angle);
        }
    }
}

void vegetation_draw(const Level *level)
{
    for (int i = 0; i < level->decor_count; i++) {
        Decor decor = level->decor[i];
        unsigned int hash = cell_hash(decor.pos);

        if (is_wall_plant(decor.kind)) {
            draw_wall_plant(level, decor.kind, decor.pos);
        } else {
            draw_shadow(decor.kind, decor.pos);
            draw_at(decor.kind, decor.pos, (float)(hash % 4) * QUARTER_TURN);
        }
    }
}
