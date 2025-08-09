
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stb_image/stb_image.h>
#include <stb_image/stb_image_resize.h>

unsigned int windowWidth = 1600;
unsigned int windowHeight = 1000;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 500

#include "opengl/opengl.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_glfw_gl4.h"

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "mylib/delta.h"

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------

// void keyboard_event(struct nk_input* in) {
//     if (in->keyboard.keys[NK_KEY_ENTER].down && in->keyboard.keys[NK_KEY_ENTER].clicked) {
//         printf("Key Enter was pressed\n");
//         return;
//     }
//     return;
// }

static void eror_callback(int e, const char *d) { printf("Error %d: %s", e, d); }

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------

Mesh scene1, capsule, light, billboard, cameraMesh;
Texture tex1;

Camera* viewedCamera = NULL;
CameraVector cameras;

FBO postProcessingFBO[2];

typedef enum {
    ENGINE_EDITOR,
    ENGINE_SCENE_EXPANDED,
    ENGINE_MIDDLE_UI
} EngineState;

EngineState engineState = ENGINE_EDITOR;

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------

void nuklear_ui(struct nk_context* ctx);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;

    glViewport(0, 0, width, height);

    FBO_Resize(&postProcessingFBO[0], width, height);
    FBO_Resize(&postProcessingFBO[1], width, height);

}

