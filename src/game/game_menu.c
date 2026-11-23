#include "raylib.h"
#include "game/game_internal.h"
#include "core/settings.h"
#include "core/sfx.h"
#include "ui/ui.h"

#define MENU_MAX_ITEMS 5

typedef enum {
    MENU_PLAY,
    MENU_CONTINUE,
    MENU_NEW_GAME,
    MENU_SETTINGS,
    MENU_QUIT,
    MENU_CANCEL,
    MENU_ERASE,
    MENU_RESUME,
    MENU_RESTART,
    MENU_TO_MAP,
    MENU_TO_TITLE,
    MENU_NEXT_LEVEL,
    MENU_ACTION_COUNT
} MenuAction;

static const char *MENU_LABELS[MENU_ACTION_COUNT] = {
    [MENU_PLAY]       = "Jouer",
    [MENU_CONTINUE]   = "Continuer",
    [MENU_NEW_GAME]   = "Nouvelle partie",
    [MENU_SETTINGS]   = "Paramètres",
    [MENU_QUIT]       = "Quitter",
    [MENU_CANCEL]     = "Annuler",
    [MENU_ERASE]      = "Oui, tout effacer",
    [MENU_RESUME]     = "Reprendre",
    [MENU_RESTART]    = "Recommencer",
    [MENU_TO_MAP]     = "Retour à la carte",
    [MENU_TO_TITLE]   = "Menu principal",
    [MENU_NEXT_LEVEL] = "Niveau suivant",
};

static const char *HINT_MOUSE = "Haut/Bas : choisir   Entrée / clic : valider   Échap : retour";
static const char *HINT_KEYS = "Haut/Bas : choisir   Entrée : valider   Échap : retour";
static const char *HINT_TITLE_MOUSE = "Haut/Bas : choisir   Entrée / clic : valider   Échap : quitter";
static const char *HINT_TITLE_KEYS = "Haut/Bas : choisir   Entrée : valider   Échap : quitter";
static const char *HINT_CONFIRM_NEW = "Nouvelle partie : toute la progression sera effacée";

typedef struct {
    const char *title;
    const char *hint;
    int count;
    MenuAction items[MENU_MAX_ITEMS];
} Menu;

static void add(Menu *menu, MenuAction action)
{
    menu->items[menu->count++] = action;
}

static Menu build_menu(const Game *g)
{
    bool mouse = settings_mouse_enabled();
    Menu menu = { .title = NULL, .hint = mouse ? HINT_MOUSE : HINT_KEYS, .count = 0 };

    switch (g->state) {
    case STATE_TITLE:
        menu.hint = mouse ? HINT_TITLE_MOUSE : HINT_TITLE_KEYS;
        if (progress_is_new(&g->progress)) {
            add(&menu, MENU_PLAY);
        } else {
            add(&menu, MENU_CONTINUE);
            add(&menu, MENU_NEW_GAME);
        }
        add(&menu, MENU_SETTINGS);
        add(&menu, MENU_QUIT);
        break;
    case STATE_CONFIRM_NEW:
        menu.hint = HINT_CONFIRM_NEW;
        add(&menu, MENU_CANCEL);
        add(&menu, MENU_ERASE);
        break;
    case STATE_PAUSED:
        menu.title = "Pause";
        add(&menu, MENU_RESUME);
        if (g->paused_from == STATE_PLAYING) {
            add(&menu, MENU_RESTART);
        }
        add(&menu, MENU_SETTINGS);
        add(&menu, g->paused_from == STATE_PLAYING ? MENU_TO_MAP : MENU_TO_TITLE);
        break;
    case STATE_WON:
        menu.title = "Bravo !";
        if (g->level_index + 1 < g->level_count) {
            add(&menu, MENU_NEXT_LEVEL);
        }
        add(&menu, MENU_TO_MAP);
        break;
    default:
        break;
    }
    return menu;
}

static void open_menu(Game *g, GameState state)
{
    g->state = state;
    g->menu_selected = 0;
}

static void start_new_game(Game *g)
{
    progress_reset(&g->progress);
    if (!load_world(g, 0)) {
        return;
    }
    g->level_index = 0;
    open_menu(g, STATE_TITLE);
    begin_transition(g, STATE_MAP, -1);
}

static void run(Game *g, MenuAction action)
{
    switch (action) {
    case MENU_PLAY:
    case MENU_CONTINUE:
    case MENU_TO_MAP:
        begin_transition(g, STATE_MAP, -1);
        break;
    case MENU_NEW_GAME:
        sfx_play(SFX_UI_CONFIRM);
        open_menu(g, STATE_CONFIRM_NEW);
        break;
    case MENU_CANCEL:
        sfx_play(SFX_UI_BACK);
        open_menu(g, STATE_TITLE);
        break;
    case MENU_ERASE:
        start_new_game(g);
        break;
    case MENU_SETTINGS:
        sfx_play(SFX_UI_CONFIRM);
        g->settings_return = g->state;
        g->settings_selected = 0;
        g->state = STATE_SETTINGS;
        break;
    case MENU_QUIT:
        g->quit = true;
        break;
    case MENU_RESUME:
        sfx_play(SFX_UI_BACK);
        g->state = g->paused_from;
        break;
    case MENU_RESTART:
        restart_level(g);
        break;
    case MENU_TO_TITLE:
        begin_transition(g, STATE_TITLE, -1);
        break;
    case MENU_NEXT_LEVEL:
        begin_transition(g, STATE_PLAYING, g->level_index + 1);
        break;
    default:
        break;
    }
}

static void go_back(Game *g)
{
    switch (g->state) {
    case STATE_TITLE:
        g->quit = true;
        break;
    case STATE_CONFIRM_NEW:
        run(g, MENU_CANCEL);
        break;
    case STATE_PAUSED:
        run(g, MENU_RESUME);
        break;
    case STATE_WON:
        run(g, MENU_TO_MAP);
        break;
    default:
        break;
    }
}

bool game_menu_active(const Game *g)
{
    return g->state == STATE_TITLE || g->state == STATE_CONFIRM_NEW
        || g->state == STATE_PAUSED || g->state == STATE_WON;
}

void game_menu_open_pause(Game *g)
{
    sfx_play(SFX_UI_CONFIRM);
    g->paused_from = g->state;
    open_menu(g, STATE_PAUSED);
}

void game_menu_update(Game *g, bool back, bool confirm)
{
    Menu menu = build_menu(g);

    if (menu.count == 0 || g->transition != TRANSITION_NONE) {
        return;
    }
    if (g->menu_selected >= menu.count) {
        g->menu_selected = 0;
    }
    g->menu_selected = ui_navigate(g->menu_selected, menu.count);
    if (back) {
        go_back(g);
    } else if (confirm || ui_menu_clicked(menu.count)) {
        run(g, menu.items[g->menu_selected]);
    }
}

void game_menu_draw(const Game *g)
{
    Menu menu = build_menu(g);
    const char *labels[MENU_MAX_ITEMS];

    for (int i = 0; i < menu.count; i++) {
        labels[i] = MENU_LABELS[menu.items[i]];
    }
    if (menu.count > 0) {
        ui_draw_menu(menu.title, labels, menu.count, g->menu_selected, menu.hint);
    }
}
