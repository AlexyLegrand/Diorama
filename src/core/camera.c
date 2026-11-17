#include <math.h>
#include "core/camera.h"
#include "raymath.h"

#define YAW_STEP_ANGLE   (45.0f * DEG2RAD)
#define PITCH_STEP_COUNT 3
#define TURN_SPEED       10.0f
#define ZOOM_SPEED       1.0f
#define DISTANCE_MIN     4.0f
#define DISTANCE_MAX     25.0f

static const float PITCH_ANGLES[PITCH_STEP_COUNT] = {
    10.0f * DEG2RAD,
    45.0f * DEG2RAD,
    80.0f * DEG2RAD,
};

OrbitCamera orbit_camera_init(Vector3 target)
{
    OrbitCamera oc;

    oc.target = target;
    oc.yaw_step = 1;
    oc.pitch_step = 1;
    oc.yaw = oc.yaw_step * YAW_STEP_ANGLE;
    oc.pitch = PITCH_ANGLES[oc.pitch_step];
    oc.distance = 12.0f;
    return oc;
}

void orbit_camera_update(OrbitCamera *oc)
{
    if (IsKeyPressed(KEY_LEFT)) {
        oc->yaw_step--;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        oc->yaw_step++;
    }
    if (IsKeyPressed(KEY_UP) && oc->pitch_step < PITCH_STEP_COUNT - 1) {
        oc->pitch_step++;
    }
    if (IsKeyPressed(KEY_DOWN) && oc->pitch_step > 0) {
        oc->pitch_step--;
    }

    float wheel = GetMouseWheelMove();
    float new_distance = oc->distance - wheel * ZOOM_SPEED;
    oc->distance = Clamp(new_distance, DISTANCE_MIN, DISTANCE_MAX);

    float target_yaw = oc->yaw_step * YAW_STEP_ANGLE;
    float target_pitch = PITCH_ANGLES[oc->pitch_step];
    float dt = GetFrameTime();
    float t = 1.0f - expf(-TURN_SPEED * dt);
    oc->yaw = Lerp(oc->yaw, target_yaw, t);
    oc->pitch = Lerp(oc->pitch, target_pitch, t);
}

float orbit_camera_snapped_yaw(const OrbitCamera *oc)
{
    return oc->yaw_step * YAW_STEP_ANGLE;
}

Camera3D orbit_camera_to_raylib(const OrbitCamera *oc)
{
    float cos_pitch = cosf(oc->pitch);
    float horizontal = oc->distance * cos_pitch;
    Camera3D camera;

    camera.position.x = oc->target.x + horizontal * cosf(oc->yaw);
    camera.position.y = oc->target.y + oc->distance * sinf(oc->pitch);
    camera.position.z = oc->target.z + horizontal * sinf(oc->yaw);
    camera.target = oc->target;
    camera.up.x = 0.0f;
    camera.up.y = 1.0f;
    camera.up.z = 0.0f;
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;
}
