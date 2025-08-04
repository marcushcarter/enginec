
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stb_image/stb_image.h>
#include <stb_image/stb_image_resize.h>

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "opengl/opengl.h"

// #include "geometry.c"

unsigned int width = 1600;
unsigned int height = 1000;

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


Mesh scene1, capsule, light;

void draw_stuff(Shader* shader, Camera* camera) {
    mat4 model;
    
    make_model_matrix((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&scene1, shader, camera);

    // PLAYER

    make_model_matrix(camera->Position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.2f, 0.2f, 0.2f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&capsule, shader, camera);
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

    // SHADERS
    
    Shader glShaderProgram_default3d = Shader_Init("shaders/vert/default3d.vert", "shaders/frag/default3d.frag", NULL);
    Shader glShaderProgram_light3d = Shader_Init("shaders/vert/light3d.vert", "shaders/frag/light3d.frag", NULL);
    Shader shadowMapProgram = Shader_Init("shaders/vert/shadowMap.vert", "shaders/frag/shadowMap.frag", NULL);
    
    Shader postFBO = Shader_Init("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/framebuffer.frag", NULL);
    Shader pixelate = Shader_Init("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/pixelate.frag", NULL);
    Shader outline = Shader_Init("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/outline.frag", NULL);
    
    Shader spriteShad = Shader_Init("shaders/sprite/sprite.vert", "shaders/sprite/sprite.frag", NULL);

    // MESHES

    const char* lighttextures[] = { "res/textures/box.png", "diffuse" };
    light = Mesh_InitFromData(lighttextures, 1, cubeVertices, sizeof(cubeVertices) / sizeof(Vertex), cubeIndices, sizeof(cubeIndices) / sizeof(GLuint));

    capsule = Import_loadMeshFromOBJ("res/models/capsule.obj");
    scene1 = Import_loadMeshFromOBJ("res/models/Untitled.obj");

    // SPRITES

    VAO quadVAO = VAO_InitQuad();

    // const char* files[] = { "res/textures/gun.png", "res/textures/gun.png" };
    // Sprite sprite = Sprite_Init(files, 2);
    
    FBO postProcessingFBO[2];
    postProcessingFBO[0] = FBO_Init(width, height);
    postProcessingFBO[1] = FBO_Init(width, height);
    int ping = 0;

    bool postProcessing = false;
    bool wireframe = false;

    glLineWidth(5.0f);

    Camera camera = Camera_Init(width, height, 2.5f, 3.0f,(vec3){0.0f, 1.0f, 3.0f}, false);

    LightSystem lightSystem = LightSystem_Init(0.15f);
    
    while(!glfwWindowShouldClose(window)) {

        dt = get_delta_time();
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "opengl window: %f FPS", fps);
        glfwSetWindowTitle(window, buffer);

        glfwJoystickEvents();
        if (joystickIsPressed(&joysticks[0], 7)) postProcessing = !postProcessing;
        if (joystickIsPressed(&joysticks[0], 6)) wireframe = !wireframe;
        
        Camera_Inputs(&camera, window, &joysticks[0], dt);
        Camera_UpdateMatrix(&camera, 45.0f, 0.1f, 100.0f);
        
        LightSystem_Clear(&lightSystem);
        // LightSystem_AddPointLight(&lightSystem, (vec3){sin(glfwGetTime()), 0.5f, cos(glfwGetTime())}, (vec4){1.0f, 0.1f, 0.05f, 1.0f}, 1.0f, 0.04f, 0.5f);
        // LightSystem_AddPointLight(&lightSystem, (vec3){-sin(glfwGetTime()), 0.5f, -cos(glfwGetTime())}, (vec4){0.2f, 1.0f, 0.2f, 1.0f}, 1.0f, 0.04f, 0.5f);
        LightSystem_SetDirectLight(&lightSystem, (vec3){cos(glfwGetTime()/25), -1.0f, sin(glfwGetTime()/25)}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.5f);
        // LightSystem_AddSpotLight(&lightSystem, (vec3){0.0f, 8.5f, 0.0f}, (vec3){0.1f, -1.0f, 0.0f}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.90f, 0.95f, 0.5f);

        LightSystem_MakeShadowMaps(&lightSystem, &shadowMapProgram, &camera, draw_stuff);

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
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (wireframe || glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        LightSystem_SetUniforms(&glShaderProgram_default3d, &lightSystem);

        draw_stuff(&glShaderProgram_default3d, &camera);
        
        LightSystem_DrawLights(&lightSystem, &light, &glShaderProgram_light3d, &camera);

        glDisable(GL_DEPTH_TEST);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // POST PROCESSING

        ping = !ping;
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        
        if (postProcessing || glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
            
            FBO_Bind(&postProcessingFBO[ping]);
            Shader_Activate(&pixelate);
            FBO_BindTexture(&postProcessingFBO[!ping], &pixelate);
            glUniform1f(glGetUniformLocation(pixelate.ID, "pixelSize"), 4.f);
            VAO_DrawQuad(&quadVAO);
            ping = !ping;
        }
        
        FBO_Unbind();
        Shader_Activate(&postFBO);
        FBO_BindTexture(&postProcessingFBO[!ping], &postFBO);
        VAO_DrawQuad(&quadVAO);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}