void draw_stuff(Shader* shader, Camera* camera) {
    mat4 model;
    
    Camera_UpdateMatrix(camera, windowWidth, windowHeight);

    make_billboard_matrix((vec3){0.0f, 1.5f, 0.0f}, camera->viewMatrix, (vec3){0.5f, 0.5f, 0.5f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&billboard, shader, camera);

    make_model_matrix((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&scene1, shader, camera);

}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Engine", NULL, NULL);
    if (window == NULL) {
        printf("window failed to create\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
    framebuffer_size_callback(window, windowWidth, windowHeight);

    struct nk_context* ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

    {   struct nk_font_atlas* atlas;
        nk_glfw3_font_stash_begin(&atlas);
        nk_glfw3_font_stash_end();
    }

    // SHADERS
    
    Shader shader_default = Shader_Init("shaders/vert/default.vert", "shaders/frag/default.frag", NULL);
    // Shader shader_defaultflat = Shader_Init("shaders/vert/default.vert", "shaders/frag/defaultflat.frag", NULL);
    Shader shader_color = Shader_Init("shaders/vert/default.vert", "shaders/frag/color.frag", NULL);
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
    cameraMesh = Import_loadMeshFromOBJ("res/models/Camera/Camera.obj");

    tex1 = Texture_Init("res/textures/box.png", "diffuse", 0);

    VAO quadVAO = VAO_InitQuad();
    VAO bbVAO = VAO_InitBillboardQuad();
    
    postProcessingFBO[0] = FBO_Init(windowWidth, windowHeight);
    postProcessingFBO[1] = FBO_Init(windowWidth, windowHeight);
    int ping = 0;

    bool postProcessing = false;
    bool wireframe = false;
    int shadowMapFreq = 8;

    glLineWidth(2.0f);

    LightSystem lightSystem = LightSystem_Init(0.15f);

    // CAMERA VECTOR TESTING

    CameraVector_Init(&cameras);
    Camera* cam = Camera_InitHeap(windowWidth, windowHeight, 45.0f, 0.1f, 100.0f, (vec3){-1.93f, 0.73f, -1.75f}, (vec3){0.67f, -0.12f, 0.73f});
    CameraVector_Push(&cameras, cam);
    viewedCamera = CameraVector_Get(&cameras, 0);
    int camNum = 0;

    printf("seconds to load objects %.2fs\n", glfwGetTime());

    while(!glfwWindowShouldClose(window)) {
        
        float dt = deltaTimeUpdate();
        
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        glfwPollEvents();
        glfwJoystickEvents();
            
        LightSystem_Clear(&lightSystem);
        LightSystem_SetDirectLight(&lightSystem, (vec3){cosf(glfwGetTime()/15), -0.4f, sinf(glfwGetTime()/15)}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.5f);
        LightSystem_AddPointLight(&lightSystem, (vec3){sin(glfwGetTime()), 0.5f, cos(glfwGetTime())}, (vec4){1.0f, 0.1f, 0.05f, 1.0f}, 1.0f, 0.04f, 0.5f);
        LightSystem_AddPointLight(&lightSystem, (vec3){-sin(glfwGetTime()), 0.5f, -cos(glfwGetTime())}, (vec4){0.2f, 1.0f, 0.2f, 1.0f}, 1.0f, 0.04f, 0.5f);
        // LightSystem_AddSpotLight(&lightSystem, (vec3){0.0f, 8.5f, 0.0f}, (vec3){0.1f, -1.0f, 0.0f}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.90f, 0.95f, 0.5f);

        int sceneWidth, sceneHeight, sceneX, sceneY;

        if (engineState == ENGINE_SCENE_EXPANDED) {

            sceneWidth  = windowWidth;
            sceneHeight = windowHeight - 30;
            sceneX = 0;
            sceneY = 0;

            viewedCamera->width = sceneWidth;
            viewedCamera->height = sceneHeight;
            
            Camera_Inputs(viewedCamera, window, &joysticks[0], dt);
            Camera_UpdateMatrix(viewedCamera, windowWidth, windowHeight);
            
            LightSystem_MakeShadowMaps(&lightSystem, &shader_shadowMap, viewedCamera, draw_stuff);
            
            FBO_Bind(&postProcessingFBO[ping]);
            glViewport(0, 0, windowWidth, windowHeight);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            glFrontFace(GL_CCW);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            LightSystem_SetUniforms(&shader_default, &lightSystem);
                glUniform1i(glGetUniformLocation(shader_default.ID, "sampleRadius"), 2);
            draw_stuff(&shader_default, viewedCamera);
            LightSystem_Draw(&lightSystem, &light, &shader_lights, viewedCamera);
            
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

            // start a framecount, if the count is the first frame thats when the variables are initialized
            // render everything including fbos, shadows with no delay etc
        }

        if (engineState == ENGINE_EDITOR) {
            sceneWidth  = windowWidth * 8/16;
            sceneHeight = windowHeight * 0.5 - 30;
            sceneX = (windowWidth - sceneWidth) / 2;
            sceneY = windowHeight - sceneHeight - 30;

            viewedCamera->width = sceneWidth;
            viewedCamera->height = sceneHeight;
            
            Camera_Inputs(viewedCamera, window, &joysticks[0], dt);
            Camera_UpdateMatrix(viewedCamera, windowWidth, windowHeight);

            if (joystickIsPressed(&joysticks[0], 5)) {
                camNum += 1;
                camNum = camNum % cameras.size;
                viewedCamera = CameraVector_Get(&cameras, camNum);
            }
            if (joystickIsPressed(&joysticks[0], 4)) {
                camNum -= 1;
                camNum = camNum % cameras.size;
                viewedCamera = CameraVector_Get(&cameras, camNum);
            }
            if (joystickIsPressed(&joysticks[0], 10)) {
                Camera* newCam = Camera_InitHeap(windowWidth, windowHeight, 45.0f, 0.1f, 100.0f, (vec3){0.0f, 1.0f, 0.0f}, viewedCamera->direction);
                CameraVector_Push(&cameras, newCam);
            }
            if (joystickIsPressed(&joysticks[0], 12) && cameras.size > 0 && CameraVector_IndexOf(&cameras, viewedCamera) != 0) {
                CameraVector_Remove(&cameras, viewedCamera);
                viewedCamera = CameraVector_Get(&cameras, 0);
            }
            if (joystickIsPressed(&joysticks[0], 7)) postProcessing = !postProcessing;
            if (joystickIsPressed(&joysticks[0], 6)) wireframe = !wireframe;

            if (!wireframe && (delta.frameCount % shadowMapFreq == 0)) 
                LightSystem_MakeShadowMaps(&lightSystem, &shader_shadowMap, viewedCamera, draw_stuff);
                
            FBO_Bind(&postProcessingFBO[ping]);
            glViewport(0, 0, windowWidth, windowHeight);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            glFrontFace(GL_CCW);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
            if (!wireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                LightSystem_SetUniforms(&shader_default, &lightSystem);
                glUniform1i(glGetUniformLocation(shader_default.ID, "sampleRadius"), 0);
                draw_stuff(&shader_default, viewedCamera);
                LightSystem_Draw(&lightSystem, &light, &shader_lights, viewedCamera);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                Shader_Activate(&shader_color);
                glUniform3fv(glGetUniformLocation(shader_color.ID, "color"), 1, (float[]){1.0f, 0.647f, 0.0f});
                LightSystem_SetUniforms(&shader_color, &lightSystem);
                draw_stuff(&shader_color, viewedCamera);
                LightSystem_Draw(&lightSystem, &light, &shader_color, viewedCamera);
            }

            Shader_Activate(&shader_color);
            glUniform3fv(glGetUniformLocation(shader_color.ID, "color"), 1, (float[]){1.0f, 1.0f, 1.0f});
            CameraVector_Draw(&cameras, &cameraMesh, &shader_color, viewedCamera);

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

        }

        if (engineState == ENGINE_MIDDLE_UI) {
            sceneWidth = sceneHeight = sceneX = sceneY = 0;
        }

        FBO_Unbind();
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(sceneX, sceneY, sceneWidth, sceneHeight);
        Shader_Activate(&shader_framebuffer);
        FBO_BindTexture(&postProcessingFBO[!ping], &shader_framebuffer);
        VAO_DrawQuad(&quadVAO);
        glViewport(0, 0, windowWidth, windowHeight);

        nuklear_ui(ctx);

        glfwSwapBuffers(window);

    }

    glfwDestroyWindow(window);
    nk_glfw3_shutdown();
    glfwTerminate();
    return 0;
}

void nuklear_ui(struct nk_context* ctx) {
    
    nk_glfw3_new_frame();
    
    if (nk_begin(ctx, "Toolbar", nk_rect(0, 0, windowWidth, 30), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR )) {
        // nk_layout_row_static(ctx, 30, 60, 5); // 5 buttons per row

        // if (nk_button_label(ctx, "New")) { /* new action */ }
        // if (nk_button_label(ctx, "Open")) { /* open action */ }
        // if (nk_button_label(ctx, "Save")) { /* save action */ }
        // if (nk_button_label(ctx, "Build")) { /* build action */ }
        // if (nk_button_label(ctx, "Run")) { /* run action */ }

        nk_layout_row_begin(ctx, NK_STATIC, 30, 5); // Height = 30px, max 5 widgets before wrapping

    // Button labels
    const char* labels[] = { "New", "Open", "Save", "Build", "Run" };
    for (int i = 0; i < 5; i++) {
        // Measure text width
        // float textWidth = nk_text_width(ctx->style.font, labels[i], strlen(labels[i]));
        // float padding = ctx->style.button.padding.x * 2; // left + right padding
        // float buttonWidth = textWidth + padding;

        // Push the width for this button
        nk_layout_row_push(ctx, 60);
        if (nk_button_label(ctx, labels[i])) {
            // Handle button click...
        }
    }

    nk_layout_row_end(ctx);
    }
    nk_end(ctx);
                
    if (nk_begin(ctx, "buffer2", nk_rect(200, 200, 100, 200), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE)) {
        
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "buffer2", NK_TEXT_CENTERED);
    
        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "EDITOR"))
            engineState = ENGINE_EDITOR;
            
        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "SCENE"))
            engineState = ENGINE_SCENE_EXPANDED;
            
        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "NONE"))
            engineState = ENGINE_MIDDLE_UI;
    }
    nk_end(ctx);

    if (nk_begin(ctx, "FPS Graph", nk_rect(50, 50, 300, 300), NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE)) {
    
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "FPS Tracker (20s)", NK_TEXT_CENTERED);

        // Show units above the graph
        nk_layout_row_dynamic(ctx, 20, 2);
        nk_label(ctx, "FPS", NK_TEXT_LEFT);
        nk_label(ctx, "Time (ms)", NK_TEXT_LEFT);

        // Graph area for FPS values
        nk_layout_row_dynamic(ctx, 150, 1);
        nk_chart_begin(ctx, NK_CHART_LINES, delta.fpsHistoryCount, 0.0f, 200.0f);

        int start = (delta.fpsHistoryIndex + FPS_HISTORY_COUNT - delta.fpsHistoryCount) % FPS_HISTORY_COUNT;
        for (int i = 0; i < delta.fpsHistoryCount; i++) {
            int idx = (start + i) % FPS_HISTORY_COUNT;
            nk_chart_push(ctx, delta.fpsHistory[idx]);
        }
        nk_chart_end(ctx);

        // Show current FPS and delta time below the graph
        nk_layout_row_dynamic(ctx, 20, 2);

        char buf_fps[64];
        snprintf(buf_fps, sizeof(buf_fps), "Current FPS: %.1f fps", delta.fps);
        nk_label(ctx, buf_fps, NK_TEXT_LEFT);

        char buf_dt[64];
        snprintf(buf_dt, sizeof(buf_dt), "Delta Time: %.2f ms", delta.ms);
        nk_label(ctx, buf_dt, NK_TEXT_LEFT);

    }
    nk_end(ctx);

    nk_glfw3_render(NK_ANTI_ALIASING_OFF);

}