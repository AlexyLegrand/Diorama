#include <math.h>
#include <stdio.h>
#include "raylib.h"
#include "ui/ui.h"
#include "core/settings.h"

static const int TITLE_SIZE = 80;
static const int TITLE_Y = 80;
static const int ITEM_SIZE = 36;
static const int ITEM_SPACING = 60;
static const int COMPACT_THRESHOLD = 5;
static const int COMPACT_ITEM_SIZE = 26;
static const int COMPACT_ITEM_SPACING = 44;
static const int COMPACT_PANEL_WIDTH = 460;
static const int PANEL_WIDTH = 380;
static const int PANEL_PADDING = 24;
static const float PULSE_SPEED = 6.0f;
static const float PULSE_AMOUNT = 3.0f;
static const float PANEL_ROUNDNESS = 0.25f;
static const int PANEL_SEGMENTS = 8;
static const int SHADOW_OFFSET = 3;
static const int HINT_SIZE = 20;
#define KEY_NAME_MAX 16
#define HINT_MAX 128

static const int HUD_MARGIN = 20;
static const int HUD_PADDING = 12;
static const int HUD_TITLE_SIZE = 30;
static const int WIN_PANEL_Y = 100;
static const int WIN_PANEL_HEIGHT = 150;
static const int WIN_TITLE_SIZE = 60;
static const int WIN_HINT_SIZE = 25;

static const float PAD_RADIUS = 0.42f;
static const float PAD_HEIGHT = 0.06f;
static const float PAD_FLOOR_OFFSET = 0.5f;
static const int PAD_SLICES = 20;
static const float LABEL_HEIGHT = 1.4f;
static const int LABEL_SIZE = 30;
static const int PROMPT_SIZE = 28;
static const int PROMPT_BOTTOM = 90;

static const Color PANEL_COLOR = { 20, 24, 40, 190 };
static const Color FOCUS_COLOR = { 255, 203, 0, 230 };
static const Color FOCUS_TEXT = { 30, 24, 10, 255 };
static const Color SHADOW_COLOR = { 0, 0, 0, 160 };
static const Color HINT_COLOR = { 200, 200, 210, 255 };
static const Color PAD_OPEN = { 245, 245, 255, 255 };
static const Color PAD_DONE = { 255, 203, 0, 255 };
static const Color PAD_LOCKED = { 90, 90, 100, 255 };
static const Color LABEL_LOCKED = { 150, 150, 160, 255 };

static void draw_text_shadowed(const char *text, int x, int y, int size, Color color)
{
    DrawText(text, x + SHADOW_OFFSET, y + SHADOW_OFFSET, size, SHADOW_COLOR);
    DrawText(text, x, y, size, color);
}

static void draw_centered_at(const char *text, int center_x, int y, int size, Color color)
{
    draw_text_shadowed(text, center_x - MeasureText(text, size) / 2, y, size, color);
}

static void draw_centered(const char *text, int y, int size, Color color)
{
    draw_centered_at(text, GetScreenWidth() / 2, y, size, color);
}

static void draw_panel(int x, int y, int width, int height)
{
    DrawRectangleRounded((Rectangle){ x, y, width, height }, PANEL_ROUNDNESS, PANEL_SEGMENTS, PANEL_COLOR);
}

typedef struct {
    int item_size;
    int spacing;
    int panel_x;
    int panel_y;
    int panel_w;
    int panel_h;
    int top;
} MenuLayout;

static MenuLayout menu_layout(int count)
{
    bool compact = count > COMPACT_THRESHOLD;
    MenuLayout m;

    m.item_size = compact ? COMPACT_ITEM_SIZE : ITEM_SIZE;
    m.spacing = compact ? COMPACT_ITEM_SPACING : ITEM_SPACING;
    m.panel_w = compact ? COMPACT_PANEL_WIDTH : PANEL_WIDTH;
    m.panel_h = count * m.spacing + PANEL_PADDING;
    m.panel_x = GetScreenWidth() / 2 - m.panel_w / 2;
    m.panel_y = GetScreenHeight() / 2 - m.panel_h / 2 + m.spacing;
    m.top = m.panel_y + PANEL_PADDING;
    return m;
}

int ui_menu_hovered(int count)
{
    if (!settings_mouse_enabled()) {
        return -1;
    }
    MenuLayout m = menu_layout(count);
    Vector2 mouse = GetMousePosition();

    for (int i = 0; i < count; i++) {
        Rectangle row = { m.panel_x, m.top + i * m.spacing - (m.spacing - m.item_size) / 2,
                          m.panel_w, m.spacing };
        if (CheckCollisionPointRec(mouse, row)) {
            return i;
        }
    }
    return -1;
}

bool ui_menu_clicked(int count)
{
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ui_menu_hovered(count) >= 0;
}

int ui_navigate(int selected, int count)
{
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        return (selected + 1) % count;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        return (selected + count - 1) % count;
    }
    Vector2 delta = GetMouseDelta();
    bool mouse_active = delta.x != 0.0f || delta.y != 0.0f || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    int hovered = mouse_active ? ui_menu_hovered(count) : -1;
    return hovered >= 0 ? hovered : selected;
}

bool ui_confirm_pressed(void)
{
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
}

