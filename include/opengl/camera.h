#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "opengl/shader.h"
#include "opengl/joystick.h"

typedef struct {
    int width, height;
    float zoom, fov;
    float nearPlane, farPlane;
    vec3 position, direction, Up;
    mat4 cameraMatrix, viewMatrix;
} Camera;

Camera Camera_InitStack(int width, int height, float fov, float nearPlane, float farPlane, vec3 position, vec3 direction);
Camera* Camera_InitHeap(int width, int height, float fov, float nearPlane, float farPlane, vec3 position, vec3 direction);

void Camera_UpdateMatrix(Camera* camera);

void Camera_Matrix(Camera* camera, Shader* shader, const char* uniform);
void Camera_MatrixCustom(Shader* shader, const char* uniform, mat4 matrix);

void rotate_vec3_axis(vec3 in, vec3 axis, float angle_rad, vec3 out);
void Camera_Inputs(Camera* camera, GLFWwindow* window, Joystick* js, float dt);

#endif