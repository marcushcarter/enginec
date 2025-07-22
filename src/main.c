
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
    0, 1, 2,
    0, 2, 3,
    4, 6, 5,
    7, 9, 8,
    10, 11, 12,
    13, 15, 14,
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

GLfloat QUADvertices[] = {
//      COORDINATES      |        RGB        |   TEXTURE
    -1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 1.0f,   0.0f,  0.0f,
    -1.0f,  1.0f,  1.0f,   1.0f, 0.0f, 0.0f,   0.0f,  1.0f,
     1.0f,  1.0f,  1.0f,   0.0f, 1.0f, 0.0f,   1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,   1.0f, 1.0f, 1.0f,   1.0f,  0.0f,
};

GLuint QUADindices[] = {
    0, 1, 2,
    0, 2, 3,
};

// double get_memory_usage_mb() {
//     PROCESS_MEMORY_COUNTERS memInfo;
//     GetProcessMemoryInfo(GetCurrentProcess(), &memInfo, sizeof(memInfo));
//     return memInfo.WorkingSetSize / (1024.0 * 1024.0); // Convert to MB
// }

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

void control_fps(float target_fps, bool limited) {
	float frame_duration = 1.0f / target_fps;
	clock_t now = clock();
	float elapsed = (float)(now - previous_time) / CLOCKS_PER_SEC;
	float remaining_time = frame_duration - elapsed;
	if (remaining_time > 0 && limited) {
		// usleep((useconds_t)(remaining_time * 1000000.0f));
	}
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
    
    Shader glShaderProgram_post_default = Shader_Init("shaders/vert/default2d.vert", "shaders/post/default.frag");
    Shader glShaderProgram_post_pixelate = Shader_Init("shaders/vert/default2d.vert", "shaders/post/pixelate.frag");
    
    Shader glShaderProgram_default2d = Shader_Init("shaders/vert/default2d.vert", "shaders/frag/default2d.frag");
    Shader glShaderProgram_raymarch = Shader_Init("shaders/vert/default2d.vert", "shaders/frag/raymarch.frag");
    Shader glShaderProgram_post_crt = Shader_Init("shaders/vert/default2d.vert", "shaders/post/crt.frag");

    // MESHES

    Texture textures[1];
    textures[0] = Texture_Init("res/textures/sydney.png", "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE);
    // textures[0] = Texture_Init("res/textures/box.png", "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE);
    // textures[1] = Texture_Init("res/textures/box_specular.png", "specular", 1, GL_RED, GL_UNSIGNED_BYTE);
    Shader glShaderProgram_default3d = Shader_Init("shaders/vert/default3d.vert", "shaders/frag/default3d.frag");
    VertexVector verts;
    CopyToVertexVector(vertices, sizeof(vertices) / sizeof(Vertex), &verts);
    GLuintVector ind;
    CopyToGLuintVector(indices, sizeof(indices) / sizeof(GLuint), &ind);
    TextureVector tex;
    CopyToTextureVector(textures, sizeof(textures) / sizeof(Texture), &tex);
    Mesh pyramid = Mesh_Init(&verts, &ind, &tex);

    Texture PLANEtextures[1];
    PLANEtextures[0] = Texture_Init("res/textures/box.png", "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE);
    VertexVector PLANEverts;
    CopyToVertexVector(PLANEvertices, sizeof(PLANEvertices) / sizeof(Vertex), &PLANEverts);
    GLuintVector PLANEind;
    CopyToGLuintVector(PLANEindices, sizeof(PLANEindices) / sizeof(GLuint), &PLANEind);
    TextureVector PLANEtex;
    CopyToTextureVector(PLANEtextures, sizeof(PLANEtextures) / sizeof(Texture), &PLANEtex);
    Mesh floor = Mesh_Init(&PLANEverts, &PLANEind, &PLANEtex);
    
    Shader glShaderProgram_light3d = Shader_Init("shaders/vert/light3d.vert", "shaders/frag/light3d.frag");
    VertexVector lightVerts;
    CopyToVertexVector(lightVertices, sizeof(lightVertices) / sizeof(Vertex), &lightVerts);
    GLuintVector lightInd;
    CopyToGLuintVector(lightIndices, sizeof(lightIndices) / sizeof(GLuint), &lightInd);
    Mesh light = Mesh_Init(&lightVerts, &lightInd, &tex);

    // LIGHTS AND MODELS

    vec4 lightColor;
    glm_vec4_copy((vec4){1.0f, 1.0f, 1.0f, 1.0f}, lightColor);

    vec3 pyramidPos;
    mat4 pyramidModel;
    glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, pyramidPos);
    glm_mat4_identity(pyramidModel);
    glm_translate(pyramidModel, pyramidPos);

    Shader_Activate(&glShaderProgram_light3d);
    glUniform4fv(glGetUniformLocation(glShaderProgram_light3d.ID, "u_lightColor"), 1, (float*)lightColor);
    Shader_Activate(&glShaderProgram_default3d);
    glUniformMatrix4fv(glGetUniformLocation(glShaderProgram_default3d.ID, "u_model"), 1, GL_FALSE, (float*)pyramidModel);
    glUniform4fv(glGetUniformLocation(glShaderProgram_default3d.ID, "u_lightColor"), 1, (float*)lightColor);

    // FRAMEBUFFER QUAD

    VAO quadVAO = VAO_Init();
    VAO_Bind(&quadVAO);
    VBO quadVBO = VBO_InitRaw(QUADvertices, sizeof(QUADvertices));
    EBO quadEBO = EBO_InitRaw(QUADindices, sizeof(QUADindices));
    VAO_LinkAttrib(&quadVBO, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
    VAO_LinkAttrib(&quadVBO, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    VAO_LinkAttrib(&quadVBO, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();
    EBO_Unbind();

    // FRAMEBUFFERS / CAMERAS

    Framebuffer pingpongFBO[2];
    pingpongFBO[0] = Framebuffer_Init(width, height);
    pingpongFBO[1] = Framebuffer_Init(width, height);
    bool ping = 0;

    Camera camera = Camera_Init(width, height, 2.5f, 3.0f,(vec3){0.0f, 1.0f, 3.0f});
    
    // GAME LOOP

    bool postProcessing = true;

    while(!glfwWindowShouldClose(window)) {
        
        dt = get_delta_time();

        glfwJoystickEvents();
        if (joystickIsPressed(&joysticks[0], 7)) postProcessing = !postProcessing;
        
        Camera_Inputs(&camera, window, &joysticks[0], dt);
        Camera_UpdateMatrix(&camera, 45.0f, 0.1f, 100.0f);
        
        // glfwGetFramebufferSize(window, &width, &height);
        // glfwGetFramebufferSize(window, &pingpongFBO[0].width, &pingpongFBO[0].height);
        // glfwGetFramebufferSize(window, &pingpongFBO[1].width, &pingpongFBO[1].height);

        Framebuffer_Bind(&pingpongFBO[ping]);
        glClearColor(0.85f, 0.85f, 0.90f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        vec3 lightPos;
        mat4 lightModel;
        glm_vec3_copy((vec3){sin(glfwGetTime()), 0.5f, cos(glfwGetTime())}, lightPos);
        glm_mat4_identity(lightModel);
        glm_translate(lightModel, lightPos);
        
        Shader_Activate(&glShaderProgram_light3d);
        glUniformMatrix4fv(glGetUniformLocation(glShaderProgram_light3d.ID, "u_model"), 1, GL_FALSE, (float*)lightModel);
        Shader_Activate(&glShaderProgram_default3d);
        glUniform3fv(glGetUniformLocation(glShaderProgram_default3d.ID, "u_lightPosition"), 1, (float*)lightPos);
        glUniform1f(glGetUniformLocation(glShaderProgram_default3d.ID, "u_time"), glfwGetTime());

        // glDisable(GL_DEPTH_TEST);
        // Texture_Bind(&textures[0]);
        // Shader_Activate(&glShaderProgram_raymarch);
        // glUniform1f(glGetUniformLocation(glShaderProgram_raymarch.ID, "u_time"), glfwGetTime());
        // glUniform3fv(glGetUniformLocation(glShaderProgram_raymarch.ID, "u_cameraPosition"), 1, (float*)&camera.Position);
        // glUniform3fv(glGetUniformLocation(glShaderProgram_raymarch.ID, "u_cameraOrientation"), 1, (float*)&camera.Orientation);
        // VAO_Bind(&quadVAO);
        // glDrawElements(GL_TRIANGLES, sizeof(QUADindices)/sizeof(int), GL_UNSIGNED_INT, 0);

        glEnable(GL_DEPTH_TEST);
        Mesh_Draw(&pyramid, &glShaderProgram_default3d, &camera);
        Mesh_Draw(&floor, &glShaderProgram_default3d, &camera);
        Mesh_Draw(&light, &glShaderProgram_light3d, &camera);


        ping = !ping;
        glDisable(GL_DEPTH_TEST);
        
        // PIXELATION PASS
        if (postProcessing) {
            Framebuffer_Bind(&pingpongFBO[ping]);
            glClear(GL_COLOR_BUFFER_BIT);
            Shader_Activate(&glShaderProgram_post_pixelate);
            Framebuffer_BindTexture(&pingpongFBO[!ping]);
            glUniform1i(glGetUniformLocation(glShaderProgram_post_pixelate.ID, "u_texture"), 0);
            glUniform2f(glGetUniformLocation(glShaderProgram_post_pixelate.ID, "resolution"), pingpongFBO[!ping].width, pingpongFBO[!ping].height);
            glUniform1f(glGetUniformLocation(glShaderProgram_post_pixelate.ID, "pixelSize"), 4.0f);
            VAO_Bind(&quadVAO);
            glDrawElements(GL_TRIANGLES, sizeof(QUADindices)/sizeof(int), GL_UNSIGNED_INT, 0);
            ping = !ping;
        }

        // FINISHING PASS
        Framebuffer_Unbind();
        glClear(GL_COLOR_BUFFER_BIT);
        Shader_Activate(&glShaderProgram_post_default);
        Framebuffer_BindTexture(&pingpongFBO[!ping]);
        glUniform1i(glGetUniformLocation(glShaderProgram_post_default.ID, "u_texture"), 0);
        glUniform2f(glGetUniformLocation(glShaderProgram_post_default.ID, "resolution"), pingpongFBO[!ping].width, pingpongFBO[!ping].height);
        VAO_Bind(&quadVAO);
        glDrawElements(GL_TRIANGLES, sizeof(QUADindices)/sizeof(int), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
        control_fps(5.0f, true);
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