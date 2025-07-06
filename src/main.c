
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// #include <glm/glm.hpp>
#include <cglm/cglm.h>
#include <KHR/khrplatform.h>

#include <stdio.h>

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL window", NULL, NULL);

    mat4 matrix;
    vec4 vec = {1.0f, 2.0f, 3.0f, 1.0f};
    glm_mat4_identity(matrix);
    vec4 test;
    glm_mat4_mulv(matrix, vec, test);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;

}