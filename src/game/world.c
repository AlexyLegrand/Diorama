#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include "game/world.h"
#include "core/settings.h"

#define WORLD_LINE_MAX 128
#define NAME_SEPARATOR '|'

static bool parse_world(const char *line, WorldInfo *world)
{
    char names[WORLD_LINE_MAX];

    if (sscanf(line, "%31s %127[^\r\n]", world->dir, names) != 2) {
        return false;
    }
    char *separator = strchr(names, NAME_SEPARATOR);
    if (separator != NULL) {
        *separator = '\0';
    }
    for (int lang = 0; lang < LANG_COUNT; lang++) {
        const char *name = (lang == LANG_FR || separator == NULL) ? names : separator + 1;
        snprintf(world->names[lang], WORLD_NAME_MAX, "%.31s", name);
    }
    return true;
}

int worlds_load(WorldInfo worlds[], int max, const char *path)
{
    char *text = LoadFileText(path);
    int count = 0;
    int line_number = 0;

    if (text == NULL) {
        return 0;
    }
    for (char *line = text; line != NULL && *line != '\0'; ) {
        char *end = strchr(line, '\n');
        char buffer[WORLD_LINE_MAX];
        int length = (end != NULL) ? (int)(end - line) : (int)strlen(line);

        line_number++;
        snprintf(buffer, sizeof(buffer), "%.*s", length, line);
        line = (end != NULL) ? end + 1 : NULL;
        if (buffer[0] == '#' || buffer[strspn(buffer, " \t\r")] == '\0') {
            continue;
        }
        if (count >= max || !parse_world(buffer, &worlds[count])) {
            TraceLog(LOG_ERROR, "WORLDS: [%s] line %d: expected '<folder> <name>' (max %d worlds)",
                     path, line_number, max);
            UnloadFileText(text);
            return 0;
        }
        count++;
    }
    UnloadFileText(text);
    return count;
}

void world_level_path(const WorldInfo *world, int level, char *out, size_t size)
{
    if (level == WORLD_MAP) {
        snprintf(out, size, "levels/%s/map.txt", world->dir);
    } else {
        snprintf(out, size, "levels/%s/level_%d.txt", world->dir, level + 1);
    }
}

const char *world_name(const WorldInfo *world)
{
    return world->names[settings_language()];
}
