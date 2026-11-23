#include "core/settings.h"
#include "entities/player.h"
#include "render/lighting.h"
#include "raymath.h"
#include "core/sfx.h"
#include <string.h>
#include <math.h>

#define PLAYER_MODEL_PATH "assets/crank/sprout.glb"
#define PLAYER_SCALE      0.7f
#define ANIM_FPS          60.0f
#define BLEND_DURATION    0.2f
#define WALK_SPEED        2.5f
#define CLIMB_SPEED       1.5f
#define CLIMB_SOUND_INTERVAL (1.0f / CLIMB_SPEED)
#define CRATE_FALL_SPEED  5.0f
#define PUSH_LEAN         0.2f
#define LEAN_SPEED        10.0f
#define CRANK_LEAN        (1.0f - PLAYER_SCALE)

static const char *ANIM_NAMES[ANIM_COUNT] = {
    [ANIM_IDLE]    = "Idle",
    [ANIM_WALK]    = "Walk",
    [ANIM_CLIMB]   = "Climb",
    [ANIM_VICTORY]    = "Victory",
    [ANIM_PUSH_START] = "PushStart",
    [ANIM_PUSH]       = "Push",
    [ANIM_CRANK]      = "Crank",
};

static float direction_to_model_angle(Direction dir)
{
    static const float angles[] = {
        [DIR_SOUTH] = 0.0f,
        [DIR_EAST]  = 90.0f,
        [DIR_NORTH] = 180.0f,
        [DIR_WEST]  = 270.0f,
    };

    return angles[dir];
}

Player player_load(int x, int y, int z, Direction dir)
{
    Player player = {0};

    player.model = LoadModel(PLAYER_MODEL_PATH);
    lighting_apply(&player.model);
    player.anims = LoadModelAnimations(PLAYER_MODEL_PATH, &player.anim_count);
    for (int i = 0; i < ANIM_COUNT; i++) {
        player.anim_lookup[i] = -1;
        for (int j = 0; j < player.anim_count; j++) {
            if (strcmp(player.anims[j].name, ANIM_NAMES[i]) == 0) {
                player.anim_lookup[i] = j;
            }
        }
        if (player.anim_lookup[i] == -1) {
            TraceLog(LOG_WARNING, "PLAYER: animation '%s' not found", ANIM_NAMES[i]);
        }
    }
    player.current = ANIM_IDLE;
    player.previous = ANIM_IDLE;
    player.blend = 1.0f;
    player.dir = dir;
    player_place(&player, x, y, z);
    return player;
}

void player_place(Player *player, int x, int y, int z)
{
    player->x = x;
    player->y = y;
    player->z = z;
    player->position.x = (float)x;
    player->position.y = (float)y;
    player->position.z = (float)z;
    player->moving = false;
    player->crate_moving = false;
    player->celebrating = false;
    player->push_wait = 0.0f;
    player->was_pushing = false;
    player->lean = 0.0f;
    player->cranking = false;
}

void player_set_crank(Player *player, bool active, float progress)
{
    player->cranking = active;
    player->crank_progress = progress;
}

void player_celebrate(Player *player, float camera_yaw)
{
    player->celebrating = true;
    player->dir = movement_input_direction(camera_yaw, 0, -1);
    player_set_anim(player, ANIM_VICTORY);
}

bool player_is_idle(const Player *player)
{
    return !player->moving && !player->crate_moving;
}

void player_unload(Player *player)
{
    UnloadModelAnimations(player->anims, player->anim_count);
    UnloadModel(player->model);
}

static bool anim_loops(PlayerAnim anim)
{
    return anim != ANIM_PUSH_START;
}

static float advance_frame(float frame, ModelAnimation anim, float dt, bool loops)
{
    float frame_count = (float)anim.keyframeCount;
    float last_frame = frame_count - 1.0f;
    float next = frame + dt * ANIM_FPS;

    if (!loops) {
        return fminf(next, last_frame);
    }
    while (next >= frame_count) {
        next -= frame_count;
    }
    return next;
}

void player_set_anim(Player *player, PlayerAnim anim)
{
    if (anim == player->current || player->anim_lookup[anim] == -1) {
        return;
    }
    player->previous = player->current;
    player->previous_frame = player->frame;
    player->current = anim;
    player->frame = 0.0f;
    player->blend = 0.0f;
}

static void read_input(int *input_x, int *input_y)
{
    *input_x = 0;
    *input_y = 0;
    if (IsKeyDown(settings_key(ACTION_UP))) {
        *input_y = 1;
    } else if (IsKeyDown(settings_key(ACTION_DOWN))) {
        *input_y = -1;
    } else if (IsKeyDown(settings_key(ACTION_RIGHT))) {
        *input_x = 1;
    } else if (IsKeyDown(settings_key(ACTION_LEFT))) {
        *input_x = -1;
    }
}

static float push_start_duration(const Player *player)
{
    int index = player->anim_lookup[ANIM_PUSH_START];
    if (index == -1) {
        return 0.0f;
    }
    return (float)player->anims[index].keyframeCount / ANIM_FPS;
}

