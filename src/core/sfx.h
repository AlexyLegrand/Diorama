#ifndef SFX_H
#define SFX_H

#include <stdbool.h>

typedef enum {
    SFX_STEP,
    SFX_CLIMB,
    SFX_PUSH_START,
    SFX_CRANK,
    SFX_VICTORY,
    SFX_BUMP,
    SFX_CRATE_LAND,
    SFX_CRANK_TURN,
    SFX_PLATFORM_STOP,
    SFX_SEED_COLLECT,
    SFX_UI_MOVE,
    SFX_UI_CONFIRM,
    SFX_UI_BACK,
    SFX_UI_VOLUME,
    SFX_UI_KEY_BOUND,
    SFX_UI_ENTRANCE,
    SFX_UI_LOCKED,
    SFX_UI_TRANSITION,
    SFX_UI_RESTART,
    SFX_UI_UNLOCK,
    SFX_COUNT
} Sfx;

typedef enum {
    SFX_LOOP_CRATE,
    SFX_LOOP_PLATFORM,
    SFX_LOOP_COUNT
} SfxLoop;

void sfx_load(void);

void sfx_unload(void);

void sfx_play(Sfx sfx);

void sfx_loop(SfxLoop loop, bool active);

#endif
