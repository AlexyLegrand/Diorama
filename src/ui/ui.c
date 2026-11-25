#include <math.h>
#include <stdio.h>
#include "raylib.h"
#include "ui/ui.h"
#include "core/settings.h"
#include "core/sfx.h"
#include "core/i18n.h"

static const int UI_REFERENCE_WIDTH = 1280;
static const int UI_REFERENCE_HEIGHT = 720;
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

static const float PAD_RADIUS = 0.42f;
static const float PAD_HEIGHT = 0.06f;
static const float PAD_FLOOR_OFFSET = 0.5f;
static const int PAD_SLICES = 20;
static const float LABEL_HEIGHT = 1.4f;
static const int LABEL_SIZE = 30;
static const int PROMPT_SIZE = 28;
static const int PROMPT_BOTTOM = 90;
static const int BOTTOM_MENU_MARGIN = 60;
static const int MENU_GAP = 20;
static const int BOTTOM_ITEM_SIZE = 30;
static const int BOTTOM_CELL_WIDTH = 260;

static const Color PANEL_COLOR = { 20, 24, 40, 190 };
static const Color FOCUS_COLOR = { 255, 203, 0, 230 };
static const Color FOCUS_TEXT = { 30, 24, 10, 255 };
static const Color SHADOW_COLOR = { 0, 0, 0, 160 };
static const Color HINT_COLOR = { 200, 200, 210, 255 };
static const Color PAD_OPEN = { 245, 245, 255, 255 };
static const Color PAD_DONE = { 255, 203, 0, 255 };
static const Color PAD_LOCKED = { 90, 90, 100, 255 };
static const Color LABEL_LOCKED = { 150, 150, 160, 255 };
static const Color PORTAL_OPEN = { 170, 110, 255, 255 };

static const char *FONT_PATH = "assets/fonts/LilitaOne-Regular.ttf";
static const int FONT_BASE_SIZE = 96;
static const float FONT_SPACING_RATIO = 0.04f;
static const int FONT_EXTRA_CODEPOINTS[] = { 0x152, 0x153, 0x2019, 0x2026 };
#define FONT_ASCII_FIRST 32
#define FONT_ASCII_LAST 126
#define FONT_LATIN1_FIRST 160
#define FONT_LATIN1_LAST 255

static Font font;

float ui_scale(void)
{
    float by_height = GetScreenHeight() / (float)UI_REFERENCE_HEIGHT;
    float by_width = GetScreenWidth() / (float)UI_REFERENCE_WIDTH;

    return by_height < by_width ? by_height : by_width;
}

int ui_px(int pixels)
{
    return (int)(pixels * ui_scale() + 0.5f);
}
static bool font_loaded;

void ui_load_font(void)
{
    int codepoints[(FONT_ASCII_LAST - FONT_ASCII_FIRST + 1) + (FONT_LATIN1_LAST - FONT_LATIN1_FIRST + 1)
                   + sizeof(FONT_EXTRA_CODEPOINTS) / sizeof(FONT_EXTRA_CODEPOINTS[0])];
    int count = 0;

    for (int c = FONT_ASCII_FIRST; c <= FONT_ASCII_LAST; c++) {
        codepoints[count++] = c;
    }
    for (int c = FONT_LATIN1_FIRST; c <= FONT_LATIN1_LAST; c++) {
        codepoints[count++] = c;
    }
    for (size_t i = 0; i < sizeof(FONT_EXTRA_CODEPOINTS) / sizeof(FONT_EXTRA_CODEPOINTS[0]); i++) {
        codepoints[count++] = FONT_EXTRA_CODEPOINTS[i];
    }
    if (!FileExists(FONT_PATH)) {
        TraceLog(LOG_WARNING, "UI: %s not found, using the default font", FONT_PATH);
        return;
    }
    font = LoadFontEx(FONT_PATH, FONT_BASE_SIZE, codepoints, count);
    font_loaded = IsFontValid(font);
    if (font_loaded) {
        SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    }
}

