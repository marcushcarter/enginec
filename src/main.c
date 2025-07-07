
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// #include <glm/glm.hpp>
#include <cglm/cglm.h>
// #include <KHR/khrplatform.h>
#include <stb_image/stb_image.h>

#include <stdio.h>
#include <math.h>

#include "opengl/opengl.h"

const unsigned int width = 800;
const unsigned int height = 800;

GLfloat vertices[] = {
//      COORDINATES    |       COLORS        |     TETXURE  
    -0.5f,  0.0f,  0.5f,   0.83f,  0.70f,  0.44f,    0.0f,  0.0f,
    -0.5f,  0.0f, -0.5f,   0.83f,  0.70f,  0.44f,    5.0f,  0.0f,
     0.5f,  0.0f, -0.5f,   0.83f,  0.70f,  0.44f,    0.0f,  0.0f,
     0.5f,  0.0f,  0.5f,   0.83f,  0.70f,  0.44f,    5.0f,  0.0f,
     0.0f,  0.8f,  0.0f,   0.92f,  0.86f,  0.76f,    2.5f,  5.0f,
};

GLuint indices[] = {
    0, 1, 2,
    0, 2, 3,
    0, 1, 4,
    1, 2, 4,
    2, 3, 4,
    3, 0, 4,
};

int main() 
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(width, height, "opengl window", NULL, NULL);
    if (window == NULL) 
    {
        printf("window failed to create\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glViewport(0, 0, width, height);

    // SHADERS

    Shader shaderProgram = Shader_Init("shaders/default.vert", "shaders/default.frag");

    // BUFFERS

    VAO VAO1 = VAO_Init();
    VAO_Bind(&VAO1);

    VBO VBO1 = VBO_Init(vertices, sizeof(vertices));
    EBO EBO1 = EBO_Init(indices, sizeof(indices));

    VAO_LinkAttrib(&VAO1, &VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
    VAO_LinkAttrib(&VAO1, &VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    VAO_LinkAttrib(&VAO1, &VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();
    EBO_Unbind();

    GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

    // TEXTURES

    Texture sydney = Texture_Init("res/textures/sydney.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    Texture_texUnit(&shaderProgram, "tex0", 0);

    // GAME LOOP

    float rotation = 0.0f;
    double prevTime = glfwGetTime();

    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window)) 
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Shader_Activate(&shaderProgram);

        double crntTime = glfwGetTime();
        if (crntTime - prevTime >= 1 / 60) {
            rotation += 0.05f;
            prevTime = crntTime;
        }

        mat4 model; glm_mat4_identity(model); glm_mat4_scale(model, 1.0f);
        mat4 view; glm_mat4_identity(view);
        mat4 proj; glm_mat4_identity(proj);

        vec3 axis = { 0.0f, 1.0f, 0.0f }; glm_rotate(model, glm_rad(rotation), axis);
        vec3 translation = {0.0f, -0.5f, -2.0f}; glm_translate(view, translation);
        glm_perspective(glm_rad(45.0f), (float)(width / height), 0.1f, 100.0f, proj);

        int modelLoc = glGetUniformLocation(shaderProgram.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)model);
        int viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float*)view);
        int projLoc = glGetUniformLocation(shaderProgram.ID, "proj");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, (float*)proj);

        glUniform1f(uniID, 0.5f);
        Texture_Bind(&sydney);
        VAO_Bind(&VAO1);
        glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(int), GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    VAO_Delete(&VAO1);
    VBO_Delete(&VBO1);
    EBO_Delete(&EBO1);
    Texture_Delete(&sydney);
    Shader_Delete(&shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}