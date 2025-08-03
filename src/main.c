
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

Mesh ground, pyramid, light, Gun, Model;

void draw_stuff(Shader* shader, Camera* camera) {
    mat4 model;

    make_model_matrix((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&pyramid, shader, camera);

    
    // make_model_matrix((vec3){0.0f, 1.5f, 0.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, model);
    // glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    // Mesh_Draw(&Gun, shader, camera);

    // for (int i = 0; i < 20; i++) {
    
    make_model_matrix((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&ground, shader, camera);
    
    // make_model_matrix((vec3){0.0f, 1.5f, 0.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, model);
    // glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    // Mesh_Draw(&Model, shader, camera);

    // }

    // PLAYER

    make_model_matrix(camera->Position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.2f, 0.2f, 0.2f}, model);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
    Mesh_Draw(&pyramid, shader, camera);
}

Mesh Init_mesh(char** texturearr, int texcount, Vertex* vertices, int vertcount, GLuint* indices, int indcount) {
    // static Texture PLANEtextures[2];
    VertexVector verts;
    GLuintVector inds;
    TextureVector texs;

    // PLANEtextures[0] = Texture_Init("res/textures/box.png", "diffuse", 0);
    // PLANEtextures[1] = Texture_Init("res/textures/box_specular.png", "specular", 1);

    Texture textures[texcount];
    for (int i = 0; i < texcount; i+=2) {
        textures[i] = Texture_Init(texturearr[i], texturearr[i+1], i);
    }

    VertexVector_Copy(vertices, vertcount, &verts);
    GLuintVector_Copy(indices, indcount, &inds);
    TextureVector_Copy(textures, texcount, &texs);

    Mesh mesh = Mesh_Init(verts, inds, texs);

    // VertexVector_Free(&verts);
    // GLuintVector_Free(&inds);
    // TextureVector_Free(&texs);

    return mesh;
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
    
    const char* pyramidtextures[] = { "res/textures/brick.jpg", "diffuse" };
    pyramid = Mesh_InitFromData(pyramidtextures, 1, pyramidVertices, sizeof(pyramidVertices) / sizeof(Vertex), pyramidIndices, sizeof(pyramidIndices) / sizeof(GLuint));
    
    const char* groundtextures[] = { "res/textures/box.png", "diffuse", "res/textures/box_specular.png", "specular" };  
    ground = Mesh_InitFromData(groundtextures, 2, planeVertices, sizeof(planeVertices) / sizeof(Vertex), planeIndices, sizeof(planeIndices) / sizeof(GLuint));
    
    const char* lighttextures[] = { "res/textures/box.png", "diffuse" };
    light = Mesh_InitFromData(lighttextures, 1, cubeVertices, sizeof(cubeVertices) / sizeof(Vertex), cubeIndices, sizeof(cubeIndices) / sizeof(GLuint));

    // SPRITES

    VAO quadVAO = VAO_InitQuad();

    const char* files[] = { "res/textures/gun.png", "res/textures/gun.png" };
    Sprite sprite = Sprite_Init(files, 2);

    // Gun = Import_loadMeshFromOBJ("res/models/Untitled.obj");

    // VertexVector modelverts;
    // VertexVector_Copy(modelVertices, sizeof(modelVertices) / sizeof(Vertex), &modelverts);
    // GLuintVector modelind;
    // GLuintVector_Copy(modelIndices, sizeof(modelIndices) / sizeof(GLuint), &modelind);
    // Model = Mesh_Init(&modelverts, &modelind, &PLANEtex);
    
    FBO postProcessingFBO[2];
    postProcessingFBO[0] = FBO_Init(width, height);
    postProcessingFBO[1] = FBO_Init(width, height);
    bool postProcessing = false;
    int ping = 0;

    bool wireframe = false;
    glLineWidth(5.0f);

    Camera camera = Camera_Init(width, height, 2.5f, 3.0f,(vec3){0.0f, 1.0f, 3.0f}, false);

    LightSystem lightSystem = LightSystem_Init(0.0f);
    
    while(!glfwWindowShouldClose(window)) {

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        wireframe = glfwGetKey(window, GLFW_KEY_4);

        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
            // wireframe = !wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        }

        dt = get_delta_time();
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "opengl window: %f FPS", fps);
        glfwSetWindowTitle(window, buffer);

        glfwJoystickEvents();
        if (joystickIsPressed(&joysticks[0], 7)) postProcessing = !postProcessing;
        
        Camera_Inputs(&camera, window, &joysticks[0], dt);
        Camera_UpdateMatrix(&camera, 45.0f, 0.1f, 100.0f);
        
        LightSystem_Clear(&lightSystem);
        LightSystem_AddPointLight(&lightSystem, (vec3){sin(glfwGetTime()), 0.5f, cos(glfwGetTime())}, (vec4){1.0f, 0.1f, 0.05f, 1.0f}, 1.0f, 0.04f, 0.5f);
        LightSystem_AddPointLight(&lightSystem, (vec3){-sin(glfwGetTime()), 0.5f, -cos(glfwGetTime())}, (vec4){0.2f, 1.0f, 0.2f, 1.0f}, 1.0f, 0.04f, 0.5f);
        LightSystem_SetDirectLight(&lightSystem, (vec3){cos(glfwGetTime()/10), -1.0f, sin(glfwGetTime()/10)}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.5f);
        LightSystem_AddSpotLight(&lightSystem, (vec3){0.0f, 8.5f, 0.0f}, (vec3){0.1f, -1.0f, 0.0f}, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, 0.90f, 0.95f, 0.5f);
        LightSystem_MakeShadowMaps(&lightSystem, &shadowMapProgram, &camera, draw_stuff);

        // if (joystickIsPressed(&joysticks[0], 6)) print_mat4(lightSystem.spotlight.lightSpaceMatrix);

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

        LightSystem_SetUniforms(&glShaderProgram_default3d, &lightSystem);

        draw_stuff(&glShaderProgram_default3d, &camera);
        
        LightSystem_DrawLights(&lightSystem, &light, &glShaderProgram_light3d, &camera);

        glDisable(GL_DEPTH_TEST);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        // mat4 proj = GLM_MAT4_IDENTITY_INIT;
        // glm_ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f, proj); // Top-left is (0,0)
        // glUniformMatrix4fv(glGetUniformLocation(shader->ID, "uProjection"), 1, GL_FALSE, (float*)proj);

        // SpriteInstance inst = {
        //     .position = {800, 500},
        //     .size = {500, 500},
        //     .rotation = 0.0f, 
        //     .color = {1.0f, 1.0f, 1.0f, 1.0f},
        //     .layer = 1
        // };

        // Sprite_Draw(&sprite, &spriteShad, &inst, proj, &quadVAO);


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