void ui_unload_font(void)
{
    if (font_loaded) {
        UnloadFont(font);
        font_loaded = false;
    }
}

static Font current_font(void)
{
    return font_loaded ? font : GetFontDefault();
}

static float spacing_for(int size)
{
    return font_loaded ? size * FONT_SPACING_RATIO : size / 10.0f;
}

void ui_text(const char *text, int x, int y, int size, Color color)
{
    DrawTextEx(current_font(), text, (Vector2){ (float)x, (float)y }, (float)size, spacing_for(size), color);
}

int ui_measure(const char *text, int size)
{
    return (int)MeasureTextEx(current_font(), text, (float)size, spacing_for(size)).x;
}

static void draw_text_shadowed(const char *text, int x, int y, int size, Color color)
{
    ui_text(text, x + ui_px(SHADOW_OFFSET), y + ui_px(SHADOW_OFFSET), size, SHADOW_COLOR);
    ui_text(text, x, y, size, color);
}

static void draw_centered_at(const char *text, int center_x, int y, int size, Color color)
{
    draw_text_shadowed(text, center_x - ui_measure(text, size) / 2, y, size, color);
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
    bool horizontal;
    int item_size;
    int spacing;
    int cell_w;
    int panel_x;
    int panel_y;
    int panel_w;
    int panel_h;
    int top;
} MenuLayout;

static MenuAnchor menu_anchor = MENU_ANCHOR_CENTER;

void ui_set_menu_anchor(MenuAnchor anchor)
{
    menu_anchor = anchor;
}

bool ui_menu_is_row(void)
{
    return menu_anchor == MENU_ANCHOR_BOTTOM;
}

static MenuLayout menu_layout(int count)
{
    bool compact = count > COMPACT_THRESHOLD;
    MenuLayout m;

    m.item_size = compact ? ui_px(COMPACT_ITEM_SIZE) : ui_px(ITEM_SIZE);
    m.spacing = compact ? ui_px(COMPACT_ITEM_SPACING) : ui_px(ITEM_SPACING);
    m.panel_w = compact ? ui_px(COMPACT_PANEL_WIDTH) : ui_px(PANEL_WIDTH);
    m.panel_h = count * m.spacing + ui_px(PANEL_PADDING);
    m.panel_x = GetScreenWidth() / 2 - m.panel_w / 2;
    int region_top = ui_px(TITLE_Y) + ui_px(TITLE_SIZE) + ui_px(MENU_GAP);
    int region_bottom = GetScreenHeight() - ui_px(HUD_MARGIN) - ui_px(HINT_SIZE) - ui_px(MENU_GAP);
    int region_h = region_bottom - region_top;
    if (m.panel_h > region_h && region_h > 0) {
        float shrink = (float)region_h / (float)m.panel_h;
        m.spacing = (int)(m.spacing * shrink);
        m.item_size = (int)(m.item_size * shrink);
        m.panel_h = count * m.spacing + ui_px(PANEL_PADDING);
    }
    m.panel_y = region_top + (region_h - m.panel_h) / 2;
    m.top = m.panel_y + ui_px(PANEL_PADDING);
    m.horizontal = menu_anchor == MENU_ANCHOR_BOTTOM;
    m.cell_w = 0;
    if (m.horizontal) {
        m.item_size = ui_px(BOTTOM_ITEM_SIZE);
        m.cell_w = ui_px(BOTTOM_CELL_WIDTH);
        m.panel_w = count * m.cell_w + 2 * ui_px(HUD_PADDING);
        m.panel_h = m.item_size + 2 * ui_px(PANEL_PADDING);
        m.panel_x = GetScreenWidth() / 2 - m.panel_w / 2;
        m.panel_y = GetScreenHeight() - m.panel_h - ui_px(BOTTOM_MENU_MARGIN);
        m.top = m.panel_y + ui_px(PANEL_PADDING);
    }
    return m;
}

