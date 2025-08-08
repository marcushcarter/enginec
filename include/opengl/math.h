#ifndef OPENGL_MATH_H
#define OPENGL_MATH_H

#include <cglm/cglm.h>
#include "camera.h"

void make_model_matrix(vec3 translation, vec3 rotation, vec3 scale, mat4 dest);
void make_billboard_matrix(vec3 position, mat4 view, vec3 scale, mat4 dest);
void print_mat4(mat4 m);
void print_Camera(Camera* camera);
void orientation_to_euler(vec3 orientation, vec3 outEuler);

#endif