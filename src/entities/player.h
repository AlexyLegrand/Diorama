#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "world/blocktype.h"
#include "world/level.h"
#include "world/movement.h"

typedef enum {
    ANIM_IDLE,
    ANIM_WALK,
    ANIM_CLIMB,
    ANIM_VICTORY,
    ANIM_PUSH_START,
    ANIM_PUSH,
    ANIM_CRANK,
    ANIM_ROCKET_LAUNCH,
    ANIM_COUNT
} PlayerAnim;

typedef struct {
    Model model;
    ModelAnimation *anims;
    int anim_count;
    int anim_lookup[ANIM_COUNT];
    PlayerAnim current;
    PlayerAnim previous;
    float frame;
    float previous_frame;
    float blend;
    int x;
    int y;
    int z;
    Direction dir;
    Vector3 position;
    MovePath path;
    int path_index;
    Vector3 segment_start;
    float segment_progress;
    bool moving;
    bool crate_moving;
    Vector3 crate_position;
    Cell crate_cell;
    bool celebrating;
    bool crank_requested;
    float push_wait;
    bool was_pushing;
    float lean;
    bool cranking;
    float crank_progress;
    float climb_sound_timer;
} Player;

Player player_load(int x, int y, int z, Direction dir);

void player_unload(Player *player);

void player_set_anim(Player *player, PlayerAnim anim);

void player_place(Player *player, int x, int y, int z);

void player_celebrate(Player *player, float camera_yaw);

bool player_is_idle(const Player *player);

bool player_pushing(const Player *player);

void player_board_rocket(Player *player, Vector3 start, Direction dir);

float player_model_angle(Direction dir);

float player_anim_duration(const Player *player, PlayerAnim anim);

void player_set_crank(Player *player, bool active, float progress);

void player_update(Player *player, Level *level, float camera_yaw, bool can_move);

bool player_moving_crate(const Player *player, GridPos *cell, Vector3 *position, Cell *crate);

void player_draw(const Player *player);

#endif
