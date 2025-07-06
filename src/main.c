
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// #include <glm/glm.hpp>
#include <cglm/cglm.h>
// #include <KHR/khrplatform.h>

#include <stdio.h>
#include <math.h>

#include "shader_class.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

GLfloat vertices[] = {
//                       COORDINATES                      |        COLORS           //
    -0.5f,      -0.5f * (float)(sqrt(3)) / 3,       0.0f,   0.8f,   0.3f,   0.02f,  // Lower left corner
     0.5f,      -0.5f * (float)(sqrt(3)) / 3,       0.0f,   0.8f,   0.3f,   0.02f,  // Lower right corner
     0.0f,       0.5f * (float)(sqrt(3)) * 2 / 3,   0.0f,   1.0f,   0.6f,   0.32f,  // Upper corner
    -0.5f / 2,   0.5f * (float)(sqrt(3)) / 6,       0.0f,   0.9f,   0.45f,  0.17f,  // Inner left
     0.5f / 2,   0.5f * (float)(sqrt(3)) / 6,       0.0f,   0.9f,   0.45f,  0.17f,  // Inner right
     0.0f,      -0.5f * (float)(sqrt(3)) / 3,       0.0f,   0.8f,   0.3f,   0.02f,  // Inner down
};

GLuint indices[] = {
    0, 3, 5, 
    3, 2, 4,
    5, 4, 1,
};

int main() 
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(800, 800, "opengl window", NULL, NULL);
    if (window == NULL) 
    {
        printf("window failed to create\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glViewport(0, 0, 800, 800);

    // SHADERS

    Shader shaderProgram = Shader_Init("shaders/default.vert", "shaders/default.frag");

    // BUFFERS

    VAO VAO1 = VAO_Init();
    VAO_Bind(&VAO1);

    VBO VBO1 = VBO_Init(vertices, sizeof(vertices));
    EBO EBO1 = EBO_Init(indices, sizeof(indices));

    VAO_LinkVBO(&VAO1, VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    VAO_LinkVBO(&VAO1, VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();
    EBO_Unbind();




    GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

    // GAME LOOP

    while(!glfwWindowShouldClose(window)) 
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        Shader_Activate(&shaderProgram);
        glUniform1f(uniID, 0.5f);
        VAO_Bind(&VAO1);
        glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    VAO_Delete(&VAO1);
    VBO_Delete(&VBO1);
    EBO_Delete(&EBO1);
    Shader_Delete(&shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}