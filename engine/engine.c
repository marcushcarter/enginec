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
// DELTA TIME
// ==============================

DeltaTime delta;

float deltaTimeUpdate() {
    delta.currentTime = clock();
    delta.dt = (float)(delta.currentTime - delta.previousTime) / CLOCKS_PER_SEC;
    delta.previousTime = delta.currentTime;

    delta.frameCount++;
    delta.frameCountFPS++;
    delta.fpsTimer += delta.dt;

    if (delta.fpsTimer >= 1.0f) {
        delta.fps = delta.frameCountFPS / delta.fpsTimer;
        delta.ms = 1000 / delta.fps;

        delta.fpsHistory[delta.fpsHistoryIndex] = delta.fps;
        delta.fpsHistoryIndex = (delta.fpsHistoryIndex + 1) % FPS_HISTORY_COUNT;
        if (delta.fpsHistoryCount < FPS_HISTORY_COUNT)
            delta.fpsHistoryCount++;

        delta.frameCountFPS = 0;
        delta.fpsTimer = 0.0f;
    }

    return delta.dt;

}

// ==============================
// MATH
// ==============================

void make_model_matrix(vec3 translation, vec3 rotation, vec3 scale, mat4 dest) {
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

void make_billboard_matrix(vec3 position, mat4 view, vec3 scale, mat4 dest) {
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

void orientation_to_euler(vec3 orientation, vec3 outEuler) {

    vec3 dir;
    glm_vec3_normalize_to(orientation, dir);

    float yaw = atan2f(-dir[0], -dir[2]);
    float pitch = asinf(dir[1]);
    float roll = 0.0f;

    outEuler[0] = pitch;
    outEuler[1] = yaw;
    outEuler[2] = roll;
}

void print_mat4(mat4 m) {
    printf("mat4:\n");
    for (int row = 0; row < 4; row++) {
        printf("[ ");
        for (int col = 0; col < 4; col++) {
            printf("%8.3f ", m[col][row]);  // cglm stores matrices column-major
        }
        printf("]\n");
    }
}

// ==============================
// VertexVector
// ==============================

#define INITIAL_VERTEX_CAPACITY 8

void VertexVector_Init(VertexVector* vec) {
    vec->data = (Vertex*)malloc(sizeof(Vertex) * INITIAL_VERTEX_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_VERTEX_CAPACITY;
}

void VertexVector_Push(VertexVector* vec, Vertex value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (Vertex*)realloc(vec->data, sizeof(Vertex) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void VertexVector_Free(VertexVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

void VertexVector_Copy(Vertex* vertices, size_t count, VertexVector* outVec) {
    VertexVector_Init(outVec);
    for (size_t i = 0; i < count; i++) {
        VertexVector_Push(outVec, vertices[i]);
    }
}

// ==============================
// VAO
// ==============================

VAO VAO_Init() {
    VAO vao;
    glGenVertexArrays(1, &vao.ID);
    return vao;
}

VAO VAO_InitQuad() {
    
    GLfloat vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
        -1.0f, -1.0f,   0.0f, 0.0f,  // bottom-left
        1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right

        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
        1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right
        1.0f,  1.0f,   1.0f, 1.0f   // top-right
    };
    
    VAO vao = VAO_Init();
    VAO_Bind(&vao);
    VBO vbo = VBO_InitRaw(vertices, sizeof(vertices));
    VAO_LinkAttrib(&vbo, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    VAO_LinkAttrib(&vbo, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();

    return vao;
}

VAO VAO_InitBillboardQuad() {
    
    GLfloat vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
        -1.0f, -1.0f,   0.0f, 0.0f,  // bottom-left
        1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right

        -1.0f,  1.0f,   0.0f, 1.0f,  // top-left
        1.0f, -1.0f,   1.0f, 0.0f,  // bottom-right
        1.0f,  1.0f,   1.0f, 1.0f   // top-right
    };
    
    VAO vao = VAO_Init();
    VAO_Bind(&vao);
    VBO vbo = VBO_InitRaw(vertices, sizeof(vertices));
    VAO_LinkAttrib(&vbo, 0, 3, GL_FLOAT, 5 * sizeof(float), (void*)0);
    VAO_LinkAttrib(&vbo, 1, 2, GL_FLOAT, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();

    return vao;
}

void VAO_Bind(VAO* vao) {
    glBindVertexArray(vao->ID);
}

void VAO_DrawQuad(VAO* vao) {
    VAO_Bind(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void VAO_Unbind() {
    glBindVertexArray(0);
}

void VAO_Delete(VAO* vao) {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao->ID);
}

// ==============================
// VBO
// ==============================

VBO VBO_InitRaw(GLfloat* vertices, GLsizeiptr size) {
    VBO vbo;
    glGenBuffers(1, &vbo.ID);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.ID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    return vbo;
}

VBO VBO_Init(VertexVector* vertices) {
    VBO vbo;
    glGenBuffers(1, &vbo.ID);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.ID);
    glBufferData(GL_ARRAY_BUFFER, vertices->size * sizeof(Vertex), vertices->data, GL_STATIC_DRAW);
    return vbo;
}

void VBO_Bind(VBO* vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo->ID);
}

void VBO_Unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO_Delete(VBO* vbo) {
    glDeleteBuffers(1, &vbo->ID);
}

void VAO_LinkAttrib(VBO* vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    VBO_Bind(vbo);
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    VBO_Unbind();
}

// ==============================
// GLuintVector
// ==============================

#define INITIAL_GLUINT_CAPACITY   8

void GLuintVector_Init(GLuintVector* vec) {
    vec->data = (GLuint*)malloc(sizeof(GLuint) * INITIAL_GLUINT_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_GLUINT_CAPACITY;
}

void GLuintVector_Push(GLuintVector* vec, GLuint value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (GLuint*)realloc(vec->data, sizeof(GLuint) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void GLuintVector_Free(GLuintVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

void GLuintVector_Copy(GLuint* data, size_t count, GLuintVector* outVec) {
    GLuintVector_Init(outVec);
    for (size_t i = 0; i < count; i++) {
        GLuintVector_Push(outVec, data[i]);
    }
}

// ==============================
// EBO
// ==============================

EBO EBO_InitRaw(GLuint* indices, GLsizeiptr size) {
    EBO ebo;
    glGenBuffers(1, &ebo.ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    return ebo;
}

EBO EBO_Init(GLuintVector* indices) {
    EBO ebo;
    glGenBuffers(1, &ebo.ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size * sizeof(GLuint), indices->data, GL_STATIC_DRAW);
    return ebo;
}

void EBO_Bind(EBO* ebo) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->ID);
}

void EBO_Unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EBO_Delete(EBO* ebo) {
    glDeleteBuffers(1, &ebo->ID);
}

// ==============================
// Shader
// ==============================

char* get_file_contents(const char* filename) {
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

void Shader_compileErrors(unsigned int shader, const char* type) {
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

Shader Shader_Init(const char* vertexFile, const char* fragmentFile, const char* geometryFile) {
    Shader shader = {0};

    const char* vertexSource = get_file_contents(vertexFile);
    const char* fragmentSource = get_file_contents(fragmentFile);
    const char* geometrySource = NULL;

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    Shader_compileErrors(vertexShader, "VERTEX");
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    Shader_compileErrors(fragmentShader, "FRAGMENT");

    GLuint geometryShader = 0;
    if (geometryFile != NULL) {
        geometrySource = get_file_contents(geometryFile);
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &geometrySource, NULL);
        glCompileShader(geometryShader);
        Shader_compileErrors(geometryShader, "GEOMETRY");
    }

    shader.ID = glCreateProgram();
    glAttachShader(shader.ID, vertexShader);
    glAttachShader(shader.ID, fragmentShader);
    if (geometryFile != NULL) glAttachShader(shader.ID, geometryShader);
    glLinkProgram(shader.ID);
    Shader_compileErrors(shader.ID, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryFile != NULL) glDeleteShader(geometryShader);

    free((void*)vertexSource);
    free((void*)fragmentSource);
    if (geometryFile != NULL) free((void*)geometrySource);

    return shader;

}

void Shader_Activate(Shader* shader) {
    glUseProgram(shader->ID);
}

void Shader_Delete(Shader* shader) {
    glDeleteProgram(shader->ID);
}

// ==============================
// FBO
// ==============================

FBO FBO_Init(int width, int height) {
    FBO fb;
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

    fb.vao = VAO_Init();
    VAO_Bind(&fb.vao);
    fb.vbo = VBO_InitRaw(vertices, sizeof(vertices));
    VAO_LinkAttrib(&fb.vbo, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    VAO_LinkAttrib(&fb.vbo, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();
    
    return fb;

}

void FBO_Resize(FBO* fbo, int width, int height) {
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

void FBO_Bind(FBO* fb) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);
    glClear(GL_COLOR_BUFFER_BIT);
}

void FBO_BindTexture(FBO* fb, Shader* shader) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fb->texture);
    glUniform1i(glGetUniformLocation(shader->ID, "screenTexture"), 0);
}

void FBO_Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO_Delete(FBO* fb) {
    glDeleteFramebuffers(1, &fb->fbo);
    glDeleteTextures(1, &fb->texture);
    glDeleteRenderbuffers(1, &fb->rbo);
    *fb = (FBO){0};
}

// ==============================
// Texture
// ==============================

Texture Texture_Init(const char* imageFile, const char* texType, GLuint slot) {
    Texture texture;

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

    return texture;
}

void Texture_texUnit(Shader* shader, const char* uniform, GLuint unit) {
    GLuint tex0Uni = glGetUniformLocation(shader->ID, uniform);
    Shader_Activate(shader);
    glUniform1i(tex0Uni, unit);
}

void Texture_Bind(Texture* texture) {
    glActiveTexture(GL_TEXTURE0 + texture->unit);
    glBindTexture(GL_TEXTURE_2D, texture->ID);
}

void Texture_Unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture_Delete(Texture* texture) {
    glDeleteTextures(1, &texture->ID);
}

// ==============================
// TextureVector
// ==============================

#define INITIAL_TEXTURE_CAPACITY 8

void TextureVector_Init(TextureVector* vec) {
    vec->data = (Texture*)malloc(sizeof(Texture) * INITIAL_TEXTURE_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_TEXTURE_CAPACITY;
}

void TextureVector_Push(TextureVector* vec, Texture value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (Texture*)realloc(vec->data, sizeof(Texture) * vec->capacity);
    }
    vec->data[vec->size++] = value;
}

void TextureVector_Free(TextureVector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

void TextureVector_Copy(Texture* textures, size_t count, TextureVector* outVec) {
    TextureVector_Init(outVec);
    for (size_t i = 0; i < count; i++) {
        TextureVector_Push(outVec, textures[i]);
    }
}

// ==============================
// JOYSTICK
// ==============================

Joystick joysticks[MAX_JOYSTICKS];
int joystickCount = MAX_JOYSTICKS;

void glfwJoystickEvents(void) {
    
    for (int i = 0; i < joystickCount; i++) {
        Joystick* js = &joysticks[i];
        if (js->present && js->buttons) {
            for (int b = 0; b < js->buttonCount && b < 16; b++) {
                js->lbuttons[b] = js->buttons[b];
            }
        }
    }

    for (int jid = 0; jid <= GLFW_JOYSTICK_LAST; jid++) {
        if (!glfwJoystickPresent(jid)) continue;

        const unsigned char* buttons;
        int buttonCount;
        buttons = glfwGetJoystickButtons(jid, &buttonCount);
        if (!buttons || buttonCount <= 7 /*|| !buttons[7]*/) continue;

        int alreadyAssigned = 0;
        for (int i = 0; i < joystickCount; i++) {
            if (joysticks[i].present && joysticks[i].id == jid) {
                alreadyAssigned = 1;
                break;
            }
        }

        if (!alreadyAssigned) {
            for (int i = 0; i < joystickCount; i++) {
                if (!joysticks[i].present) {
                    Joystick* js = &joysticks[i];

                    js->id = jid;
                    js->present = 1;
                    js->name = glfwGetJoystickName(jid);
                    js->axes = glfwGetJoystickAxes(jid, &js->axisCount);
                    js->buttons = buttons;
                    js->buttonCount = buttonCount;
                    js->hats = glfwGetJoystickHats(jid, &js->hatCount);
                    js->deadzone = 0.05f;

                    memset(js->lbuttons, 0, buttonCount);

                    printf("Player %d controller connected\n", js->id+1);

                    break;
                }
            }
        }
    }

    for (int i = 0; i < joystickCount; i++) {
        Joystick* js = &joysticks[i];
        if (!js->present) continue;

        if (!glfwJoystickPresent(js->id)) {
            printf("Player %d controller disonnected\n", js->id+1);
            *js = (Joystick){0}; // reset the struct
            continue;
        }

        js->axes = glfwGetJoystickAxes(js->id, &js->axisCount);
        js->buttons = glfwGetJoystickButtons(js->id, &js->buttonCount);
        js->hats = glfwGetJoystickHats(js->id, &js->hatCount);
    }
}

int joystickIsPressed(Joystick* js, int button) {
    return js->buttons && js->buttons[button] && !js->lbuttons[button];
}

int joystickIsReleased(Joystick* js, int button) {
    return js->buttons && !js->buttons[button] && js->lbuttons[button];
}

int joystickIsHeld(Joystick* js, int button) {
    return js->buttons && js->buttons[button];
}

float joystickGetAxis(Joystick* js, int axis) {
    return js->axes && js->axes[axis];
}

// ==============================
// CAMERA
// ==============================

Camera Camera_InitStack(int width, int height, float fov, float nearPlane, float farPlane, vec3 position, vec3 direction) {
    Camera camera;

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
    glm_mat4_copy(mat, camera.cameraMatrix);

    return camera;
}

Camera* Camera_InitHeap(int width, int height, float fov, float nearPlane, float farPlane, vec3 position, vec3 direction) {
    Camera* camera = (Camera*)malloc(sizeof(Camera));

    camera->width = width;
    camera->height = height;
    glm_vec3_copy(position, camera->position);
    glm_vec3_copy(direction, camera->direction);

    vec3 up = { 0.0f, 1.0f, 0.0f };

    glm_vec3_copy(up, camera->Up);

    camera->zoom = 1.0f;
    camera->fov = fov;

    camera->nearPlane = nearPlane;
    camera->farPlane = farPlane;

    glm_mat4_identity(camera->cameraMatrix);

    return camera;
}

void Camera_UpdateMatrix(Camera* camera, int width, int height) {
    mat4 view;
    mat4 projection;
    mat4 ortho;
    mat4 projView;

    // 3D cam matrix

    float fov = camera->fov;

    vec3 target;
    glm_vec3_add(camera->position, camera->direction, target);
    glm_lookat(camera->position, target, camera->Up, view);
    glm_perspective(glm_rad(fov), (float)camera->width / (float)camera->height, camera->nearPlane, camera->farPlane, projection);
    glm_mat4_mul(projection, view, projView);
    glm_mat4_copy(projView, camera->cameraMatrix);
    glm_mat4_copy(view, camera->viewMatrix);
    
}

void Camera_Matrix(Camera* camera, Shader* shader, const char* uniform) {
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, uniform), 1, GL_FALSE, (float*)camera->cameraMatrix);
}

void Camera_MatrixCustom(Shader* shader, const char* uniform, mat4 matrix) {
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, uniform), 1, GL_FALSE, (float*)matrix);
}

void rotate_vec3_axis(vec3 in, vec3 axis, float angle_rad, vec3 out) {
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

void Camera_Inputs(Camera* camera, GLFWwindow* window, Joystick* js, float dt) {

    // MOVEMENT VECTORS

    float speed = 2.5f;
    float sensitivity = 3.0f;
    // if (js && js->buttons[8]) speed = 5.0f;

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
        rotate_vec3_axis(camera->direction, d_up, dt*sensitivity, d_direction);
        glm_vec3_normalize_to(d_direction, camera->direction);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        rotate_vec3_axis(camera->direction, d_up, -dt*sensitivity, d_direction);
        glm_vec3_normalize_to(d_direction, camera->direction);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        rotate_vec3_axis(camera->direction, d_right, -dt*sensitivity, d_direction);
        glm_vec3_normalize_to(d_direction, camera->direction);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        rotate_vec3_axis(camera->direction, d_right, dt*sensitivity, d_direction);
        glm_vec3_normalize_to(d_direction, camera->direction);
    }

    
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        camera->fov += 10*dt;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        camera->fov -= 10*dt;
    }
    
    glm_vec3_add(camera->position, v_move, camera->position);

    if (js->present && js != NULL) {

        if (fabsf(js->axes[1]) > js->deadzone) {
            glm_vec3_scale(v_forward, -speed*dt*js->axes[1], v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }
        if (fabsf(js->axes[0]) > js->deadzone) {
            glm_vec3_scale(v_right, speed*dt*js->axes[0], v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }
        if (js->buttons[0] || joystickIsHeld(js, 0)) {
            glm_vec3_scale(v_up, -speed*dt, v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }
        if (js->buttons[1] || js->buttons[9]) {
            glm_vec3_scale(v_up, speed*dt, v_vector);
            glm_vec3_add(v_move, v_vector, v_move);
        }

        // CAMERA ROTATION

        if (fabsf(js->axes[2]) > js->deadzone) {
            rotate_vec3_axis(camera->direction, d_up, -dt*sensitivity*js->axes[2], d_direction);
            glm_vec3_normalize_to(d_direction, camera->direction);
        }
        if (fabsf(js->axes[3]) > js->deadzone) {
            rotate_vec3_axis(camera->direction, d_right, dt*sensitivity*js->axes[3], d_direction);
            glm_vec3_normalize_to(d_direction, camera->direction);
        }
        
        glm_vec3_add(camera->position, v_move, camera->position);
    }

}

void print_Camera(Camera* camera) {
    printf("pos: %f %f %f dir: %f %f %f zoom: %f fov: %f\n", camera->position[0], camera->position[1], camera->position[2], camera->direction[0], camera->direction[1], camera->direction[2], camera->zoom, camera->fov);
}

// ==============================
// CameraVector
// ==============================

#define INITIAL_CAMERA_CAPACITY 8

void CameraVector_Init(CameraVector* vec) {
    vec->data = (Camera**)malloc(sizeof(Camera*) * INITIAL_CAMERA_CAPACITY);
    vec->size = 0;
    vec->capacity = INITIAL_CAMERA_CAPACITY;
}

void CameraVector_Push(CameraVector* vec, Camera* cam) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->data = (Camera**)realloc(vec->data, sizeof(Camera*) * vec->capacity);
    }
    vec->data[vec->size++] = cam;
}

Camera* CameraVector_Get(CameraVector* vec, size_t index) {
    return (index < vec->size) ? vec->data[index] : NULL;
}

void CameraVector_Remove(CameraVector* vec, Camera* cam) {
    for (size_t i = 0; i < vec->size; i++) {
        if (vec->data[i] == cam) {
            CameraVector_RemoveAt(vec, i);
            return;
        }
    }
}

void CameraVector_RemoveAt(CameraVector* vec, size_t index) {
    if (index >= vec->size) return;
    free(vec->data[index]);
    for (size_t i = index; i < vec->size - 1; i++) {
        vec->data[i] = vec->data[i + 1];
    }
    vec->size--;
}

size_t CameraVector_IndexOf(CameraVector* vec, Camera* cam) {
    for (size_t i = 0; i < vec->size; i++) {
        if (vec->data[i] == cam) {
            return i;
        }
    }
    return SIZE_MAX;
}

void CameraVector_Free(CameraVector* vec) {
    for (size_t i = 0; i < vec->size; i++) {
        free(vec->data[i]);
    }
    free(vec->data);
    vec->data = NULL;
    vec->size = vec->capacity = 0;
}

// ==============================
// MESH
// ==============================

Mesh Mesh_Init(VertexVector vertices, GLuintVector indices, TextureVector textures) {
    Mesh mesh;

    mesh.vertices = vertices;
    mesh.indices = indices;
    mesh.textures = textures;

    VAO VAO1 = VAO_Init();
    VAO_Bind(&VAO1);
    VBO VBO1 = VBO_Init(&vertices);
    EBO EBO1 = EBO_Init(&indices);
    VAO_LinkAttrib(&VBO1, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
    VAO_LinkAttrib(&VBO1, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
    VAO_LinkAttrib(&VBO1, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
    VAO_LinkAttrib(&VBO1, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();
    EBO_Unbind();

    mesh.vao = VAO1;

    return mesh;
}

Mesh Mesh_InitFromData(const char** texbuffer, int texcount, Vertex* vertices, int vertcount, GLuint* indices, int indcount) {

    VertexVector verts;
    GLuintVector inds;
    TextureVector texs;

    Texture textures[texcount];
    for (int i = 0; i < texcount; i++) {
        textures[i] = Texture_Init(texbuffer[i*2], texbuffer[i*2+1], i);
    }

    VertexVector_Copy(vertices, vertcount, &verts);
    GLuintVector_Copy(indices, indcount, &inds);
    TextureVector_Copy(textures, texcount, &texs);

    Mesh mesh = Mesh_Init(verts, inds, texs);

    return mesh;
}

void Mesh_Draw(Mesh* mesh, Shader* shader, Camera* camera) {
    Shader_Activate(shader);
    VAO_Bind(&mesh->vao);

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
            Texture_texUnit(shader, uniformName, i);
            Texture_Bind(&mesh->textures.data[i]);
        }
    }
    glUniform3fv(glGetUniformLocation(shader->ID, "camPos"), 1, (float*)camera->position); 
    Camera_Matrix(camera, shader, "camMatrix");

    glDrawElements(GL_TRIANGLES, mesh->indices.size, GL_UNSIGNED_INT, 0);
}

void Mesh_DrawBillboard(Mesh* mesh, Shader* shader, Camera* camera, Texture* texture) {
    Shader_Activate(shader);
    VAO_Bind(&mesh->vao);

    Texture_texUnit(shader, "diffuse0", texture->unit);
    Texture_Bind(texture);

    glUniform3fv(glGetUniformLocation(shader->ID, "camPos"), 1, (float*)camera->position); 
    Camera_Matrix(camera, shader, "camMatrix");

    glDrawElements(GL_TRIANGLES, mesh->indices.size, GL_UNSIGNED_INT, 0);
}

// ==============================
// IMPORT
// ==============================

int find_or_add_vertex(Vertex* vertices, int* verticesCount, Vertex v) {
    for (int i = 0; i < *verticesCount; i++) {
        if (memcmp(&vertices[i], &v, sizeof(Vertex)) == 0) {
            return i;
        }
    }

    int index = (*verticesCount)++;
    vertices[index] = v;
    return index;
}

int count_face_vertices(const char* line) {
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

void replacePathSuffix(const char* path, const char* newsuffix, char* dest, int destsize) {
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

Mesh Import_loadMeshFromOBJ(const char* obj_path) {
    Mesh mesh;

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

    Vertex* vertices = (Vertex*)malloc(sizeof(Vertex) * 100000);
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
            replacePathSuffix(obj_path, mtl_file, mtl_filepath, sizeof(mtl_filepath));

            textures = Mesh_getTexturesFromMTL(mtl_filepath, &texturesCount);

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
            
            int faceVertCount = count_face_vertices(line);

            char* token = strtok(line+2, " \t\r\n");
            
            Vertex* verts = (Vertex*)malloc(sizeof(Vertex) * faceVertCount);
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
                int i0 = find_or_add_vertex(vertices, &verticesCount, verts[0]);
                int i1 = find_or_add_vertex(vertices, &verticesCount, verts[i]);
                int i2 = find_or_add_vertex(vertices, &verticesCount, verts[i + 1]);

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
    
    mesh = Mesh_InitFromData(textures, 1, vertices, verticesCount, indices, indicesCount);

    free(positions);
    free(normals);
    free(uvs);
    free(vertices);
    free(indices);

    MSG_INFO(obj_path, lineNum, "model loaded succesfully");
    return mesh;
}

const char** Mesh_getTexturesFromMTL(const char* mtl_path, int* outCount) {

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
            replacePathSuffix(mtl_path, fileRelPath, texturePath, sizeof(texturePath));

            textures[count++] = strdup(texturePath);
            textures[count++] = strdup("diffuse");

        } else if (strncmp(line, "map_Ks ", 7) == 0) {
            
            char fileRelPath[256];
            sscanf(line, "map_Ks %s", fileRelPath);

            char texturePath[512];
            replacePathSuffix(mtl_path, fileRelPath, texturePath, sizeof(texturePath));

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
// VECTOR DRAW
// ==============================

void CameraVector_Draw(CameraVector* cameras, Mesh* mesh, Shader* shader, Camera* camera) {
    
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    mat4 model;
    vec3 ori;

    for (size_t i = 0; i < cameras->size; i++) {
        if (CameraVector_Get(cameras, i) == camera || CameraVector_Get(cameras, i) == 0) continue;
    
        Camera* cam = cameras->data[i];

        orientation_to_euler(cam->direction, ori);
        make_model_matrix(cam->position, ori, (vec3){0.25f * cam->width/1000 * cam->fov/45, 0.25f * cam->height/1000, 0.2f * cam->zoom}, model);
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)model);
        Mesh_Draw(mesh, shader, camera);

    }
    
    glEnable(GL_CULL_FACE);
}

// ==============================
// SHADOW MAP
// ==============================

ShadowMapFBO ShadowMapFBO_Init(int width, int height, int layers) {
    ShadowMapFBO smfbo = {0};
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

void ShadowMapFBO_BindLayer(ShadowMapFBO* smfbo, int layer) {
    glBindFramebuffer(GL_FRAMEBUFFER, smfbo->fbo);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, smfbo->depthTextureArray, 0, layer);
   
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
}

void ShadowMapFBO_Delete(ShadowMapFBO* smfbo) {
    glDeleteFramebuffers(1, &smfbo->fbo);
    glDeleteTextures(1, &smfbo->depthTextureArray);
}

// ==============================
// LIGHTS
// ==============================

LightSystem LightSystem_Init(float ambient) {
    LightSystem lightSystem;
    lightSystem.ambient = ambient;
    lightSystem.numPointLights = 0;
    lightSystem.numSpotLights = 0;
    
    lightSystem.directShadowFBO = ShadowMapFBO_Init(1024*10, 1024*10, 1);
    // lightSystem.pointShadowFBO = ShadowMapFBO_Init(1024*4, 1024*4, 1);
    lightSystem.spotShadowFBO = ShadowMapFBO_Init(250, 250, MAX_SPOT_LIGHTS);

    return lightSystem;
}

void LightSystem_Clear(LightSystem* lightSystem) {
    lightSystem->numPointLights = 0;
    lightSystem->numSpotLights = 0;

    // for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
    //     glm_vec3_zero(lightSystem->pointlights[i].position);
    //     glm_vec4_zero(lightSystem->pointlights[i].color);
    //     lightSystem->pointlights[i].a = 0.0f;
    //     lightSystem->pointlights[i].b = 0.0f;
    //     lightSystem->pointlights[i].specular = 0.0f;
    // }

    for (int i = 0; i < MAX_SPOT_LIGHTS; i++) {
        glm_vec3_zero(lightSystem->spotlights[i].position);
        glm_vec3_zero(lightSystem->spotlights[i].direction);
        glm_vec4_zero(lightSystem->spotlights[i].color);
        lightSystem->spotlights[i].innerCone = 0.0f;
        lightSystem->spotlights[i].outerCone = 0.0f;
        lightSystem->spotlights[i].specular = 0.0f;
    }
}

void LightSystem_SetDirectLight(LightSystem* lightSystem, vec3 direction, vec4 color, float specular) {
    glm_vec3_copy(direction, lightSystem->directlight.direction);
    glm_vec4_copy(color, lightSystem->directlight.color);
    lightSystem->directlight.specular = specular;

    mat4 orthographicProjection, lightView;
    glm_ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, 100.0f, orthographicProjection);
    vec3 lightPos;
    glm_vec3_scale(direction, -DIRECT_LIGHT_DIST, lightPos);
    glm_lookat(lightPos, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f}, lightView);
    glm_mat4_mul(orthographicProjection, lightView, lightSystem->directlight.lightSpaceMatrix);
}

void LightSystem_AddPointLight(LightSystem* lightSystem, vec3 position, vec4 color, float a, float b, float specular) {
    if (lightSystem->numPointLights >= MAX_POINT_LIGHTS) return;
    
    PointLight light;
    glm_vec3_copy(position, light.position);
    glm_vec4_copy(color, light.color);
    light.a = a;
    light.b = b;
    light.specular = specular;
    lightSystem->pointlights[lightSystem->numPointLights++] = light;
}

void LightSystem_AddSpotLight(LightSystem* lightSystem, vec3 position, vec3 direction, vec4 color, float innerConeCos, float outerConeCos, float specular) {
    if (lightSystem->numSpotLights >= MAX_SPOT_LIGHTS) return;
    
    SpotLight light;
    
    glm_vec3_copy(position, light.position);
    vec3 normDir;
    glm_normalize_to(direction, normDir);
    glm_vec3_copy(normDir, light.direction);
    glm_vec4_copy(color, light.color);
    light.innerCone = innerConeCos;
    light.outerCone = outerConeCos;
    light.specular = specular;
    
    float outerAngleRad = acosf(outerConeCos);
    float outerAngleDeg = glm_deg(outerAngleRad);
    float fov = outerAngleDeg * 2.0f;

    mat4 proj;
    glm_perspective(glm_rad(fov), 1.0f, 0.1f, 100.0f, proj);
    
    vec3 target;
    glm_vec3_add(position, normDir, target);

    vec3 up = {0.0f, 1.0f, 0.0f};
    if (fabsf(glm_dot(normDir, up)) > 0.99f)
        glm_vec3_copy((vec3){1.0f, 0.0f, 0.0f}, up);

    mat4 view;
    glm_lookat(position, target, up, view);

    glm_mat4_mul(proj, view, light.lightSpaceMatrix);

    lightSystem->spotlights[lightSystem->numSpotLights++] = light;

}

void LightSystem_SetUniforms(Shader* shader, LightSystem* lightSystem) {
    
    Shader_Activate(shader);

    glUniform1f(glGetUniformLocation(shader->ID, "ambient"), lightSystem->ambient);
    glUniform1i(glGetUniformLocation(shader->ID, "NR_POINT_LIGHTS"), lightSystem->numPointLights);
    glUniform1i(glGetUniformLocation(shader->ID, "NR_SPOT_LIGHTS"), lightSystem->numSpotLights);


    glUniform3fv(glGetUniformLocation(shader->ID, "directlight.direction"), 1, (float*)lightSystem->directlight.direction);
    glUniform4fv(glGetUniformLocation(shader->ID, "directlight.color"), 1, (float*)lightSystem->directlight.color);
    glUniform1f(glGetUniformLocation(shader->ID, "directlight.specular"), lightSystem->directlight.specular);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "directlight.lightSpaceMatrix"), 1, GL_FALSE, (float*)lightSystem->directlight.lightSpaceMatrix);

    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D_ARRAY, lightSystem->directShadowFBO.depthTextureArray);
    glUniform1i(glGetUniformLocation(shader->ID, "directShadowMapArray"), 3);

    char buffer[256];

    snprintf(buffer, sizeof(buffer), "", 0);

    for (int i = 0; i < lightSystem->numPointLights; i++) {

        snprintf(buffer, sizeof(buffer), "pointlights[%d].position", i);
        glUniform3fv(glGetUniformLocation(shader->ID, buffer), 1, (float*)lightSystem->pointlights[i].position);

        snprintf(buffer, sizeof(buffer), "pointlights[%d].color", i);
        glUniform4fv(glGetUniformLocation(shader->ID, buffer), 1, (float*)lightSystem->pointlights[i].color);

        snprintf(buffer, sizeof(buffer), "pointlights[%d].a", i);
        glUniform1f(glGetUniformLocation(shader->ID, buffer), lightSystem->pointlights[i].a);

        snprintf(buffer, sizeof(buffer), "pointlights[%d].b", i);
        glUniform1f(glGetUniformLocation(shader->ID, buffer), lightSystem->pointlights[i].b);

        snprintf(buffer, sizeof(buffer), "pointlights[%d].specular", i);
        glUniform1f(glGetUniformLocation(shader->ID, buffer), lightSystem->pointlights[i].specular);  

    }

    for (int i = 0; i < lightSystem->numSpotLights; i++) {

        snprintf(buffer, sizeof(buffer), "spotlights[%d].position", i);
        glUniform3fv(glGetUniformLocation(shader->ID, buffer), 1, (float*)lightSystem->spotlights[i].position);

        snprintf(buffer, sizeof(buffer), "spotlights[%d].direction", i);
        glUniform3fv(glGetUniformLocation(shader->ID, buffer), 1, (float*)lightSystem->spotlights[i].direction);

        snprintf(buffer, sizeof(buffer), "spotlights[%d].color", i);
        glUniform4fv(glGetUniformLocation(shader->ID, buffer), 1, (float*)lightSystem->spotlights[i].color);

        snprintf(buffer, sizeof(buffer), "spotlights[%d].innerCone", i);
        glUniform1f(glGetUniformLocation(shader->ID, buffer), lightSystem->spotlights[i].innerCone);

        snprintf(buffer, sizeof(buffer), "spotlights[%d].outerCone", i);
        glUniform1f(glGetUniformLocation(shader->ID, buffer), lightSystem->spotlights[i].outerCone);

        snprintf(buffer, sizeof(buffer), "spotlights[%d].specular", i);
        glUniform1f(glGetUniformLocation(shader->ID, buffer), lightSystem->spotlights[i].specular); 

        // snprintf(buffer, sizeof(buffer), "spotlights[%d].lightSpaceMatrix", i);
        // glUniformMatrix4fv(glGetUniformLocation(shader->ID, buffer), 1, GL_FALSE, (float*)lightSystem->spotlights[i].lightSpaceMatrix);

    }
    
    // glActiveTexture(GL_TEXTURE0 + 5);
    // glBindTexture(GL_TEXTURE_2D_ARRAY, lightSystem->spotShadowFBO.depthTextureArray);
    // glUniform1i(glGetUniformLocation(shader->ID, "spotShadowMapArray"), 5);

}

void LightSystem_MakeShadowMaps(LightSystem* lightSystem, Shader* lightShader, Camera* camera, ShadowRenderFunc renderFunc) {
    
    Shader_Activate(lightShader);
    glEnable(GL_DEPTH_TEST);

    ShadowMapFBO_BindLayer(&lightSystem->directShadowFBO, 0);
    glViewport(0, 0, lightSystem->directShadowFBO.width, lightSystem->directShadowFBO.height);
    glClear(GL_DEPTH_BUFFER_BIT);
    glUniformMatrix4fv(glGetUniformLocation(lightShader->ID, "lightSpaceMatrix"), 1, GL_FALSE, (float*)lightSystem->directlight.lightSpaceMatrix);
    renderFunc(lightShader, camera);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // for (int i = 0; i < lightSystem->numSpotLights; i++) {
        
    //     ShadowMapFBO_BindLayer(&lightSystem->spotShadowFBO, i);
    //     glViewport(0, 0, lightSystem->spotShadowFBO.width, lightSystem->spotShadowFBO.height);
    //     glClear(GL_DEPTH_BUFFER_BIT);
    //     glUniformMatrix4fv(glGetUniformLocation(lightShader->ID, "lightSpaceMatrix"), 1, GL_FALSE, (float*)lightSystem->spotlights[i].lightSpaceMatrix);
    //     renderFunc(lightShader, camera);
    //     glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // }

    // glEnable(GL_DEPTH_TEST);
    // ShadowMapFBO_BindLayer(&lightSystem->directShadowFBO, 0);
    // glViewport(0, 0, lightSystem->directShadowFBO.width, lightSystem->directShadowFBO.height);
    // glClear(GL_DEPTH_BUFFER_BIT);
    // glUniformMatrix4fv(glGetUniformLocation(lightShader->ID, "lightSpaceMatrix"), 1, GL_FALSE, (float*)lightSystem->directlight.lightSpaceMatrix);
    // renderFunc(lightShader, camera);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LightSystem_Draw(LightSystem* lightSystem, Mesh* mesh, Shader* shader, Camera* camera) {
    
    Shader_Activate(shader);

    vec3 lightScale = { 0.1f, 0.1f, 0.1f };
    mat4 lightModel;
    
    // glm_mat4_identity(lightModel);
    // glm_translate(lightModel, (vec3){ 0.0f, 2.0f, 0.0f });
    // glm_scale(lightModel, lightScale);
    // glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)lightModel);
    // glUniform4fv(glGetUniformLocation(shader->ID, "lightColor"), 1, (float*)lightSystem->directlight.color);   
    // Mesh_Draw(mesh, shader, camera); 

    for (int i = 0; i < lightSystem->numPointLights; i++) {
        glm_mat4_identity(lightModel);
        glm_translate(lightModel, lightSystem->pointlights[i].position);
        glm_scale(lightModel, lightScale);
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)lightModel);
        glUniform4fv(glGetUniformLocation(shader->ID, "lightColor"), 1, (float*)lightSystem->pointlights[i].color);   
        Mesh_Draw(mesh, shader, camera); 
    }
    
    for (int i = 0; i < lightSystem->numSpotLights; i++) {
        glm_mat4_identity(lightModel);
        glm_translate(lightModel, lightSystem->spotlights[i].position);
        glm_scale(lightModel, lightScale);
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)lightModel);
        glUniform4fv(glGetUniformLocation(shader->ID, "lightColor"), 1, (float*)lightSystem->spotlights[i].color);  
        Mesh_Draw(mesh, shader, camera);  
    }
}

