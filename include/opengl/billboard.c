#include "billboard.h"

Billboard Billboard_Init(vec3 position, vec2 size, GLuint tex) {
    Billboard bb;

    glm_vec3_copy(position, bb.position);
    glm_vec2_copy(size, bb.size);
    bb.textureID = tex;
}