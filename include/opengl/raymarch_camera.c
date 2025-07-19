#include "opengl/raymarch_camera.h"

RaymarchCamera RaymarchCamera_Init(vec3 Position, vec3 Orientation) {
// RaymarchCamera RaymarchCamera_Init(vec2 dimension, vec3 Position, vec3 Orientation) {
    RaymarchCamera camera;
    
    // glm_vec2_copy(dimension, camera.dimension);
    glm_vec3_copy(Position, camera.Position);
    glm_vec3_copy(Orientation, camera.Orientation);

    camera.speed = 1.0f;
    camera.sensitivity = 2.0f;

    return camera;
}

// vec3 rotate_y(vec3 v, float angle) {
//     float c = cosf(angle);
//     float s = sinf(angle);
//     vec3 new = { c * v[0] + s * v[2], v[1], -s * v[0] + c * v[2] };
//     return new;
// }

// void rotateY(vec3 *v, float angle) {
//     float c = cosf(angle);
//     float s = sinf(angle);
//     vec3 new = { c * (*v[0]) + s * (*v[2]), (*v[1]), -s * (*v[0]) + c * (*v[2]) };
//     glm_vec3_copy(new, &v);
// }

// void rotate_vec3_axis(vec3 in, vec3 axis, float angle_rad, vec3 out) {
//     vec3 axis_n;
//     glm_vec3_normalize_to(axis, axis_n);
//
//     float cosA = cosf(angle_rad);
//     float sinA = sinf(angle_rad);
//
//     vec3 term1, term2, term3;
//     glm_vec3_scale(in, cosA, term1);
//
//     glm_vec3_cross(axis_n, in, term2);
//     glm_vec3_scale(term2, sinA, term2);
//
//     float dotAV = glm_vec3_dot(axis_n, in);
//     glm_vec3_scale(axis_n, dotAV * (1.0f - cosA), term3);
//
//     glm_vec3_add(term1, term2, out);
//     glm_vec3_add(out, term3, out);
// }

void RaymarchCamera_Inputs(RaymarchCamera* camera, GLFWwindow* window, float dt) {

    // MOVEMENT VECTORS

    vec3 v_forward, v_right, v_up, v_move;
    glm_vec3_zero(v_move);

    glm_vec3_copy(camera->Orientation, v_forward);
    // v_forward[1] = 0.0f;
    glm_vec3_normalize(v_forward);

    glm_vec3_cross(v_forward, (vec3){0.0f, 1.0f, 0.0f}, v_right);
    glm_vec3_normalize(v_right);
    
    // DIRECTION VECTORS

    vec3 d_up = { 0.0f, 1.0f, 0.0f };
    vec3 d_right, d_Orientation;

    glm_vec3_cross(d_up, camera->Orientation, d_right);
    glm_vec3_normalize(d_right);

    // MOVEMENT

    vec3 v_vector;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_scale(v_forward, camera->speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_scale(v_forward, -camera->speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_scale(v_right, camera->speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_scale(v_right, -camera->speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }

    // CAMERA ROTATION

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        rotate_vec3_axis(camera->Orientation, d_up, -camera->sensitivity*dt, d_Orientation);
        glm_vec3_normalize_to(d_Orientation, camera->Orientation);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        rotate_vec3_axis(camera->Orientation, d_up, camera->sensitivity*dt, d_Orientation);
        glm_vec3_normalize_to(d_Orientation, camera->Orientation);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        rotate_vec3_axis(camera->Orientation, d_right, -camera->sensitivity*dt, d_Orientation);
        glm_vec3_normalize_to(d_Orientation, camera->Orientation);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        rotate_vec3_axis(camera->Orientation, d_right, camera->sensitivity*dt, d_Orientation);
        glm_vec3_normalize_to(d_Orientation, camera->Orientation);
    }
    
    glm_vec3_add(camera->Position, v_move, camera->Position);

}