void LightSystem_Merge(LightSystem* dest, LightSystem* a, LightSystem* b) {
    dest->ambient = a->ambient;

    // dest->ambient = (a->ambient + b->ambient) * 0.5f;

    dest->directlight = a->directlight;

    dest->numPointLights = 0;
    for (int i = 0; i < a->numPointLights && dest->numPointLights < MAX_POINT_LIGHTS; i++) {
        dest->pointlights[dest->numPointLights++] = a->pointlights[i];
    }
    for (int i = 0; i < b->numPointLights && dest->numPointLights < MAX_POINT_LIGHTS; i++) {
        dest->pointlights[dest->numPointLights++] = b->pointlights[i];
    }

    dest->numSpotLights = 0;
    for (int i = 0; i < a->numSpotLights && dest->numSpotLights < MAX_SPOT_LIGHTS; i++) {
        dest->spotlights[dest->numSpotLights++] = a->spotlights[i];
    }
    for (int i = 0; i < b->numSpotLights && dest->numSpotLights < MAX_SPOT_LIGHTS; i++) {
        dest->spotlights[dest->numSpotLights++] = b->spotlights[i];
    }
}

// ==============================
// JOYSTICK
// ==============================

// ==============================
// JOYSTICK
// ==============================

// ==============================
// JOYSTICK
// ==============================

// ==============================
// JOYSTICK
// ==============================

// ==============================
// JOYSTICK
// ==============================

// ==============================
// JOYSTICK
// ==============================

// ==============================
// JOYSTICK
// ==============================

// ==============================
// JOYSTICK
// ==============================

// ==============================
// ENGINE
// ==============================









