#include <ctype.h>
#include <string.h>
#include "raylib.h"
#include "core/settings.h"

const char *glfwGetKeyName(int key, int scancode);

#define SETTINGS_VERSION 4
#define SETTINGS_VERSION_ONE_VOLUME 3
#define SETTINGS_VERSION_NO_MOUSE 2

static const char *SETTINGS_PATH = "settings.dat";
static const float DEFAULT_VOLUME = 0.8f;
static const int DEFAULT_KEYS[ACTION_COUNT] = {
    [ACTION_UP] = KEY_W,
    [ACTION_DOWN] = KEY_S,
    [ACTION_LEFT] = KEY_A,
    [ACTION_RIGHT] = KEY_D,
    [ACTION_RESTART] = KEY_R,
};
static const char *ACTION_NAMES[ACTION_COUNT] = {
    [ACTION_UP] = "Avancer",
    [ACTION_DOWN] = "Reculer",
    [ACTION_LEFT] = "Gauche",
    [ACTION_RIGHT] = "Droite",
    [ACTION_RESTART] = "Recommencer",
};
static const int KEY_CODE_MAX = 512;

typedef struct {
    int version;
    float volumes[VOLUME_COUNT];
    bool fullscreen;
    bool show_fps;
    int keys[ACTION_COUNT];
    bool mouse;
} Settings;

typedef struct {
    int version;
    float volume;
    bool fullscreen;
    bool show_fps;
    int keys[ACTION_COUNT];
    bool mouse;
} SettingsOneVolume;

/* Layout of settings.dat before the mouse option (version 2), kept to migrate it. */
typedef struct {
    int version;
    float volume;
    bool fullscreen;
    bool show_fps;
    int keys[ACTION_COUNT];
} SettingsNoMouse;

static Settings _settings;

static void reset_defaults(void)
{
    _settings.version = SETTINGS_VERSION;
    for (int i = 0; i < VOLUME_COUNT; i++) {
        _settings.volumes[i] = DEFAULT_VOLUME;
    }
    _settings.fullscreen = false;
    _settings.show_fps = false;
    memcpy(_settings.keys, DEFAULT_KEYS, sizeof(_settings.keys));
    _settings.mouse = true;
}

static bool is_valid(const Settings *s)
{
    if (s->version != SETTINGS_VERSION) {
        return false;
    }
    for (int i = 0; i < VOLUME_COUNT; i++) {
        if (s->volumes[i] < 0.0f || s->volumes[i] > 1.0f) {
            return false;
        }
    }
    for (int i = 0; i < ACTION_COUNT; i++) {
        if (s->keys[i] <= KEY_NULL || s->keys[i] >= KEY_CODE_MAX) {
            return false;
        }
    }
    return true;
}

void settings_load(void)
{
    int size = 0;
    unsigned char *data = FileExists(SETTINGS_PATH) ? LoadFileData(SETTINGS_PATH, &size) : NULL;

    reset_defaults();
    bool no_mouse = data != NULL && size == (int)sizeof(SettingsNoMouse)
        && ((const SettingsNoMouse *)data)->version == SETTINGS_VERSION_NO_MOUSE;
    bool one_volume = data != NULL && size == (int)sizeof(SettingsOneVolume)
        && ((const SettingsOneVolume *)data)->version == SETTINGS_VERSION_ONE_VOLUME;
    if (no_mouse || one_volume) {
        const SettingsNoMouse *old = (const SettingsNoMouse *)data;
        Settings migrated = _settings;
        for (int i = 0; i < VOLUME_COUNT; i++) {
            migrated.volumes[i] = old->volume;
        }
        migrated.fullscreen = old->fullscreen;
        migrated.show_fps = old->show_fps;
        memcpy(migrated.keys, old->keys, sizeof(migrated.keys));
        if (one_volume) {
            migrated.mouse = ((const SettingsOneVolume *)data)->mouse;
        }
        if (is_valid(&migrated)) {
            _settings = migrated;
        }
    } else if (data != NULL && size == (int)sizeof(Settings) && is_valid((const Settings *)data)) {
        memcpy(&_settings, data, sizeof(Settings));
    } else if (data != NULL) {
        TraceLog(LOG_WARNING, "SETTINGS: %s is invalid, using defaults", SETTINGS_PATH);
    }
    UnloadFileData(data);
    if (_settings.fullscreen != IsWindowFullscreen()) {
        ToggleFullscreen();
    }
}

void settings_save(void)
{
    if (!SaveFileData(SETTINGS_PATH, &_settings, sizeof(Settings))) {
        TraceLog(LOG_WARNING, "SETTINGS: could not save %s", SETTINGS_PATH);
    }
}

int settings_key(Action action)
{
    return _settings.keys[action];
}

void settings_set_key(Action action, int key)
{
    for (int i = 0; i < ACTION_COUNT; i++) {
        if (i != (int)action && _settings.keys[i] == key) {
            _settings.keys[i] = _settings.keys[action];
        }
    }
    _settings.keys[action] = key;
}

void settings_reset_keys(void)
{
    memcpy(_settings.keys, DEFAULT_KEYS, sizeof(_settings.keys));
}

float settings_volume(VolumeKind kind)
{
    return _settings.volumes[kind];
}

void settings_change_volume(VolumeKind kind, float delta)
{
    float volume = _settings.volumes[kind] + delta;
    _settings.volumes[kind] = volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume);
}

void settings_toggle_fullscreen(void)
{
    ToggleFullscreen();
    _settings.fullscreen = IsWindowFullscreen();
}

bool settings_show_fps(void)
{
    return _settings.show_fps;
}

void settings_toggle_show_fps(void)
{
    _settings.show_fps = !_settings.show_fps;
}

bool settings_mouse_enabled(void)
{
    return _settings.mouse;
}

void settings_toggle_mouse(void)
{
    _settings.mouse = !_settings.mouse;
}

const char *settings_action_name(Action action)
{
    return ACTION_NAMES[action];
}

const char *settings_key_name(int key)
{
    const char *printable = glfwGetKeyName(key, 0);

    if (printable != NULL && printable[0] != '\0') {
        unsigned char first = (unsigned char)printable[0];
        return (first < 128) ? TextFormat("%c", toupper(first)) : printable;
    }
    switch (key) {
    case KEY_SPACE: return "Espace";
    case KEY_ENTER: return "Entrée";
    case KEY_TAB: return "Tab";
    case KEY_BACKSPACE: return "Retour arr.";
    case KEY_UP: return "Haut";
    case KEY_DOWN: return "Bas";
    case KEY_LEFT: return "Flèche G";
    case KEY_RIGHT: return "Flèche D";
    case KEY_LEFT_SHIFT: return "Maj G";
    case KEY_RIGHT_SHIFT: return "Maj D";
    case KEY_LEFT_CONTROL: return "Ctrl G";
    case KEY_RIGHT_CONTROL: return "Ctrl D";
    }
    return TextFormat("Touche %d", key);
}
