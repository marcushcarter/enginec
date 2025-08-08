#include "camera.h"
#include "vector.h"
#include "math.h"
#include "mesh.h"

Camera Camera_InitStack(int width, int height, float fov, float nearPlane, float farPlane, vec3 position, vec3 direction) {
    Camera camera;

    camera.width = width;
    camera.height = height;
    glm_vec3_copy(position, camera.position);
    glm_vec3_copy(direction, camera.direction);

    vec3 up = { 0.0f, 1.0f, 0.0f };

    glm_vec3_copy(up, camera.Up);

    camera.zoom = 1.0f;
    camera.fov = fov;

    camera.nearPlane = nearPlane;
    camera.farPlane = farPlane;

    mat4 mat;
    glm_mat4_identity(mat);
    glm_mat4_copy(mat, camera.cameraMatrix);

    return camera;
}

Camera* Camera_InitHeap(int width, int height, float fov, float nearPlane, float farPlane, vec3 position, vec3 direction) {
    Camera* camera = (Camera*)malloc(sizeof(Camera));

    camera->width = width;
    camera->height = height;
    glm_vec3_copy(position, camera->position);
    glm_vec3_copy(direction, camera->direction);

    vec3 up = { 0.0f, 1.0f, 0.0f };

    glm_vec3_copy(up, camera->Up);

    camera->zoom = 1.0f;
    camera->fov = fov;

    camera->nearPlane = nearPlane;
    camera->farPlane = farPlane;

    glm_mat4_identity(camera->cameraMatrix);

    return camera;
}

void Camera_UpdateMatrix(Camera* camera) {
    mat4 view;
    mat4 projection;
    mat4 ortho;
    mat4 projView;

    // 3D cam matrix

    float fov = camera->fov;

    vec3 target;
    glm_vec3_add(camera->position, camera->direction, target);
    glm_lookat(camera->position, target, camera->Up, view);
    glm_perspective(glm_rad(fov), (float)camera->width / (float)camera->height, camera->nearPlane, camera->farPlane, projection);
    glm_mat4_mul(projection, view, projView);
    glm_mat4_copy(projView, camera->cameraMatrix);
    glm_mat4_copy(view, camera->viewMatrix);
    
}

void Camera_Matrix(Camera* camera, Shader* shader, const char* uniform) {
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, uniform), 1, GL_FALSE, (float*)camera->cameraMatrix);
}

void Camera_MatrixCustom(Shader* shader, const char* uniform, mat4 matrix) {
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, uniform), 1, GL_FALSE, (float*)matrix);
}

void rotate_vec3_axis(vec3 in, vec3 axis, float angle_rad, vec3 out) {
    vec3 axis_n;
    glm_vec3_normalize_to(axis, axis_n);

    float cosA = cosf(angle_rad);
    float sinA = sinf(angle_rad);

    vec3 term1, term2, term3;
    glm_vec3_scale(in, cosA, term1);

    glm_vec3_cross(axis_n, in, term2);
    glm_vec3_scale(term2, sinA, term2);

    float dotAV = glm_vec3_dot(axis_n, in);
    glm_vec3_scale(axis_n, dotAV * (1.0f - cosA), term3);

    glm_vec3_add(term1, term2, out);
    glm_vec3_add(out, term3, out);
}

void Camera_Inputs(Camera* camera, GLFWwindow* window, Joystick* js, float dt) {

    // MOVEMENT VECTORS

    float speed = 2.5f;
    float sensitivity = 3.0f;
    // if (js && js->buttons[8]) speed = 5.0f;

    vec3 v_forward, v_right, v_up, v_move;
    glm_vec3_zero(v_move);

    glm_vec3_copy(camera->direction, v_forward);
    v_forward[1] = 0.0f;
    glm_vec3_normalize(v_forward);

    glm_vec3_cross(v_forward, (vec3){0.0f, 1.0f, 0.0f}, v_right);
    glm_vec3_normalize(v_right);

    glm_vec3_cross(v_forward, v_right, v_up);
    glm_vec3_normalize(v_up);
    
    // DIRECTION VECTORS

    vec3 d_up = { 0.0f, 1.0f, 0.0f };
    vec3 d_right, d_direction;

    glm_vec3_cross(d_up, camera->direction, d_right);
    glm_vec3_normalize(d_right);

    // MOVEMENT

    vec3 v_vector;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_scale(v_forward, speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_scale(v_forward, -speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_scale(v_right, -speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_scale(v_right, speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        glm_vec3_scale(v_up, -speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        glm_vec3_scale(v_up, speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }

    // CAMERA ROTATION

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        rotate_vec3_axis(camera->direction, d_up, dt*sensitivity, d_direction);
        glm_vec3_normalize_to(d_direction, camera->direction);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        rotate_vec3_axis(camera->direction, d_up, -dt*sensitivity, d_direction);
        glm_vec3_normalize_to(d_direction, camera->direction);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        rotate_vec3_axis(camera->direction, d_right, -dt*sensitivity, d_direction);
        glm_vec3_normalize_to(d_direction, camera->direction);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        rotate_vec3_axis(camera->direction, d_right, dt*sensitivity, d_direction);
        glm_vec3_normalize_to(d_direction, camera->direction);
    }

    
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        camera->fov += 10*dt;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        camera->fov -= 10*dt;
    }
    
    glm_vec3_add(camera->position, v_move, camera->position);

    if (js->present && js != NULL) {

        if (fabsf(js->axes[1]) > js->deadzone) {
            glm_vec3_scale(v_forward, -speed*dt*js->axes[1], v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }
        if (fabsf(js->axes[0]) > js->deadzone) {
            glm_vec3_scale(v_right, speed*dt*js->axes[0], v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }
        if (js->buttons[0] || joystickIsHeld(js, 0)) {
            glm_vec3_scale(v_up, -speed*dt, v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }
        if (js->buttons[1] || js->buttons[9]) {
            glm_vec3_scale(v_up, speed*dt, v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }

        // CAMERA ROTATION

        if (fabsf(js->axes[2]) > js->deadzone) {
            rotate_vec3_axis(camera->direction, d_up, -dt*sensitivity*js->axes[2], d_direction);
            glm_vec3_normalize_to(d_direction, camera->direction);
        }
        if (fabsf(js->axes[3]) > js->deadzone) {
            rotate_vec3_axis(camera->direction, d_right, dt*sensitivity*js->axes[3], d_direction);
            glm_vec3_normalize_to(d_direction, camera->direction);
        }
        
        glm_vec3_add(camera->position, v_move, camera->position);
    }

}

// void Camera_SetVectors(Camera* camera, vec3 position, vec3 direction) {}

// void Camera_Transform(Camera* camera, vec3 position, vec3 direction) {}