
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stb_image/stb_image.h>

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "opengl/joystick.h"

#include "opengl/mesh.h"
#include "opengl/FBO.h"

unsigned int width = 1600;
unsigned int height = 1000;

Vertex vertices[] = {
    // { { -1.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    // { { -1.0f,  0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
    // { {  1.0f,  0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
    // { {  1.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } }

    { { -0.5f,  0.0f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.0f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 0.0f, 5.0f } },
    { {  0.5f,  0.0f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 5.0f, 5.0f } },
    { {  0.5f,  0.0f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 5.0f, 0.0f } },

    { { -0.5f,  0.0f,  0.5f }, { -0.8f,  0.5f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.0f, -0.5f }, { -0.8f,  0.5f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 5.0f, 0.0f } },
    { {  0.0f,  0.8f,  0.0f }, { -0.8f,  0.5f,  0.0f }, { 0.92f, 0.86f, 0.76f }, { 2.5f, 5.0f } },

    { { -0.5f,  0.0f, -0.5f }, {  0.0f,  0.5f, -0.8f }, { 0.83f, 0.70f, 0.44f }, { 5.0f, 0.0f } },
    { {  0.5f,  0.0f, -0.5f }, {  0.0f,  0.5f, -0.8f }, { 0.83f, 0.70f, 0.44f }, { 0.0f, 0.0f } },
    { {  0.0f,  0.8f,  0.0f }, {  0.0f,  0.5f, -0.8f }, { 0.92f, 0.86f, 0.76f }, { 2.5f, 5.0f } },

    { {  0.5f,  0.0f, -0.5f }, {  0.8f,  0.5f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.0f,  0.5f }, {  0.8f,  0.5f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 5.0f, 0.0f } },
    { {  0.0f,  0.8f,  0.0f }, {  0.8f,  0.5f,  0.0f }, { 0.92f, 0.86f, 0.76f }, { 2.5f, 5.0f } },

    { {  0.5f,  0.0f,  0.5f }, {  0.0f,  0.5f,  0.8f }, { 0.83f, 0.70f, 0.44f }, { 5.0f, 0.0f } },
    { { -0.5f,  0.0f,  0.5f }, {  0.0f,  0.5f,  0.8f }, { 0.83f, 0.70f, 0.44f }, { 0.0f, 0.0f } },
    { {  0.0f,  0.8f,  0.0f }, {  0.0f,  0.5f,  0.8f }, { 0.92f, 0.86f, 0.76f }, { 2.5f, 5.0f } },

};

GLuint indices[] = {
    0, 2, 1,
    0, 3, 2,
    4, 5, 6,
    7, 8, 9,
    10, 11, 12,
    13, 14, 15,
};

