#include "raylib.h"
#include "core/settings.h"
#include "game/game.h"

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;
static const int TARGET_FPS = 60;

int main(void)
{
    static Game game;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Diorama");
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL);
    InitAudioDevice();
    settings_load();
    if (!game_init(&game)) {
        CloseAudioDevice();
        CloseWindow();
        return 1;
    }
    while (!WindowShouldClose() && !game.quit) {
        game_update(&game);
        game_draw(&game);
    }
    game_unload(&game);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
