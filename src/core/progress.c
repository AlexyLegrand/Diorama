#include <string.h>
#include "raylib.h"
#include "core/progress.h"

static const char *PROGRESS_PATH = "progress.dat";
static const int PROGRESS_VERSION = 3;
static const int PROGRESS_VERSION_NO_COUNTERS = 1;
static const int PROGRESS_VERSION_NO_COLLECTED = 2;

typedef struct {
    int version;
    int world;
    bool completed[WORLD_MAX][LEVEL_MAX_ENTRANCES];
} ProgressNoCounters;

typedef struct {
    int version;
    int world;
    bool completed[WORLD_MAX][LEVEL_MAX_ENTRANCES];
    int seeds;
    int collectibles;
} ProgressNoCollected;

static bool is_valid(const Progress *p)
{
    const unsigned char *bytes = (const unsigned char *)p->completed;

    if (p->version != PROGRESS_VERSION || p->world < 0 || p->world >= WORLD_MAX
        || p->seeds < 0 || p->collectibles < 0) {
        return false;
    }
    for (size_t i = 0; i < sizeof(p->completed); i++) {
        if (bytes[i] > 1) {
            return false;
        }
    }
    return true;
}

void progress_reset(Progress *progress)
{
    memset(progress, 0, sizeof(*progress));
    progress->version = PROGRESS_VERSION;
}

static int count_completed(const Progress *progress)
{
    int count = 0;

    for (int w = 0; w < WORLD_MAX; w++) {
        for (int i = 0; i < LEVEL_MAX_ENTRANCES; i++) {
            count += progress->completed[w][i] ? 1 : 0;
        }
    }
    return count;
}

void progress_load(Progress *progress)
{
    int size = 0;
    unsigned char *data = FileExists(PROGRESS_PATH) ? LoadFileData(PROGRESS_PATH, &size) : NULL;

    progress_reset(progress);
    bool no_counters = data != NULL && size == (int)sizeof(ProgressNoCounters)
        && ((const ProgressNoCounters *)data)->version == PROGRESS_VERSION_NO_COUNTERS;
    bool no_collected = data != NULL && size == (int)sizeof(ProgressNoCollected)
        && ((const ProgressNoCollected *)data)->version == PROGRESS_VERSION_NO_COLLECTED;
    if (no_counters || no_collected) {
        Progress migrated = *progress;
        memcpy(&migrated, data, no_counters ? sizeof(ProgressNoCounters) : sizeof(ProgressNoCollected));
        migrated.version = PROGRESS_VERSION;
        if (no_counters) {
            migrated.seeds = count_completed(&migrated);
        }
        if (is_valid(&migrated)) {
            *progress = migrated;
        }
    } else if (data != NULL && size == (int)sizeof(Progress) && is_valid((const Progress *)data)) {
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

bool progress_is_new(const Progress *progress)
{
    for (int w = 0; w < WORLD_MAX; w++) {
        for (int i = 0; i < LEVEL_MAX_ENTRANCES; i++) {
            if (progress->completed[w][i]) {
                return false;
            }
        }
    }
    return true;
}

void progress_complete_level(Progress *progress, int world, int level)
{
    if (!progress->completed[world][level]) {
        progress->completed[world][level] = true;
        progress->seeds++;
    }
}

bool progress_has_collectible(const Progress *progress, int world, int level, int index)
{
    return (progress->collected[world][level] >> index) & 1u;
}

void progress_collect(Progress *progress, int world, int level, int index)
{
    if (!progress_has_collectible(progress, world, level, index)) {
        progress->collected[world][level] |= 1u << index;
        progress->collectibles++;
    }
}
