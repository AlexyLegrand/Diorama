#include "raylib.h"
#include "core/settings.h"
#include "game/game.h"

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;
static const int TARGET_FPS = 60;
static const int WINDOW_MIN_WIDTH = 800;
static const int WINDOW_MIN_HEIGHT = 450;
static const char *ICON_PATH = "assets/logo/icon.png";

static void set_window_icon(void)
{
    Image icon = LoadImage(ICON_PATH);

    if (IsImageValid(icon)) {
        SetWindowIcon(icon);
        UnloadImage(icon);
    }
}

int main(void)
{
    static Game game;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Diorama");
    SetWindowMinSize(WINDOW_MIN_WIDTH, WINDOW_MIN_HEIGHT);
    set_window_icon();
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
