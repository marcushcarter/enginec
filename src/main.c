
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// #include <glm/glm.hpp>
#include <cglm/cglm.h>
// #include <KHR/khrplatform.h>
#include <stb_image/stb_image.h>

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "opengl/opengl.h"

unsigned int width = 1600;
unsigned int height = 1000;

GLfloat vertices[] = {
//      COORDINATES    |       COLORS        |     TETXURE  
    -0.5f,  0.0f,  0.5f,   0.83f,  0.70f,  0.44f,    0.0f,  0.0f,
    -0.5f,  0.0f, -0.5f,   0.83f,  0.70f,  0.44f,    5.0f,  0.0f,
     0.5f,  0.0f, -0.5f,   0.83f,  0.70f,  0.44f,    0.0f,  0.0f,
     0.5f,  0.0f,  0.5f,   0.83f,  0.70f,  0.44f,    5.0f,  0.0f,
     0.0f,  0.8f,  0.0f,   0.92f,  0.86f,  0.76f,    2.5f,  5.0f,
};

GLuint indices[] = {
    0, 1, 2,
    0, 2, 3,
    0, 1, 4,
    1, 2, 4,
    2, 3, 4,
    3, 0, 4,
};

GLfloat lightVertices[] = {
    -0.1f, -0.1f,  0.1f,
    -0.1f, -0.1f, -0.1f,
     0.1f, -0.1f, -0.1f,
     0.1f, -0.1f,  0.1f,
    -0.1f,  0.1f,  0.1f,
    -0.1f,  0.1f, -0.1f,
     0.1f,  0.1f, -0.1f,
     0.1f,  0.1f,  0.1f,
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
	// if (remaining_time > 0 && limited) {
	// 	usleep((useconds_t)(remaining_time * 1000000.0f));
	// }
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------------

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
    
    Shader glShaderProgram_default3d = Shader_Init("shaders/vert/default3d.vert", "shaders/frag/default3d.frag");
    Shader glShaderProgram_light3d = Shader_Init("shaders/vert/light3d.vert", "shaders/frag/light3d.frag");
    Shader glShaderProgram_default2d = Shader_Init("shaders/vert/default2d.vert", "shaders/frag/default2d.frag");

    Shader glShaderProgram_raymarch = Shader_Init("shaders/vert/default2d.vert", "shaders/frag/raymarch.frag");
    
    Shader glShaderProgram_post_default = Shader_Init("shaders/vert/default2d.vert", "shaders/post/default.frag");
    Shader glShaderProgram_post_crt = Shader_Init("shaders/vert/default2d.vert", "shaders/post/crt.frag");
    Shader glShaderProgram_post_pixelate = Shader_Init("shaders/vert/default2d.vert", "shaders/post/pixelate.frag");

    // BUFFERS

    VAO VAO1 = VAO_Init();
    VAO_Bind(&VAO1);
    VBO VBO1 = VBO_Init(vertices, sizeof(vertices));
    EBO EBO1 = EBO_Init(indices, sizeof(indices));
    VAO_LinkAttrib(&VAO1, &VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
    VAO_LinkAttrib(&VAO1, &VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    VAO_LinkAttrib(&VAO1, &VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();
    EBO_Unbind();
    
    VAO lightVAO = VAO_Init();
    VAO_Bind(&lightVAO);
    VBO lightVBO = VBO_Init(lightVertices, sizeof(lightVertices));
    EBO lightEBO = EBO_Init(lightIndices, sizeof(lightIndices));
    VAO_LinkAttrib(&lightVAO, &lightVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
    VAO_Unbind();
    VBO_Unbind();
    EBO_Unbind();

    vec3 lightPos;
    mat4 lightModel;
    glm_vec3_copy((vec3){0.5f, 0.5f, 0.5f}, lightPos);
    glm_mat4_copy((mat4){1.0f, 1.0f, 1.0f, 1.0f}, lightModel);
    glm_translate(lightModel, lightPos);

    vec3 pyramidPos;
    mat4 pyramidModel;
    glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, pyramidPos);
    glm_mat4_copy((mat4){1.0f, 1.0f, 1.0f, 1.0f}, pyramidModel);
    glm_translate(pyramidModel, pyramidPos);

    Shader_Activate(&glShaderProgram_light3d);
    glUniformMatrix4fv(glGetUniformLocation(glShaderProgram_light3d.ID, "u_model"), 1, GL_FALSE, (float*)lightModel);
    Shader_Activate(&glShaderProgram_default3d);
    glUniformMatrix4fv(glGetUniformLocation(glShaderProgram_default3d.ID, "u_model"), 1, GL_FALSE, (float*)pyramidModel);

    VAO quadVAO = VAO_Init();
    VAO_Bind(&quadVAO);
    VBO quadVBO = VBO_Init(QUADvertices, sizeof(QUADvertices));
    EBO quadEBO = EBO_Init(QUADindices, sizeof(QUADindices));
    VAO_LinkAttrib(&quadVAO, &quadVBO, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
    VAO_LinkAttrib(&quadVAO, &quadVBO, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    VAO_LinkAttrib(&quadVAO, &quadVBO, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();
    EBO_Unbind();

    // TEXTURES

    Texture sydney = Texture_Init("res/textures/sydney.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    Texture_texUnit(&glShaderProgram_default3d, "u_texture", 0);

    Framebuffer pingpongFBO[2];
    pingpongFBO[0] = Framebuffer_Init(width, height);
    pingpongFBO[1] = Framebuffer_Init(width, height);
    bool ping = 0;

    // GAME LOOP

    Camera camera = Camera_Init(width, height, (vec3){0.0f, 0.0f, 9.0f});
    RaymarchCamera raymarchCamera = RaymarchCamera_Init( (vec3){ 0, 0, 3.0f }, (vec3){ 0.0f, 0.0f, -1.0f } );

    while(!glfwWindowShouldClose(window)) {
        
        dt = get_delta_time();
        
        // glfwGetFramebufferSize(window, &width, &height);
        glfwGetFramebufferSize(window, &pingpongFBO[0].width, &pingpongFBO[0].height);
        glfwGetFramebufferSize(window, &pingpongFBO[1].width, &pingpongFBO[1].height);
        // glViewport(0, 0, width, height);

        Framebuffer_Bind(&pingpongFBO[ping]);
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glDisable(GL_DEPTH_TEST);
        // RaymarchCamera_Inputs(&raymarchCamera, window, dt);
        // Texture_Bind(&sydney);
        // Shader_Activate(&glShaderProgram_raymarch);
        // glUniform1f(glGetUniformLocation(glShaderProgram_raymarch.ID, "u_time"), glfwGetTime());
        // glUniform3fv(glGetUniformLocation(glShaderProgram_raymarch.ID, "u_cameraPosition"), 1, (float*)&raymarchCamera.Position);
        // glUniform3fv(glGetUniformLocation(glShaderProgram_raymarch.ID, "u_cameraOrientation"), 1, (float*)&raymarchCamera.Orientation);
        // VAO_Bind(&quadVAO);
        // glDrawElements(GL_TRIANGLES, sizeof(QUADindices)/sizeof(int), GL_UNSIGNED_INT, 0);

        glEnable(GL_DEPTH_TEST);
        Shader_Activate(&glShaderProgram_default3d);
        Camera_Inputs(&camera, window, dt);
        Camera_UpdateMatrix(&camera, 45.0f, 0.1f, 100.0f);
        Camera_Matrix(&camera, &glShaderProgram_default3d, "u_camMatrix");
        Texture_Bind(&sydney);
        VAO_Bind(&VAO1);
        glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(int), GL_UNSIGNED_INT, 0);

        ping = !ping;
        glDisable(GL_DEPTH_TEST);
        
        // PIXELATION PASS
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
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
        control_fps(120.0f, false);
    }

    VAO_Delete(&VAO1);
    VBO_Delete(&VBO1);
    EBO_Delete(&EBO1);
    Texture_Delete(&sydney);
    Shader_Delete(&glShaderProgram_default3d);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}