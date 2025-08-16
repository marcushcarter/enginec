#include "engine/engine_internal.h"

#define WINDOW_WIDTH 1440
#define WINDOW_HEIGHT 900

#include <stdio.h>
#include <math.h>
#include <time.h>

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------

GLFWwindow* window;
int windowWidth, windowHeight;

BE_FBO FBOs[2];
    
BE_FrameStats timer;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;

    glViewport(0, 0, width, height);

    // BE_FBOResize(&FBOs[0], width, height);
    // BE_FBOResize(&FBOs[1], width, height);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow*window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Engine", NULL, NULL);
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
    BE_Shader shader_shadowMap = BE_ShaderInit("shaders/vert/shadowMap.vert", "shaders/frag/blank.frag", NULL);
    
    BE_Shader shader_framebuffer = BE_ShaderInit("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/framebuffer.frag", NULL);
    BE_Shader shader_pixelate = BE_ShaderInit("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/pixelate.frag", NULL);
    BE_Shader shader_outline = BE_ShaderInit("shaders/framebuffer/framebuffer.vert", "shaders/framebuffer/outline.frag", NULL);

    // MESHES

    BE_Mesh mesh_cube = BE_LoadOBJToMesh("res/models/cube.obj");
    BE_Mesh mesh_scene1 = BE_LoadOBJToMesh("res/models/scene.obj");
    BE_Mesh mesh_camera = BE_LoadOBJToMesh("res/models/camera.obj");
    
    // BE_VAO vao_quad = BE_VAOInitQuad();
    // BE_VAO vao_billboard = BE_VAOInitBillboardQuad();
    
    // FBOs[0] = BE_FBOInit(windowWidth, windowHeight);
    // FBOs[1] = BE_FBOInit(windowWidth, windowHeight);
    // int ping = 0;

    BE_Joystick joystick;

    // SCENE

    BE_SceneVector scenes;
    BE_SceneVectorInit(&scenes);
    BE_SceneVectorPush(&scenes, BE_SceneInit());
    BE_Scene* activeScene = &scenes.data[0];
    
    BE_CameraVectorPush(&activeScene->cameras, BE_CameraInit(windowWidth, windowHeight, 45.0f, 0.1f, 100.0f, (vec3){-1.93f, 0.73f, -1.75f}, (vec3){0.67f, -0.12f, 0.73f}));
    BE_LightVectorPush(&activeScene->lights, BE_LightInit(LIGHT_DIRECT, (vec3){0,0,0}, (vec3){0.5f, -0.4f, 0.5f}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.5f, 0, 0, 0, 0));
    BE_LightVectorPush(&activeScene->lights, BE_LightInit(LIGHT_POINT, (vec3){0,0,0}, (vec3){0,0,0}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.5f, 1.0f, 0.04f, 0, 0));
    BE_ModelVectorPush(&activeScene->models, BE_ModelInit(&mesh_scene1, BE_TransformInit((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f})));

    BE_Camera* selectedCamera = &activeScene->cameras.data[0];

    fprintf(stdout, "Time to load scene -> %.2fs\n", glfwGetTime());

    while(!glfwWindowShouldClose(window)) {

        // UPDATES

        BE_UpdateFrameTimeInfo(&timer);

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Engine %.1f fps %.2f ms", timer.fps, timer.ms);
        glfwSetWindowTitle(window, buffer);
        
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        glfwPollEvents();
        BE_JoystickUpdate(&joystick);

        glm_vec3_copy((vec3){cosf(glfwGetTime()/25), -0.4f, sinf(glfwGetTime()/25)}, activeScene->lights.data[0].direction);
        glm_vec3_copy((vec3){sin(glfwGetTime()), 0.5f, cos(glfwGetTime())}, activeScene->lights.data[1].position);
        vec4 rainbowColor = {
            sinf(glfwGetTime()*0.5f) * 0.5f + 0.5f,
            sinf(glfwGetTime()*0.5f + 2.0943951f) * 0.5f + 0.5f,
            sinf(glfwGetTime()*0.5f + 4.1887902f) * 0.5f + 0.5f,
            1.0f
        };
        glm_vec4_copy(rainbowColor, activeScene->lights.data[1].color);
        
        BE_CameraInputs(selectedCamera, window, &joystick, timer.dt);

        // RENDERS

        BE_CameraVectorUpdateMatrix(&activeScene->cameras, windowWidth, windowHeight);
        BE_LightVectorUpdateMatrix(&activeScene->lights);
        BE_LightVectorUpdateMultiMaps(&activeScene->lights, &activeScene->models, &shader_shadowMap, (glfwGetKey(window, GLFW_KEY_3) != GLFW_PRESS));
        
        // BE_FBOBind(&FBOs[ping]);
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

        BE_LightVectorUpload(&activeScene->lights, &shader_default);
        BE_CameraMatrixUpload(selectedCamera, &shader_default, "camMatrix");
        BE_CameraMatrixUpload(selectedCamera, &shader_color, "camMatrix");
        glUniform1i(glGetUniformLocation(shader_default.ID, "sampleRadius"), 0);

        BE_ModelVectorDraw(&activeScene->models, &shader_default);
        BE_LightVectorDraw(&activeScene->lights, &mesh_cube, &shader_color);
        BE_CameraVectorDraw(&activeScene->cameras, &mesh_camera, &shader_color, selectedCamera);

        // ping = !ping;
        // glDisable(GL_DEPTH_TEST);
        // glDisable(GL_CULL_FACE);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // BE_FBOUnbind();
        // glViewport(0, 0, windowWidth, windowHeight);
        // // glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        // // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // // glViewport(sceneX, sceneY, sceneWidth, sceneHeight);
        // BE_ShaderActivate(&shader_pixelate);
        // BE_FBOBindTexture(&FBOs[ping], &shader_pixelate);
        // BE_VAODrawQuad(&vao_quad);
        // glViewport(0, 0, windowWidth, windowHeight);

        glfwSwapBuffers(window);

    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}