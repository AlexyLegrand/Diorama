#include "render/title_logo.h"
#include "render/lighting.h"

static const char *LOGO_MODEL_PATH = "assets/logo/logo.glb";
static const float LOGO_SCALE = 0.1f;
static const Vector3 LOGO_OFFSET = { 0.0f, 1.6f, -2.0f };

static Model logo;
static bool loaded;

void title_logo_load(void)
{
    logo = LoadModel(LOGO_MODEL_PATH);
    loaded = IsModelValid(logo);
    if (loaded) {
        lighting_apply(&logo);
    }
}

void title_logo_unload(void)
{
    if (loaded) {
        UnloadModel(logo);
        loaded = false;
    }
}

void title_logo_draw(Vector3 scene_center)
{
    if (!loaded) {
        return;
    }
    Vector3 position = {
        scene_center.x + LOGO_OFFSET.x,
        scene_center.y + LOGO_OFFSET.y,
        scene_center.z + LOGO_OFFSET.z,
    };
    DrawModel(logo, position, LOGO_SCALE, WHITE);
}
