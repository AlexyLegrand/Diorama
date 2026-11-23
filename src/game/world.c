#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include "game/world.h"

#define WORLD_LINE_MAX 128

static bool parse_world(const char *line, WorldInfo *world)
{
    return sscanf(line, "%31s %31[^\r\n]", world->dir, world->name) == 2;
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