static Rectangle item_rect(MenuLayout m, int index)
{
    float pad = (float)ui_px(HUD_PADDING);

    if (m.horizontal) {
        return (Rectangle){ m.panel_x + pad + index * m.cell_w, m.top - pad / 2, (float)m.cell_w, m.item_size + pad };
    }
    return (Rectangle){ m.panel_x + pad, m.top + index * m.spacing - pad / 2, m.panel_w - 2 * pad, m.item_size + pad };
}

int ui_menu_hovered(int count)
{
    if (!settings_mouse_enabled()) {
        return -1;
    }
    MenuLayout m = menu_layout(count);
    Vector2 mouse = GetMousePosition();

    for (int i = 0; i < count; i++) {
        if (CheckCollisionPointRec(mouse, item_rect(m, i))) {
            return i;
        }
    }
    return -1;
}

bool ui_menu_clicked(int count)
{
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ui_menu_hovered(count) >= 0;
}

static int navigate(int selected, int count)
{
    bool horizontal = menu_anchor == MENU_ANCHOR_BOTTOM;
    bool next = IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)
                || (horizontal && (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)));
    bool previous = IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)
                    || (horizontal && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)));

    if (next) {
        return (selected + 1) % count;
    }
    if (previous) {
        return (selected + count - 1) % count;
    }
    Vector2 delta = GetMouseDelta();
    bool mouse_active = delta.x != 0.0f || delta.y != 0.0f || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    int hovered = mouse_active ? ui_menu_hovered(count) : -1;
    return hovered >= 0 ? hovered : selected;
}

int ui_navigate(int selected, int count)
{
    int next = navigate(selected, count);

    if (next != selected) {
        sfx_play(SFX_UI_MOVE);
    }
    return next;
}

bool ui_confirm_pressed(void)
{
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
}

