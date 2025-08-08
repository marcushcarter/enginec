#include "math.h"

void make_model_matrix(vec3 translation, vec3 rotation, vec3 scale, mat4 dest) {
    mat4 trans, rotX, rotY, rotZ, rot, scl;

    glm_translate_make(trans, translation);

    glm_rotate_make(rotX, rotation[0], (vec3){1.0f, 0.0f, 0.0f});
    glm_rotate_make(rotY, rotation[1], (vec3){0.0f, 1.0f, 0.0f});
    glm_rotate_make(rotZ, rotation[2], (vec3){0.0f, 0.0f, 1.0f});

    glm_mat4_mul(rotY, rotX, rot);
    glm_mat4_mul(rotZ, rot, rot);

    glm_scale_make(scl, scale);

    mat4 rs;
    glm_mat4_mul(rot, scl, rs);
    glm_mat4_mul(trans, rs, dest);
}

void make_billboard_matrix(vec3 position, mat4 view, vec3 scale, mat4 dest) {
    mat4 model = GLM_MAT4_IDENTITY_INIT;

    // Copy camera rotation from view matrix (transpose of upper-left 3x3)
    model[0][0] = view[0][0];
    model[0][1] = view[1][0];
    model[0][2] = view[2][0];

    model[1][0] = view[0][1];
    model[1][1] = view[1][1];
    model[1][2] = view[2][1];

    model[2][0] = view[0][2];
    model[2][1] = view[1][2];
    model[2][2] = view[2][2];

    // Apply scaling
    glm_scale(model, scale);

    // Set position
    model[3][0] = position[0];
    model[3][1] = position[1];
    model[3][2] = position[2];

    glm_mat4_copy(model, dest);
}

void print_mat4(mat4 m) {
    printf("mat4:\n");
    for (int row = 0; row < 4; row++) {
        printf("[ ");
        for (int col = 0; col < 4; col++) {
            printf("%8.3f ", m[col][row]);  // cglm stores matrices column-major
        }
        printf("]\n");
    }
}

void orientation_to_euler(vec3 orientation, vec3 outEuler) {
    // Ensure orientation is normalized
    vec3 dir;
    glm_vec3_normalize_to(orientation, dir);

    // Calculate yaw and pitch
    float yaw = atan2f(dir[0], -dir[2]);     // around Y axis
    float pitch = asinf(dir[1]);            // around X axis
    float roll = 0.0f;                      // zero roll

    outEuler[0] = pitch;   // rotation X (pitch)
    outEuler[1] = yaw;     // rotation Y (yaw)
    outEuler[2] = roll;    // rotation Z (roll)
}