static bool facing_crank(const Player *player, const Level *level, Direction dir)
{
    static const int DX[4] = { [DIR_NORTH] = 0, [DIR_EAST] = 1, [DIR_SOUTH] = 0, [DIR_WEST] = -1 };
    static const int DZ[4] = { [DIR_NORTH] = -1, [DIR_EAST] = 0, [DIR_SOUTH] = 1, [DIR_WEST] = 0 };
    int x = player->x + DX[dir];
    int z = player->z + DZ[dir];
    bool inside = x >= 0 && x < level->width && z >= 0 && z < level->depth;

    if (!inside) {
        return false;
    }
    Cell cell = level->cells[player->y][x][z];
    Direction wheel_side = (dir + 2) % 4;
    return cell.type == BLOCK_CRANK && cell.dir == wheel_side;
}

static bool path_climbs(const MovePath *path)
{
    for (int i = 0; i < path->count; i++) {
        if (path->points[i].climbing) {
            return true;
        }
    }
    return false;
}

static void try_start_move(Player *player, Level *level, float camera_yaw)
{
    int input_x;
    int input_y;
    read_input(&input_x, &input_y);
    if (input_x == 0 && input_y == 0) {
        return;
    }

    Direction dir = movement_input_direction(camera_yaw, input_x, input_y);
    player->dir = dir;
    if (!movement_plan(level, player->x, player->y, player->z, dir, &player->path)) {
        bool fresh_press = IsKeyPressed(settings_key(ACTION_UP)) || IsKeyPressed(settings_key(ACTION_LEFT))
                           || IsKeyPressed(settings_key(ACTION_DOWN)) || IsKeyPressed(settings_key(ACTION_RIGHT));
        player->crank_requested = fresh_press && facing_crank(player, level, dir);
        if (fresh_press && !player->crank_requested) {
            sfx_play(SFX_BUMP);
        }
        return;
    }
    if (player->path.push) {
        GridPos from = player->path.crate_from;
        player->crate_cell = level->cells[from.y][from.x][from.z];
        player->crate_position.x = (float)from.x;
        player->crate_position.y = (float)from.y;
        player->crate_position.z = (float)from.z;
        player->crate_moving = true;
        movement_apply(level, &player->path);
        if (!player->was_pushing) {
            player->push_wait = push_start_duration(player);
            sfx_play(SFX_PUSH_START);
        }
    } else if (!path_climbs(&player->path)) {
        sfx_play(SFX_STEP);
    }
    player->climb_sound_timer = 0.0f;
    player->moving = true;
    player->path_index = 0;
    player->segment_start = player->position;
    player->segment_progress = 0.0f;
}

static void sync_push_frame(Player *player)
{
    int index = player->anim_lookup[ANIM_PUSH];
    if (index == -1) {
        return;
    }
    float last_frame = (float)(player->anims[index].keyframeCount - 1);
    float progress = fminf(player->segment_progress, 1.0f);

    player->frame = progress * last_frame;
}

static void sync_crank_frame(Player *player)
{
    int index = player->anim_lookup[ANIM_CRANK];
    if (index == -1) {
        return;
    }
    float last_frame = (float)(player->anims[index].keyframeCount - 1);
    player->frame = player->crank_progress * last_frame;
}

static void finish_crate_slide(Player *player)
{
    if (!player->crate_moving) {
        return;
    }
    GridPos side = player->path.crate_side;
    player->crate_position.x = (float)side.x;
    player->crate_position.y = (float)side.y;
    player->crate_position.z = (float)side.z;
    if (player->path.crate_to.y == side.y) {
        player->crate_moving = false;
    }
}

static void play_climb_sound(Player *player, float dt)
{
    player->climb_sound_timer -= dt;
    if (player->climb_sound_timer <= 0.0f) {
        sfx_play(SFX_CLIMB);
        player->climb_sound_timer = CLIMB_SOUND_INTERVAL;
    }
}

static void advance_move(Player *player, float dt)
{
    PathPoint target = player->path.points[player->path_index];
    float length = Vector3Distance(player->segment_start, target.position);
    float speed = target.climbing ? CLIMB_SPEED : WALK_SPEED;

    player->dir = target.facing;
    if (player->push_wait > 0.0f) {
        player->push_wait -= dt;
        player_set_anim(player, ANIM_PUSH_START);
        return;
    }
    PlayerAnim anim = ANIM_WALK;
    if (target.climbing) {
        anim = ANIM_CLIMB;
        play_climb_sound(player, dt);
    } else if (player->path.push) {
        anim = ANIM_PUSH;
    }
    player_set_anim(player, anim);
    if (length > 0.0f) {
        player->segment_progress += speed * dt / length;
    } else {
        player->segment_progress = 1.0f;
    }

    if (anim == ANIM_PUSH) {
        sync_push_frame(player);
    }
    if (player->segment_progress < 1.0f) {
        player->position = Vector3Lerp(player->segment_start, target.position, player->segment_progress);
        if (player->crate_moving) {
            GridPos from = player->path.crate_from;
            GridPos side = player->path.crate_side;
            Vector3 crate_from = { (float)from.x, (float)from.y, (float)from.z };
            Vector3 crate_side = { (float)side.x, (float)side.y, (float)side.z };
            player->crate_position = Vector3Lerp(crate_from, crate_side, player->segment_progress);
        }
        return;
    }
    player->position = target.position;
    player->segment_start = target.position;
    player->segment_progress = 0.0f;
    player->path_index++;
    if (player->path_index >= player->path.count) {
        player->moving = false;
        player->was_pushing = player->path.push;
        finish_crate_slide(player);
        player->x = player->path.end_x;
        player->y = player->path.end_y;
        player->z = player->path.end_z;
    }
}

