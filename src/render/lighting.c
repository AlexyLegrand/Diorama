#include "render/lighting.h"

static const char *VERTEX_SHADER =
    "#version 330\n"
    "in vec3 vertexPosition;\n"
    "in vec2 vertexTexCoord;\n"
    "in vec3 vertexNormal;\n"
    "in vec4 vertexColor;\n"
    "uniform mat4 mvp;\n"
    "uniform mat4 matModel;\n"
    "uniform mat4 matNormal;\n"
    "out vec3 fragPosition;\n"
    "out vec2 fragTexCoord;\n"
    "out vec4 fragColor;\n"
    "out vec3 fragNormal;\n"
    "void main()\n"
    "{\n"
    "    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));\n"
    "    fragTexCoord = vertexTexCoord;\n"
    "    fragColor = vertexColor;\n"
    "    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";

static const char *FRAGMENT_SHADER =
    "#version 330\n"
    "in vec3 fragPosition;\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "in vec3 fragNormal;\n"
    "uniform sampler2D texture0;\n"
    "uniform vec4 colDiffuse;\n"
    "uniform vec3 viewPos;\n"
    "out vec4 finalColor;\n"
    "void main()\n"
    "{\n"
    "    vec4 base = texture(texture0, fragTexCoord) * colDiffuse * fragColor;\n"
    "    vec3 sun_color = vec3(1.0, 0.95, 0.85);\n"
    "    vec3 sky_color = vec3(0.70, 0.80, 1.00);\n"
    "    vec3 ground_color = vec3(0.70, 0.62, 0.58);\n"
    "    vec3 to_light = normalize(vec3(0.4, 1.0, 0.3));\n"
    "    vec3 normal = normalize(fragNormal);\n"
    "    vec3 to_view = normalize(viewPos - fragPosition);\n"
    "    float diffuse = max(dot(normal, to_light), 0.0);\n"
    "    float sky_amount = normal.y * 0.5 + 0.5;\n"
    "    vec3 ambient = mix(ground_color, sky_color, sky_amount);\n"
    "    vec3 halfway = normalize(to_light + to_view);\n"
    "    float specular = pow(max(dot(normal, halfway), 0.0), 32.0) * 0.25;\n"
    "    float rim = pow(1.0 - max(dot(normal, to_view), 0.0), 3.0) * 0.2;\n"
    "    vec3 light = ambient * 0.75 + sun_color * diffuse * 0.6;\n"
    "    vec3 color = base.rgb * light + sun_color * specular + sky_color * rim;\n"
    "    finalColor = vec4(color, base.a);\n"
    "}\n";

static Shader lighting_shader;
static int view_position_loc;

void lighting_load(void)
{
    lighting_shader = LoadShaderFromMemory(VERTEX_SHADER, FRAGMENT_SHADER);
    lighting_shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(lighting_shader, "matModel");
    lighting_shader.locs[SHADER_LOC_MATRIX_NORMAL] = GetShaderLocation(lighting_shader, "matNormal");
    view_position_loc = GetShaderLocation(lighting_shader, "viewPos");
}

void lighting_unload(void)
{
    UnloadShader(lighting_shader);
}

void lighting_apply(Model *model)
{
    for (int i = 0; i < model->materialCount; i++) {
        model->materials[i].shader = lighting_shader;
    }
}

void lighting_update(Vector3 camera_position)
{
    SetShaderValue(lighting_shader, view_position_loc, &camera_position, SHADER_UNIFORM_VEC3);
}
