#include <stdio.h>
#include "raylib.h"
#include "core/sfx.h"
#include "core/settings.h"

#define STEP_VARIANTS 4
#define SFX_PATH_MAX 64

static const char *SFX_PATHS[SFX_COUNT] = {
    [SFX_STEP]          = "assets/music/player/step_%d.mp3",
    [SFX_CLIMB]         = "assets/music/player/climb.mp3",
    [SFX_PUSH_START]    = "assets/music/player/push_start.mp3",
    [SFX_CRANK]         = "assets/music/player/crank.mp3",
    [SFX_VICTORY]       = "assets/music/player/victory.mp3",
    [SFX_BUMP]          = "assets/music/player/bump.mp3",
    [SFX_CRATE_LAND]    = "assets/music/world/crate_land.mp3",
    [SFX_CRANK_TURN]    = "assets/music/world/crank_turn.mp3",
    [SFX_PLATFORM_STOP] = "assets/music/world/platform_stop.mp3",
    [SFX_SEED_COLLECT]  = "assets/music/world/seed_collect.mp3",
    [SFX_UI_MOVE]       = "assets/music/ui/move.mp3",
    [SFX_UI_CONFIRM]    = "assets/music/ui/confirm.mp3",
    [SFX_UI_BACK]       = "assets/music/ui/back.mp3",
    [SFX_UI_VOLUME]     = "assets/music/ui/volume.mp3",
    [SFX_UI_KEY_BOUND]  = "assets/music/ui/key_bound.mp3",
    [SFX_UI_ENTRANCE]   = "assets/music/ui/entrance.mp3",
    [SFX_UI_LOCKED]     = "assets/music/ui/locked.mp3",
    [SFX_UI_TRANSITION] = "assets/music/ui/transition.mp3",
    [SFX_UI_RESTART]    = "assets/music/ui/restart.mp3",
    [SFX_UI_UNLOCK]     = "assets/music/ui/unlock.mp3",
};

static const char *LOOP_PATHS[SFX_LOOP_COUNT] = {
    [SFX_LOOP_CRATE]    = "assets/music/world/crate_slide.mp3",
    [SFX_LOOP_PLATFORM] = "assets/music/world/platform_move.mp3",
};
static const float LOOP_FADE_TIME = 0.15f;

static Sound sounds[SFX_COUNT];
static Music loops[SFX_LOOP_COUNT];
static float loop_levels[SFX_LOOP_COUNT];
static Sound steps[STEP_VARIANTS];
static int last_step = -1;

static Sound load_optional(const char *path)
{
    return FileExists(path) ? LoadSound(path) : (Sound){0};
}

void sfx_load(void)
{
    char path[SFX_PATH_MAX];

    for (int i = 0; i < STEP_VARIANTS; i++) {
        snprintf(path, sizeof(path), SFX_PATHS[SFX_STEP], i + 1);
        steps[i] = load_optional(path);
    }
    for (int i = 0; i < SFX_COUNT; i++) {
        if (i != SFX_STEP) {
            sounds[i] = load_optional(SFX_PATHS[i]);
        }
    }
    for (int i = 0; i < SFX_LOOP_COUNT; i++) {
        loops[i] = FileExists(LOOP_PATHS[i]) ? LoadMusicStream(LOOP_PATHS[i]) : (Music){0};
        loop_levels[i] = 0.0f;
    }
}

void sfx_unload(void)
{
    for (int i = 0; i < STEP_VARIANTS; i++) {
        if (IsSoundValid(steps[i])) {
            UnloadSound(steps[i]);
        }
    }
    for (int i = 0; i < SFX_COUNT; i++) {
        if (IsSoundValid(sounds[i])) {
            UnloadSound(sounds[i]);
        }
    }
    for (int i = 0; i < SFX_LOOP_COUNT; i++) {
        if (IsMusicValid(loops[i])) {
            UnloadMusicStream(loops[i]);
        }
    }
}

static Sound *pick(Sfx sfx)
{
    if (sfx != SFX_STEP) {
        return &sounds[sfx];
    }
    int step = GetRandomValue(0, STEP_VARIANTS - 2);
    if (step >= last_step) {
        step++;
    }
    last_step = step;
    return &steps[step];
}

void sfx_play(Sfx sfx)
{
    Sound *sound = pick(sfx);

    if (IsSoundValid(*sound)) {
        SetSoundVolume(*sound, settings_volume(VOLUME_SFX));
        PlaySound(*sound);
    }
}

void sfx_loop(SfxLoop loop, bool active)
{
    Music music = loops[loop];
    float step = GetFrameTime() / LOOP_FADE_TIME;
    float level = loop_levels[loop] + (active ? step : -step);

    if (!IsMusicValid(music)) {
        return;
    }
    level = level < 0.0f ? 0.0f : (level > 1.0f ? 1.0f : level);
    loop_levels[loop] = level;
    if (level > 0.0f && !IsMusicStreamPlaying(music)) {
        PlayMusicStream(music);
    } else if (level <= 0.0f && IsMusicStreamPlaying(music)) {
        StopMusicStream(music);
    }
    SetMusicVolume(music, level * settings_volume(VOLUME_SFX));
    UpdateMusicStream(music);
}
