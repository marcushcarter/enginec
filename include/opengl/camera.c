#include "opengl/camera.h"

Camera Camera_Init(int width, int height, float speed, float sensitivity, vec3 position) {
    Camera camera;

    camera.width = width;
    camera.height = height;
    glm_vec3_copy(position, camera.Position);

    vec3 orientation = { 0.0f, 0.0f, -1.0f };
    vec3 up = { 0.0f, 1.0f, 0.0f };

    glm_vec3_copy(orientation, camera.Orientation);
    glm_vec3_copy(up, camera.Up);

    camera.speed = speed;
    camera.sensitivity = sensitivity;
    mat4 mat;
    glm_mat4_identity(mat);
    glm_mat4_copy(mat, camera.cameraMatrix);

    return camera;
}

void Camera_UpdateMatrix(Camera* camera, float FOVdeg, float nearPlane, float farPlane) {
    mat4 view;
    mat4 projection;
    mat4 projView;

    vec3 target;
    glm_vec3_add(camera->Position, camera->Orientation, target);
    glm_lookat(camera->Position, target, camera->Up, view);

    glm_perspective(glm_rad(FOVdeg), (float)camera->width / (float)camera->height, nearPlane, farPlane, projection);
    glm_mat4_mul(projection, view, projView);

    glm_mat4_copy(projView, camera->cameraMatrix);
}

void Camera_Matrix(Camera* camera, Shader* shader, const char* uniform) {
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, uniform), 1, GL_FALSE, (float*)camera->cameraMatrix);
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

    float speed = camera->speed;
    if (js->buttons[8]) speed = camera->speed*2;

    vec3 v_forward, v_right, v_up, v_move;
    glm_vec3_zero(v_move);

    glm_vec3_copy(camera->Orientation, v_forward);
    v_forward[1] = 0.0f;
    glm_vec3_normalize(v_forward);

    glm_vec3_cross(v_forward, (vec3){0.0f, 1.0f, 0.0f}, v_right);
    glm_vec3_normalize(v_right);

    glm_vec3_cross(v_forward, v_right, v_up);
    glm_vec3_normalize(v_up);
    
    // DIRECTION VECTORS

    vec3 d_up = { 0.0f, 1.0f, 0.0f };
    vec3 d_right, d_Orientation;

    glm_vec3_cross(d_up, camera->Orientation, d_right);
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
        rotate_vec3_axis(camera->Orientation, d_up, dt*camera->sensitivity, d_Orientation);
        glm_vec3_normalize_to(d_Orientation, camera->Orientation);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        rotate_vec3_axis(camera->Orientation, d_up, -dt*camera->sensitivity, d_Orientation);
        glm_vec3_normalize_to(d_Orientation, camera->Orientation);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        rotate_vec3_axis(camera->Orientation, d_right, -dt*camera->sensitivity, d_Orientation);
        glm_vec3_normalize_to(d_Orientation, camera->Orientation);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        rotate_vec3_axis(camera->Orientation, d_right, dt*camera->sensitivity, d_Orientation);
        glm_vec3_normalize_to(d_Orientation, camera->Orientation);
    }
    
    glm_vec3_add(camera->Position, v_move, camera->Position);

    if (js->present && js != NULL) {

        if (fabsf(js->axes[1]) > js->deadzone) {
            glm_vec3_scale(v_forward, -speed*dt*js->axes[1], v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }
        if (fabsf(js->axes[0]) > js->deadzone) {
            glm_vec3_scale(v_right, speed*dt*js->axes[0], v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }
        if (js->buttons[0]) {
            glm_vec3_scale(v_up, -speed*dt, v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }
        if (js->buttons[1] || js->buttons[9]) {
            glm_vec3_scale(v_up, speed*dt, v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }

        // CAMERA ROTATION

        if (fabsf(js->axes[2]) > js->deadzone) {
            rotate_vec3_axis(camera->Orientation, d_up, -dt*camera->sensitivity*js->axes[2], d_Orientation);
            glm_vec3_normalize_to(d_Orientation, camera->Orientation);
        }
        if (fabsf(js->axes[3]) > js->deadzone) {
            rotate_vec3_axis(camera->Orientation, d_right, dt*camera->sensitivity*js->axes[3], d_Orientation);
            glm_vec3_normalize_to(d_Orientation, camera->Orientation);
        }
        
        glm_vec3_add(camera->Position, v_move, camera->Position);
    }

}
