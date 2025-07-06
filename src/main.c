
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// #include <glm/glm.hpp>
#include <cglm/cglm.h>
// #include <KHR/khrplatform.h>

#include <stdio.h>

#include "shader_class.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

int main() 
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLfloat vertices[] = 
    {
        -0.5f, -0.5f * (float)(sqrt(3)) / 3, 0.0f,
         0.5f, -0.5f * (float)(sqrt(3)) / 3, 0.0f,
         0.0f,  0.5f * (float)(sqrt(3)) * 2 / 3, 0.0f,
        -0.5f / 2, 0.5f * (float)(sqrt(3)) / 6, 0.0f,
         0.5f / 2,  0.5f * (float)(sqrt(3)) / 6, 0.0f,
         0.0f, -0.5f * (float)(sqrt(3)) / 3, 0.0f,
    };

    GLuint indices[] = {
        0, 3, 5, 
        3, 2, 4,
        5, 4, 1,
    };

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

    // INITIALIZE SHADERS

    // GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // glCompileShader(vertexShader);
    
    // GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    // glCompileShader(fragmentShader);

    // GLuint shaderProgram = glCreateProgram();
    // glAttachShader(shaderProgram, vertexShader);
    // glAttachShader(shaderProgram, fragmentShader);
    // glLinkProgram(shaderProgram);

    // glDeleteShader(vertexShader);
    // glDeleteShader(fragmentShader);

    Shader shaderProgram = Shader_Init("shaders/default.vert", "shaders/default.frag");


    // BUFFERS

    // GLuint VAO, VBO, EBO;

    // glGenVertexArrays(1, &VAO);
    // glGenBuffers(1, &VBO);
    // glGenBuffers(1, &EBO);


    // glBindVertexArray(VAO);

    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);

    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindVertexArray(0);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    VAO VAO1 = VAO_Init();
    VAO_Bind(&VAO1);

    VBO VBO1 = VBO_Init(vertices, sizeof(vertices));
    EBO EBO1 = EBO_Init(indices, sizeof(indices));

    VAO_LinkVBO(&VAO1, VBO1, 0);
    VAO_Unbind();
    VBO_Unbind();
    EBO_Unbind();

    











    


    while(!glfwWindowShouldClose(window)) 
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        Shader_Activate(&shaderProgram);
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