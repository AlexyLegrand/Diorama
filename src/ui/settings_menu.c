#include <stdio.h>
#include "raylib.h"
#include "core/settings.h"
#include "ui/ui.h"
#include "ui/settings_menu.h"

typedef enum {
    SETTINGS_VOLUME,
    SETTINGS_FULLSCREEN,
    SETTINGS_SHOW_FPS,
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

bool settings_menu_update(int *selected, int *waiting)
{
    if (*waiting != NOT_WAITING) {
        int key = GetKeyPressed();
        if (key == KEY_ESCAPE) {
            *waiting = NOT_WAITING;
        } else if (key != KEY_NULL && !is_reserved_key(key)) {
            settings_set_key((Action)*waiting, key);
            settings_save();
            *waiting = NOT_WAITING;
        }
        return false;
    }
    *selected = ui_navigate(*selected, SETTINGS_COUNT);
    if (*selected == SETTINGS_VOLUME && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))) {
        settings_change_volume(IsKeyPressed(KEY_RIGHT) ? VOLUME_STEP : -VOLUME_STEP);
        settings_save();
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        return true;
    }
    if (!ui_confirm_pressed()) {
        return false;
    }
    if (*selected == SETTINGS_FULLSCREEN) {
        settings_toggle_fullscreen();
    } else if (*selected == SETTINGS_SHOW_FPS) {
        settings_toggle_show_fps();
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

    snprintf(rows[SETTINGS_VOLUME], SETTINGS_LABEL_MAX, "Volume :  <  %d%%  >", (int)(settings_volume() * 100.0f + 0.5f));
    snprintf(rows[SETTINGS_FULLSCREEN], SETTINGS_LABEL_MAX, "Plein ecran : %s", IsWindowFullscreen() ? "Oui" : "Non");
    snprintf(rows[SETTINGS_SHOW_FPS], SETTINGS_LABEL_MAX, "Afficher les FPS : %s", settings_show_fps() ? "Oui" : "Non");
    for (int a = 0; a < ACTION_COUNT; a++) {
        const char *key = (a == waiting) ? "..." : settings_key_name(settings_key((Action)a));
        snprintf(rows[SETTINGS_KEYS_FIRST + a], SETTINGS_LABEL_MAX, "%s : %s", settings_action_name((Action)a), key);
    }
    snprintf(rows[SETTINGS_RESET_KEYS], SETTINGS_LABEL_MAX, "Touches par defaut");
    snprintf(rows[SETTINGS_BACK], SETTINGS_LABEL_MAX, "Retour");
    for (int i = 0; i < SETTINGS_COUNT; i++) {
        labels[i] = rows[i];
    }
    const char *hint = (waiting != NOT_WAITING)
        ? "Appuyez sur une touche (sauf fleches, Entree, Espace)   Echap : annuler"
        : (selected == SETTINGS_VOLUME) ? "Gauche/Droite : regler   Echap : retour"
                                        : "Haut/Bas : choisir   Entree : modifier   Echap : retour";
    ui_draw_menu("Parametres", labels, SETTINGS_COUNT, selected, hint);
}
