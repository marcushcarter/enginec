#ifndef RAYMARCH_CAMERA_CLASS_H
#define RAYMARCH_CAMERA_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "opengl/shader.h"

typedef struct {
    // vec2 dimension;
    vec3 Position;
    vec3 Orientation;
    float speed, sensitivity;
} RaymarchCamera;

RaymarchCamera RaymarchCamera_Init(vec3 position, vec3 orientation);
// RaymarchCamera RaymarchCamera_Init(vec2 dimension, vec3 position, vec3 orientation);
void rotate_vec3_axis(vec3 in, vec3 axis, float angle_rad, vec3 out);
void RaymarchCamera_Inputs(RaymarchCamera* camera, GLFWwindow* window, float dt);

#endif