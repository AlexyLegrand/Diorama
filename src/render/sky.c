#include "raylib.h"
#include "rlgl.h"
#include "render/sky.h"

static const Color SKY_TOP = { 92, 150, 222, 255 };
static const Color SKY_HORIZON = { 214, 234, 250, 255 };
static const Color VIGNETTE_EDGE = { 10, 20, 40, 70 };
static const Color VIGNETTE_CLEAR = { 10, 20, 40, 0 };
static const float VIGNETTE_SIZE = 0.18f;

void sky_draw_background(void)
{
    ClearBackground(SKY_HORIZON);
    rlDisableDepthMask();
    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), SKY_TOP, SKY_HORIZON);
    rlDrawRenderBatchActive();
    rlEnableDepthMask();
}

void sky_draw_vignette(void)
{
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    int band_x = (int)(w * VIGNETTE_SIZE);
    int band_y = (int)(h * VIGNETTE_SIZE);

    DrawRectangleGradientV(0, 0, w, band_y, VIGNETTE_EDGE, VIGNETTE_CLEAR);
    DrawRectangleGradientV(0, h - band_y, w, band_y, VIGNETTE_CLEAR, VIGNETTE_EDGE);
    DrawRectangleGradientH(0, 0, band_x, h, VIGNETTE_EDGE, VIGNETTE_CLEAR);
    DrawRectangleGradientH(w - band_x, 0, band_x, h, VIGNETTE_CLEAR, VIGNETTE_EDGE);
}
