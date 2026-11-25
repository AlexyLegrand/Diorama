#include <stdio.h>
#include "raylib.h"
#include "core/settings.h"
#include "ui/ui.h"
#include "ui/settings_menu.h"
#include "core/sfx.h"
#include "core/i18n.h"

typedef enum {
    SETTINGS_MUSIC_VOLUME,
    SETTINGS_SFX_VOLUME,
    SETTINGS_FULLSCREEN,
    SETTINGS_SHOW_FPS,
    SETTINGS_MOUSE,
    SETTINGS_LANGUAGE,
    SETTINGS_KEYS_FIRST,
    SETTINGS_RESET_KEYS = SETTINGS_KEYS_FIRST + ACTION_COUNT,
    SETTINGS_BACK,
    SETTINGS_COUNT
} SettingsItem;

#define SETTINGS_LABEL_MAX 48
static const float VOLUME_STEP = 0.1f;

static bool is_reserved_key(int key)
{
    return key == KEY_ENTER || key == KEY_SPACE || key == KEY_ESCAPE
        || key == KEY_UP || key == KEY_DOWN || key == KEY_LEFT || key == KEY_RIGHT;
}

static bool back_pressed(void)
{
    return IsKeyPressed(KEY_ESCAPE)
        || (settings_mouse_enabled() && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT));
}

static int volume_kind(int item)
{
    if (item == SETTINGS_MUSIC_VOLUME) {
        return VOLUME_MUSIC;
    }
    return item == SETTINGS_SFX_VOLUME ? VOLUME_SFX : -1;
}

static void change_volume(VolumeKind kind, bool louder)
{
    settings_change_volume(kind, louder ? VOLUME_STEP : -VOLUME_STEP);
    settings_save();
    sfx_play(SFX_UI_VOLUME);
}

static const char *yes_no(bool value)
{
    return tr(value ? TXT_YES : TXT_NO);
}

static int percent(VolumeKind kind)
{
    return (int)(settings_volume(kind) * 100.0f + 0.5f);
}

bool settings_menu_update(int *selected, int *waiting)
{
    if (*waiting != NOT_WAITING) {
        int key = GetKeyPressed();
        if (key == KEY_ESCAPE || (key == KEY_NULL && back_pressed())) {
            *waiting = NOT_WAITING;
            sfx_play(SFX_UI_BACK);
        } else if (key != KEY_NULL && !is_reserved_key(key)) {
            settings_set_key((Action)*waiting, key);
            settings_save();
            *waiting = NOT_WAITING;
            sfx_play(SFX_UI_KEY_BOUND);
        }
        return false;
    }
    *selected = ui_navigate(*selected, SETTINGS_COUNT);
    bool clicked = ui_menu_clicked(SETTINGS_COUNT);
    int volume = volume_kind(*selected);
    if (volume >= 0 && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))) {
        change_volume((VolumeKind)volume, IsKeyPressed(KEY_RIGHT));
    }
    if (clicked && volume >= 0) {
        change_volume((VolumeKind)volume, GetMousePosition().x >= GetScreenWidth() / 2.0f);
        return false;
    }
    if (back_pressed()) {
        sfx_play(SFX_UI_BACK);
        return true;
    }
    if (!ui_confirm_pressed() && !clicked) {
        return false;
    }
    sfx_play(*selected == SETTINGS_BACK ? SFX_UI_BACK : SFX_UI_CONFIRM);
    if (*selected == SETTINGS_FULLSCREEN) {
        settings_toggle_fullscreen();
    } else if (*selected == SETTINGS_SHOW_FPS) {
        settings_toggle_show_fps();
    } else if (*selected == SETTINGS_MOUSE) {
        settings_toggle_mouse();
    } else if (*selected == SETTINGS_LANGUAGE) {
        settings_toggle_language();
    } else if (*selected >= SETTINGS_KEYS_FIRST && *selected < SETTINGS_RESET_KEYS) {
        *waiting = *selected - SETTINGS_KEYS_FIRST;
    } else if (*selected == SETTINGS_RESET_KEYS) {
        settings_reset_keys();
    } else if (*selected == SETTINGS_BACK) {
        return true;
    }
    settings_save();
    return false;
}

void settings_menu_draw(int selected, int waiting)
{
    char rows[SETTINGS_COUNT][SETTINGS_LABEL_MAX];
    const char *labels[SETTINGS_COUNT];

    snprintf(rows[SETTINGS_MUSIC_VOLUME], SETTINGS_LABEL_MAX, tr(TXT_SET_MUSIC), percent(VOLUME_MUSIC));
    snprintf(rows[SETTINGS_SFX_VOLUME], SETTINGS_LABEL_MAX, tr(TXT_SET_SFX), percent(VOLUME_SFX));
    snprintf(rows[SETTINGS_FULLSCREEN], SETTINGS_LABEL_MAX, tr(TXT_SET_FULLSCREEN), yes_no(IsWindowFullscreen()));
    snprintf(rows[SETTINGS_SHOW_FPS], SETTINGS_LABEL_MAX, tr(TXT_SET_SHOW_FPS), yes_no(settings_show_fps()));
    snprintf(rows[SETTINGS_MOUSE], SETTINGS_LABEL_MAX, tr(TXT_SET_MOUSE), yes_no(settings_mouse_enabled()));
    snprintf(rows[SETTINGS_LANGUAGE], SETTINGS_LABEL_MAX, tr(TXT_SET_LANGUAGE), tr(TXT_LANGUAGE_NAME));
    for (int a = 0; a < ACTION_COUNT; a++) {
        const char *key = (a == waiting) ? "..." : settings_key_name(settings_key((Action)a));
        snprintf(rows[SETTINGS_KEYS_FIRST + a], SETTINGS_LABEL_MAX, tr(TXT_SET_KEY_ROW), settings_action_name((Action)a), key);
    }
    snprintf(rows[SETTINGS_RESET_KEYS], SETTINGS_LABEL_MAX, "%s", tr(TXT_SET_RESET_KEYS));
    snprintf(rows[SETTINGS_BACK], SETTINGS_LABEL_MAX, "%s", tr(TXT_SET_BACK));
    for (int i = 0; i < SETTINGS_COUNT; i++) {
        labels[i] = rows[i];
    }
    bool mouse = settings_mouse_enabled();
    const char *hint = (waiting != NOT_WAITING)
        ? tr(TXT_HINT_WAIT_KEY)
        : (volume_kind(selected) >= 0)
            ? tr(mouse ? TXT_HINT_VOLUME_MOUSE : TXT_HINT_VOLUME_KEYS)
            : tr(mouse ? TXT_HINT_SETTINGS_MOUSE : TXT_HINT_SETTINGS_KEYS);
    ui_draw_menu(tr(TXT_SETTINGS), labels, SETTINGS_COUNT, selected, hint);
}
