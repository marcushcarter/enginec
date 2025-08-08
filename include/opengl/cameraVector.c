#include "cameraVector.h"

void CameraVector_Draw(CameraVector* cameras, Mesh* mesh, Shader* shader, Camera* camera) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    mat4 model;
    vec3 ori;

    for (size_t i = 0; i < cameras->size; i++) {
    
        Camera* cam = cameras->data[i];

        orientation_to_euler(cam->Orientation, ori);
        make_model_matrix(cam->Position, ori, (vec3){0.2f, 0.2f, 0.2f}, model);
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
        Mesh_Draw(mesh, shader, camera);

    }
}