static void update_crate_fall(Player *player, float dt)
{
    GridPos side = player->path.crate_side;
    GridPos to = player->path.crate_to;
    float landing_y = (float)to.y;

    player->crate_position.x = (float)side.x;
    player->crate_position.z = (float)side.z;
    player->crate_position.y -= CRATE_FALL_SPEED * dt;
    if (player->crate_position.y <= landing_y) {
        player->crate_position.y = landing_y;
        player->crate_moving = false;
        sfx_play(SFX_CRATE_LAND);
    }
}

bool player_moving_crate(const Player *player, GridPos *cell, Vector3 *position, Cell *crate)
{
    if (!player->crate_moving) {
        return false;
    }
    *cell = player->path.crate_to;
    *position = player->crate_position;
    *crate = player->crate_cell;
    return true;
}

static void update_animation(Player *player, float dt)
{
    int current_index = player->anim_lookup[player->current];
    if (current_index == -1) {
        return;
    }

    ModelAnimation current = player->anims[current_index];
    bool pushing = player->current == ANIM_PUSH && player->moving;
    bool cranking = player->current == ANIM_CRANK && player->cranking;
    bool frame_driven_by_move = pushing || cranking;
    if (!frame_driven_by_move) {
        player->frame = advance_frame(player->frame, current, dt, anim_loops(player->current));
    }

    if (player->blend >= 1.0f) {
        UpdateModelAnimation(player->model, current, player->frame);
        return;
    }

    ModelAnimation previous = player->anims[player->anim_lookup[player->previous]];
    player->previous_frame = advance_frame(player->previous_frame, previous, dt, anim_loops(player->previous));
    player->blend += dt / BLEND_DURATION;
    if (player->blend > 1.0f) {
        player->blend = 1.0f;
    }
    UpdateModelAnimationEx(player->model, previous, player->previous_frame, current, player->frame, player->blend);
}

void player_update(Player *player, Level *level, float camera_yaw, bool can_move)
{
    float dt = GetFrameTime();

    player->crank_requested = false;
    if (player->celebrating) {
        update_animation(player, dt);
        return;
    }
    if (can_move && !player->moving && !player->crate_moving) {
        try_start_move(player, level, camera_yaw);
        if (!player->moving) {
            player->was_pushing = false;
        }
    }
    float lean_target = (player->moving && player->path.push) ? PUSH_LEAN : 0.0f;
    if (player->cranking) {
        lean_target = CRANK_LEAN;
    }
    float lean_step = fminf(1.0f, LEAN_SPEED * dt);
    player->lean += (lean_target - player->lean) * lean_step;
    if (player->moving) {
        advance_move(player, dt);
    } else if (player->cranking) {
        player_set_anim(player, ANIM_CRANK);
        sync_crank_frame(player);
    } else {
        player_set_anim(player, ANIM_IDLE);
        if (player->crate_moving) {
            update_crate_fall(player, dt);
        }
    }
    update_animation(player, dt);
}

void player_draw(const Player *player)
{
    Vector3 position;
    static const float FORWARD_X[4] = { [DIR_NORTH] = 0.0f, [DIR_EAST] = 1.0f, [DIR_SOUTH] = 0.0f, [DIR_WEST] = -1.0f };
    static const float FORWARD_Z[4] = { [DIR_NORTH] = -1.0f, [DIR_EAST] = 0.0f, [DIR_SOUTH] = 1.0f, [DIR_WEST] = 0.0f };
    position.x = player->position.x + FORWARD_X[player->dir] * player->lean;
    position.y = player->position.y - 0.5f;
    position.z = player->position.z + FORWARD_Z[player->dir] * player->lean;

    Vector3 axis;
    axis.x = 0.0f;
    axis.y = 1.0f;
    axis.z = 0.0f;

    Vector3 scale;
    scale.x = PLAYER_SCALE;
    scale.y = PLAYER_SCALE;
    scale.z = PLAYER_SCALE;

    Vector3 shadow_position;
    shadow_position.x = position.x;
    shadow_position.y = position.y + 0.01f;
    shadow_position.z = position.z;
    Color shadow_color = Fade(BLACK, 0.3f);
    DrawCylinder(shadow_position, 0.25f, 0.25f, 0.005f, 16, shadow_color);

    float angle = direction_to_model_angle(player->dir);
    DrawModelEx(player->model, position, axis, angle, scale, WHITE);
}

bool player_pushing(const Player *player)
{
    return player->moving && player->path.push && player->push_wait <= 0.0f;
}
