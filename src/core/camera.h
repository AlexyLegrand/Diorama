#ifndef CAMERA_H
#define CAMERA_H

#include "raylib.h"

typedef struct {
    Vector3 target;
    int yaw_step;
    int pitch_step;
    float yaw;
    float pitch;
    float distance;
} OrbitCamera;

OrbitCamera orbit_camera_init(Vector3 target);

void orbit_camera_update(OrbitCamera *oc);

void orbit_camera_fit(OrbitCamera *oc, Vector3 size);

float orbit_camera_snapped_yaw(const OrbitCamera *oc);

Camera3D orbit_camera_to_raylib(const OrbitCamera *oc);

#endif
