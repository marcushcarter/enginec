#include "engine/engine_internal.h"
#include "engine/engine_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#define MSG_ERROR(file, line, msg, ...)    fprintf(stderr, "%s:%d:\033[31m error:\033[0m " msg "\n", file, line, ##__VA_ARGS__)
#define MSG_WARNING(file, line, msg, ...)  fprintf(stderr, "%s:%d:\033[35m warning:\033[0m " msg "\n", file, line, ##__VA_ARGS__)
#define MSG_FATAL(file, line, msg, ...)    do { fprintf(stderr, "%s:%d:\033[91m fatal error:\033[0m " msg "\n", file, line, ##__VA_ARGS__); exit(1); } while (0)
#define MSG_INFO(file, line, msg, ...)     fprintf(stdout, "%s:%d:\033[37m info:\033[0m " msg "\n", file, line, ##__VA_ARGS__)

// ==============================
// MATH
// ==============================

#define PRINT_MAT4(m) do { \
    printf("mat4:\n"); \
    printf("[ %8.3f %8.3f %8.3f %8.3f ]\n", (m)[0][0], (m)[1][0], (m)[2][0], (m)[3][0]); \
    printf("[ %8.3f %8.3f %8.3f %8.3f ]\n", (m)[0][1], (m)[1][1], (m)[2][1], (m)[3][1]); \
    printf("[ %8.3f %8.3f %8.3f %8.3f ]\n", (m)[0][2], (m)[1][2], (m)[2][2], (m)[3][2]); \
    printf("[ %8.3f %8.3f %8.3f %8.3f ]\n", (m)[0][3], (m)[1][3], (m)[2][3], (m)[3][3]); \
} while(0)

void BE_MatrixMakeModel(vec3 translation, vec3 rotation, vec3 scale, mat4 dest) {
    mat4 trans, rotX, rotY, rotZ, rot, scl;

    glm_translate_make(trans, translation);

    glm_rotate_make(rotX, rotation[0], (vec3){1.0f, 0.0f, 0.0f});
    glm_rotate_make(rotY, rotation[1], (vec3){0.0f, 1.0f, 0.0f});
    glm_rotate_make(rotZ, rotation[2], (vec3){0.0f, 0.0f, 1.0f});

    glm_mat4_mul(rotY, rotX, rot);
    glm_mat4_mul(rotZ, rot, rot);

    glm_scale_make(scl, scale);

    mat4 rs;
    glm_mat4_mul(rot, scl, rs);
    glm_mat4_mul(trans, rs, dest);
}

void BE_MatrixMakeBillboard(vec3 position, mat4 view, vec3 scale, mat4 dest) {
    mat4 model = GLM_MAT4_IDENTITY_INIT;

    // Copy camera rotation from view matrix (transpose of upper-left 3x3)
    model[0][0] = view[0][0];
    model[0][1] = view[1][0];
    model[0][2] = view[2][0];

    model[1][0] = view[0][1];
    model[1][1] = view[1][1];
    model[1][2] = view[2][1];

    model[2][0] = view[0][2];
    model[2][1] = view[1][2];
    model[2][2] = view[2][2];

    // Apply scaling
    glm_scale(model, scale);

    // Set position
    model[3][0] = position[0];
    model[3][1] = position[1];
    model[3][2] = position[2];

    glm_mat4_copy(model, dest);
}

void BE_OritentationToEuler(vec3 orientation, vec3 outEuler) {

    vec3 dir;
    glm_vec3_normalize_to(orientation, dir);

    float yaw = atan2f(-dir[0], -dir[2]);
    float pitch = asinf(dir[1]);
    float roll = 0.0f;

    outEuler[0] = pitch;
    outEuler[1] = yaw;
    outEuler[2] = roll;
}

void BE_Vec3RotateAxis(vec3 in, vec3 axis, float angle_rad, vec3 out) {
    vec3 axis_n;
    glm_vec3_normalize_to(axis, axis_n);

    float cosA = cosf(angle_rad);
    float sinA = sinf(angle_rad);

    vec3 term1, term2, term3;
    glm_vec3_scale(in, cosA, term1);

    glm_vec3_cross(axis_n, in, term2);
    glm_vec3_scale(term2, sinA, term2);

    float dotAV = glm_vec3_dot(axis_n, in);
    glm_vec3_scale(axis_n, dotAV * (1.0f - cosA), term3);

    glm_vec3_add(term1, term2, out);
    glm_vec3_add(out, term3, out);
}

// ==============================
// Time
// ==============================

float BE_UpdateFrameTimeInfo(BE_FrameStats* info) {
    info->currentTime = clock();
    info->dt = (float)(info->currentTime - info->previousTime) / CLOCKS_PER_SEC;
    info->previousTime = info->currentTime;

    info->frameCount++;
    info->frameCountFPS++;
    info->fpsTimer += info->dt;

    if (info->fpsTimer >= 1.0f) {
        info->fps = info->frameCountFPS / info->fpsTimer;
        info->ms = 1000 / info->fps;

        info->fpsHistory[info->fpsHistoryIndex] = info->fps;
        info->fpsHistoryIndex = (info->fpsHistoryIndex + 1) % FPS_HISTORY_COUNT;
        if (info->fpsHistoryCount < FPS_HISTORY_COUNT)
            info->fpsHistoryCount++;

        info->frameCountFPS = 0;
        info->fpsTimer = 0.0f;
    }

    return info->dt;

}

// ==============================
// Joystick
// ==============================

void BE_JoystickUpdate(BE_Joystick* joystick) {

    if (!glfwJoystickPresent(joystick->id)) {
        if (joystick->present) {
            printf("Player %d controller disconnected\n", joystick->id + 1);
            *joystick = (BE_Joystick){0};
        }
        return;
    }

    if (!joystick->present) {
        joystick->present = 1;
        joystick->name = glfwGetJoystickName(joystick->id);
        joystick->deadzone = 0.05f;
        memset(joystick->lbuttons, 0, sizeof(joystick->lbuttons));
        printf("Player %d controller connected: %s\n", joystick->id + 1, joystick->name);
    }

    if (joystick->buttons) {
        for (int b = 0; b < joystick->buttonCount && b < 16; b++) {
            joystick->lbuttons[b] = joystick->buttons[b];
        }
    }
    
    joystick->axes = glfwGetJoystickAxes(joystick->id, &joystick->axisCount);
    joystick->buttons = glfwGetJoystickButtons(joystick->id, &joystick->buttonCount);
    joystick->hats = glfwGetJoystickHats(joystick->id, &joystick->hatCount);

}

int BE_JoystickIsPressed(BE_Joystick* joystick, int button) {
    return joystick->buttons && joystick->buttons[button] && !joystick->lbuttons[button];
}

int BE_JoystickIsReleased(BE_Joystick* joystick, int button) {
    return joystick->buttons && !joystick->buttons[button] && joystick->lbuttons[button];
}

int BE_JoystickIsHeld(BE_Joystick* joystick, int button) {
    return joystick->buttons && joystick->buttons[button];
}

float BE_JoystickGetAxis(BE_Joystick* joystick, int axis) {
    return joystick->axes && joystick->axes[axis];
}

// ==============================
// VertexVector
// ==============================

#define INITIAL_VERTEX_CAPACITY 256