Vertex lightVertices[] = {
    { { -0.1f, -0.1f,  0.1f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.1f, -0.1f, -0.1f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.1f, -0.1f, -0.1f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.1f, -0.1f,  0.1f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.1f,  0.1f,  0.1f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.1f,  0.1f, -0.1f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.1f,  0.1f, -0.1f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.1f,  0.1f,  0.1f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }
};

GLuint lightIndices[] = {
    0, 1, 2,
    0, 2, 3,
    0, 4, 7,
    0, 7, 3,
    3, 7, 6,
    3, 6, 2,
    2, 6, 5,
    2, 5, 1,
    1, 5, 4,
    1, 4, 0,
    4, 5, 6,
    4, 6, 7
};

Vertex PLANEvertices[] = {
    { { -1.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -1.0f,  0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
    { {  1.0f,  0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
    { {  1.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } }
};

GLuint PLANEindices[] = {
    0, 1, 2,
    0, 2, 3,
};

GLfloat frameBufferVertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
    -1.0f, -1.0f,   0.0f, 0.0f,  // bottom-left
     1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right

    -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
     1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right
     1.0f,  1.0f,   1.0f, 1.0f   // top-right
};

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------

clock_t previous_time = 0;
float dt;

int frame_count = 0;
float fps_timer = 0.0f;
float fps = 0.0f;

float get_delta_time() {
    clock_t current_time = clock();
    float delta_time = (float)(current_time - previous_time) / CLOCKS_PER_SEC;
    previous_time = current_time;

    frame_count++;
    fps_timer += delta_time;

    if (fps_timer >= 1.0f) {
        fps = frame_count / fps_timer;
        frame_count = 0;
        fps_timer = 0.0f;
        // printf("FPS: %.2f\n", fps);
    }

    return delta_time;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------

void CopyToVertexVector(Vertex* vertices, size_t count, VertexVector* outVec) {
    VertexVector_Init(outVec);
    for (size_t i = 0; i < count; i++) {
        VertexVector_Push(outVec, vertices[i]);
    }
}
void CopyToGLuintVector(GLuint* data, size_t count, GLuintVector* outVec) {
    GLuintVector_Init(outVec);
    for (size_t i = 0; i < count; i++) {
        GLuintVector_Push(outVec, data[i]);
    }
}
void CopyToTextureVector(Texture* textures, size_t count, TextureVector* outVec) {
    TextureVector_Init(outVec);
    for (size_t i = 0; i < count; i++) {
        TextureVector_Push(outVec, textures[i]);
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "opengl window", NULL, NULL);
    if (window == NULL) {
        printf("window failed to create\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glViewport(0, 0, width, height);

    // OTHER SHADERS
    
    Shader glShaderProgram_default3d = Shader_Init("shaders/vert/default3d.vert", "shaders/frag/default3d.frag", NULL);
    Shader glShaderProgram_light3d = Shader_Init("shaders/vert/light3d.vert", "shaders/frag/light3d.frag", NULL);
    
    Shader postFramebuffer = Shader_Init("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/framebuffer.frag", NULL);
    Shader pixelate = Shader_Init("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/pixelate.frag", NULL);
    Shader outline = Shader_Init("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/outline.frag", NULL);

    Texture textures[1];
    textures[0] = Texture_Init("res/textures/sydney.png", "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE);
    VertexVector verts;
    CopyToVertexVector(vertices, sizeof(vertices) / sizeof(Vertex), &verts);
    GLuintVector ind;
    CopyToGLuintVector(indices, sizeof(indices) / sizeof(GLuint), &ind);
    TextureVector tex;
    CopyToTextureVector(textures, sizeof(textures) / sizeof(Texture), &tex);
    Mesh pyramid = Mesh_Init(&verts, &ind, &tex);

    Texture PLANEtextures[2];
    PLANEtextures[0] = Texture_Init("res/textures/box.png", "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE);
    PLANEtextures[1] = Texture_Init("res/textures/box_specular.png", "specular", 1, GL_RED, GL_UNSIGNED_BYTE);
    VertexVector PLANEverts;
    CopyToVertexVector(PLANEvertices, sizeof(PLANEvertices) / sizeof(Vertex), &PLANEverts);
    GLuintVector PLANEind;
    CopyToGLuintVector(PLANEindices, sizeof(PLANEindices) / sizeof(GLuint), &PLANEind);
    TextureVector PLANEtex;
    CopyToTextureVector(PLANEtextures, sizeof(PLANEtextures) / sizeof(Texture), &PLANEtex);
    Mesh floor = Mesh_Init(&PLANEverts, &PLANEind, &PLANEtex);
    
    VertexVector lightVerts;
    CopyToVertexVector(lightVertices, sizeof(lightVertices) / sizeof(Vertex), &lightVerts);
    GLuintVector lightInd;
    CopyToGLuintVector(lightIndices, sizeof(lightIndices) / sizeof(GLuint), &lightInd);
    Mesh light = Mesh_Init(&lightVerts, &lightInd, &tex);

    VAO framebufferVAO = VAO_Init();
    VAO_Bind(&framebufferVAO);
    VBO framebufferVBO = VBO_InitRaw(frameBufferVertices, sizeof(frameBufferVertices));
    VAO_LinkAttrib(&framebufferVBO, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    VAO_LinkAttrib(&framebufferVBO, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();
    
    Framebuffer postProcessingFBO[2];
    postProcessingFBO[0] = Framebuffer_Init(width, height);
    postProcessingFBO[1] = Framebuffer_Init(width, height);
    bool postProcessing = 0;
    int ping = 0;

    // LIGHTS AND MODELS

    vec4 lightColor;
    glm_vec4_copy((vec4){1.0f, 1.0f, 1.0f, 1.0f}, lightColor);

    vec3 pyramidPos;
    mat4 pyramidModel;
    glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, pyramidPos);
    glm_mat4_identity(pyramidModel);
    glm_translate(pyramidModel, pyramidPos);

    Shader_Activate(&glShaderProgram_light3d);
    glUniform4fv(glGetUniformLocation(glShaderProgram_light3d.ID, "lightColor"), 1, (float*)lightColor);
    Shader_Activate(&glShaderProgram_default3d);
    glUniformMatrix4fv(glGetUniformLocation(glShaderProgram_default3d.ID, "model"), 1, GL_FALSE, (float*)pyramidModel);
    glUniform4fv(glGetUniformLocation(glShaderProgram_default3d.ID, "lightColor"), 1, (float*)lightColor);

    Camera camera = Camera_Init(width, height, 2.5f, 3.0f,(vec3){0.0f, 1.0f, 3.0f});

    while(!glfwWindowShouldClose(window)) {
        
        dt = get_delta_time();
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "opengl window: %f FPS", fps);
        glfwSetWindowTitle(window, buffer);

        glfwJoystickEvents();
        if (joystickIsPressed(&joysticks[0], 7)) postProcessing = !postProcessing;
        
        Camera_Inputs(&camera, window, &joysticks[0], dt);
        Camera_UpdateMatrix(&camera, 45.0f, 0.1f, 100.0f);

        Framebuffer_Bind(&postProcessingFBO[ping]);
        glClearColor(0.85f, 0.85f, 0.90f, 1.0f);
        // glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vec3 lightPos;
        mat4 lightModel;
        glm_vec3_copy((vec3){sin(glfwGetTime()), 0.5f, cos(glfwGetTime())}, lightPos);
        glm_mat4_identity(lightModel);
        glm_translate(lightModel, lightPos);
        
        Shader_Activate(&glShaderProgram_light3d);
        glUniformMatrix4fv(glGetUniformLocation(glShaderProgram_light3d.ID, "model"), 1, GL_FALSE, (float*)lightModel);
        Shader_Activate(&glShaderProgram_default3d);
        glUniform3fv(glGetUniformLocation(glShaderProgram_default3d.ID, "lightPos"), 1, (float*)lightPos);
        glUniform1f(glGetUniformLocation(glShaderProgram_default3d.ID, "time"), glfwGetTime());

        // glDisable(GL_DEPTH_TEST);
        // Texture_Bind(&textures[0]);
        // Shader_Activate(&glShaderProgram_raymarch);
        // glUniform1f(glGetUniformLocation(glShaderProgram_raymarch.ID, "time"), glfwGetTime());
        // glUniform3fv(glGetUniformLocation(glShaderProgram_raymarch.ID, "camPos"), 1, (float*)&camera.Position);
        // glUniform3fv(glGetUniformLocation(glShaderProgram_raymarch.ID, "u_cameraOrientation"), 1, (float*)&camera.Orientation);
        // VAO_Bind(&quadVAO);
        // glDrawElements(GL_TRIANGLES, sizeof(QUADindices)/sizeof(int), GL_UNSIGNED_INT, 0);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST);

        Mesh_Draw(&pyramid, &glShaderProgram_default3d, &camera);
        Mesh_Draw(&floor, &glShaderProgram_default3d, &camera);
        Mesh_Draw(&light, &glShaderProgram_light3d, &camera);

        // POST PROCESSING

        ping = !ping;
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        
        if (postProcessing) {
            Framebuffer_Bind(&postProcessingFBO[ping]);
            glClear(GL_COLOR_BUFFER_BIT);
            Shader_Activate(&pixelate);
            Framebuffer_BindTexture(&postProcessingFBO[!ping]);
            glUniform1i(glGetUniformLocation(pixelate.ID, "u_texture"), 0);
            glUniform2f(glGetUniformLocation(pixelate.ID, "resolution"), postProcessingFBO[!ping].width, postProcessingFBO[!ping].height);
            glUniform1f(glGetUniformLocation(pixelate.ID, "pixelSize"), 4.0f);
            VAO_Bind(&framebufferVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            ping = !ping;

            // Framebuffer_Bind(&postProcessingFBO[ping]);
            // glClear(GL_COLOR_BUFFER_BIT);
            // Shader_Activate(&outline);
            // Framebuffer_BindTexture(&postProcessingFBO[!ping]);
            // glUniform1i(glGetUniformLocation(pixelate.ID, "u_texture"), 0);
            // VAO_Bind(&framebufferVAO);
            // glDrawArrays(GL_TRIANGLES, 0, 6);
            // ping = !ping;
        }
        
        Framebuffer_Unbind();
        glClear(GL_COLOR_BUFFER_BIT);
        Shader_Activate(&postFramebuffer);
        Framebuffer_BindTexture(&postProcessingFBO[!ping]);
        glUniform1i(glGetUniformLocation(outline.ID, "u_texture"), 0);
        VAO_Bind(&framebufferVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // VAO_Delete(&VAO1);
    // VBO_Delete(&VBO1);
    // EBO_Delete(&EBO1);
    // Texture_Delete(&sydney);
    for (int i = 0; i < sizeof(textures) / sizeof(Texture); i++) { Texture_Delete(&textures[i]); }
    Shader_Delete(&glShaderProgram_default3d);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}