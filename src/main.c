
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// #include <glm/glm.hpp>
#include <cglm/cglm.h>
// #include <KHR/khrplatform.h>
#include <stb_image/stb_image.h>

#include <stdio.h>
#include <math.h>

#include "opengl/opengl.h"

GLfloat vertices[] = {
//      COORDINATES    |       COLORS        |     TETXURE     //
    -0.5, -0.5,  0.0f,   1.0f,  0.0f,  0.0f,    0.0f,  0.0f,    // Lower left corner
    -0.5,  0.5,  0.0f,   0.0f,  1.0f,  0.0f,    0.0f,  1.0f,    // Upper left corner
     0.5,  0.5,  0.0f,   0.0f,  0.0f,  1.0f,    1.0f,  1.0f,    // Upper right corner
     0.5, -0.5,  0.0f,   1.0f,  1.0f,  1.0f,    1.0f,  0.0f,    // Lower right corner
};

GLuint indices[] = {
    0, 2, 1, // Upper tri
    0, 3, 2, // Lower tri
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

    VAO_LinkVBO(&VAO1, VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
    VAO_LinkVBO(&VAO1, VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    VAO_LinkVBO(&VAO1, VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();
    EBO_Unbind();

    GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

    // TEXTURES

    // int widthImg, heightImg, numColCh;
    // stbi_set_flip_vertically_on_load(true);
    // unsigned char* bytes = stbi_load("res/textures/sydney.png", &widthImg, &heightImg, &numColCh, 0);

    // GLuint texture;
    // glGenTextures(1, &texture);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, texture);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImg, heightImg, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    // glGenerateMipmap(GL_TEXTURE_2D);

    // stbi_image_free(bytes);
    // glBindTexture(GL_TEXTURE_2D, 0);

    Texture sydney = Texture_Init("res/textures/sydney.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    Texture_texUnit(shaderProgram, "tex0", 0);

    // GLuint tex0Uni = glGetUniformLocation(shaderProgram.ID, "tex0");
    // Shader_Activate(&shaderProgram);
    // glUniform1i(tex0Uni, 0);

    // GAME LOOP

    while(!glfwWindowShouldClose(window)) 
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        Shader_Activate(&shaderProgram);
        glUniform1f(uniID, 0.5f);
        Texture_Bind(&sydney);
        // glBindTexture(GL_TEXTURE_2D, texture);
        VAO_Bind(&VAO1);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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