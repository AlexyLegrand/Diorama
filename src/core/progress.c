#include <string.h>
#include "raylib.h"
#include "core/progress.h"

static const char *PROGRESS_PATH = "progress.dat";
static const int PROGRESS_VERSION = 1;

static bool is_valid(const Progress *p)
{
    const unsigned char *bytes = (const unsigned char *)p->completed;

    if (p->version != PROGRESS_VERSION || p->world < 0 || p->world >= WORLD_MAX) {
        return false;
    }
    for (size_t i = 0; i < sizeof(p->completed); i++) {
        if (bytes[i] > 1) {
            return false;
        }
    }
    return true;
}

void progress_load(Progress *progress)
{
    int size = 0;
    unsigned char *data = FileExists(PROGRESS_PATH) ? LoadFileData(PROGRESS_PATH, &size) : NULL;

    memset(progress, 0, sizeof(*progress));
    progress->version = PROGRESS_VERSION;
    if (data != NULL && size == (int)sizeof(Progress) && is_valid((const Progress *)data)) {
        memcpy(progress, data, sizeof(Progress));
    } else if (data != NULL) {
        TraceLog(LOG_WARNING, "PROGRESS: %s is invalid, starting a new game", PROGRESS_PATH);
    }
    UnloadFileData(data);
}

void progress_save(const Progress *progress)
{
    if (!SaveFileData(PROGRESS_PATH, (void *)progress, sizeof(Progress))) {
        TraceLog(LOG_WARNING, "PROGRESS: could not save %s", PROGRESS_PATH);
    }
}
