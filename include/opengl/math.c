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
