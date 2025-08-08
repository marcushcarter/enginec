
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stb_image/stb_image.h>
#include <stb_image/stb_image_resize.h>

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "opengl/opengl.h"

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

Texture tex1;

Mesh scene1, capsule, light, billboard;

Camera* activeCamera = NULL;

CameraVector cameras;

void draw_stuff(Shader* shader, Camera* camera) {
    mat4 model;
    
    Camera_UpdateMatrix(camera, 45.0f, 0.1f, 100.0f);

    make_billboard_matrix((vec3){0.0f, 1.5f, 0.0f}, camera->viewMatrix, (vec3){0.5f, 0.5f, 0.5f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&billboard, shader, camera);

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
    
    Shader shader_default = Shader_Init("shaders/vert/default.vert", "shaders/frag/default.frag", NULL);
    Shader shader_wires = Shader_Init("shaders/vert/default.vert", "shaders/frag/wires.frag", NULL);
    Shader shader_lights = Shader_Init("shaders/vert/lights.vert", "shaders/frag/lights.frag", NULL);
    Shader shader_shadowMap = Shader_Init("shaders/vert/shadowMap.vert", "shaders/frag/blank.frag", NULL);
    
    Shader shader_framebuffer = Shader_Init("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/framebuffer.frag", NULL);
    Shader shader_pixelate = Shader_Init("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/pixelate.frag", NULL);
    Shader shader_outline = Shader_Init("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/outline.frag", NULL);

    // MESHES

    const char* lighttextures[] = { "res/textures/box.png", "diffuse" };
    light = Mesh_InitFromData(lighttextures, 1, cubeVertices, sizeof(cubeVertices) / sizeof(Vertex), cubeIndices, sizeof(cubeIndices) / sizeof(GLuint));

    capsule = Import_loadMeshFromOBJ("res/models/capsule.obj");
    scene1 = Import_loadMeshFromOBJ("res/models/Untitled.obj");
    billboard = Import_loadMeshFromOBJ("res/models/billboard.obj");

    tex1 = Texture_Init("res/textures/box.png", "diffuse", 0);

    // SPRITES

    VAO quadVAO = VAO_InitQuad();
    VAO bbVAO = VAO_InitBillboardQuad();
    
    FBO postProcessingFBO[2];
    postProcessingFBO[0] = FBO_Init(width, height);
    postProcessingFBO[1] = FBO_Init(width, height);
    int ping = 0;

    bool postProcessing = false;
    bool wireframe = true;

    glLineWidth(2.0f);

    // Camera defaultCamera = Camera_InitStack(width, height, 2.5f, 3.0f,(vec3){0.0f, 1.0f, 3.0f});
    // Camera secondCam = Camera_InitStack(width, height, 2.5f, 3.0f,(vec3){0.0f, 1.0f, 3.0f});

    LightSystem lightSystem = LightSystem_Init(0.15f);

    // CAMERA VECTOR TESTING
    CameraVector_Init(&cameras);
    Camera* cam1 = Camera_InitHeap(width, height, 2.5f, 3.0f,(vec3){0.0f, 1.0f, 3.0f});
    CameraVector_Push(&cameras, cam1);
    Camera* cam2 = Camera_InitHeap(width, height, 2.5f, 3.0f,(vec3){0.0f, 1.0f, 3.0f});
    CameraVector_Push(&cameras, cam2);

    activeCamera = CameraVector_Get(&cameras, 0);

    // Camera* cam = Camera_InitHeap(800, 600, 0.05f, 100.0f, position);
    // CameraVector_Push(&cameras, cam);
    
    while(!glfwWindowShouldClose(window)) {

        dt = get_delta_time();
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "opengl window: %f FPS", fps);
        glfwSetWindowTitle(window, buffer);

        glfwJoystickEvents();
        if (joystickIsPressed(&joysticks[0], 7)) postProcessing = !postProcessing;
        if (joystickIsPressed(&joysticks[0], 6)) wireframe = !wireframe;
        
        Camera_Inputs(CameraVector_Get(&cameras, 0), window, &joysticks[0], dt);
        Camera_UpdateMatrix(CameraVector_Get(&cameras, 0), 45.0f, 0.1f, 100.0f);

        if (glfwGetKey(window, GLFW_KEY_5)) {
            activeCamera = CameraVector_Get(&cameras, 0);
        }
        
        if (glfwGetKey(window, GLFW_KEY_6)) {
            activeCamera = CameraVector_Get(&cameras, 1);
        }
        
        LightSystem_Clear(&lightSystem);
        // LightSystem_AddPointLight(&lightSystem, (vec3){sin(glfwGetTime()), 0.5f, cos(glfwGetTime())}, (vec4){1.0f, 0.1f, 0.05f, 1.0f}, 1.0f, 0.04f, 0.5f);
        // LightSystem_AddPointLight(&lightSystem, (vec3){-sin(glfwGetTime()), 0.5f, -cos(glfwGetTime())}, (vec4){0.2f, 1.0f, 0.2f, 1.0f}, 1.0f, 0.04f, 0.5f);
        LightSystem_SetDirectLight(&lightSystem, (vec3){cos(glfwGetTime()/15), -0.4f, sin(glfwGetTime()/15)}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.5f);
        // LightSystem_AddSpotLight(&lightSystem, (vec3){0.0f, 8.5f, 0.0f}, (vec3){0.1f, -1.0f, 0.0f}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.90f, 0.95f, 0.5f);

        if (!wireframe) LightSystem_MakeShadowMaps(&lightSystem, &shader_shadowMap, activeCamera, draw_stuff);
            
        glViewport(0, 0, width, height);
        FBO_Bind(&postProcessingFBO[ping]);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (!wireframe) {
            // Camera_Draws(&cameras, &capsule, &wireframe, activeCamera);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            LightSystem_SetUniforms(&shader_default, &lightSystem);
            draw_stuff(&shader_default, activeCamera);
            LightSystem_Draw(&lightSystem, &light, &shader_lights, activeCamera);
        } else {
            // Camera_Draws(&cameras, &capsule, &wireframe, activeCamera);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            LightSystem_SetUniforms(&shader_wires, &lightSystem);
            draw_stuff(&shader_wires, activeCamera);
            LightSystem_Draw(&lightSystem, &light, &shader_wires, activeCamera);
        }

        CameraVector_Draw(&cameras, &capsule, &shader_wires, activeCamera);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // POST PROCESSING

        ping = !ping;
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        
        if (postProcessing || glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
            
            FBO_Bind(&postProcessingFBO[ping]);
            Shader_Activate(&shader_pixelate);
            FBO_BindTexture(&postProcessingFBO[!ping], &shader_pixelate);
            glUniform1f(glGetUniformLocation(shader_pixelate.ID, "pixelSize"), 4.f);
            VAO_DrawQuad(&quadVAO);
            ping = !ping;
        }
        
        FBO_Unbind();
        Shader_Activate(&shader_framebuffer);
        FBO_BindTexture(&postProcessingFBO[!ping], &shader_framebuffer);
        VAO_DrawQuad(&quadVAO);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}