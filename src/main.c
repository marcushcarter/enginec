
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stb_image/stb_image.h>

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "opengl/vector.h"

#include "opengl/mesh.h"
#include "opengl/FBO.h"
#include "opengl/shadowMapFBO.h"
#include "opengl/joystick.h"
#include "opengl/lights.h"

unsigned int width = 1600;
unsigned int height = 1000;

Vertex pyramidVertices[] = {
    // { { -1.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    // { { -1.0f,  0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
    // { {  1.0f,  0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
    // { {  1.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } }

    { { -0.5f,  0.0f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.0f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 0.0f, 1.0f } },
    { {  0.5f,  0.0f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 1.0f, 1.0f } },
    { {  0.5f,  0.0f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 1.0f, 0.0f } },

    { { -0.5f,  0.0f,  0.5f }, { -0.8f,  0.5f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.0f, -0.5f }, { -0.8f,  0.5f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 1.0f, 0.0f } },
    { {  0.0f,  0.8f,  0.0f }, { -0.8f,  0.5f,  0.0f }, { 0.92f, 0.86f, 0.76f }, { 0.5f, 1.0f } },

    { { -0.5f,  0.0f, -0.5f }, {  0.0f,  0.5f, -0.8f }, { 0.83f, 0.70f, 0.44f }, { 1.0f, 0.0f } },
    { {  0.5f,  0.0f, -0.5f }, {  0.0f,  0.5f, -0.8f }, { 0.83f, 0.70f, 0.44f }, { 0.0f, 0.0f } },
    { {  0.0f,  0.8f,  0.0f }, {  0.0f,  0.5f, -0.8f }, { 0.92f, 0.86f, 0.76f }, { 0.5f, 1.0f } },

    { {  0.5f,  0.0f, -0.5f }, {  0.8f,  0.5f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.0f,  0.5f }, {  0.8f,  0.5f,  0.0f }, { 0.83f, 0.70f, 0.44f }, { 1.0f, 0.0f } },
    { {  0.0f,  0.8f,  0.0f }, {  0.8f,  0.5f,  0.0f }, { 0.92f, 0.86f, 0.76f }, { 0.5f, 1.0f } },

    { {  0.5f,  0.0f,  0.5f }, {  0.0f,  0.5f,  0.8f }, { 0.83f, 0.70f, 0.44f }, { 1.0f, 0.0f } },
    { { -0.5f,  0.0f,  0.5f }, {  0.0f,  0.5f,  0.8f }, { 0.83f, 0.70f, 0.44f }, { 0.0f, 0.0f } },
    { {  0.0f,  0.8f,  0.0f }, {  0.0f,  0.5f,  0.8f }, { 0.92f, 0.86f, 0.76f }, { 0.5f, 1.0f } },

};

GLuint pyramidIndices[] = {
    0, 2, 1,
    0, 3, 2,
    4, 5, 6,
    7, 8, 9,
    10, 11, 12,
    13, 14, 15,
};