void ui_draw_menu(const char *title, const char *const labels[], int count, int selected, const char *hint)
{
    MenuLayout m = menu_layout(count);
    int item_size = m.item_size;
    int spacing = m.spacing;
    int panel_w = m.panel_w;
    int panel_x = m.panel_x;
    int top = m.top;
    int screen_w = GetScreenWidth();
    float pulse = sinf((float)GetTime() * PULSE_SPEED) * PULSE_AMOUNT;

    draw_centered(title, TITLE_Y, TITLE_SIZE, GOLD);
    draw_panel(panel_x, m.panel_y, panel_w, m.panel_h);
    for (int i = 0; i < count; i++) {
        int item_y = top + i * spacing;
        if (i != selected) {
            draw_centered(labels[i], item_y, item_size, WHITE);
            continue;
        }
        Rectangle bar = { panel_x + HUD_PADDING - pulse, item_y - HUD_PADDING / 2,
                          panel_w - 2 * HUD_PADDING + 2 * pulse, item_size + HUD_PADDING };
        DrawRectangleRounded(bar, PANEL_ROUNDNESS * 2, PANEL_SEGMENTS, FOCUS_COLOR);
        DrawText(labels[i], screen_w / 2 - MeasureText(labels[i], item_size) / 2, item_y, item_size, FOCUS_TEXT);
    }
    draw_centered(hint, GetScreenHeight() - HUD_MARGIN - HINT_SIZE, HINT_SIZE, HINT_COLOR);
}

static Vector3 cell_position(GridPos cell)
{
    return (Vector3){ (float)cell.x, (float)cell.y, (float)cell.z };
}

bool ui_level_unlocked(const bool completed[], int index)
{
    return index == 0 || completed[index - 1];
}

static Color pad_color(const bool completed[], int index)
{
    if (completed[index]) {
        return PAD_DONE;
    }
    return ui_level_unlocked(completed, index) ? PAD_OPEN : PAD_LOCKED;
}

void ui_draw_entrances(const Level *map, const bool completed[])
{
    for (int i = 0; i < map->entrance_count; i++) {
        Vector3 pad = cell_position(map->entrances[i]);
        pad.y -= PAD_FLOOR_OFFSET;
        DrawCylinder(pad, PAD_RADIUS, PAD_RADIUS, PAD_HEIGHT, PAD_SLICES, pad_color(completed, i));
    }
}

void ui_draw_map_hud(Camera3D camera, const Level *map, const bool completed[], int entrance_here)
{
    for (int i = 0; i < map->entrance_count; i++) {
        Vector3 above = cell_position(map->entrances[i]);
        above.y += LABEL_HEIGHT;
        Vector2 screen = GetWorldToScreen(above, camera);
        bool unlocked = ui_level_unlocked(completed, i);
        const char *label = completed[i] ? TextFormat("%d *", i + 1)
                          : unlocked     ? TextFormat("%d", i + 1)
                                         : TextFormat("%d (ferme)", i + 1);
        Color color = completed[i] ? GOLD : (unlocked ? WHITE : LABEL_LOCKED);
        draw_centered_at(label, (int)screen.x, (int)screen.y, LABEL_SIZE, color);
    }
    char names[4][KEY_NAME_MAX];
    const Action walk_actions[4] = { ACTION_UP, ACTION_LEFT, ACTION_DOWN, ACTION_RIGHT };
    for (int i = 0; i < 4; i++) {
        snprintf(names[i], KEY_NAME_MAX, "%s", settings_key_name(settings_key(walk_actions[i])));
    }
    char walk[HINT_MAX];
    snprintf(walk, HINT_MAX, "%s%s%s%s : marcher   Fleches : camera   Echap : menu",
             names[0], names[1], names[2], names[3]);
    draw_text_shadowed(walk,
                       HUD_MARGIN, GetScreenHeight() - HUD_MARGIN - HINT_SIZE, HINT_SIZE, HINT_COLOR);
    if (entrance_here < 0) {
        return;
    }
    const char *prompt = ui_level_unlocked(completed, entrance_here)
        ? TextFormat("Niveau %d  -  Entree : jouer", entrance_here + 1)
        : TextFormat("Niveau %d verrouille  -  terminez le niveau %d", entrance_here + 1, entrance_here);
    int prompt_w = MeasureText(prompt, PROMPT_SIZE) + 2 * PANEL_PADDING;
    int prompt_y = GetScreenHeight() - PROMPT_BOTTOM - PROMPT_SIZE;
    draw_panel(GetScreenWidth() / 2 - prompt_w / 2, prompt_y - HUD_PADDING, prompt_w, PROMPT_SIZE + 2 * HUD_PADDING);
    draw_centered(prompt, prompt_y, PROMPT_SIZE, WHITE);
}

void hud_draw(bool won, int level_index)
{
    const char *title = TextFormat("Niveau %d", level_index + 1);
    const char *hint = TextFormat("%s : recommencer   Echap : carte", settings_key_name(settings_key(ACTION_RESTART)));
    int title_w = MeasureText(title, HUD_TITLE_SIZE);

    draw_panel(HUD_MARGIN, HUD_MARGIN, title_w + 2 * HUD_PADDING, HUD_TITLE_SIZE + 2 * HUD_PADDING);
    DrawText(title, HUD_MARGIN + HUD_PADDING, HUD_MARGIN + HUD_PADDING, HUD_TITLE_SIZE, WHITE);
    draw_text_shadowed(hint, HUD_MARGIN, GetScreenHeight() - HUD_MARGIN - HINT_SIZE, HINT_SIZE, HINT_COLOR);
    if (!won) {
        return;
    }
    draw_panel(GetScreenWidth() / 2 - PANEL_WIDTH / 2, WIN_PANEL_Y, PANEL_WIDTH, WIN_PANEL_HEIGHT);
    draw_centered("Bravo !", WIN_PANEL_Y + PANEL_PADDING, WIN_TITLE_SIZE, GOLD);
    draw_centered("Entree : retour a la carte", WIN_PANEL_Y + WIN_PANEL_HEIGHT - PANEL_PADDING - WIN_HINT_SIZE,
                  WIN_HINT_SIZE, WHITE);
}
