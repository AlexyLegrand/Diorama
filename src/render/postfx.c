#include <stddef.h>
#include "render/postfx.h"

static const char *FRAGMENT_SHADER =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform vec2 resolution;\n"
    "uniform float focusCenter;\n"
    "uniform float focusWidth;\n"
    "uniform float focusFalloff;\n"
    "uniform float maxBlur;\n"
    "uniform float saturation;\n"
    "uniform float contrast;\n"
    "uniform vec3 shadowTint;\n"
    "uniform vec3 highlightTint;\n"
    "out vec4 finalColor;\n"
    "const vec3 LUMA = vec3(0.299, 0.587, 0.114);\n"
    "const vec2 DISC[12] = vec2[](\n"
    "    vec2(-0.326, -0.406), vec2(-0.840, -0.074), vec2(-0.696, 0.457), vec2(-0.203, 0.621),\n"
    "    vec2(0.962, -0.195), vec2(0.473, -0.480), vec2(0.519, 0.767), vec2(0.185, -0.893),\n"
    "    vec2(0.507, 0.064), vec2(0.896, 0.412), vec2(-0.322, -0.933), vec2(-0.792, -0.598));\n"
    "vec3 sampleColor(vec2 uv) { return texture(texture0, uv).rgb; }\n"
    "vec3 fxaa(vec2 uv, vec2 px) {\n"
    "    vec3 nw = sampleColor(uv + vec2(-1.0, -1.0) * px);\n"
    "    vec3 ne = sampleColor(uv + vec2(1.0, -1.0) * px);\n"
    "    vec3 sw = sampleColor(uv + vec2(-1.0, 1.0) * px);\n"
    "    vec3 se = sampleColor(uv + vec2(1.0, 1.0) * px);\n"
    "    vec3 m = sampleColor(uv);\n"
    "    float lnw = dot(nw, LUMA); float lne = dot(ne, LUMA);\n"
    "    float lsw = dot(sw, LUMA); float lse = dot(se, LUMA); float lm = dot(m, LUMA);\n"
    "    float lmin = min(lm, min(min(lnw, lne), min(lsw, lse)));\n"
    "    float lmax = max(lm, max(max(lnw, lne), max(lsw, lse)));\n"
    "    vec2 dir = vec2(-((lnw + lne) - (lsw + lse)), (lnw + lsw) - (lne + lse));\n"
    "    float reduce = max((lnw + lne + lsw + lse) * (0.25 / 8.0), 1.0 / 128.0);\n"
    "    float rcp = 1.0 / (min(abs(dir.x), abs(dir.y)) + reduce);\n"
    "    dir = clamp(dir * rcp, -8.0, 8.0) * px;\n"
    "    vec3 a = 0.5 * (sampleColor(uv + dir * (1.0 / 3.0 - 0.5)) + sampleColor(uv + dir * (2.0 / 3.0 - 0.5)));\n"
    "    vec3 b = a * 0.5 + 0.25 * (sampleColor(uv - dir * 0.5) + sampleColor(uv + dir * 0.5));\n"
    "    float lb = dot(b, LUMA);\n"
    "    return (lb < lmin || lb > lmax) ? a : b;\n"
    "}\n"
    "void main() {\n"
    "    vec2 px = 1.0 / resolution;\n"
    "    vec2 uv = fragTexCoord;\n"
    "    vec3 color = fxaa(uv, px);\n"
    "    float distance = abs(uv.y - focusCenter);\n"
    "    float blur = smoothstep(focusWidth, focusWidth + focusFalloff, distance) * maxBlur;\n"
    "    if (blur > 0.3) {\n"
    "        vec3 sum = color;\n"
    "        for (int i = 0; i < 12; i++) {\n"
    "            sum += sampleColor(uv + DISC[i] * blur * px);\n"
    "        }\n"
    "        color = sum / 13.0;\n"
    "    }\n"
    "    float luma = dot(color, LUMA);\n"
    "    color = mix(vec3(luma), color, saturation);\n"
    "    color = (color - 0.5) * contrast + 0.5;\n"
    "    color *= mix(shadowTint, highlightTint, clamp(luma, 0.0, 1.0));\n"
    "    finalColor = vec4(clamp(color, 0.0, 1.0), 1.0);\n"
    "}\n";

static const float FOCUS_CENTER = 0.5f;
static const float FOCUS_WIDTH = 0.3f;
static const float FOCUS_FALLOFF = 0.25f;
static const float MAX_BLUR_PIXELS = 2.5f;
static const float REFERENCE_HEIGHT = 720.0f;
static const float SATURATION = 1.12f;
static const float CONTRAST = 1.06f;
static const Vector3 SHADOW_TINT = { 0.95f, 0.96f, 1.05f };
static const Vector3 HIGHLIGHT_TINT = { 1.05f, 1.01f, 0.94f };

static Shader shader;
static RenderTexture2D target;
static bool ready;
static float blur_strength = 1.0f;
static int loc_resolution;
static int loc_max_blur;

static void set_float(const char *name, float value)
{
    SetShaderValue(shader, GetShaderLocation(shader, name), &value, SHADER_UNIFORM_FLOAT);
}

static void set_vec3(const char *name, Vector3 value)
{
    SetShaderValue(shader, GetShaderLocation(shader, name), &value, SHADER_UNIFORM_VEC3);
}

static void resize_target(void)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    if (IsRenderTextureValid(target) && target.texture.width == width && target.texture.height == height) {
        return;
    }
    if (IsRenderTextureValid(target)) {
        UnloadRenderTexture(target);
    }
    target = LoadRenderTexture(width, height);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
}

void postfx_load(void)
{
    shader = LoadShaderFromMemory(NULL, FRAGMENT_SHADER);
    ready = IsShaderValid(shader);
    if (!ready) {
        TraceLog(LOG_WARNING, "POSTFX: shader failed, rendering without post-processing");
        return;
    }
    loc_resolution = GetShaderLocation(shader, "resolution");
    loc_max_blur = GetShaderLocation(shader, "maxBlur");
    set_float("focusCenter", FOCUS_CENTER);
    set_float("focusWidth", FOCUS_WIDTH);
    set_float("focusFalloff", FOCUS_FALLOFF);
    set_float("saturation", SATURATION);
    set_float("contrast", CONTRAST);
    set_vec3("shadowTint", SHADOW_TINT);
    set_vec3("highlightTint", HIGHLIGHT_TINT);
}

void postfx_unload(void)
{
    if (IsRenderTextureValid(target)) {
        UnloadRenderTexture(target);
    }
    if (ready) {
        UnloadShader(shader);
    }
}

void postfx_set_blur(float strength)
{
    blur_strength = strength;
}

void postfx_begin(void)
{
    if (!ready) {
        return;
    }
    resize_target();
    BeginTextureMode(target);
}

void postfx_end(void)
{
    if (!ready) {
        return;
    }
    EndTextureMode();
    Vector2 resolution = { (float)target.texture.width, (float)target.texture.height };
    float max_blur = MAX_BLUR_PIXELS * (resolution.y / REFERENCE_HEIGHT) * blur_strength;
    SetShaderValue(shader, loc_resolution, &resolution, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, loc_max_blur, &max_blur, SHADER_UNIFORM_FLOAT);
    Rectangle source = { 0.0f, 0.0f, resolution.x, -resolution.y };
    BeginShaderMode(shader);
    DrawTextureRec(target.texture, source, (Vector2){ 0.0f, 0.0f }, WHITE);
    EndShaderMode();
}
