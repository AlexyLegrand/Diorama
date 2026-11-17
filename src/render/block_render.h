#ifndef BLOCK_RENDER_H
#define BLOCK_RENDER_H

#include "world/blocktype.h"

void block_render_load(void);

void block_render_unload(void);

void block_render_update(void);

void block_render_reward_collect(void);

bool block_render_reward_collected(void);

void block_render_reward_reset(void);

void block_render_set_crank_progress(float progress);

void draw_block(int x, int y, int z, Cell cell);

void draw_block_at(Vector3 center, Cell cell);

#endif