void BE_VertexVectorInit(BE_VertexVector* vec) {
    vec->data = (BE_Vertex*)malloc(sizeof(BE_Vertex) * INITIAL_VERTEX_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_VERTEX_CAPACITY;
}

void BE_VertexVectorPush(BE_VertexVector* vec, BE_Vertex value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (BE_Vertex*)realloc(vec->data, sizeof(BE_Vertex) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void BE_VertexVectorFree(BE_VertexVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

void BE_VertexVectorCopy(BE_Vertex* vertices, size_t count, BE_VertexVector* outVec) {
    BE_VertexVectorInit(outVec);
    for (size_t i = 0; i < count; i++) {
        BE_VertexVectorPush(outVec, vertices[i]);
    }
}

// ==============================
// VAO
// ==============================

BE_VAO BE_VAOInit() {
    BE_VAO vao;
    glGenVertexArrays(1, &vao.ID);
    return vao;
}

BE_VAO BE_VAOInitQuad() {
    
    GLfloat vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
        -1.0f, -1.0f,   0.0f, 0.0f,  // bottom-left
        1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right

        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
        1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right
        1.0f,  1.0f,   1.0f, 1.0f   // top-right
    };
    
    BE_VAO vao = BE_VAOInit();
    BE_VAOBind(&vao);
    BE_VBO vbo = BE_VBOInitFromData(vertices, sizeof(vertices));
    BE_LinkVertexAttribToVBO(&vbo, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    BE_LinkVertexAttribToVBO(&vbo, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    BE_VAOUnbind();
    BE_VBOUnbind();

    return vao;
}

BE_VAO BE_VAOInitSprite() {
    
    GLfloat vertices[] = {
        // First triangle
        -0.5f, -0.5f, 0.0f, 0.0f,  // bottom-left
        0.5f, -0.5f, 1.0f, 0.0f,  // bottom-right
        0.5f,  0.5f, 1.0f, 1.0f,  // top-right

        // Second triangle
        -0.5f, -0.5f, 0.0f, 0.0f,  // bottom-left
        0.5f,  0.5f, 1.0f, 1.0f,  // top-right
        -0.5f,  0.5f, 0.0f, 1.0f   // top-left
    };
    
    BE_VAO vao = BE_VAOInit();
    BE_VAOBind(&vao);
    BE_VBO vbo = BE_VBOInitFromData(vertices, sizeof(vertices));
    BE_LinkVertexAttribToVBO(&vbo, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    BE_LinkVertexAttribToVBO(&vbo, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    BE_VAOUnbind();
    BE_VBOUnbind();

    return vao;
}

BE_VAO BE_VAOInitBillboardQuad() {
    
    GLfloat vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
        -1.0f, -1.0f,   0.0f, 0.0f,  // bottom-left
        1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right

        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
        1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right
        1.0f,  1.0f,   1.0f, 1.0f   // top-right
    };
    
    BE_VAO vao = BE_VAOInit();
    BE_VAOBind(&vao);
    BE_VBO vbo = BE_VBOInitFromData(vertices, sizeof(vertices));
    BE_LinkVertexAttribToVBO(&vbo, 0, 3, GL_FLOAT, 5 * sizeof(float), (void*)0);
    BE_LinkVertexAttribToVBO(&vbo, 1, 2, GL_FLOAT, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    BE_VAOUnbind();
    BE_VBOUnbind();

    return vao;
}

void BE_VAOBind(BE_VAO* vao) {
    glBindVertexArray(vao->ID);
}

void BE_VAODrawQuad(BE_VAO* vao) {
    BE_VAOBind(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void BE_VAOUnbind() {
    glBindVertexArray(0);
}

void BE_VAODelete(BE_VAO* vao) {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao->ID);
}

// ==============================
// VBO
// ==============================

BE_VBO BE_VBOInitFromData(GLfloat* vertices, GLsizeiptr size) {
    BE_VBO vbo;
    glGenBuffers(1, &vbo.ID);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.ID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    return vbo;
}

BE_VBO BE_VBOInitFromVector(BE_VertexVector* vertices) {
    BE_VBO vbo;
    glGenBuffers(1, &vbo.ID);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.ID);
    glBufferData(GL_ARRAY_BUFFER, vertices->size * sizeof(BE_Vertex), vertices->data, GL_STATIC_DRAW);
    return vbo;
}

void BE_VBOBind(BE_VBO* vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo->ID);
}

void BE_VBOUnbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BE_VBODelete(BE_VBO* vbo) {
    glDeleteBuffers(1, &vbo->ID);
}

void BE_LinkVertexAttribToVBO(BE_VBO* vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    BE_VBOBind(vbo);
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    BE_VBOUnbind();
}

// ==============================
// GLuintVector
// ==============================

#define INITIAL_GLUINT_CAPACITY 256

void BE_GLuintVectorInit(BE_GLuintVector* vec) {
    vec->data = (GLuint*)malloc(sizeof(GLuint) * INITIAL_GLUINT_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_GLUINT_CAPACITY;
}

void BE_GLuintVectorPush(BE_GLuintVector* vec, GLuint value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (GLuint*)realloc(vec->data, sizeof(GLuint) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void BE_GLuintVectorFree(BE_GLuintVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

void BE_GLuintVectorCopy(GLuint* data, size_t count, BE_GLuintVector* outVec) {
    BE_GLuintVectorInit(outVec);
    for (size_t i = 0; i < count; i++) {
        BE_GLuintVectorPush(outVec, data[i]);
    }
}

// ==============================
// EBO
// ==============================

BE_EBO BE_EBOInitFromData(GLuint* indices, GLsizeiptr size) {
    BE_EBO ebo;
    glGenBuffers(1, &ebo.ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    return ebo;
}

BE_EBO BE_EBOInitFromVector(BE_GLuintVector* indices) {
    BE_EBO ebo;
    glGenBuffers(1, &ebo.ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size * sizeof(GLuint), indices->data, GL_STATIC_DRAW);
    return ebo;
}

void BE_EBOBind(BE_EBO* ebo) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->ID);
}

void BE_EBOUnbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void BE_EBODelete(BE_EBO* ebo) {
    glDeleteBuffers(1, &ebo->ID);
}

// ==============================
// Shader
// ==============================

char* BE_GetFileContents(const char* filename) {
    FILE* file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(length + 1);

    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;

}

void BE_ShaderGetCompileErrors(unsigned int shader, const char* type) {
    GLint hasCompiled;
    char infolog[1024];
    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE) {
            glGetShaderInfoLog(shader, 1024, NULL, infolog);
            printf("SHADER_COMPILATION_ERROR for: %s\n%s\n", type, infolog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE) {
            glGetProgramInfoLog(shader, 1024, NULL, infolog);
            printf("SHADER_LINKING_ERROR for: %s\n%s\n", type, infolog);
        }
    }
}

BE_Shader BE_ShaderInit(const char* vertexFile, const char* fragmentFile, const char* geometryFile, const char* computeFile) {
    BE_Shader shader = {0};
    
    const char* vertexSource = NULL;
    const char* fragmentSource = NULL;
    const char* geometrySource = NULL;
    const char* computeSource = NULL;
    
    GLuint vertexShader = 0;
    if (vertexFile != NULL) {
        vertexSource = BE_GetFileContents(vertexFile);
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        BE_ShaderGetCompileErrors(vertexShader, "VERTEX");
    }
    
    GLuint fragmentShader = 0;
    if (fragmentFile != NULL) {
        fragmentSource = BE_GetFileContents(fragmentFile);
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        BE_ShaderGetCompileErrors(fragmentShader, "FRAGMENT");
    }

    GLuint geometryShader = 0;
    if (geometryFile != NULL) {
        geometrySource = BE_GetFileContents(geometryFile);
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &geometrySource, NULL);
        glCompileShader(geometryShader);
        BE_ShaderGetCompileErrors(geometryShader, "GEOMETRY");
    }

    GLuint computeShader = 0;
    if (computeFile != NULL) {
        computeSource = BE_GetFileContents(computeFile);
        computeShader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(computeShader, 1, &computeSource, NULL);
        glCompileShader(computeShader);
        BE_ShaderGetCompileErrors(computeShader, "COMPUTE");
    }

    shader.ID = glCreateProgram();
    if (vertexFile != NULL) glAttachShader(shader.ID, vertexShader);
    if (fragmentFile != NULL) glAttachShader(shader.ID, fragmentShader);
    if (geometryFile != NULL) glAttachShader(shader.ID, geometryShader);
    if (computeFile != NULL) glAttachShader(shader.ID, computeShader);
    glLinkProgram(shader.ID);
    BE_ShaderGetCompileErrors(shader.ID, "PROGRAM");

    if (vertexFile != NULL) glDeleteShader(vertexShader);
    if (fragmentFile != NULL) glDeleteShader(fragmentShader);
    if (geometryFile != NULL) glDeleteShader(geometryShader);
    if (computeFile != NULL) glDeleteShader(computeShader);

    if (vertexFile != NULL) free((void*)vertexSource);
    if (fragmentFile != NULL) free((void*)fragmentSource);
    if (geometryFile != NULL) free((void*)geometrySource);
    if (computeFile != NULL) free((void*)computeSource);

    return shader;

}

void BE_ShaderActivate(BE_Shader* shader) {
    glUseProgram(shader->ID);
}

void BE_ShaderDelete(BE_Shader* shader) {
    glDeleteProgram(shader->ID);
}

// ==============================
// FBO
// ==============================

BE_FBO BE_FBOInit(int width, int height) {
    BE_FBO fb;
    fb.width = width;
    fb.height = height;

    glGenFramebuffers(1, &fb.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

    glGenTextures(1, &fb.texture);
    glBindTexture(GL_TEXTURE_2D, fb.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.texture, 0);

    glGenRenderbuffers(1, &fb.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fb.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb.rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("ERROR: Framebuffer is not complete1\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLfloat vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
        -1.0f, -1.0f,   0.0f, 0.0f,  // bottom-left
         1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right

        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
         1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right
         1.0f,  1.0f,   1.0f, 1.0f   // top-right
    };

    fb.vao = BE_VAOInit();
    BE_VAOBind(&fb.vao);
    fb.vbo = BE_VBOInitFromData(vertices, sizeof(vertices));
    BE_LinkVertexAttribToVBO(&fb.vbo, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    BE_LinkVertexAttribToVBO(&fb.vbo, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    BE_VAOUnbind();
    BE_VBOUnbind();
    
    return fb;

}

void BE_FBOResize(BE_FBO* fbo, int width, int height) {
    fbo->width = width;
    fbo->height = height;

    // Delete old texture and renderbuffer
    glDeleteTextures(1, &fbo->texture);
    glDeleteRenderbuffers(1, &fbo->rbo);

    // Create new texture
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);

    glGenTextures(1, &fbo->texture);
    glBindTexture(GL_TEXTURE_2D, fbo->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, fbo->texture, 0);

    // Create new renderbuffer
    glGenRenderbuffers(1, &fbo->rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fbo->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, fbo->rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("ERROR: Resized framebuffer is not complete!\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BE_FBOBind(BE_FBO* fb) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);
    glClear(GL_COLOR_BUFFER_BIT);
}

void BE_FBOBindTexture(BE_FBO* fb, BE_Shader* shader) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fb->texture);
    glUniform1i(glGetUniformLocation(shader->ID, "screenTexture"), 0);
}

void BE_FBOUnbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BE_FBODelete(BE_FBO* fb) {
    glDeleteFramebuffers(1, &fb->fbo);
    glDeleteTextures(1, &fb->texture);
    glDeleteRenderbuffers(1, &fb->rbo);
    *fb = (BE_FBO){0};
}

// ==============================
// Textures
// ==============================

BE_Texture BE_TextureInit(const char* imageFile, const char* texType, GLuint slot) {
    BE_Texture texture;

    texture.type = (char*)malloc(strlen(texType) + 1);
    if (!texture.type) {
        MSG_FATAL(imageFile, 1, "could not allocate memory for texture type");
        exit(1);
    }
    strcpy(texture.type, texType);

    int widthImg, heightImg, numColCh;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* bytes = stbi_load(imageFile, &widthImg, &heightImg, &numColCh, 0);
    if (!bytes) {
        MSG_ERROR(imageFile, 1, "failed to load texture: '%s", stbi_failure_reason());
        exit(1);
    }

    glGenTextures(1, &texture.ID);
    glActiveTexture(GL_TEXTURE0 + slot);
    texture.unit = slot;
    glBindTexture(GL_TEXTURE_2D, texture.ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLenum format;
    if (numColCh == 4) format = GL_RGBA;
    else if (numColCh == 3) format = GL_RGB;
    else if (numColCh == 1) format = GL_RED;
    else {
        MSG_ERROR(imageFile, 1, "unsupported color channel count '%d", numColCh);
        stbi_image_free(bytes);
        exit(1);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, widthImg, heightImg, 0, format, GL_UNSIGNED_BYTE, bytes);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(bytes);
    glBindTexture(GL_TEXTURE_2D, 0);

    // MSG_INFO(imageFile, 1, "texture loaded succesfully");
    return texture;
}

void BE_TextureSetUniformUnit(BE_Shader* shader, const char* uniform, GLuint unit) {
    GLuint tex0Uni = glGetUniformLocation(shader->ID, uniform);
    BE_ShaderActivate(shader);
    glUniform1i(tex0Uni, unit);
}

void BE_TextureBind(BE_Texture* texture) {
    glActiveTexture(GL_TEXTURE0 + texture->unit);
    glBindTexture(GL_TEXTURE_2D, texture->ID);
}

void BE_TextureUnbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void BE_TextureDelete(BE_Texture* texture) {
    glDeleteTextures(1, &texture->ID);
}

#define INITIAL_TEXTURE_CAPACITY 8

void BE_TextureVectorInit(BE_TextureVector* vec) {
    vec->data = (BE_Texture*)malloc(sizeof(BE_Texture) * INITIAL_TEXTURE_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_TEXTURE_CAPACITY;
}

void BE_TextureVectorPush(BE_TextureVector* vec, BE_Texture value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (BE_Texture*)realloc(vec->data, sizeof(BE_Texture) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void BE_TextureVectorFree(BE_TextureVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

void BE_TextureVectorCopy(BE_Texture* textures, size_t count, BE_TextureVector* outVec) {
    BE_TextureVectorInit(outVec);
    for (size_t i = 0; i < count; i++) {
        BE_TextureVectorPush(outVec, textures[i]);
    }
}

// ==============================
// Cameras
// ==============================

BE_Camera BE_CameraInit(int width, int height, float fov, float nearPlane, float farPlane, vec3 position, vec3 direction) {
    BE_Camera camera;

    camera.width = width;
    camera.height = height;
    glm_vec3_copy(position, camera.position);
    glm_vec3_copy(direction, camera.direction);

    vec3 up = { 0.0f, 1.0f, 0.0f };

    glm_vec3_copy(up, camera.Up);

    camera.zoom = 1.0f;
    camera.fov = fov;

    camera.nearPlane = nearPlane;
    camera.farPlane = farPlane;

    mat4 mat;
    glm_mat4_identity(mat);
    glm_mat4_copy(mat, camera.projPersp);
    glm_mat4_copy(mat, camera.projOrtho);

    return camera;
}

void BE_CameraInputs(BE_Camera* camera, GLFWwindow* window, BE_Joystick* joystick, float dt) {

    // MOVEMENT VECTORS

    float speed = 2.5f;
    float sensitivity = 3.0f;
    // if (joystick && joystick->buttons[8]) speed = 5.0f;

    vec3 v_forward, v_right, v_up, v_move;
    glm_vec3_zero(v_move);

    glm_vec3_copy(camera->direction, v_forward);
    v_forward[1] = 0.0f;
    glm_vec3_normalize(v_forward);

    glm_vec3_cross(v_forward, (vec3){0.0f, 1.0f, 0.0f}, v_right);
    glm_vec3_normalize(v_right);

    glm_vec3_cross(v_forward, v_right, v_up);
    glm_vec3_normalize(v_up);
    
    // DIRECTION VECTORS

    vec3 d_up = { 0.0f, 1.0f, 0.0f };
    vec3 d_right, d_direction;

    glm_vec3_cross(d_up, camera->direction, d_right);
    glm_vec3_normalize(d_right);

    // MOVEMENT

    vec3 v_vector;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_scale(v_forward, speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_scale(v_forward, -speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_scale(v_right, -speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_scale(v_right, speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        glm_vec3_scale(v_up, -speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        glm_vec3_scale(v_up, speed*dt, v_vector);
        glm_vec3_add(v_move, v_vector, v_move);
    }

    // CAMERA ROTATION

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        BE_Vec3RotateAxis(camera->direction, d_up, dt*sensitivity, d_direction);
        glm_vec3_normalize_to(d_direction, camera->direction);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        BE_Vec3RotateAxis(camera->direction, d_up, -dt*sensitivity, d_direction);
        glm_vec3_normalize_to(d_direction, camera->direction);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        BE_Vec3RotateAxis(camera->direction, d_right, -dt*sensitivity, d_direction);
        glm_vec3_normalize_to(d_direction, camera->direction);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        BE_Vec3RotateAxis(camera->direction, d_right, dt*sensitivity, d_direction);
        glm_vec3_normalize_to(d_direction, camera->direction);
    }

    
    // if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
    //     camera->fov += 10*dt;
    // }
    // if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
    //     camera->fov -= 10*dt;
    // }
    
    glm_vec3_add(camera->position, v_move, camera->position);

    if (joystick->present && joystick != NULL) {

        if (fabsf(joystick->axes[1]) > joystick->deadzone) {
            glm_vec3_scale(v_forward, -speed*dt*joystick->axes[1], v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }
        if (fabsf(joystick->axes[0]) > joystick->deadzone) {
            glm_vec3_scale(v_right, speed*dt*joystick->axes[0], v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }
        if (joystick->buttons[0] || BE_JoystickIsHeld(joystick, 0)) {
            glm_vec3_scale(v_up, -speed*dt, v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }
        if (joystick->buttons[1] || joystick->buttons[9]) {
            glm_vec3_scale(v_up, speed*dt, v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }

        // CAMERA ROTATION

        if (fabsf(joystick->axes[2]) > joystick->deadzone) {
            BE_Vec3RotateAxis(camera->direction, d_up, -dt*sensitivity*joystick->axes[2], d_direction);
            glm_vec3_normalize_to(d_direction, camera->direction);
        }
        if (fabsf(joystick->axes[3]) > joystick->deadzone) {
            BE_Vec3RotateAxis(camera->direction, d_right, dt*sensitivity*joystick->axes[3], d_direction);
            glm_vec3_normalize_to(d_direction, camera->direction);
        }
        
        glm_vec3_add(camera->position, v_move, camera->position);
    }

}

void BE_CameraMatrixUploadPersp(BE_Camera* camera, BE_Shader* shader, const char* uniform) {
    BE_ShaderActivate(shader);
    glUniform3fv(glGetUniformLocation(shader->ID, "camPos"), 1, (float*)camera->position);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, uniform), 1, GL_FALSE, (float*)camera->projPersp);
}

void BE_CameraMatrixUploadOrtho(BE_Camera* camera, BE_Shader* shader, const char* uniform) {
    BE_ShaderActivate(shader);
    glUniform3fv(glGetUniformLocation(shader->ID, "camPos"), 1, (float*)camera->position);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, uniform), 1, GL_FALSE, (float*)camera->projOrtho);
}

void BE_CameraMatrixUploadCustom(BE_Shader* shader, const char* uniform, vec3 position, mat4 matrix) {
    BE_ShaderActivate(shader);
    glUniform3fv(glGetUniformLocation(shader->ID, "camPos"), 1, (float*)position);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, uniform), 1, GL_FALSE, (float*)matrix);
}

#define INITIAL_CAMERA_CAPACITY 4

void BE_CameraVectorInit(BE_CameraVector* vec) {
    vec->data = (BE_Camera*)malloc(sizeof(BE_Camera) * INITIAL_CAMERA_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_CAMERA_CAPACITY;
}

void BE_CameraVectorPush(BE_CameraVector* vec, BE_Camera value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (BE_Camera*)realloc(vec->data, sizeof(BE_Camera) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void BE_CameraVectorFree(BE_CameraVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

void BE_CameraVectorCopy(BE_Camera* lights, size_t count, BE_CameraVector* outVec) {
    BE_CameraVectorInit(outVec);
    for (size_t i = 0; i < count; i++) {
        BE_CameraVectorPush(outVec, lights[i]);
    }
}

void BE_CameraVectorUpdateMatrix(BE_CameraVector* vec, int width, int height) {
    
    mat4 view;
    mat4 projection;
    mat4 projView;
    
    for (size_t i = 0; i < vec->size; i++) {
        BE_Camera* camera = &vec->data[i];
        
        float fov = camera->fov;

        camera->width = width;
        camera->height = height;

        vec3 target;
        glm_vec3_add(camera->position, camera->direction, target);
        glm_lookat(camera->position, target, camera->Up, view);
        glm_perspective(glm_rad(fov), (float)camera->width / (float)camera->height, camera->nearPlane, camera->farPlane, projection);
        glm_mat4_mul(projection, view, projView);
        glm_mat4_copy(projView, camera->projPersp);
        glm_mat4_copy(view, camera->viewMatrix);

        glm_ortho(0.0f, (float)camera->width, (float)camera->height, 0.0f, -1.0f, 1.0f, camera->projOrtho);

    }
}

void BE_CameraVectorDraw(BE_CameraVector* vec, BE_Mesh* mesh, BE_Shader* shader, BE_Camera* selected) {

    BE_ShaderActivate(shader);
    glUniform3fv(glGetUniformLocation(shader->ID, "color"), 1, (float[]){1.0f, 1.0f, 1.0f});
    
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    mat4 model;
    vec3 ori;

    for (size_t i = 0; i < vec->size; i++) {
        BE_Camera* camera = &vec->data[i];

        if (camera == selected) continue;
        
        BE_OritentationToEuler(camera->direction, ori);

        BE_MatrixMakeModel(camera->position, ori, (vec3){0.25f * camera->width/1000 * camera->fov/45, 0.25f * camera->height/1000, 0.2f * camera->zoom}, model);
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
        BE_MeshDraw(mesh, shader);

    }

    glEnable(GL_CULL_FACE);
}

// ==============================
// Mesh / Import
// ==============================

BE_Mesh BE_MeshInitFromVertex(BE_VertexVector vertices, BE_GLuintVector indices, BE_TextureVector textures) {
    BE_Mesh mesh;

    mesh.vertices = vertices;
    mesh.indices = indices;
    mesh.textures = textures;

    BE_VAO VAO1 = BE_VAOInit();
    BE_VAOBind(&VAO1);
    BE_VBO VBO1 = BE_VBOInitFromVector(&vertices);
    BE_EBO EBO1 = BE_EBOInitFromVector(&indices);
    BE_LinkVertexAttribToVBO(&VBO1, 0, 3, GL_FLOAT, sizeof(BE_Vertex), (void*)0);
    BE_LinkVertexAttribToVBO(&VBO1, 1, 3, GL_FLOAT, sizeof(BE_Vertex), (void*)(3 * sizeof(float)));
    BE_LinkVertexAttribToVBO(&VBO1, 2, 3, GL_FLOAT, sizeof(BE_Vertex), (void*)(6 * sizeof(float)));
    BE_LinkVertexAttribToVBO(&VBO1, 3, 2, GL_FLOAT, sizeof(BE_Vertex), (void*)(9 * sizeof(float)));
    BE_VAOUnbind();
    BE_VBOUnbind();
    BE_EBOUnbind();

    mesh.vao = VAO1;

    return mesh;
}

BE_Mesh BE_MeshInitFromData(const char** texbuffer, int texcount, BE_Vertex* vertices, int vertcount, GLuint* indices, int indcount) {

    BE_VertexVector verts;
    BE_GLuintVector inds;
    BE_TextureVector texs;

    BE_Texture textures[texcount];
    for (int i = 0; i < texcount; i++) {
        textures[i] = BE_TextureInit(texbuffer[i*2], texbuffer[i*2+1], i);
    }

    BE_VertexVectorCopy(vertices, vertcount, &verts);
    BE_GLuintVectorCopy(indices, indcount, &inds);
    BE_TextureVectorCopy(textures, texcount, &texs);

    BE_Mesh mesh = BE_MeshInitFromVertex(verts, inds, texs);

    return mesh;
}

void BE_MeshDraw(BE_Mesh* mesh, BE_Shader* shader) {
    BE_ShaderActivate(shader);
    BE_VAOBind(&mesh->vao);

    unsigned int numDiffuse = 0;
    unsigned int numSpecular = 0;

    if (mesh->textures.data) {
        for (unsigned int i = 0; i < mesh->textures.size; i++) {
            char num[16];
            char uniformName[256];
            char* type = mesh->textures.data[i].type;
            if (strcmp(type, "diffuse") == 0) {
                sprintf(num, "%d", numDiffuse++);
            } else if (strcmp(type, "specular") == 0) {
                sprintf(num, "%d", numSpecular++);
            }
            snprintf(uniformName, sizeof(uniformName), "%s%s", type, num);
            BE_TextureSetUniformUnit(shader, uniformName, i);
            BE_TextureBind(&mesh->textures.data[i]);
        }
    }

    glDrawElements(GL_TRIANGLES, mesh->indices.size, GL_UNSIGNED_INT, 0);
}

void BE_MeshDrawBillboard(BE_Mesh* mesh, BE_Shader* shader, BE_Texture* texture) {
    BE_ShaderActivate(shader);
    BE_VAOBind(&mesh->vao);

    BE_TextureSetUniformUnit(shader, "diffuse0", texture->unit);
    BE_TextureBind(texture);

    glDrawElements(GL_TRIANGLES, mesh->indices.size, GL_UNSIGNED_INT, 0);
}

int BE_FindOrAddVertex(BE_Vertex* vertices, int* verticesCount, BE_Vertex v) {
    for (int i = 0; i < *verticesCount; i++) {
        if (memcmp(&vertices[i], &v, sizeof(BE_Vertex)) == 0) {
            return i;
        }
    }

    int index = (*verticesCount)++;
    vertices[index] = v;
    return index;
}

int BE_CountFaceVertices(const char* line) {
    const char* ptr = line + 2;
    int count = 0;

    while (*ptr) {
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n') ptr++;
        if (*ptr == '\0') break;

        const char* token_start = ptr;

        while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\r' && *ptr != '\n') ptr++;

        char token[64];
        size_t len = ptr - token_start;
        if (len >= sizeof(token)) len = sizeof(token) - 1;
        memcpy(token, token_start, len);
        token[len] = '\0';

        int vi, vti, vni;
        if (sscanf(token, "%d/%d/%d", &vi, &vti, &vni) == 3 ||
            sscanf(token, "%d//%d", &vi, &vni) == 2 ||
            sscanf(token, "%d/%d", &vi, &vti) == 2 ||
            sscanf(token, "%d", &vi) == 1) {
            count++;
        }
    }

    return count;
}

void BE_ReplacePathSuffix(const char* path, const char* newsuffix, char* dest, int destsize) {
    const char* lastSlash = strrchr(path, '/');

    if (!lastSlash) {
        snprintf(dest, destsize, "%s", newsuffix);
        return;
    }

    int dirLen = lastSlash - path + 1;

    if (dirLen >= destsize) dirLen = destsize - 1;

    strncpy(dest, path, dirLen);
    dest[dirLen] = '\0';

    strncat(dest, newsuffix, destsize - strlen(dest) - 1);
}

BE_Mesh BE_LoadOBJToMesh(const char* obj_path) {
    BE_Mesh mesh;

    FILE* file = fopen(obj_path, "r");
    if (!file) {
        MSG_ERROR(obj_path, 1, "could not open file");
        exit(1);
    }

    vec3* positions = (vec3*)malloc(sizeof(vec3) * 100000);
    int positionsCount = 0;
    if (!positions) {
        MSG_FATAL(obj_path, 1, "could not allocate memory for mesh positions");
        exit(1);
    }
    
    vec3* normals = (vec3*)malloc(sizeof(vec3) * 100000);
    int normalsCount = 0;
    if (!normals) {
        MSG_FATAL(obj_path, 1, "could not allocate memory for mesh normals");
        exit(1);
    }
    
    vec2* uvs = (vec2*)malloc(sizeof(vec2) * 100000);
    int uvsCount = 0;
    if (!uvs) {
        MSG_FATAL(obj_path, 1, "could not allocate memory for mesh uvs");
        exit(1);
    }

    BE_Vertex* vertices = (BE_Vertex*)malloc(sizeof(BE_Vertex) * 100000);
    int verticesCount = 0;
    if (!vertices) {
        MSG_FATAL(obj_path, 1, "could not allocate memory for mesh vertices");
        exit(1);
    }
    
    GLuint* indices =  (GLuint*)malloc(sizeof(GLuint) * 100000);
    int indicesCount = 0;
    if (!indices) {
        MSG_FATAL(obj_path, 1, "could not allocate memory for mesh indices");
        exit(1);
    }

    const char** textures;
    int texturesCount = 0;

    char line[546];
    int lineNum = 0;

    while (fgets(line, sizeof(line), file)) {
        lineNum++;
        
        if ( 
            line[0] == '#' || 
            line[0] == '\n' || 
            strncmp(line, "o ", 2) == 0 || 
            strncmp(line, "s ", 2) == 0
        ) continue;

        if (strncmp(line, "mtllib ", 7) == 0) {
            char mtl_file[256] = {0};
            char mtl_filepath[256] = {0};

            sscanf(line, "mtllib %s", mtl_file);
            BE_ReplacePathSuffix(obj_path, mtl_file, mtl_filepath, sizeof(mtl_filepath));

            textures = BE_LoadMTLTextures(mtl_filepath, &texturesCount);

        } else if (strncmp(line, "v ", 2) == 0) {

            vec3 v;
            if (sscanf(line, "v %f %f %f", &v[0], &v[1], &v[2]) == 3) {
                glm_vec3_copy(v, positions[positionsCount++]);
            } else {
                MSG_ERROR(obj_path, lineNum, "broken position vertex");
                continue;
            }

        } else if (strncmp(line, "vt ", 3) == 0) {

            vec2 vt;
            if (sscanf(line, "vt %f %f", &vt[0], &vt[1]) == 2) {
                glm_vec2_copy(vt, uvs[uvsCount++]);
            } else {
                MSG_ERROR(obj_path, lineNum, "broken uv vertex");
                continue;
            }

        } else if (strncmp(line, "vn ", 3) == 0) {

            vec3 vn;
            if (sscanf(line, "vn %f %f %f", &vn[0], &vn[1], &vn[2]) == 3) {
                glm_vec3_copy(vn, normals[normalsCount++]);
            } else {
                MSG_ERROR(obj_path, lineNum, "broken normal vertex");
                continue;
            }

        } else if (strncmp(line, "f ", 2) == 0) {
            
            int faceVertCount = BE_CountFaceVertices(line);

            char* token = strtok(line+2, " \t\r\n");
            
            BE_Vertex* verts = (BE_Vertex*)malloc(sizeof(BE_Vertex) * faceVertCount);
            int numVerts = 0;

            while (token != NULL) {

                int vi, vti, vni;

                if (sscanf(token, "%d/%d/%d", &vi, &vti, &vni) == 3) {

                    vi--; vti--; vni--;
                    glm_vec3_copy(positions[vi], verts[numVerts].position);
                    glm_vec3_copy(normals[vni], verts[numVerts].normal);
                    glm_vec3_copy((vec3){1.0f,1.0f,1.0f}, verts[numVerts].color);
                    glm_vec2_copy(uvs[vti], verts[numVerts].texUV);

                } else if (sscanf(token, "%d//%d", &vi, &vni) == 2) {

                    vi--; vni--;
                    glm_vec3_copy(positions[vi], verts[numVerts].position);
                    glm_vec3_copy(normals[vni], verts[numVerts].normal);
                    glm_vec3_copy((vec3){1.0f,1.0f,1.0f}, verts[numVerts].color);
                    glm_vec2_copy((vec2){0.0f,0.0f}, verts[numVerts].texUV);

                } else if (sscanf(token, "%d/%d", &vi, &vti) == 2) {

                    vi--; vni--;
                    glm_vec3_copy(positions[vi], verts[numVerts].position);
                    glm_vec3_copy((vec3){0.0f,0.0f,1.0f}, verts[numVerts].normal);
                    glm_vec3_copy((vec3){1.0f,1.0f,1.0f}, verts[numVerts].color);
                    glm_vec2_copy(uvs[vti], verts[numVerts].texUV);

                } else if (sscanf(token, "%d", &vi) == 1) {

                    vi--;
                    glm_vec3_copy(positions[vi], verts[numVerts].position);
                    glm_vec3_copy((vec3){0.0f,0.0f,1.0f}, verts[numVerts].normal);
                    glm_vec3_copy((vec3){1.0f,1.0f,1.0f}, verts[numVerts].color);
                    glm_vec2_copy((vec2){0.0f,0.0f}, verts[numVerts].texUV);

                } else {
                    MSG_ERROR(obj_path, lineNum, "broken face vertex '%s'", token);
                }

                token = strtok(NULL, " \t\r\n");
                numVerts++;
            }

            for (int i = 1; i < numVerts - 1; i++) {
                int i0 = BE_FindOrAddVertex(vertices, &verticesCount, verts[0]);
                int i1 = BE_FindOrAddVertex(vertices, &verticesCount, verts[i]);
                int i2 = BE_FindOrAddVertex(vertices, &verticesCount, verts[i + 1]);

                indices[indicesCount++] = i1;
                indices[indicesCount++] = i0;
                indices[indicesCount++] = i2;
            }

            free(verts);
            
        } else {
            line[strcspn(line, "\n")] = '\0';
            MSG_WARNING(obj_path, lineNum, "unsupported OBJ directive '%s'", line);
            continue;
        }

    }

    fclose(file);

    if (texturesCount == 0) {
        static const char* fallbackTextures[] = {
            "res/textures/null.jpg",
            "diffuse"
        };
        textures = fallbackTextures;
        texturesCount = 2;
    }
    
    mesh = BE_MeshInitFromData(textures, 1, vertices, verticesCount, indices, indicesCount);

    free(positions);
    free(normals);
    free(uvs);
    free(vertices);
    free(indices);

    MSG_INFO(obj_path, lineNum, "model loaded succesfully");
    return mesh;
}

const char** BE_LoadMTLTextures(const char* mtl_path, int* outCount) {

    FILE* file = fopen(mtl_path, "r");
    if (!file) {
        MSG_ERROR(mtl_path, 1, "could not open file");
        exit(1);
    }
    
    const char** textures = (const char**)malloc(sizeof(char*) * 50);
    int count = 0;
    if (!textures) {
        MSG_FATAL(mtl_path, 1, "could not allocate memory for mesh textures");
        exit(1);
    }

    char line[256];
    int lineNum = 0;

    while (fgets(line, sizeof(line), file)) {
        lineNum++;
        
        if ( 
            line[0] == '#' || 
            line[0] == '\n'
        ) continue;

        if (strncmp(line, "map_Kd ", 7) == 0) {

            char fileRelPath[256];
            sscanf(line, "map_Kd %s", fileRelPath);

            char texturePath[512];
            BE_ReplacePathSuffix(mtl_path, fileRelPath, texturePath, sizeof(texturePath));

            textures[count++] = strdup(texturePath);
            textures[count++] = strdup("diffuse");

        } else if (strncmp(line, "map_Ks ", 7) == 0) {
            
            char fileRelPath[256];
            sscanf(line, "map_Ks %s", fileRelPath);

            char texturePath[512];
            BE_ReplacePathSuffix(mtl_path, fileRelPath, texturePath, sizeof(texturePath));

            textures[count++] = strdup(texturePath);
            textures[count++] = strdup("specular");
        
        } else {
            line[strcspn(line, "\n")] = '\0';
            MSG_WARNING(mtl_path, lineNum, "unsupported MTL directive '%s'", line);
            continue;
        }
    }

    fclose(file);

    if (outCount) *outCount = count;
    return textures;
}

// ==============================
// Models
// ==============================

BE_Transform BE_TransformInit(vec3 position, vec3 rotation, vec3 scale) {
    BE_Transform transform;
    glm_vec3_copy(position, transform.position);
    glm_vec3_copy(rotation, transform.rotation);
    glm_vec3_copy(scale, transform.scale);
    return transform;
}

BE_Model BE_ModelInit(BE_Mesh* mesh, BE_Transform transform) {
    BE_Model model;
    model.mesh = mesh;
    model.transform = transform;
    // glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, model.transform.position);
    // glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, model.transform.rotation);
    // glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, model.transform.scale);
    return model;
}

#define INITIAL_MODEL_CAPACITY 16

void BE_ModelVectorInit(BE_ModelVector* vec) {
    vec->data = (BE_Model*)malloc(sizeof(BE_Model) * INITIAL_MODEL_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_MODEL_CAPACITY;
}

void BE_ModelVectorPush(BE_ModelVector* vec, BE_Model value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (BE_Model*)realloc(vec->data, sizeof(BE_Model) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void BE_ModelVectorFree(BE_ModelVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

void BE_ModelVectorCopy(BE_Model* models, size_t count, BE_ModelVector* outVec) {
    BE_ModelVectorInit(outVec);
    for (size_t i = 0; i < count; i++) {
        BE_ModelVectorPush(outVec, models[i]);
    }
}

void BE_ModelVectorDraw(BE_ModelVector* vec, BE_Shader* shader) {
    
    BE_ShaderActivate(shader);

    mat4 modelMatrix;

    for (size_t i = 0; i < vec->size; i++) {
        BE_Model* model = &vec->data[i];

        BE_MatrixMakeModel(model->transform.position, model->transform.rotation, model->transform.scale, modelMatrix);
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)modelMatrix);
        BE_MeshDraw(model->mesh, shader);

    }

}

// ==============================
// Lights
// ==============================

BE_ShadowMapFBO BE_ShadowMapFBOInit(int width, int height, int layers) {
    BE_ShadowMapFBO smfbo = {0};
    smfbo.width = width;
    smfbo.height = height;
    smfbo.layers = layers;

    glGenFramebuffers(1, &smfbo.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, smfbo.fbo);

    glGenTextures(1, &smfbo.depthTextureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, smfbo.depthTextureArray);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32,
                 width, height, layers, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, smfbo.depthTextureArray, 0, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return smfbo;
}

void BE_ShadowMapFBOBindLayer(BE_ShadowMapFBO* smfbo, int layer) {
    glBindFramebuffer(GL_FRAMEBUFFER, smfbo->fbo);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, smfbo->depthTextureArray, 0, layer);
   
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
}

void BE_ShadowMapFBODelete(BE_ShadowMapFBO* smfbo) {
    glDeleteFramebuffers(1, &smfbo->fbo);
    glDeleteTextures(1, &smfbo->depthTextureArray);
}

BE_Light BE_LightInit(int type, vec3 position, vec3 direction, vec4 color, float specular, float a, float b, float innerCone, float outerCone) {

    BE_Light light = {0};
    
    light.type = type;
    glm_vec4_copy(color, light.color);
    light.specular = specular;

    switch (type) {
        case LIGHT_DIRECT:
            glm_vec3_copy(direction, light.direction);
            break;

        case LIGHT_POINT:
            glm_vec3_copy(position, light.position);
            light.a = a;
            light.b = b;
            break;

        case LIGHT_SPOT:
            glm_vec3_copy(position, light.position);
            glm_vec3_copy(direction, light.direction);
            light.innerCone = innerCone;
            light.outerCone = outerCone;
            break;
        
        default:
            printf("invalid light type\n");
            break;
    }

    return light;
}

#define INITIAL_LIGHT_CAPACITY 4

void BE_LightVectorInit(BE_LightVector* vec) {
    vec->data = (BE_Light*)malloc(sizeof(BE_Light) * INITIAL_LIGHT_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_LIGHT_CAPACITY;

    vec->ambient = 0.15f;
    vec->directShadowFBO = BE_ShadowMapFBOInit(1024*4, 1024*4, 1);
    vec->pointShadowFBO = BE_ShadowMapFBOInit(250, 250, 10);
    vec->spotShadowFBO = BE_ShadowMapFBOInit(250, 250, 10);
}

void BE_LightVectorPush(BE_LightVector* vec, BE_Light value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (BE_Light*)realloc(vec->data, sizeof(BE_Light) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void BE_LightVectorFree(BE_LightVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

void BE_LightVectorCopy(BE_Light* lights, size_t count, BE_LightVector* outVec) {
    BE_LightVectorInit(outVec);
    for (size_t i = 0; i < count; i++) {
        BE_LightVectorPush(outVec, lights[i]);
    }
}

void BE_LightVectorUpdateMatrix(BE_LightVector* vec) {
    mat4 projection, view;
    vec3 position, direction;
    
    for (size_t i = 0; i < vec->size; i++) {
        BE_Light* light = &vec->data[i];

        switch (vec->data[i].type) {
            case LIGHT_DIRECT:
                glm_normalize_to(light->direction, direction);
                glm_vec3_scale(direction, -DIRECT_LIGHT_DIST, position);
                glm_ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, 100.0f, projection);
                glm_lookat(position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f}, view);
                glm_mat4_mul(projection, view, light->lightSpaceMatrix);
                
                break;
            case LIGHT_POINT:
                break;
            case LIGHT_SPOT:
                break;
            default:
                break;
        }
    }
}

void BE_LightVectorUpdateMaps(BE_LightVector* vec, BE_Shader* shadowShader, ShadowRenderFunc renderFunc, bool enabled) {
    
    BE_ShaderActivate(shadowShader);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (!enabled) {

        if (vec->shadowsDirty == 1) {
            for (int layer = 0; layer < vec->directShadowFBO.layers; layer++) {
                BE_ShadowMapFBOBindLayer(&vec->directShadowFBO, layer);
                glViewport(0, 0, vec->directShadowFBO.width, vec->directShadowFBO.height);
                glClear(GL_DEPTH_BUFFER_BIT);
            }

            // for (int layer = 0; layer < vec->pointShadowFBO.layers; layer++) {
            //     BE_ShadowMapFBOBindLayer(&vec->pointShadowFBO, layer);
            //     glViewport(0, 0, vec->pointShadowFBO.width, vec->pointShadowFBO.height);
            //     glClear(GL_DEPTH_BUFFER_BIT);
            // }

            for (int layer = 0; layer < vec->spotShadowFBO.layers; layer++) {
                BE_ShadowMapFBOBindLayer(&vec->spotShadowFBO, layer);
                glViewport(0, 0, vec->spotShadowFBO.width, vec->spotShadowFBO.height);
                glClear(GL_DEPTH_BUFFER_BIT);
            }

            BE_FBOUnbind();

            vec->shadowsDirty = 0;
        }

        return;
    } else {
        vec->shadowsDirty = 1;
    }

    for (size_t i = 0; i < vec->size; i++) {
        BE_Light* light = &vec->data[i];

        switch (vec->data[i].type) {
            case LIGHT_DIRECT:
                BE_ShadowMapFBOBindLayer(&vec->directShadowFBO, 0);
                glViewport(0, 0, vec->directShadowFBO.width, vec->directShadowFBO.height);
                glClear(GL_DEPTH_BUFFER_BIT);
                glUniformMatrix4fv(glGetUniformLocation(shadowShader->ID, "lightSpaceMatrix"), 1, GL_FALSE, (float*)light->lightSpaceMatrix);
                renderFunc(shadowShader);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                break;
            case LIGHT_POINT:
                break;
            case LIGHT_SPOT:
                BE_ShadowMapFBOBindLayer(&vec->spotShadowFBO, i);
                glViewport(0, 0, vec->spotShadowFBO.width, vec->spotShadowFBO.height);
                glClear(GL_DEPTH_BUFFER_BIT);
                glUniformMatrix4fv(glGetUniformLocation(shadowShader->ID, "lightSpaceMatrix"), 1, GL_FALSE, (float*)light->lightSpaceMatrix);
                renderFunc(shadowShader);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                break;
            default:
                break;
        }
    }

}

void BE_LightVectorUpdateMultiMaps(BE_LightVector* vec, BE_ModelVector* models, BE_Shader* shadowShader, bool enabled) {
    
    BE_ShaderActivate(shadowShader);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (!enabled) {

        if (vec->shadowsDirty == 1) {
            for (int layer = 0; layer < vec->directShadowFBO.layers; layer++) {
                BE_ShadowMapFBOBindLayer(&vec->directShadowFBO, layer);
                glViewport(0, 0, vec->directShadowFBO.width, vec->directShadowFBO.height);
                glClear(GL_DEPTH_BUFFER_BIT);
            }

            // for (int layer = 0; layer < vec->pointShadowFBO.layers; layer++) {
            //     BE_ShadowMapFBOBindLayer(&vec->pointShadowFBO, layer);
            //     glViewport(0, 0, vec->pointShadowFBO.width, vec->pointShadowFBO.height);
            //     glClear(GL_DEPTH_BUFFER_BIT);
            // }

            for (int layer = 0; layer < vec->spotShadowFBO.layers; layer++) {
                BE_ShadowMapFBOBindLayer(&vec->spotShadowFBO, layer);
                glViewport(0, 0, vec->spotShadowFBO.width, vec->spotShadowFBO.height);
                glClear(GL_DEPTH_BUFFER_BIT);
            }

            BE_FBOUnbind();

            vec->shadowsDirty = 0;
        }

        return;
    } else {
        vec->shadowsDirty = 1;
    }

    for (size_t i = 0; i < vec->size; i++) {
        BE_Light* light = &vec->data[i];

        switch (vec->data[i].type) {
            case LIGHT_DIRECT:
                BE_ShadowMapFBOBindLayer(&vec->directShadowFBO, 0);
                glViewport(0, 0, vec->directShadowFBO.width, vec->directShadowFBO.height);
                glClear(GL_DEPTH_BUFFER_BIT);
                glUniformMatrix4fv(glGetUniformLocation(shadowShader->ID, "lightSpaceMatrix"), 1, GL_FALSE, (float*)light->lightSpaceMatrix);

                BE_ModelVectorDraw(models, shadowShader);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                break;
            case LIGHT_POINT:
                break;
            case LIGHT_SPOT:
                BE_ShadowMapFBOBindLayer(&vec->spotShadowFBO, i);
                glViewport(0, 0, vec->spotShadowFBO.width, vec->spotShadowFBO.height);
                glClear(GL_DEPTH_BUFFER_BIT);
                glUniformMatrix4fv(glGetUniformLocation(shadowShader->ID, "lightSpaceMatrix"), 1, GL_FALSE, (float*)light->lightSpaceMatrix);

                BE_ModelVectorDraw(models, shadowShader);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                break;
            default:
                break;
        }
    }

}

void BE_LightVectorUpload(BE_LightVector* vec, BE_Shader* shader) {
    
    BE_ShaderActivate(shader);

    int numLights[3] = {0, 0, 0};
    
    glUniform1f(glGetUniformLocation(shader->ID, "ambient"), vec->ambient);
    
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D_ARRAY, vec->directShadowFBO.depthTextureArray);
    glUniform1i(glGetUniformLocation(shader->ID, "directShadowMapArray"), 3);
    
    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D_ARRAY, vec->pointShadowFBO.depthTextureArray);
    glUniform1i(glGetUniformLocation(shader->ID, "pointShadowMapArray"), 4);
    
    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_2D_ARRAY, vec->spotShadowFBO.depthTextureArray);
    glUniform1i(glGetUniformLocation(shader->ID, "spotShadowMapArray"), 5);

    char buffer[256];

    for (size_t i = 0; i < vec->size; i++) {
        BE_Light* light = &vec->data[i];

        switch (vec->data[i].type) {
            case LIGHT_DIRECT:
                snprintf(buffer, sizeof(buffer), "directlights[%d].direction", numLights[light->type]);
                glUniform3fv(glGetUniformLocation(shader->ID, buffer), 1, (float*)light->direction);
                snprintf(buffer, sizeof(buffer), "directlights[%d].color", numLights[light->type]);
                glUniform4fv(glGetUniformLocation(shader->ID, buffer), 1, (float*)light->color);
                snprintf(buffer, sizeof(buffer), "directlights[%d].specular", numLights[light->type]);
                glUniform1f(glGetUniformLocation(shader->ID, buffer), light->specular);
                snprintf(buffer, sizeof(buffer), "directlights[%d].lightSpaceMatrix", numLights[light->type]);
                glUniformMatrix4fv(glGetUniformLocation(shader->ID, buffer), 1, GL_FALSE, (float*)light->lightSpaceMatrix);
             
                break;
            case LIGHT_POINT:
                snprintf(buffer, sizeof(buffer), "pointlights[%d].position", numLights[light->type]);
                glUniform3fv(glGetUniformLocation(shader->ID, buffer), 1, (float*)light->position);
                snprintf(buffer, sizeof(buffer), "pointlights[%d].color", numLights[light->type]);
                glUniform4fv(glGetUniformLocation(shader->ID, buffer), 1, (float*)light->color);
                snprintf(buffer, sizeof(buffer), "pointlights[%d].specular", numLights[light->type]);
                glUniform1f(glGetUniformLocation(shader->ID, buffer), light->specular);
                snprintf(buffer, sizeof(buffer), "pointlights[%d].a", numLights[light->type]);
                glUniform1f(glGetUniformLocation(shader->ID, buffer), light->a);
                snprintf(buffer, sizeof(buffer), "pointlights[%d].b", numLights[light->type]);
                glUniform1f(glGetUniformLocation(shader->ID, buffer), light->b);
                snprintf(buffer, sizeof(buffer), "pointlights[%d].lightSpaceMatrix", numLights[light->type]);
                glUniformMatrix4fv(glGetUniformLocation(shader->ID, buffer), 1, GL_FALSE, (float*)light->lightSpaceMatrix);

                break;
            case LIGHT_SPOT:
                snprintf(buffer, sizeof(buffer), "spotlights[%d].position", numLights[light->type]);
                glUniform3fv(glGetUniformLocation(shader->ID, buffer), 1, (float*)light->position);
                snprintf(buffer, sizeof(buffer), "spotlights[%d].direction", numLights[light->type]);
                glUniform3fv(glGetUniformLocation(shader->ID, buffer), 1, (float*)light->direction);
                snprintf(buffer, sizeof(buffer), "spotlights[%d].color", numLights[light->type]);
                glUniform4fv(glGetUniformLocation(shader->ID, buffer), 1, (float*)light->color);
                snprintf(buffer, sizeof(buffer), "spotlights[%d].specular", numLights[light->type]);
                glUniform1f(glGetUniformLocation(shader->ID, buffer), light->specular);
                snprintf(buffer, sizeof(buffer), "spotlights[%d].innerCone", numLights[light->type]);
                glUniform1f(glGetUniformLocation(shader->ID, buffer), light->innerCone);
                snprintf(buffer, sizeof(buffer), "spotlights[%d].outerCone", numLights[light->type]);
                glUniform1f(glGetUniformLocation(shader->ID, buffer), light->outerCone);
                snprintf(buffer, sizeof(buffer), "spotlights[%d].lightSpaceMatrix", numLights[light->type]);
                glUniformMatrix4fv(glGetUniformLocation(shader->ID, buffer), 1, GL_FALSE, (float*)light->lightSpaceMatrix);

                break;
            default:
                break;
        }

        numLights[light->type]+=1;
    }

    for (int i = 0; i < 3; i++) {
        glUniform1i(glGetUniformLocation(shader->ID, "numDirects"), (int)numLights[0]);
        glUniform1i(glGetUniformLocation(shader->ID, "numPoints"), (int)numLights[1]);
        glUniform1i(glGetUniformLocation(shader->ID, "numSpots"), (int)numLights[2]);
    }

}

void BE_LightVectorDraw(BE_LightVector* vec, BE_Mesh* mesh, BE_Shader* shader) {

    BE_ShaderActivate(shader);

    vec3 scale = { 0.1f, 0.1f, 0.1f };
    mat4 model;

    for (size_t i = 0; i < vec->size; i++) {
        BE_Light* light = &vec->data[i];
        
        switch (vec->data[i].type) {
            case LIGHT_DIRECT:
                // BE_MatrixMakeModel((vec3){0.0f, 2.0f, 0.0f}, light->direction, scale, model);
                continue;
                // break;
            case LIGHT_POINT:
                BE_MatrixMakeModel(light->position, (vec3){0.0f, 0.0f, 0.0f}, scale, model);
                break;
            case LIGHT_SPOT:
                BE_MatrixMakeModel(light->position, light->direction, scale, model);
                break;
            default:
                continue;
        }
        
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
        glUniform3fv(glGetUniformLocation(shader->ID, "color"), 1, (float*)light->color);
        BE_VAOBind(&mesh->vao);
        glDrawElements(GL_TRIANGLES, mesh->indices.size, GL_UNSIGNED_INT, 0);
    }

}

// ==============================
// Text
// ==============================

// BE_Font BE_FontLoad(const char* ttf_path, int pixelSize) {
//     BE_Font font = {0};
//
//     FILE* file = fopen(ttf_path, "rb");
//     if (!file) {
//         MSG_ERROR(ttf_path, 1, "could not open file");
//         exit(1);
//     }
//     fseek(file, 0, SEEK_END);
//     long size = ftell(file);
//     rewind(file);
//
//     unsigned char* ttfbuffer = malloc(size);
//     fread(ttfbuffer, 1, size, file);
//     fclose(file);
//
//     stbtt_fontinfo info;
//     if (!stbtt_InitFont(&info, ttfbuffer, 0)) {
//         MSG_ERROR(ttf_path, 1, "failed to load font");
//         free(ttfbuffer);
//         exit(1);
//     }
//
//     font.glyphCount = 128;
//     font.glyphs = calloc(font.glyphCount, sizeof(BE_Glyph));
//
//     font.atlasWidth = 1024;
//     font.atlasHeight = 1024;
//     unsigned char* atlasPixels = calloc(font.atlasWidth * font.atlasHeight, 1);
//
//     int penX = 0, penY = 0, rowHeight = 0;
//     float scale = stbtt_ScaleForPixelHeight(&info, (float)pixelSize);
//
//     for (unsigned char c = 32; c < 128; c++) {
//         int width, height, xoff, yoff;
//         unsigned char* bmp = stbtt_GetCodepointBitmap(&info, 0, scale, c, &width, &height, &xoff, &yoff);
//
//         if (penX + width >= font.atlasWidth) {
//             penX = 0;
//             penY += rowHeight + 1;
//             rowHeight = 0;
//         }
//
//         for (int y = 0; y < height; y++) {
//             for (int x =0; x < width; x++) {
//                 int atlasIndex = (penY + y) * font.atlasWidth + (penX + x);
//                 atlasPixels[atlasIndex] = bmp[y * width + x];
//             }
//         }
//
//         font.glyphs[c].u0 = (float)penX / font.atlasWidth;
//         font.glyphs[c].v0 = (float)penY / font.atlasHeight;
//         font.glyphs[c].u1 = (float)(penX + width) / font.atlasWidth;
//         font.glyphs[c].v1 = (float)(penY + height) / font.atlasHeight;
//      
//         font.glyphs[c].width = width;
//         font.glyphs[c].height = height;
//         font.glyphs[c].bearingX = xoff;
//         font.glyphs[c].bearingY = yoff;
//         int advance, lsb;
//         stbtt_GetCodepointHMetrics(&info, c, &advance, &lsb);
//         font.glyphs[c].advance = (int)(advance * scale);
//
//         if (height > rowHeight) rowHeight = height;
//         penX += width + 1;
//
//         stbtt_FreeBitmap(bmp, NULL);
//     }
//
//     int ascent, descent, lineGap;
//     stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
//     font.lineHeight = (int)((ascent - descent + lineGap) * scale);
//
//     glGenTextures(1, &font.atlasTex);
//     glBindTexture(GL_TEXTURE_2D, font.atlasTex);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font.atlasWidth, font.atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, atlasPixels);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//     free(atlasPixels);
//     free(ttfbuffer);
//  
//     MSG_INFO(ttf_path, 1, "font loaded succesfully");
//     return font;
// }

// BE_Text BE_TextInit(const char* text, BE_Font* font, vec3 position, float scale, vec4 color, bool dynamic) {
//     BE_Text t = {0};
//
//     t.font = font;
//     t.text = strdup(text);
//     glm_vec3_copy(position, t.position);
//     glm_vec4_copy(color, t.color);
//     t.scale = scale;
//     t.isDynamic = dynamic;
//     t.quadCount = strlen(text);
//
//     glGenVertexArrays(1, &t.vao);
//     glGenBuffers(1, &t.vbo);
//
//     glBindVertexArray(t.vao);
//     glBindBuffer(GL_ARRAY_BUFFER, t.vbo);
//
//     size_t bufferSize = t.quadCount * 6 * 4 * sizeof(float);
//     glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, t.isDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
//
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
//
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
//
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
//
//     if (!dynamic) {
//         BE_TextUpdate(&t, text);
//     }
//
//     return t;
// }

// void BE_TextUpdate(BE_Text* t, const char* newText) {
//     if (!t->isDynamic) return;
//
//     if (strcmp(t->text, newText) == 0) return;
//
//     free(t->text);
//     t->text = strdup(newText);
//
//     int len = strlen(newText);
//     t->quadCount = len;
//     t->dirty = true;
//
//     printf("%s\n", newText);
//
//     size_t bufferSize = len * 6 * 4 * sizeof(float);
//     float* vertices = (float*)malloc(bufferSize);
//
//     float x = t->position[0];
//     float y = t->position[1];
//     int cursor = 0;
//
//     for (int i = 0; i < len; i++) {
//         unsigned char c = newText[i];
//         if (c < 32 || c >= 128) continue;
//
//         BE_Glyph* g = &t->font->glyphs[c];
//
//         float xpos = x + g->bearingX * t->scale;
//         float ypos = y - (g->height - g->bearingY) * t->scale;
//         float w = g->width * t->scale;
//         float h = g->height * t->scale;
//
//         float quad[6][4] = {
//             { xpos,     ypos + h, g->u0, g->v1 },
//             { xpos,     ypos,     g->u0, g->v0 },
//             { xpos + w, ypos,     g->u1, g->v0 },
//
//             { xpos,     ypos + h, g->u0, g->v1 },
//             { xpos + w, ypos,     g->u1, g->v0 },
//             { xpos + w, ypos + h, g->u1, g->v1 }
//         };
//
//         memcpy(vertices + cursor, quad, sizeof(quad));
//         cursor += 6 * 4;
//
//         x += g->advance * t->scale;
//     }
//
//     glBindBuffer(GL_ARRAY_BUFFER, t->vbo);
//     glBufferData(GL_ARRAY_BUFFER, bufferSize, vertices, GL_DYNAMIC_DRAW);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//     free(vertices);
// }

// void BE_TextRender(BE_Text* t, GLuint shaderProgram, mat4 proj) {
//     glUseProgram(shaderProgram);
//
//     glActiveTexture(GL_TEXTURE0);
//     glBindTexture(GL_TEXTURE_2D, t->font->atlasTex);
//     glUniform1i(glGetUniformLocation(shaderProgram, "uFontAtlas"), 0);
//
//     glUniform4fv(glGetUniformLocation(shaderProgram, "uTextColor"), 1, t->color);
//     glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, proj[0]);
//
//     glEnable(GL_BLEND);
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//     glDisable(GL_DEPTH_TEST);
//
//     glBindVertexArray(t->vao);
//     glDrawArrays(GL_TRIANGLES, 0, t->quadCount * 6);
//     glBindVertexArray(0);
//
//     glUseProgram(0);
// }

// #define INITIAL_TEXT_CAPACITY 4

// void BE_TextVectorInit(BE_TextVector* vec) {
//     vec->data = (BE_Text*)malloc(sizeof(BE_Text) * INITIAL_TEXT_CAPACITY);
//     vec->size = 0;
//     vec->capacity = INITIAL_TEXT_CAPACITY;
// }

// void BE_TextVectorPush(BE_TextVector* vec, BE_Text value) {
//     if (vec->size >= vec->capacity) {
//         vec->capacity *= 2;
//         vec->data = (BE_Text*)realloc(vec->data, sizeof(BE_Text) * vec->capacity);
//     }
//     vec->data[vec->size++] = value;
// }

// void BE_TextVectorFree(BE_TextVector* vec) {
//     free(vec->data);
//     vec->data = NULL;
//     vec->size = 0;
//     vec->capacity = 0;
// }

// void BE_TextVectorCopy(BE_Text* texts, size_t count, BE_TextVector* outVec) {
//     BE_TextVectorInit(outVec);
//     for (size_t i = 0; i < count; i++) {
//         BE_TextVectorPush(outVec, texts[i]);
//     }
// }

// ==============================
// Sprite
// ==============================

BE_Sprite BE_SpriteInit(vec3 position, vec2 scale, float rotation, vec3 color, BE_Texture* texture) {
    BE_Sprite sprite = {0};

    glm_vec3_copy(position, sprite.position);
    glm_vec2_copy(scale, sprite.scale);
    glm_vec3_copy(color, sprite.color);

    sprite.rotation = rotation;
    sprite.texture = texture;
    
    return sprite;
}

#define INITIAL_SPRITE_CAPACITY 4

void BE_SpriteVectorInit(BE_SpriteVector* vec) {
    vec->data = (BE_Sprite*)malloc(sizeof(BE_Sprite) * INITIAL_SPRITE_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_SPRITE_CAPACITY;

    vec->vao = BE_VAOInitSprite();
}

void BE_SpriteVectorPush(BE_SpriteVector* vec, BE_Sprite value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (BE_Sprite*)realloc(vec->data, sizeof(BE_Sprite) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void BE_SpriteVectorFree(BE_SpriteVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

void BE_SpriteVectorCopy(BE_Sprite* sprites, size_t count, BE_SpriteVector* outVec) {
    BE_SpriteVectorInit(outVec);
    for (size_t i = 0; i < count; i++) {
        BE_SpriteVectorPush(outVec, sprites[i]);
    }
}

void BE_SpriteVectorDraw(BE_SpriteVector* vec, BE_Shader* shader) {
    BE_ShaderActivate(shader);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    mat4 model;

    for (size_t i = 0; i < vec->size; i++) {
        BE_Sprite* sprite = &vec->data[i];

        glm_mat4_identity(model);
        glm_translate(model, sprite->position);
        glm_translate(model, (vec3){0.0f, 0.0f, 0.0f}); // optional pivot
        glm_rotate(model, sprite->rotation, (vec3){0.0f, 0.0f, 1.0f});
        glm_scale(model, (vec3){sprite->scale[0], sprite->scale[1], 1.0f});

        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
        glUniform3fv(glGetUniformLocation(shader->ID, "spriteColor"), 1, (float*)sprite->color);

        BE_TextureBind(sprite->texture);
        BE_TextureSetUniformUnit(shader, "spriteTexture", 0);

        BE_VAOBind(&vec->vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

// ==============================
// Particles
// ==============================

// BE_GPUParticles BE_ParticlesInit(const char* vertexFile, const char* fragmentFile, const char* computeFile, const char* imageFile) {
//     BE_GPUParticles ps = {0};
//
//     ps.particleCount = MAX_PARTICLES;
//
//     ps.sprite = BE_TextureInit(imageFile, "sprite", 0);
//     // ps.computeShader = BE_ShaderInit(NULL, NULL, NULL, computeFile);
//     // ps.renderShader = BE_ShaderInit(vertexFile, fragmentFile, NULL, NULL);
//
//     glGenBuffers(1, &ps.ssbo);
//     glBindBuffer(GL_SHADER_STORAGE_BUFFER, ps.ssbo);
//     GLsizeiptr bufSize = (GLsizeiptr)(MAX_PARTICLES * sizeof(BE_ParticleGPU));
//     glBufferData(GL_SHADER_STORAGE_BUFFER, bufSize, NULL, GL_DYNAMIC_DRAW);
//
//     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ps.ssbo);
//
//     BE_ParticleGPU* ptr = (BE_ParticleGPU*)glMapBufferRange(
//         GL_SHADER_STORAGE_BUFFER,
//         0,
//         bufSize,
//         GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
//     );
//
//     if (ptr == NULL) {
//         fprintf(stderr, "ERROR: glMapBufferRange returned NULL — SSBO mapping failed.\n");
//         glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//         return ps;
//     }
//
//     for (int i = 0; i < MAX_PARTICLES; i++) {
//         glm_vec4_copy((vec4){0.0f,0.0f,0.0f,0.0f}, ptr[i].pos_life);
//         glm_vec4_copy((vec4){0.0f,0.0f,0.0f,0.0f}, ptr[i].vel_age);
//         glm_vec4_copy((vec4){1.0f,1.0f,1.0f,16.0f}, ptr[i].col_size);
//     }
//
//     glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
//     glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//
//     ps.vao = BE_VAOInit();
//
//     return ps;
// }

// void BE_ParticlesUpdate(BE_GPUParticles* ps, float dt, uint32_t frame) {
//     BE_ShaderActivate(&ps->computeShader);
//
//     glUniform1f(glGetUniformLocation(ps->computeShader.ID,"u_dt"), dt);
//     glUniform3f(glGetUniformLocation(ps->computeShader.ID,"u_gravity"), 0.0f, -9.81f, 0.0f);
//     glUniform1ui(glGetUniformLocation(ps->computeShader.ID,"u_frame"), frame);
//     glUniform1ui(glGetUniformLocation(ps->computeShader.ID,"u_spawnBudget"), 1000u);
//
//     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ps->ssbo);
//
//     GLuint groups = (MAX_PARTICLES + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
//     glDispatchCompute(groups, 1, 1);
//     glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
// }

// void BE_ParticlesDraw(BE_GPUParticles* ps, mat4 view, mat4 proj, int additive) {
//     BE_ShaderActivate(&ps->renderShader);
//
//     glUniformMatrix4fv(glGetUniformLocation(ps->renderShader.ID,"u_view"), 1, GL_FALSE, (const float*)view);
//     glUniformMatrix4fv(glGetUniformLocation(ps->renderShader.ID,"u_proj"), 1, GL_FALSE, (const float*)proj);
//     BE_TextureSetUniformUnit(&ps.renderShader, "u_sprite", ps.sprite.unit);
//     glUniform1i(glGetUniformLocation(ps.renderShader.ID,"u_additive"), additive);
//
//     BE_VAOBind(ps->vao);
//     BE_TextureBind(&ps->sprite);
//
//     glEnable(GL_BLEND);
//     if(additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
//     else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//     glDepthMask(GL_FALSE);
//     glEnable(GL_PROGRAM_POINT_SIZE);
//
//     glDrawArrays(GL_POINTS, 0, ps.particleCount);
//
//     glDepthMask(GL_TRUE);
//     BE_TextureUnbind();
//     BE_VAOUnbind();
//
// }

// ==============================
// Audio
// ==============================

void BE_SoundUpdateListener(BE_Camera* cam) {
    vec3 forward;
    glm_vec3_copy(cam->direction, forward);
    glm_vec3_normalize(forward);

    vec3 up = {0.f, 1.f, 0.f};

    alListener3f(AL_POSITION, cam->position[0], cam->position[1], cam->position[2]);
    
    vec3 velocity = {0.f, 0.f, 0.f};
    alListener3f(AL_VELOCITY, velocity[0], velocity[1], velocity[2]);

    float orientation[6] = {
        forward[0], forward[1], forward[2],
        up[0], up[1], up[2]
    };

    alListenerfv(AL_ORIENTATION, orientation);
}

BE_Sound BE_LoadWav(const char* path, const char* name) {
    BE_Sound buf = {0};
    drwav wav;
    if (!drwav_init_file(&wav, path, NULL)) {
        fprintf(stderr, "Failed to load WAV: %s\n", path);
        return buf;
    }

    // Allocate memory and read PCM samples
    uint16_t* pcm = malloc(wav.totalPCMFrameCount * wav.channels * sizeof(uint16_t));
    drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, pcm);

    // Create OpenAL buffer
    alGenBuffers(1, &buf.bufferID);

    // Determine format
    ALenum format;
    if (wav.channels == 1) format = AL_FORMAT_MONO16;
    else if (wav.channels == 2) format = AL_FORMAT_STEREO16;
    else {
        fprintf(stderr, "Unsupported channel count: %d\n", wav.channels);
        drwav_uninit(&wav);
        free(pcm);
        return buf;
    }

    alBufferData(buf.bufferID, format, pcm, (ALsizei)(wav.totalPCMFrameCount * wav.channels * sizeof(uint16_t)), wav.sampleRate);

    drwav_uninit(&wav);
    free(pcm);

    // Set name
    if (name) {
        buf.name = malloc(strlen(name)+1);
        strcpy(buf.name,name);
    } else buf.name = NULL;

    return buf;
}

BE_SoundSource BE_SoundSourceInit(BE_Sound* buffer, vec3 position, bool spatial) {
    BE_SoundSource e = {0};

    e.buffer = buffer;
    glm_vec3_copy(position, e.position);
    e.gain = 1.f;
    e.pitch = 1.f;
    e.looping = false;
    e.spatial = spatial;

    alGenSources(1, &e.sourceID);
    alSourcei(e.sourceID, AL_BUFFER, buffer->bufferID);
    
    alSourcei(e.sourceID, AL_LOOPING, AL_FALSE);

    if(spatial){
        alSource3f(e.sourceID, AL_POSITION, position[0], position[1], position[2]);
        alSourcei(e.sourceID, AL_SOURCE_RELATIVE, AL_FALSE);
        alSourcef(e.sourceID, AL_ROLLOFF_FACTOR, 5.f);
        alSourcef(e.sourceID, AL_REFERENCE_DISTANCE, 5.f);
        alSourcef(e.sourceID, AL_MAX_DISTANCE, 50.f);
    } else {
        alSourcei(e.sourceID, AL_SOURCE_RELATIVE, AL_TRUE);
    }

    alSourcef(e.sourceID, AL_GAIN, e.gain);
    alSourcef(e.sourceID, AL_PITCH, e.pitch);

    return e;
}

void BE_SoundSourcePlay(BE_SoundSource* e) {
    alSourcePlay(e->sourceID);
}

void BE_SoundSourceStop(BE_SoundSource* e) {
    alSourceStop(e->sourceID);
}

void BE_SoundSourceSetPosition(BE_SoundSource* e, vec3 position) {
    glm_vec3_copy(position, e->position);
    if(e->spatial) alSource3f(e->sourceID,AL_POSITION, position[0], position[1], position[2]);
}

void BE_SoundSourceSetGain(BE_SoundSource* e, float gain) {
    e->gain = gain;
    alSourcef(e->sourceID,AL_GAIN,gain);
}

BE_SoundSystem BE_SoundSystemInit() {
    BE_SoundSystem sys = {0};
    sys.device = alcOpenDevice(NULL);
    if (!sys.device) {
        fprintf(stderr,"Failed to open device\n");
        exit(1); 
    }

    sys.context = alcCreateContext(sys.device, NULL);
    if (!alcMakeContextCurrent(sys.context)) {
        fprintf(stderr,"Failed to make context current\n");
        exit(1);
    }

    alListener3f(AL_POSITION, 0, 0, 0);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    float ori[6] = {0, 0, -1, 0, 1, 0};
    alListenerfv(AL_ORIENTATION, ori);

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    return sys;
}

// ==============================
// Scene
// ==============================

BE_Scene BE_SceneInit() {
    BE_Scene scene;
    BE_CameraVectorInit(&scene.cameras);
    BE_CameraVectorPush(&scene.cameras, BE_CameraInit(1, 1, 45.0f, 0.1f, 100.0f, (vec3){-1.93f, 0.73f, -1.75f}, (vec3){0.67f, -0.12f, 0.73f}));
    BE_LightVectorInit(&scene.lights);
    BE_ModelVectorInit(&scene.models);
    BE_SpriteVectorInit(&scene.sprites);
    return scene;
}

void BE_SceneDraw(BE_Scene* scene, BE_Shader* shader, BE_Shader* shadow, BE_Camera* camera, int width, int height) {

    BE_CameraVectorUpdateMatrix(&scene->cameras, width, height);
    BE_LightVectorUpdateMatrix(&scene->lights);
    BE_LightVectorUpdateMultiMaps(&scene->lights, &scene->models, shadow, true);
    
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    BE_LightVectorUpload(&scene->lights, shader);
    BE_CameraMatrixUploadPersp(camera, shader, "camMatrix");

    glUniform1i(glGetUniformLocation(shader->ID, "sampleRadius"), 0);
    BE_ModelVectorDraw(&scene->models, shader);
 
}

#define INITIAL_SCENE_CAPACITY 2

void BE_SceneVectorInit(BE_SceneVector* vec) {
    vec->data = (BE_Scene*)malloc(sizeof(BE_Scene) * INITIAL_SCENE_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_SCENE_CAPACITY;
}

void BE_SceneVectorPush(BE_SceneVector* vec, BE_Scene value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (BE_Scene*)realloc(vec->data, sizeof(BE_Scene) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void BE_SceneVectorFree(BE_SceneVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

void BE_SceneVectorCopy(BE_Scene* models, size_t count, BE_SceneVector* outVec) {
    BE_SceneVectorInit(outVec);
    for (size_t i = 0; i < count; i++) {
        BE_SceneVectorPush(outVec, models[i]);
    }
}
