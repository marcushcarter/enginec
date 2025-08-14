#include "engine/engine_internal.h"
#include "engine/geometry.h"

#define WINDOW_WIDTH 1440
#define WINDOW_HEIGHT 900

int sceneWidth, sceneHeight, sceneX, sceneY;

#include <stdio.h>
#include <math.h>
#include <time.h>


// -----------------------------------------------------------------------------------------------------------------------------------------------------------------

BE_Mesh scene1, capsule, light, billboard, cameraMesh;

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;

    glViewport(0, 0, width, height);

    BE_FBOResize(&FBOs[0], width, height);
    BE_FBOResize(&FBOs[1], width, height);

}

void draw_stuff(BE_Shader* shader) {
    mat4 model;

    BE_MatrixMakeModel((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    BE_MeshDraw(&scene1, shader);
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
    
    BE_VAO quadVAO = BE_VAOInitQuad();
    BE_VAO bbVAO = BE_VAOInitBillboardQuad();
    
    FBOs[0] = BE_FBOInit(windowWidth, windowHeight);
    FBOs[1] = BE_FBOInit(windowWidth, windowHeight);
    int ping = 0;

    BE_CameraVectorInit(&cameras);
    BE_CameraVectorPush(&cameras, BE_CameraInit(windowWidth, windowHeight, 45.0f, 0.1f, 100.0f, (vec3){-1.93f, 0.73f, -1.75f}, (vec3){0.67f, -0.12f, 0.73f}));
    BE_CameraVectorPush(&cameras, BE_CameraInit(windowWidth, windowHeight, 45.0f, 0.1f, 100.0f, (vec3){-1.93f, 0.73f, -1.75f}, (vec3){0.67f, -0.12f, 0.73f}));
    selectedCamera = &cameras.data[0];

    BE_LightVector lights;
    BE_LightVectorInit(&lights);
    BE_LightVectorPush(&lights, BE_LightInit(LIGHT_DIRECT, (vec3){0,0,0}, (vec3){0.5f, -0.4f, 0.5f}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.5f, 0, 0, 0, 0));
    BE_LightVectorPush(&lights, BE_LightInit(LIGHT_POINT, (vec3){0,0,0}, (vec3){0,0,0}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.5f, 1.0f, 0.04f, 0, 0));

    printf("seconds to load objects %.2fs\n", glfwGetTime());

    while(!glfwWindowShouldClose(window)) {

        BE_UpdateFrameTimeInfo(&timer);

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Engine %.1f fps %.2f ms", timer.fps, timer.ms);
        glfwSetWindowTitle(window, buffer);
        
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        glfwPollEvents();

        glm_vec3_copy((vec3){cosf(glfwGetTime()/25), -0.4f, sinf(glfwGetTime()/25)}, lights.data[0].direction);
        glm_vec3_copy((vec3){sin(glfwGetTime()), 0.5f, cos(glfwGetTime())}, lights.data[1].position);
        vec4 rainbowColor = {
            sinf(glfwGetTime()*0.5f) * 0.5f + 0.5f,
            sinf(glfwGetTime()*0.5f + 2.0943951f) * 0.5f + 0.5f,
            sinf(glfwGetTime()*0.5f + 4.1887902f) * 0.5f + 0.5f,
            1.0f
        };
        glm_vec4_copy(rainbowColor, lights.data[1].color);
        
        BE_CameraInputs(selectedCamera, window, &joystick, timer.dt);
        BE_CameraVectorUpdateMatrix(&cameras, windowWidth, windowHeight);
        
        BE_LightVectorUpdateMatrix(&lights);
        BE_LightVectorUpdateMaps(&lights, &shader_shadowMap, selectedCamera, draw_stuff, (glfwGetKey(window, GLFW_KEY_3) != GLFW_PRESS));
        
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

        BE_LightVectorUpload(&lights, &shader_default);
        BE_CameraMatrixUpload(selectedCamera, &shader_default, "camMatrix");

        glUniform1i(glGetUniformLocation(shader_default.ID, "sampleRadius"), 0);
        draw_stuff(&shader_default);
        BE_LightVectorDraw(&lights, &light, &shader_lights);
        
        BE_CameraVectorDraw(&cameras, &cameraMesh, &shader_color, selectedCamera);

        ping = !ping;
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        BE_FBOUnbind();
        glViewport(0, 0, windowWidth, windowHeight);
        // glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glViewport(sceneX, sceneY, sceneWidth, sceneHeight);
        BE_ShaderActivate(&shader_framebuffer);
        BE_FBOBindTexture(&FBOs[ping], &shader_framebuffer);
        BE_VAODrawQuad(&quadVAO);
        glViewport(0, 0, windowWidth, windowHeight);

        glfwSwapBuffers(window);

    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}