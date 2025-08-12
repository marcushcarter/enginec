#include "engine/engine_internal.h"
#include "engine/geometry.h"

#define WINDOW_WIDTH 1440
#define WINDOW_HEIGHT 900

int sceneWidth, sceneHeight, sceneX, sceneY;

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

struct nk_image my_image;

#include <stdio.h>
#include <math.h>
#include <time.h>

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------

struct nk_context* ctx;

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------

BE_Mesh scene1, capsule, light, billboard, cameraMesh;
BE_Texture tex1;

GLFWwindow* window;
int windowWidth, windowHeight;

BE_FBO FBOs[2];
    
BE_EngineState state;

BE_Camera* selectedCamera;
BE_CameraVector cameras;

BE_FrameStats timer;

BE_Joystick joystick;

bool postProcessing = false;
bool wireframe = false;
int shadowMapFreq = 8;

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------

void nuklear_ui(struct nk_context* ctx);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;

    glViewport(0, 0, width, height);

    BE_FBOResize(&FBOs[0], width, height);
    BE_FBOResize(&FBOs[1], width, height);

}

void draw_stuff(BE_Shader* shader, BE_Camera* camera) {
    mat4 model;
    
    // BE_CameraUpdateMatrix(camera, windowWidth, windowHeight);

    // BE_MatrixMakeBillboard((vec3){0.0f, 1.5f, 0.0f}, camera->viewMatrix, (vec3){0.5f, 0.5f, 0.5f}, model);
    // glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    // BE_MeshDraw(&billboard, shader, camera);

    BE_MatrixMakeModel((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    BE_MeshDraw(&scene1, shader, camera);

}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Engine", NULL, NULL);
    if (window == NULL) {
        printf("window failed to create\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
    framebuffer_size_callback(window, windowWidth, windowHeight);

    ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

    {   struct nk_font_atlas* atlas;
        nk_glfw3_font_stash_begin(&atlas);
        nk_glfw3_font_stash_end();
    }

    // SHADERS
    
    BE_Shader shader_default = BE_ShaderInit("shaders/vert/default.vert", "shaders/frag/default.frag", NULL);
    BE_Shader shader_color = BE_ShaderInit("shaders/vert/default.vert", "shaders/frag/color.frag", NULL);
    BE_Shader shader_lights = BE_ShaderInit("shaders/vert/lights.vert", "shaders/frag/lights.frag", NULL);
    BE_Shader shader_shadowMap = BE_ShaderInit("shaders/vert/shadowMap.vert", "shaders/frag/blank.frag", NULL);
    
    BE_Shader shader_framebuffer = BE_ShaderInit("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/framebuffer.frag", NULL);
    BE_Shader shader_pixelate = BE_ShaderInit("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/pixelate.frag", NULL);
    BE_Shader shader_outline = BE_ShaderInit("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/outline.frag", NULL);

    // MESHES

    const char* lighttextures[] = { "res/textures/box.png", "diffuse" };
    light = BE_MeshInitFromData(lighttextures, 1, cubeVertices, cubeVertexCount, cubeIndices, cubeIndexCount);

    capsule = BE_LoadOBJToMesh("res/models/capsule.obj");
    scene1 = BE_LoadOBJToMesh("res/models/Untitled.obj");
    billboard = BE_LoadOBJToMesh("res/models/billboard.obj");
    cameraMesh = BE_LoadOBJToMesh("res/models/Camera/Camera.obj");

    tex1 = BE_TextureInit("res/textures/box.png", "diffuse", 0);

    BE_VAO quadVAO = BE_VAOInitQuad();
    BE_VAO bbVAO = BE_VAOInitBillboardQuad();
    
    FBOs[0] = BE_FBOInit(windowWidth, windowHeight);
    FBOs[1] = BE_FBOInit(windowWidth, windowHeight);
    int ping = 0;

    glLineWidth(2.0f);

    BE_LightManager lightSystem = BE_LightManagerInit(0.15f);

    // CAMERA VECTOR TESTING

    BE_CameraVectorInit(&cameras);
    BE_Camera* cam = BE_CameraInitHeap(windowWidth, windowHeight, 45.0f, 0.1f, 100.0f, (vec3){-1.93f, 0.73f, -1.75f}, (vec3){0.67f, -0.12f, 0.73f});
    BE_CameraVectorPush(&cameras, cam);
    selectedCamera = BE_CameraVectorGetByIndex(&cameras, 0);

    printf("seconds to load objects %.2fs\n", glfwGetTime());

    while(!glfwWindowShouldClose(window)) {

        BE_UpdateFrameTimeInfo(&timer);

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Engine %.1f fps %.2f ms", timer.fps, timer.ms);
        glfwSetWindowTitle(window, buffer);
        
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        glfwPollEvents();
        BE_JoystickUpdate(&joystick);
        // glfwJoystickEvents();
            
        BE_LightManagerClear(&lightSystem);
        BE_LightManagerSetDirectLight(&lightSystem, (vec3){cosf(glfwGetTime()/15), -0.4f, sinf(glfwGetTime()/15)}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.5f);
        BE_LightManagerAddPointLight(&lightSystem, (vec3){sin(glfwGetTime()), 0.5f, cos(glfwGetTime())}, (vec4){1.0f, 0.1f, 0.05f, 1.0f}, 1.0f, 0.04f, 0.5f);
        BE_LightManagerAddPointLight(&lightSystem, (vec3){-sin(glfwGetTime()), 0.5f, -cos(glfwGetTime())}, (vec4){0.2f, 1.0f, 0.2f, 1.0f}, 1.0f, 0.04f, 0.5f);
        // BE_LightManagerAddSpotLight(&lightSystem, (vec3){0.0f, 8.5f, 0.0f}, (vec3){0.1f, -1.0f, 0.0f}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.90f, 0.95f, 0.5f);

        if (state == ENGINE_SCENE_EXPANDED) {

            sceneWidth  = windowWidth;
            sceneHeight = windowHeight - 30;
            sceneX = 0;
            sceneY = 0;

            selectedCamera->width = sceneWidth;
            selectedCamera->height = sceneHeight;
            
            BE_CameraInputs(selectedCamera, window, &joystick, timer.dt);
            BE_CameraUpdateMatrix(selectedCamera, windowWidth, windowHeight);
            
            BE_LightManagerMakeShadowMaps(&lightSystem, &shader_shadowMap, selectedCamera, draw_stuff);
            
            BE_FBOBind(&FBOs[ping]);
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

            BE_LightManagerSetUniforms(&shader_default, &lightSystem);
                glUniform1i(glGetUniformLocation(shader_default.ID, "sampleRadius"), 2);
            draw_stuff(&shader_default, selectedCamera);
            BE_LightManagerDraw(&lightSystem, &light, &shader_lights, selectedCamera);
            
            ping = !ping;
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            
            if (postProcessing || glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
                
                BE_FBOBind(&FBOs[ping]);
                BE_ShaderActivate(&shader_pixelate);
                BE_FBOBindTexture(&FBOs[!ping], &shader_pixelate);
                glUniform1f(glGetUniformLocation(shader_pixelate.ID, "pixelSize"), 4.f);
                BE_VAODrawQuad(&quadVAO);
                ping = !ping;
            }

            // start a framecount, if the count is the first frame thats when the variables are initialized
            // render everything including fbos, shadows with no delay etc
        }

        if (state == ENGINE_EDITOR) {
            sceneWidth  = windowWidth * 10 / 16;
            sceneHeight = windowHeight * 7 / 10 - 30;
            sceneX = (windowWidth - sceneWidth) / 2 - windowWidth * 1 / 16;
            sceneY = windowHeight - sceneHeight - 30;

            selectedCamera->width = sceneWidth;
            selectedCamera->height = sceneHeight;
            
            BE_CameraInputs(selectedCamera, window, &joystick, timer.dt);
            BE_CameraUpdateMatrix(selectedCamera, windowWidth, windowHeight);

            // if (BE_JoystickIsPressed(&joystick, 5)) {
            //     camNum += 1;
            //     camNum = camNum % cameras.size;
            //     selectedCamera = BE_CameraVectorGetByIndex(&cameras, camNum);
            // }
            // if (BE_JoystickIsPressed(&joystick, 4)) {
            //     camNum -= 1;
            //     camNum = camNum % cameras.size;
            //     selectedCamera = BE_CameraVectorGetByIndex(&cameras, camNum);
            // }
            // if (BE_JoystickIsPressed(&joystick, 10)) {
            //     Camera* newCam = BE_CameraInitHeap(windowWidth, windowHeight, 45.0f, 0.1f, 100.0f, (vec3){0.0f, 1.0f, 0.0f}, selectedCamera->direction);
            //     BE_CameraVectorPush(&cameras, newCam);
            // }
            // if (BE_JoystickIsPressed(&joystick, 12) && cameras.size > 0 && BE_CameraVectorGetIndex(&cameras, selectedCamera) != 0) {
            //     BE_CameraVectorRemove(&cameras, selectedCamera);
            //     selectedCamera = BE_CameraVectorGetByIndex(&cameras, 0);
            // }
            
            if (BE_JoystickIsPressed(&joystick, 10)) postProcessing = !postProcessing;
            if (BE_JoystickIsPressed(&joystick, 11)) wireframe = !wireframe;

            if (!wireframe && (timer.frameCount % shadowMapFreq == 0)) 
                BE_LightManagerMakeShadowMaps(&lightSystem, &shader_shadowMap, selectedCamera, draw_stuff);
                
            BE_FBOBind(&FBOs[ping]);
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

                BE_LightManagerSetUniforms(&shader_default, &lightSystem);
                glUniform1i(glGetUniformLocation(shader_default.ID, "sampleRadius"), 0);
                draw_stuff(&shader_default, selectedCamera);
                BE_LightManagerDraw(&lightSystem, &light, &shader_lights, selectedCamera);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                BE_ShaderActivate(&shader_color);
                glUniform3fv(glGetUniformLocation(shader_color.ID, "color"), 1, (float[]){1.0f, 0.647f, 0.0f});
                BE_LightManagerSetUniforms(&shader_color, &lightSystem);
                draw_stuff(&shader_color, selectedCamera);
                BE_LightManagerDraw(&lightSystem, &light, &shader_color, selectedCamera);
            }

            BE_ShaderActivate(&shader_color);
            glUniform3fv(glGetUniformLocation(shader_color.ID, "color"), 1, (float[]){1.0f, 1.0f, 1.0f});
            BE_CameraVectorDraw(&cameras, &cameraMesh, &shader_color, selectedCamera);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // POST PROCESSING

            ping = !ping;
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            
            if (postProcessing || glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
                
                BE_FBOBind(&FBOs[ping]);
                BE_ShaderActivate(&shader_pixelate);
                BE_FBOBindTexture(&FBOs[!ping], &shader_pixelate);
                glUniform1f(glGetUniformLocation(shader_pixelate.ID, "pixelSize"), 4.f);
                BE_VAODrawQuad(&quadVAO);
                ping = !ping;
            }

        }

        if (state == ENGINE_SCENE_HIDDEN) {
            sceneWidth  = windowWidth * 8 / 16;
            sceneHeight = windowHeight * 0.55 - 30;
            sceneX = (windowWidth - sceneWidth) / 2 - windowWidth * 1 / 16;
            sceneY = windowHeight - sceneHeight - 30;
        }

        BE_FBOUnbind();
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(sceneX, sceneY, sceneWidth, sceneHeight);
        BE_ShaderActivate(&shader_framebuffer);
        BE_FBOBindTexture(&FBOs[!ping], &shader_framebuffer);
        BE_VAODrawQuad(&quadVAO);
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
    
    /* TOOLBAR */
    if (nk_begin(ctx, "Toolbar", nk_rect(0, 0, windowWidth, 30), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR )) {
        nk_layout_row_static(ctx, 30, 60, 5); // 5 buttons per row

        if (nk_button_label(ctx, "File")) { /* New, Open, Save, Save As, Save Copy, Save Incremental, Import, Export, Quit */ }
        if (nk_button_label(ctx, "Edit")) { /* Undo, Redo, Lock/Unlock Project, Preferences*/ }
        if (nk_button_label(ctx, "Window")) { /* Open 3D Viewport, Save Screenshot, Save Screenshot (editor), Toggle System Console, Toggle Window Fullscreen, SHow status bar */ }
        if (nk_button_label(ctx, "Help")) { /* Release Notes, Documentation, Support, Tutorials, Report a Bug */ }
    }
    nk_end(ctx);

    if (state != ENGINE_SCENE_EXPANDED) {

        /* RIGHT PANEL */
        if (nk_begin(ctx, "Right Panel", nk_rect(sceneX + sceneWidth, 30, windowWidth - (sceneX + sceneWidth), windowHeight), NK_WINDOW_BORDER | NK_WINDOW_TITLE )) {}
        nk_end(ctx);
        
        /* Top Left PANEL */
        if (nk_begin(ctx, "Top Left Panel", nk_rect(0, 30, sceneX, (windowHeight - 30)/2), NK_WINDOW_BORDER | NK_WINDOW_TITLE )) {
            nk_layout_row_static(ctx, 30, 150, 1); // 5 buttons per row
            if (nk_button_label(ctx, "Toggle Pixel Shader")) { postProcessing = !postProcessing; }
            if (nk_button_label(ctx, "Toggle Wireframe")) { wireframe = !wireframe; }
            if (nk_slider_int(ctx, 1, &shadowMapFreq, 3, 1)) {
                // value changed, do something if needed
            }
        }
        nk_end(ctx);

        /* Bottom Left Panel */
        if (nk_begin(ctx, "Bot Left Panel", nk_rect(0, 30 + (windowHeight - 30)/2, sceneX, (windowHeight - 30)/2), NK_WINDOW_BORDER | NK_WINDOW_TITLE )) {}
        nk_end(ctx);

        if (state == ENGINE_SCENE_HIDDEN) {
            /* Middle Panel */
            if (nk_begin(ctx, "Middle Panel", nk_rect(sceneX, 30, sceneWidth, sceneHeight), NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE )) {}
            nk_end(ctx);
        }

        /* Bottom Panel */
        if (nk_begin(ctx, "Bottom Panel", nk_rect(sceneX, sceneHeight + 30, sceneWidth, windowHeight - sceneHeight - 30), NK_WINDOW_BORDER | NK_WINDOW_TITLE )) {}
        nk_end(ctx);
    
    }
    
    /* PERFORMANCE */
    if (nk_begin(ctx, "FPS Graph", nk_rect(50, 50, 300, 300), NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE )) {
    
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "FPS Tracker (20s)", NK_TEXT_CENTERED);
    
        // Show units above the graph
        nk_layout_row_dynamic(ctx, 20, 2);
        nk_label(ctx, "FPS", NK_TEXT_LEFT);
        nk_label(ctx, "Time (ms)", NK_TEXT_LEFT);
    
        // Graph area for FPS values
        nk_layout_row_dynamic(ctx, 150, 1);
        nk_chart_begin(ctx, NK_CHART_LINES, timer.fpsHistoryCount, 0.0f, 200.0f);
        int start = (timer.fpsHistoryIndex + FPS_HISTORY_COUNT - timer.fpsHistoryCount) % FPS_HISTORY_COUNT;
        for (int i = 0; i < timer.fpsHistoryCount; i++) {
            int idx = (start + i) % FPS_HISTORY_COUNT;
            nk_chart_push(ctx, timer.fpsHistory[idx]);
        }
        nk_chart_end(ctx);
    
        // Show current FPS and timer time below the graph
        nk_layout_row_dynamic(ctx, 20, 2);
    
        char buf_fps[64];
        snprintf(buf_fps, sizeof(buf_fps), "Current FPS: %.1f fps", timer.fps);
        nk_label(ctx, buf_fps, NK_TEXT_LEFT);
    
        char buf_dt[64];
        snprintf(buf_dt, sizeof(buf_dt), "Delta Time: %.2f ms", timer.ms);
        nk_label(ctx, buf_dt, NK_TEXT_LEFT);
    
        // next one
        nk_layout_row_dynamic(ctx, 20, 2);
    
        snprintf(buf_fps, sizeof(buf_fps), "w:%d h:%d", windowWidth, windowHeight);
        nk_label(ctx, buf_fps, NK_TEXT_LEFT);
    
        // snprintf(buf_dt, sizeof(buf_dt), "Delta Time: %.2f ms", timer.ms);
        // nk_label(ctx, buf_dt, NK_TEXT_LEFT);
    
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "buffer2", NK_TEXT_CENTERED);
    
        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "EDITOR"))
            state = ENGINE_EDITOR;
         
        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "SCENE"))
            state = ENGINE_SCENE_EXPANDED;
         
        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "NONE"))
            state = ENGINE_SCENE_HIDDEN;
    
    }
    nk_end(ctx);

    nk_glfw3_render(NK_ANTI_ALIASING_OFF);

}