Vertex cubeVertices[] = {
    { { -1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
    { {  1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
    { {  1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
    { { -1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
    { {  1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
    { {  1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 0.5f, 0.5f, 0.5f }, { 1.0f, 0.0f } }
};

GLuint cubeIndices[] = {
    0, 2, 1,
    0, 3, 2,
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

Vertex planeVertices[] = {
    { { -1.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -1.0f,  0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
    { {  1.0f,  0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
    { {  1.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } }
};

GLuint planeIndices[] = {
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

Mesh ground, pyramid, light;

void make_model_matrix(vec3 translation, vec3 rotation, vec3 scale, mat4 dest) {
    mat4 trans, rotX, rotY, rotZ, rot, scl;

    glm_translate_make(trans, translation);

    glm_rotate_make(rotX, rotation[0], (vec3){1.0f, 0.0f, 0.0f});
    glm_rotate_make(rotY, rotation[1], (vec3){0.0f, 1.0f, 0.0f});
    glm_rotate_make(rotZ, rotation[2], (vec3){0.0f, 0.0f, 1.0f});

    glm_mat4_mul(rotY, rotX, rot);    // R = Ry * Rx
    glm_mat4_mul(rotZ, rot, rot);     // R = Rz * (Ry * Rx)

    glm_scale_make(scl, scale);

    mat4 rs;
    glm_mat4_mul(rot, scl, rs);       // RS = R * S
    glm_mat4_mul(trans, rs, dest);    // M = T * (R * S)
}

void draw_stuff(Shader* shader, Camera* camera) {
    mat4 model;

    make_model_matrix((vec3){0.0f, 0.5f, 0.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&pyramid, shader, camera);
    
    make_model_matrix((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&ground, shader, camera);

    // PLAYER

    make_model_matrix(camera->Position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.2f, 0.2f, 0.2f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&pyramid, shader, camera);
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
    Shader shadowMapProgram = Shader_Init("shaders/vert/shadowMap.vert", "shaders/frag/shadowMap.frag", NULL);
    
    Shader postFBO = Shader_Init("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/framebuffer.frag", NULL);
    Shader pixelate = Shader_Init("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/pixelate.frag", NULL);
    Shader outline = Shader_Init("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/outline.frag", NULL);

    Texture textures[1];
    textures[0] = Texture_Init("res/textures/brick.jpg", "diffuse", 0);
    VertexVector verts;
    VertexVector_Copy(pyramidVertices, sizeof(pyramidVertices) / sizeof(Vertex), &verts);
    GLuintVector ind;
    GLuintVector_Copy(pyramidIndices, sizeof(pyramidIndices) / sizeof(GLuint), &ind);
    TextureVector tex;
    TextureVector_Copy(textures, sizeof(textures) / sizeof(Texture), &tex);
    pyramid = Mesh_Init(&verts, &ind, &tex);

    Texture PLANEtextures[2];
    PLANEtextures[0] = Texture_Init("res/textures/box.png", "diffuse", 0);
    PLANEtextures[1] = Texture_Init("res/textures/box_specular.png", "specular", 1);
    VertexVector PLANEverts;
    VertexVector_Copy(planeVertices, sizeof(planeVertices) / sizeof(Vertex), &PLANEverts);
    GLuintVector PLANEind;
    GLuintVector_Copy(planeIndices, sizeof(planeIndices) / sizeof(GLuint), &PLANEind);
    TextureVector PLANEtex;
    TextureVector_Copy(PLANEtextures, sizeof(PLANEtextures) / sizeof(Texture), &PLANEtex);
    ground = Mesh_Init(&PLANEverts, &PLANEind, &PLANEtex);
    
    VertexVector lightVerts;
    VertexVector_Copy(cubeVertices, sizeof(cubeVertices) / sizeof(Vertex), &lightVerts);
    GLuintVector lightInd;
    GLuintVector_Copy(cubeIndices, sizeof(cubeIndices) / sizeof(GLuint), &lightInd);
    light = Mesh_Init(&lightVerts, &lightInd, &tex);

    // FRAMEBUFFER

    VAO framebufferVAO = VAO_Init();
    VAO_Bind(&framebufferVAO);
    VBO framebufferVBO = VBO_InitRaw(frameBufferVertices, sizeof(frameBufferVertices));
    VAO_LinkAttrib(&framebufferVBO, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    VAO_LinkAttrib(&framebufferVBO, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();
    
    FBO postProcessingFBO[2];
    postProcessingFBO[0] = FBO_Init(width, height);
    postProcessingFBO[1] = FBO_Init(width, height);
    bool postProcessing = false;
    int ping = 0;

    ShadowMapFBO shadowFBO = ShadowMapFBO_Init(4096, 4096);

    Camera camera = Camera_Init(width, height, 2.5f, 3.0f,(vec3){0.0f, 1.0f, 3.0f});

    LightSystem staticLights = LightSystem_Init(0.0f);
    staticLights.directlight.shadowFBO = ShadowMapFBO_Init(1000, 1000);
    
    while(!glfwWindowShouldClose(window)) {

        dt = get_delta_time();
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "opengl window: %f FPS", fps);
        glfwSetWindowTitle(window, buffer);

        glfwJoystickEvents();
        if (joystickIsPressed(&joysticks[0], 7)) postProcessing = !postProcessing;
        
        Camera_Inputs(&camera, window, &joysticks[0], dt);
        Camera_UpdateMatrix(&camera, 45.0f, 0.1f, 100.0f);
        
        LightSystem dynamicLights = LightSystem_Init(0.0f);
        // LightSystem_AddPointLight(&dynamicLights, (vec3){sin(glfwGetTime()), 1.0f, cos(glfwGetTime())}, (vec4){1.0f, 0.1f, 0.05f, 1.0f}, 1.0f, 0.04f, 0.5f);
        // LightSystem_AddPointLight(&dynamicLights, (vec3){-sin(glfwGetTime()), 1.0f, -cos(glfwGetTime())}, (vec4){0.2f, 1.0f, 0.2f, 1.0f}, 1.0f, 0.04f, 0.5f);
        // LightSystem_AddSpotLight(&dynamicLights, (camera.Position), (camera.Orientation), (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.90f, 0.95f, 0.5f);
        LightSystem_SetDirectLight(&staticLights, (vec3){sin(glfwGetTime()), -0.5f, cos(glfwGetTime())}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.5f);
        // LightSystem_SetDirectLight(&staticLights, (vec3){1.0f, -0.5f, 1.0f}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.5f);

        LightSystem mergedLights = LightSystem_Init(0.0f);
        LightSystem_Merge(&mergedLights, &staticLights, &dynamicLights);

        LightSystem_MakeShadowMaps(&mergedLights, &shadowMapProgram, &camera, draw_stuff);

        glViewport(0, 0, width, height);
        FBO_Bind(&postProcessingFBO[ping]);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

        LightSystem_SetUniforms(&glShaderProgram_default3d, &mergedLights);

        draw_stuff(&glShaderProgram_default3d, &camera);
        
        LightSystem_DrawLights(&mergedLights, &light, &glShaderProgram_light3d, &camera);

        // POST PROCESSING

        ping = !ping;
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        
        if (postProcessing) {
            FBO_Bind(&postProcessingFBO[ping]);
            glClear(GL_COLOR_BUFFER_BIT);
            Shader_Activate(&pixelate);
            FBO_BindTexture(&postProcessingFBO[!ping]);
            glUniform1i(glGetUniformLocation(pixelate.ID, "u_texture"), 0);
            glUniform2f(glGetUniformLocation(pixelate.ID, "resolution"), postProcessingFBO[!ping].width, postProcessingFBO[!ping].height);
            glUniform1f(glGetUniformLocation(pixelate.ID, "pixelSize"), 4.0f);
            VAO_Bind(&framebufferVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            ping = !ping;

            // FBO_Bind(&postProcessingFBO[ping]);
            // glClear(GL_COLOR_BUFFER_BIT);
            // Shader_Activate(&outline);
            // FBO_BindTexture(&postProcessingFBO[!ping]);
            // glUniform1i(glGetUniformLocation(pixelate.ID, "u_texture"), 0);
            // VAO_Bind(&framebufferVAO);
            // glDrawArrays(GL_TRIANGLES, 0, 6);
            // ping = !ping;
        }
        
        FBO_Unbind();
        glClear(GL_COLOR_BUFFER_BIT);
        Shader_Activate(&postFBO);
        FBO_BindTexture(&postProcessingFBO[!ping]);
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