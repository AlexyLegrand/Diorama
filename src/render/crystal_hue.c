#include "render/crystal_hue.h"

static const float CRYSTAL_HUE_MIN = 170.0f;
static const float CRYSTAL_HUE_MAX = 215.0f;
static const float CRYSTAL_SATURATION_MIN = 0.2f;

bool crystal_hue_matches(Color color)
{
    Vector3 hsv = ColorToHSV(color);

    return hsv.x >= CRYSTAL_HUE_MIN && hsv.x <= CRYSTAL_HUE_MAX && hsv.y >= CRYSTAL_SATURATION_MIN;
}

Color crystal_hue_shift(Color color, float hue)
{
    Vector3 hsv = ColorToHSV(color);
    Color shifted = ColorFromHSV(hue, hsv.y, hsv.z);

    shifted.a = color.a;
    return shifted;
}