void ui_draw_menu(const char *title, const char *const labels[], int count, int selected, const char *hint)
{
    MenuLayout m = menu_layout(count);
    float pulse = sinf((float)GetTime() * PULSE_SPEED) * (PULSE_AMOUNT * ui_scale());

    if (title != NULL) {
        draw_centered(title, ui_px(TITLE_Y), ui_px(TITLE_SIZE), GOLD);
    }
    draw_panel(m.panel_x, m.panel_y, m.panel_w, m.panel_h);
    for (int i = 0; i < count; i++) {
        Rectangle cell = item_rect(m, i);
        int center_x = (int)(cell.x + cell.width / 2);
        int item_y = (int)(cell.y + (cell.height - m.item_size) / 2);
        if (i != selected) {
            draw_centered_at(labels[i], center_x, item_y, m.item_size, WHITE);
            continue;
        }
        Rectangle bar = { cell.x - pulse, cell.y, cell.width + 2 * pulse, cell.height };
        DrawRectangleRounded(bar, PANEL_ROUNDNESS * 2, PANEL_SEGMENTS, FOCUS_COLOR);
        ui_text(labels[i], center_x - ui_measure(labels[i], m.item_size) / 2, item_y, m.item_size, FOCUS_TEXT);
    }
    draw_centered(hint, GetScreenHeight() - ui_px(HUD_MARGIN) - ui_px(HINT_SIZE), ui_px(HINT_SIZE), HINT_COLOR);
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

void ui_draw_portal(GridPos cell, bool open)
{
    Vector3 pad = cell_position(cell);
    pad.y -= PAD_FLOOR_OFFSET;
    DrawCylinder(pad, PAD_RADIUS, PAD_RADIUS, PAD_HEIGHT, PAD_SLICES, open ? PORTAL_OPEN : PAD_LOCKED);
}

static void draw_prompt(const char *prompt)
{
    int prompt_w = ui_measure(prompt, ui_px(PROMPT_SIZE)) + 2 * ui_px(PANEL_PADDING);
    int prompt_y = GetScreenHeight() - ui_px(PROMPT_BOTTOM) - ui_px(PROMPT_SIZE);
    draw_panel(GetScreenWidth() / 2 - prompt_w / 2, prompt_y - ui_px(HUD_PADDING), prompt_w, ui_px(PROMPT_SIZE) + 2 * ui_px(HUD_PADDING));
    draw_centered(prompt, prompt_y, ui_px(PROMPT_SIZE), WHITE);
}

static void draw_title_panel(const char *title)
{
    int title_w = ui_measure(title, ui_px(HUD_TITLE_SIZE));

    draw_panel(ui_px(HUD_MARGIN), ui_px(HUD_MARGIN), title_w + 2 * ui_px(HUD_PADDING), ui_px(HUD_TITLE_SIZE) + 2 * ui_px(HUD_PADDING));
    ui_text(title, ui_px(HUD_MARGIN) + ui_px(HUD_PADDING), ui_px(HUD_MARGIN) + ui_px(HUD_PADDING), ui_px(HUD_TITLE_SIZE), WHITE);
}

void ui_draw_map_hud(Camera3D camera, const Level *map, const bool completed[], int entrance_here,
                     const char *world_name, const char *portal_prompt)
{
    draw_title_panel(world_name);
    for (int i = 0; i < map->entrance_count; i++) {
        Vector3 above = cell_position(map->entrances[i]);
        above.y += LABEL_HEIGHT;
        Vector2 screen = GetWorldToScreen(above, camera);
        bool unlocked = ui_level_unlocked(completed, i);
        const char *label = completed[i] ? TextFormat("%d *", i + 1)
                          : unlocked     ? TextFormat("%d", i + 1)
                                         : TextFormat(tr(TXT_MAP_LOCKED_LABEL), i + 1);
        Color color = completed[i] ? GOLD : (unlocked ? WHITE : LABEL_LOCKED);
        draw_centered_at(label, (int)screen.x, (int)screen.y, ui_px(LABEL_SIZE), color);
    }
    char names[4][KEY_NAME_MAX];
    const Action walk_actions[4] = { ACTION_UP, ACTION_LEFT, ACTION_DOWN, ACTION_RIGHT };
    for (int i = 0; i < 4; i++) {
        snprintf(names[i], KEY_NAME_MAX, "%s", settings_key_name(settings_key(walk_actions[i])));
    }
    char walk[HINT_MAX];
    char keys[HINT_MAX];
    snprintf(keys, HINT_MAX, "%s%s%s%s", names[0], names[1], names[2], names[3]);
    snprintf(walk, HINT_MAX, tr(TXT_MAP_WALK_HINT), keys);
    draw_text_shadowed(walk,
                       ui_px(HUD_MARGIN), GetScreenHeight() - ui_px(HUD_MARGIN) - ui_px(HINT_SIZE), ui_px(HINT_SIZE), HINT_COLOR);
    if (entrance_here >= 0) {
        draw_prompt(ui_level_unlocked(completed, entrance_here)
            ? TextFormat(tr(TXT_MAP_LEVEL_PROMPT), entrance_here + 1)
            : TextFormat(tr(TXT_MAP_LEVEL_LOCKED), entrance_here + 1, entrance_here));
    } else if (portal_prompt != NULL) {
        draw_prompt(portal_prompt);
    }
}

void hud_draw(const char *world_name, int level_index)
{
    const char *hint = TextFormat(tr(TXT_HUD_HINT), settings_key_name(settings_key(ACTION_RESTART)));

    draw_title_panel(TextFormat(tr(TXT_HUD_TITLE), world_name, level_index + 1));
    draw_text_shadowed(hint, ui_px(HUD_MARGIN), GetScreenHeight() - ui_px(HUD_MARGIN) - ui_px(HINT_SIZE), ui_px(HINT_SIZE), HINT_COLOR);
}
