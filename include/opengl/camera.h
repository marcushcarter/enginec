#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "opengl/shader.h"
#include "opengl/joystick.h"

typedef struct {
    int width, height;
    vec3 Position;
    vec3 Orientation;
    vec3 Up;
    mat4 cameraMatrix;
    mat4 skyboxMatrix;
    float speed, sensitivity;
} Camera;

Camera Camera_Init(int width, int height, float speed, float sensitivity, vec3 position);
void Camera_UpdateMatrix(Camera* camera, float FOVdeg, float nearPlane, float farPlane);
void Camera_Matrix(Camera* camera, Shader* shader, const char* uniform);
void Camera_MatrixCustom(Shader* shader, const char* uniform, mat4 matrix);
void rotate_vec3_axis(vec3 in, vec3 axis, float angle_rad, vec3 out);
void Camera_Inputs(Camera* camera, GLFWwindow* window, Joystick* js, float dt);

#endif