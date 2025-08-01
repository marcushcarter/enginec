#ifndef OPENGL_MATH_H
#define OPENGL_MATH_H

#include <cglm/cglm.h>

void make_model_matrix(vec3 translation, vec3 rotation, vec3 scale, mat4 dest);
void print_mat4(mat4 m);

#endif