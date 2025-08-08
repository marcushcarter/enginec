
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stb_image/stb_image.h>
#include <stb_image/stb_image_resize.h>

unsigned int width = 1600;
unsigned int height = 1000;

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

Texture tex1;

Mesh scene1, capsule, light, billboard, cameraMesh;

Camera* viewedCamera = NULL;

CameraVector cameras;

void draw_stuff(Shader* shader, Camera* camera) {
    mat4 model;
    
    Camera_UpdateMatrix(camera);

    make_billboard_matrix((vec3){0.0f, 1.5f, 0.0f}, camera->viewMatrix, (vec3){0.5f, 0.5f, 0.5f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&billboard, shader, camera);

    make_model_matrix((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&scene1, shader, camera);

    // PLAYER

    make_model_matrix(camera->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.2f, 0.2f, 0.2f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&capsule, shader, camera);


}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(width, height, "opengl window", NULL, NULL);
    if (window == NULL) {
        printf("window failed to create\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glViewport(0, 0, width, height);

    struct nk_context* ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

    {struct nk_font_atlas* atlas;
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();}

    // SHADERS
    
    Shader shader_default = Shader_Init("shaders/vert/default.vert", "shaders/frag/default.frag", NULL);
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

    LightSystem lightSystem = LightSystem_Init(0.15f);

    // CAMERA VECTOR TESTING

    CameraVector_Init(&cameras);
    Camera* cam1 = Camera_InitHeap(width, height, 45.0f, 0.1f, 100.0f, (vec3){0.0f, 1.0f, 3.0f}, (vec3){0.0f, 0.0f, -1.0f});
    CameraVector_Push(&cameras, cam1);
    Camera* cam2 = Camera_InitHeap(width, height, 45.0f, 0.1f, 100.0f, (vec3){-1.93f, 0.73f, -1.75f}, (vec3){0.67f, -0.12f, 0.73f});
    CameraVector_Push(&cameras, cam2);
    Camera* cam3 = Camera_InitHeap(width, height, 45.0f, 0.1f, 100.0f, (vec3){2.23f, 3.01f, 2.9f}, (vec3){-0.49f, -0.6f, -0.63f});
    CameraVector_Push(&cameras, cam3);

    viewedCamera = CameraVector_Get(&cameras, 0);
    int camNum = 0;

    printf("seconds to compile %.2fs\n", glfwGetTime());
    while(!glfwWindowShouldClose(window)) {

        dt = get_delta_time();
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "opengl window: %f FPS", fps);
        glfwSetWindowTitle(window, buffer);

        glfwPollEvents();
        glfwJoystickEvents();

        if (joystickIsPressed(&joysticks[0], 7)) postProcessing = !postProcessing;
        if (joystickIsPressed(&joysticks[0], 6)) wireframe = !wireframe;
        
        Camera_Inputs(viewedCamera, window, &joysticks[0], dt);
        Camera_UpdateMatrix(viewedCamera);

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
            Camera* newCam = Camera_InitHeap(width, height, 45.0f, 0.1f, 100.0f, (vec3){0.0f, 1.0f, 0.0f}, viewedCamera->direction);
            CameraVector_Push(&cameras, newCam); // Adds a new camera
        }
        if (joystickIsPressed(&joysticks[0], 12) && cameras.size > 0 && CameraVector_IndexOf(&cameras, viewedCamera) != 0) {
            CameraVector_Remove(&cameras, viewedCamera);
            viewedCamera = CameraVector_Get(&cameras, 0);
        }
        
        LightSystem_Clear(&lightSystem);
        // LightSystem_AddPointLight(&lightSystem, (vec3){sin(glfwGetTime()), 0.5f, cos(glfwGetTime())}, (vec4){1.0f, 0.1f, 0.05f, 1.0f}, 1.0f, 0.04f, 0.5f);
        // LightSystem_AddPointLight(&lightSystem, (vec3){-sin(glfwGetTime()), 0.5f, -cos(glfwGetTime())}, (vec4){0.2f, 1.0f, 0.2f, 1.0f}, 1.0f, 0.04f, 0.5f);
        LightSystem_SetDirectLight(&lightSystem, (vec3){cosf(glfwGetTime()/15), -0.4f, sinf(glfwGetTime()/15)}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.5f);
        // LightSystem_AddSpotLight(&lightSystem, (vec3){0.0f, 8.5f, 0.0f}, (vec3){0.1f, -1.0f, 0.0f}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.90f, 0.95f, 0.5f);

        if (!wireframe) LightSystem_MakeShadowMaps(&lightSystem, &shader_shadowMap, viewedCamera, draw_stuff);
            
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
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            LightSystem_SetUniforms(&shader_default, &lightSystem);
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

        
        // nk_glfw3_new_frame();
        // /* GUI */
        // if (nk_begin(ctx, "Demo", nk_rect(0, 0, 200, 200), 0)) {
        //     nk_layout_row_dynamic(ctx, 120, 1);
        //     nk_label(ctx, "Hello, world!", NK_TEXT_CENTERED);

        //     nk_layout_row_static(ctx, 30, 80, 1);
        //     if (nk_button_label(ctx, "button"))
        //         fprintf(stdout, "button pressed\n");
        // }
        // nk_end(ctx);

        // glDisable(GL_DEPTH_TEST);
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // FBO_Bind(&postProcessingFBO[ping]);
        // nk_glfw3_render(NK_ANTI_ALIASING_OFF);
        // ping = !ping;
        
        FBO_Unbind();
        Shader_Activate(&shader_framebuffer);
        FBO_BindTexture(&postProcessingFBO[!ping], &shader_framebuffer);
        VAO_DrawQuad(&quadVAO);

        nk_glfw3_new_frame();
        if (nk_begin(ctx, "Demo", nk_rect(0, 0, 200, 200), 0)) {
            nk_layout_row_dynamic(ctx, 120, 1);
            nk_label(ctx, "Hello, world!", NK_TEXT_CENTERED);

            nk_layout_row_static(ctx, 30, 80, 1);
            if (nk_button_label(ctx, "button"))
                fprintf(stdout, "button pressed\n");
        }
        nk_end(ctx);
        nk_glfw3_render(NK_ANTI_ALIASING_OFF);

        glfwSwapBuffers(window);
    }

    nk_glfw3_shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}