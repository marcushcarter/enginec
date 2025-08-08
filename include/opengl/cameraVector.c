#include "cameraVector.h"

void CameraVector_Draw(CameraVector* cameras, Mesh* mesh, Shader* shader, Camera* camera) {
    
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    mat4 model;
    vec3 ori;

    for (size_t i = 0; i < cameras->size; i++) {
        if (CameraVector_Get(cameras, i) == camera) continue;
    
        Camera* cam = cameras->data[i];

        orientation_to_euler(cam->direction, ori);
        make_model_matrix(cam->position, ori, (vec3){0.25f * cam->width/1000 * cam->fov/45, 0.25f * cam->height/1000, 0.2f * cam->zoom}, model);
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
        Mesh_Draw(mesh, shader, camera);

    }
    
    glEnable(GL_CULL_FACE);
}