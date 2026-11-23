#include "world/level.h"
#include "render/block_render.h"
#include <string.h>

static const struct {
    char symbol;
    Vegetation kind;
    BlockType cell;
} VEGETATION_CHARS[] = {
    { 'T', VEG_TREE_ROUND, BLOCK_PROP },
    { 'P', VEG_TREE_PINE,  BLOCK_PROP },
    { 'B', VEG_BUSH,       BLOCK_PROP },
    { 'F', VEG_FERN,       BLOCK_EMPTY },
    { 'g', VEG_GRASS,      BLOCK_EMPTY },
    { 'f', VEG_FLOWERS,    BLOCK_EMPTY },
    { 'm', VEG_MUSHROOMS,  BLOCK_EMPTY },
    { 'I', VEG_IVY,        BLOCK_GROUND },
    { 'V', VEG_VINE,       BLOCK_GROUND },
};
#define VEGETATION_CHAR_COUNT ((int)(sizeof(VEGETATION_CHARS) / sizeof(VEGETATION_CHARS[0])))

static int vegetation_index(char c)
{
    for (int i = 0; i < VEGETATION_CHAR_COUNT; i++) {
        if (VEGETATION_CHARS[i].symbol == c) {
            return i;
        }
    }
    return -1;
}

static bool char_to_cell(char c, Cell *cell)
{
    int veg = vegetation_index(c);
    if (veg >= 0) {
        *cell = (Cell){ VEGETATION_CHARS[veg].cell, DIR_NORTH };
        return true;
    }
    if ((c >= '1' && c <= '9') || c == '+' || c == '-') {
        *cell = (Cell){ BLOCK_EMPTY, DIR_NORTH };
        return true;
    }
    switch (c) {
    case '.': *cell = (Cell){ BLOCK_EMPTY, DIR_NORTH }; return true;
    case '@': *cell = (Cell){ BLOCK_EMPTY, DIR_NORTH }; return true;
    case '#': *cell = (Cell){ BLOCK_GROUND, DIR_NORTH }; return true;
    case 'c': *cell = (Cell){ BLOCK_CRATE, DIR_NORTH }; return true;
    case '*': *cell = (Cell){ BLOCK_STAR, DIR_NORTH }; return true;
    case 'k': *cell = (Cell){ BLOCK_CRANK, DIR_SOUTH }; return true;
    case 'p': *cell = (Cell){ BLOCK_PLATFORM, DIR_NORTH }; return true;
    case '^': *cell = (Cell){ BLOCK_STAIRS, DIR_NORTH }; return true;
    case '>': *cell = (Cell){ BLOCK_STAIRS, DIR_EAST }; return true;
    case 'v': *cell = (Cell){ BLOCK_STAIRS, DIR_SOUTH }; return true;
    case '<': *cell = (Cell){ BLOCK_STAIRS, DIR_WEST }; return true;
    case 'n': *cell = (Cell){ BLOCK_LADDER, DIR_NORTH }; return true;
    case 'e': *cell = (Cell){ BLOCK_LADDER, DIR_EAST }; return true;
    case 's': *cell = (Cell){ BLOCK_LADDER, DIR_SOUTH }; return true;
    case 'w': *cell = (Cell){ BLOCK_LADDER, DIR_WEST }; return true;
    }
    return false;
}

static bool load_fail(const char *path, int line, const char *reason)
{
    TraceLog(LOG_WARNING, "LEVEL: [%s] line %d: %s", path, line, reason);
    return false;
}

static bool finish_layer(Level *level, int *y, int *z, const char *path, int line_number)
{
    if (level->depth == 0) {
        level->depth = *z;
    }
    if (*z != level->depth) {
        return load_fail(path, line_number, "layers have different depths");
    }
    (*y)++;
    *z = 0;
    return true;
}

static bool parse_row(Level *level, const char *row, int length, int y, int z, bool *has_start)
{
    for (int x = 0; x < length; x++) {
        if (!char_to_cell(row[x], &level->cells[y][x][z])) {
            return false;
        }
        if (row[x] == '@') {
            level->start_x = x;
            level->start_y = y;
            level->start_z = z;
            *has_start = true;
        }
        int veg = vegetation_index(row[x]);
        if (veg >= 0) {
            if (level->decor_count >= LEVEL_MAX_DECOR) {
                return false;
            }
            level->decor[level->decor_count++] = (Decor){ VEGETATION_CHARS[veg].kind, { x, y, z } };
        }
        if (row[x] == '+') {
            level->portal_next = (GridPos){ x, y, z };
            level->has_portal_next = true;
        } else if (row[x] == '-') {
            level->portal_prev = (GridPos){ x, y, z };
            level->has_portal_prev = true;
        }
        if (row[x] >= '1' && row[x] <= '9') {
            int index = row[x] - '1';
            level->entrances[index] = (GridPos){ x, y, z };
            if (index + 1 > level->entrance_count) {
                level->entrance_count = index + 1;
            }
        }
    }
    return true;
}

static bool parse_level(Level *level, const char *text, const char *path)
{
    int y = 0;
    int z = 0;
    int line_number = 0;
    bool has_start = false;
    const char *cursor = text;

    *level = (Level){0};
    while (cursor != NULL) {
        line_number++;
        const char *end = strchr(cursor, '\n');
        int length = (end != NULL) ? (int)(end - cursor) : (int)strlen(cursor);
        if (length > 0 && cursor[length - 1] == '\r') {
            length--;
        }

        if (length == 0) {
            if (z > 0 && !finish_layer(level, &y, &z, path, line_number)) {
                return false;
            }
        } else {
            if (y >= LEVEL_MAX_HEIGHT) {
                return load_fail(path, line_number, "too many layers");
            }
            if (z >= LEVEL_MAX_DEPTH) {
                return load_fail(path, line_number, "too many rows in layer");
            }
            if (level->width == 0) {
                level->width = length;
            }
            if (length != level->width || length > LEVEL_MAX_WIDTH) {
                return load_fail(path, line_number, "bad row width");
            }
            if (!parse_row(level, cursor, length, y, z, &has_start)) {
                return load_fail(path, line_number, "unknown character or too much vegetation");
            }
            z++;
        }

        cursor = (end != NULL) ? end + 1 : NULL;
    }
    if (z > 0 && !finish_layer(level, &y, &z, path, line_number)) {
        return false;
    }
    level->height = y;
    if (level->height == 0) {
        return load_fail(path, line_number, "empty level");
    }
    if (!has_start) {
        return load_fail(path, line_number, "no start '@'");
    }
    return true;
}

bool level_load(Level *level, const char *path)
{
    char *text = LoadFileText(path);

    if (text == NULL) {
        return false;
    }
    bool ok = parse_level(level, text, path);
    UnloadFileText(text);
    return ok;
}

Vector3 level_center(const Level *level)
{
    Vector3 center = {
        .x = (level->width - 1) / 2.0f,
        .y = (level->height - 1) / 2.0f,
        .z = (level->depth - 1) / 2.0f,
    };

    return center;
}

void level_draw(const Level *level, const GridPos *hidden, bool hide_platforms)
{
    for (int y = 0; y < level->height; y++) {
        for (int x = 0; x < level->width; x++) {
            for (int z = 0; z < level->depth; z++) {
                bool is_hidden = hidden != NULL && hidden->x == x && hidden->y == y && hidden->z == z;
                bool is_platform = level->cells[y][x][z].type == BLOCK_PLATFORM;
                if (is_hidden || (hide_platforms && is_platform)) {
                    continue;
                }
                draw_block(x, y, z, level->cells[y][x][z]);
            }
        }
    }
}
