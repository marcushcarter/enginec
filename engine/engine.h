#pragma once

#include <engine/glad/glad.h>
#include <engine/GLFW/glfw3.h>
#include <engine/cglm/cglm.h>
#include <engine/stb_image/stb_image.h>
#include <engine/stb_image/stb_image_resize.h>
#include <engine/stb_image/stb_truetype.h>
#include <engine/fmod/fmod.h>
#include <time.h>
#include <string.h>

#define BE_vec2(x,y) ((vec2){x,y})
#define BE_vec3(x,y,z) ((vec3){x,y,z})
#define BE_vec4(x,y,z,w) ((vec4){x,y,z,w})
#define BE_versor(x,y,z,w) ((versor){x,y,z,w})
#define BE_mat2(m00, m01, m10, m11) ((mat2){{m00, m01}, {m10, m11}})
#define BE_mat3(m00, m01, m02, m10, m11, m12, m20, m21, m22) ((mat3){{m00, m01, m02}, {m10, m11, m12}, {m20, m21, m22}})
#define BE_mat4(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33) ((mat4){{m00, m01, m02, m03}, {m10, m11, m12, m13}, {m20, m21, m22, m23}, {m30, m31, m32, m33}})

#define BE_CheckEngineActive(file, line, _return) do { \
    if (!g_engine) { BE_IMPL_Message(2, "Engine", file, line, "No engine is currently bound"); return _return; } \
} while (0)

#define BE_CheckSceneActive(file, line, _return) do { \
    if (!g_engine) { BE_IMPL_Message(2, "Engine", file, line, "No engine is currently bound"); return _return; } \
    if (!g_engine->activeScene) { BE_IMPL_Message(2, "Engine", file, line, "No scene is currently bound"); return _return; } \
} while (0)

#define BE_CheckCameraActive(file, line, _return) do { \
    if (!g_engine) { BE_IMPL_Message(2, "Engine", file, line, "No engine is currently bound"); return _return; } \
    if (!g_engine->activeScene) { BE_IMPL_Message(2, "Engine", file, line, "No scene is currently bound"); return _return; } \
    if (!g_engine->activeScene->activeCamera) { BE_IMPL_Message(2, "Engine", file, line, "No camera is currently bound"); return _return; } \
} while (0)

#define PRINT_MAT4(m) do { \
    printf("mat4:\n"); \
    printf("[ %8.3f %8.3f %8.3f %8.3f ]\n", (m)[0][0], (m)[1][0], (m)[2][0], (m)[3][0]); \
    printf("[ %8.3f %8.3f %8.3f %8.3f ]\n", (m)[0][1], (m)[1][1], (m)[2][1], (m)[3][1]); \
    printf("[ %8.3f %8.3f %8.3f %8.3f ]\n", (m)[0][2], (m)[1][2], (m)[2][2], (m)[3][2]); \
    printf("[ %8.3f %8.3f %8.3f %8.3f ]\n", (m)[0][3], (m)[1][3], (m)[2][3], (m)[3][3]); \
} while(0)

void BE_MakeModelMatrix(vec3 translation, vec3 rotation, vec3 scale, mat4 dest);
void BE_MakeBillboardMatrix(vec3 position, mat4 view, vec3 scale, mat4 dest);
void BE_OritentationToEuler(vec3 orientation, vec3 outEuler);
void BE_VersorToEuler(versor q, vec3 outEuler);
void BE_Vec3RotateAxis(vec3 in, vec3 axis, float angle_rad, vec3 out);

#define FPS_HISTORY_COUNT 20

typedef struct {
    clock_t previousTime;
    clock_t currentTime;
    float dt;

    int frameCount;
    int frameCountFPS;

    float fpsTimer;
    float fps;
    float ms;

    float fpsHistory[FPS_HISTORY_COUNT];
    int fpsHistoryIndex;
    int fpsHistoryCount;

} BE_FrameStats;

float BE_UpdateFrameTimeInfo(BE_FrameStats* info);

#define MAX_JOYSTICKS GLFW_JOYSTICK_16+1

/**
 * AXES
 * 0 - LS X
 * 1 - LS Y
 * 2 - RS X
 * 3 - RS Y
 * 4 - LT
 * 5 - RT
 * 
 * BUTTONS
 * 0 - A
 * 1 - B
 * 2 - X
 * 3 - Y
 * 4 - LB
 * 5 - RB
 * 6 - BACK
 * 7 - START
 * 8 - LS
 * 9 - RS
 * 10 - D UP
 * 11 - D RIGHT
 * 12 - D DOWN
 * 13 - D LEFT
 * 14 - 
 * 15 - 
 * 
 * HATS (BITS)
 * 0000 - CENTERED
 * 1000 - UP
 * 0100 - RIGHT
 * 0010 - DOWN
 * 0001 - LEFT
 * 
 */
typedef struct {
    int id;  // like GLFW_JOYSTICK_1
    int present;
    
    const float* axes;
    
    const unsigned char* buttons;
    unsigned char lbuttons[16]; // last-frame buttons

    const unsigned char* hats;
    const char* name;

    int axisCount;
    int buttonCount;
    int hatCount;

    float deadzone;
} BE_Joystick;

void BE_JoystickUpdate(BE_Joystick* joystick);
int BE_JoystickIsPressed(BE_Joystick* js, int button);
int BE_JoystickIsReleased(BE_Joystick* js, int button);
int BE_JoystickIsHeld(BE_Joystick* js, int button);
float BE_JoystickGetAxis(BE_Joystick* js, int axis);

typedef struct {
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 texUV;
} BE_Vertex;

typedef struct {
    BE_Vertex* data;
    size_t size;
    size_t capacity;
} BE_VertexVector;

void BE_VertexVectorInit(BE_VertexVector* vec);
void BE_VertexVectorPush(BE_VertexVector* vec, BE_Vertex value);
void BE_VertexVectorFree(BE_VertexVector* vec);
void BE_VertexVectorCopy(BE_Vertex* vertices, size_t count, BE_VertexVector* outVec);

typedef struct {
    char* name;
    GLuint ID;
} BE_VAO;

typedef struct {
    BE_VAO* data;
    size_t size;
    size_t capacity;
} BE_VAOVector;

BE_VAO BE_VAOInit(const char* name);
BE_VAO BE_VAOInitQuad(const char* name);
BE_VAO BE_VAOInitSprite(const char* name);
BE_VAO BE_VAOInitBillboardQuad(const char* name);
void BE_VAOBind(BE_VAO* vao);
void BE_VAODrawQuad(BE_VAO* vao);
void BE_VAOUnbind();
void BE_VAODelete(BE_VAO* vao);

void BE_VAOVectorInit(BE_VAOVector* vec);
void BE_VAOVectorPush(BE_VAOVector* vec, BE_VAO value);
void BE_VAOVectorFree(BE_VAOVector* vec);
void BE_VAOVectorCopy(BE_VAO* vaos, size_t count, BE_VAOVector* outVec);

typedef struct {
    GLuint ID;
} BE_VBO;

BE_VBO BE_VBOInitFromData(GLfloat* vertices, GLsizeiptr size);
BE_VBO BE_VBOInitFromVector(BE_VertexVector* vertices);
void BE_VBOBind(BE_VBO* vbo);
void BE_VBOUnbind();
void BE_VBODelete(BE_VBO* vbo);
void BE_LinkVertexAttribToVBO(BE_VBO* vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);

typedef struct {
    GLuint* data;
    size_t size;
    size_t capacity;
} BE_GLuintVector;

void BE_GLuintVectorInit(BE_GLuintVector* vec);
void BE_GLuintVectorPush(BE_GLuintVector* vec, GLuint value);
void BE_GLuintVectorFree(BE_GLuintVector* vec);
void BE_GLuintVectorCopy(GLuint* data, size_t count, BE_GLuintVector* outVec);

typedef struct {
    GLuint ID;
} BE_EBO;

BE_EBO BE_EBOInitFromData(GLuint* indices, GLsizeiptr size);
BE_EBO BE_EBOInitFromVector(BE_GLuintVector* indices);
void BE_EBOBind(BE_EBO* ebo);
void BE_EBOUnbind();
void BE_EBODelete(BE_EBO* ebo);

typedef struct {
    char* name;
    GLuint ID;
} BE_Shader;

typedef struct {
    BE_Shader* data;
    size_t size;
    size_t capacity;
} BE_ShaderVector;

char* BE_GetFileContents(const char* filename);
void BE_ShaderGetCompileErrors(unsigned int shader, const char* type);
BE_Shader BE_ShaderInit(const char* name, const char* vertexFile, const char* fragmentFile, const char* geometryFile, const char* computeFile);
BE_Shader BE_ShaderInitString(const char* name, const char* vertexSource, const char* fragmentSource, const char* geometrySource, const char* computeSource);
void BE_ShaderActivate(BE_Shader* shader);
void BE_ShaderDelete(BE_Shader* shader);

void BE_ShaderVectorInit(BE_ShaderVector* vec);
void BE_ShaderVectorPush(BE_ShaderVector* vec, BE_Shader value);
void BE_ShaderVectorFree(BE_ShaderVector* vec);
void BE_ShaderVectorCopy(BE_Shader* shaders, size_t count, BE_ShaderVector* outVec);

static inline BE_Shader* BE_FindShaderPtr(BE_ShaderVector* vec, const char* name) {
    for (size_t i = 0; i < vec->size; i++) {
        if (strcmp(vec->data[i].name, name) == 0) {
            return &vec->data[i];
        }
    }
    return NULL;
}

typedef struct {
    GLuint fbo;
    GLuint texture;
    GLuint rbo;
    BE_VAO vao;
    BE_VBO vbo;
    int width, height;
} BE_FBO;

BE_FBO BE_FBOInit(int width, int height);
void BE_FBOResize(BE_FBO* fbo, int newWidth, int newHeight);
BE_FBO BE_FBOInitShadowMap(int width, int height, float clampColor[4]);
void BE_FBOBind(BE_FBO* fb);
void BE_FBOBindTexture(BE_FBO* fb, BE_Shader* shader);
void BE_FBOUnbind();
void BE_FBODelete(BE_FBO* fb);

typedef struct {
    char* name;
    GLuint ID;
    char* type;
    GLuint unit;
} BE_Texture;

typedef struct {
    BE_Texture* data;
    size_t size;
    size_t capacity;
} BE_TextureVector;

BE_Texture BE_TextureInit(const char* name, const char* imageFile, const char* texType, GLenum slot);
void BE_TextureSetUniformUnit(BE_Shader* shader, const char* uniform, GLuint unit);
void BE_TextureBind(BE_Texture* texture);
void BE_TextureUnbind();
void BE_TextureDelete(BE_Texture* texture);

void BE_TextureVectorInit(BE_TextureVector* vec);
void BE_TextureVectorPush(BE_TextureVector* vec, BE_Texture value);
void BE_TextureVectorFree(BE_TextureVector* vec);
void BE_TextureVectorCopy(BE_Texture* textures, size_t count, BE_TextureVector* outVec);

static inline BE_Texture* BE_FindTexturePtr(BE_TextureVector* vec, const char* name) {
    for (size_t i = 0; i < vec->size; i++) {
        if (strcmp(vec->data[i].name, name) == 0) {
            return &vec->data[i];
        }
    }
    return NULL;
}

typedef struct {
    char* name;
    int width, height;
    float zoom, fov;
    float nearPlane, farPlane;
    vec3 position;
    versor orientation;
    float yaw, pitch, roll;
    mat4 projPersp, projOrtho;
    mat4 viewMatrix;
} BE_Camera;

typedef struct {
    BE_Camera* data;
    size_t size;
    size_t capacity;
} BE_CameraVector;

BE_Camera BE_CameraInit(const char* name, int width, int height, float fov, float nearPlane, float farPlane, vec3 position, vec3 direction);
void BE_CameraRotate(BE_Camera* camera, vec3 axis, float angle);
void BE_CameraInputs(BE_Camera* camera, GLFWwindow* window, float dt);
void BE_CameraInputsJoystick(BE_Camera* camera, BE_Joystick* joystick, float dt);
void BE_CameraMatrixUploadPersp(BE_Camera* camera, BE_Shader* shader, const char* uniform);
void BE_CameraMatrixUploadOrtho(BE_Camera* camera, BE_Shader* shader, const char* uniform);
void BE_CameraMatrixUploadCustom(BE_Shader* shader, const char* uniform, vec3 position, mat4 matrix);

void BE_CameraVectorInit(BE_CameraVector* vec);
void BE_CameraVectorPush(BE_CameraVector* vec, BE_Camera value);
void BE_CameraVectorFree(BE_CameraVector* vec);
void BE_CameraVectorCopy(BE_Camera* lights, size_t count, BE_CameraVector* outVec);
void BE_CameraVectorUpdateMatrix(BE_CameraVector* vec, int width, int height);

static inline BE_Camera* BE_FindCameraPtr(BE_CameraVector* vec, const char* name) {
    for (size_t i = 0; i < vec->size; i++) {
        if (strcmp(vec->data[i].name, name) == 0) {
            return &vec->data[i];
        }
    }
    return NULL;
}

typedef struct {
    char* name;
    BE_VertexVector vertices;
    BE_GLuintVector indices;
    BE_TextureVector textures;
    BE_VAO vao;
} BE_Mesh;

typedef struct {
    BE_Mesh* data;
    size_t size;
    size_t capacity;
} BE_MeshVector;

BE_Mesh BE_MeshInitFromVertex(const char* name, BE_VertexVector vertices, BE_GLuintVector indices, BE_TextureVector textures);
BE_Mesh BE_MeshInitFromData(const char* name, const char** texbuffer, int texcount, BE_Vertex* vertices, int vertcount, GLuint* indices, int indcount);
void BE_MeshDraw(BE_Mesh* mesh, BE_Shader* shader);
void BE_MeshDrawBillboard(BE_Mesh* mesh, BE_Shader* shader, BE_Texture* texture);

int BE_FindOrAddVertex(BE_Vertex* vertices, int* verticesCount, BE_Vertex v);
void BE_ReplacePathSuffix(const char* path, const char* newsuffix, char* dest, int destsize);
int BE_CountFaceVertices(const char* line);
BE_Mesh BE_LoadOBJToMesh(const char* name, const char* obj_path);
BE_Mesh BE_LoadOBJFromString(const char* name, const char* obj_contents);
const char** BE_LoadMTLTextures(const char* mtl_path, int* outCount);

void BE_MeshVectorInit(BE_MeshVector* vec);
void BE_MeshVectorPush(BE_MeshVector* vec, BE_Mesh value);
void BE_MeshVectorFree(BE_MeshVector* vec);
void BE_MeshVectorCopy(BE_Mesh* meshes, size_t count, BE_MeshVector* outVec);
void BE_CameraVectorDraw(BE_CameraVector* vec, BE_Mesh* mesh, BE_Shader* shader, BE_Camera* selected);

static inline BE_Mesh* BE_FindMeshPtr(BE_MeshVector* vec, const char* name) {
    for (size_t i = 0; i < vec->size; i++) {
        if (strcmp(vec->data[i].name, name) == 0) {
            return &vec->data[i];
        }
    }
    return NULL;
}

typedef struct {
    vec3 position;
    versor orientation;
    vec3 scale;
} BE_Transform;

typedef struct {
    char* name;
    BE_Mesh* mesh;
    BE_Transform transform;
} BE_Model;

typedef struct {
    BE_Model* data;
    size_t size;
    size_t capacity;
} BE_ModelVector;

BE_Transform BE_TransformInit(vec3 position, vec3 eulerRotation, vec3 scale);
void BE_TransformUpdateMatrix(BE_Transform* transform, mat4 outMatrix);

BE_Model BE_ModelInit(const char* name, BE_Mesh* mesh, BE_Transform transform);
void BE_ModelRotate(BE_Model* model, vec3 axis, float angle);

void BE_ModelVectorInit(BE_ModelVector* vec);
void BE_ModelVectorPush(BE_ModelVector* vec, BE_Model value);
void BE_ModelVectorFree(BE_ModelVector* vec);
void BE_ModelVectorCopy(BE_Model* models, size_t count, BE_ModelVector* outVec);
void BE_ModelVectorDraw(BE_ModelVector* vec, BE_Shader* shader);

static inline BE_Model* BE_FindModelPtr(BE_ModelVector* vec, const char* name) {
    for (size_t i = 0; i < vec->size; i++) {
        if (strcmp(vec->data[i].name, name) == 0) {
            return &vec->data[i];
        }
    }
    return NULL;
}

#define DIRECT_LIGHT_DIST 50

typedef struct {
    GLuint fbo;
    GLuint depthTextureArray;
    int width, height;
    int layers;
} BE_ShadowMapFBO;

typedef enum {
    BE_LIGHT_DIRECT,
    BE_LIGHT_POINT,
    BE_LIGHT_SPOT
} BE_LightType;

typedef struct {
    int type;
    char* name;

    vec3 position;
    vec3 direction;
    versor orientation;
    vec4 color;
    mat4 lightSpaceMatrix;
    float specular;
    
    // pointlight
    float a;
    float b;
    
    // spotlight
    float innerCone;
    float outerCone;
} BE_Light;

typedef struct {
    BE_Light* data;
    size_t size;
    size_t capacity;
    
    float ambient;
    BE_ShadowMapFBO directShadowFBO;
    BE_ShadowMapFBO pointShadowFBO;
    BE_ShadowMapFBO spotShadowFBO;

    int shadowsDirty;
} BE_LightVector;

BE_ShadowMapFBO BE_ShadowMapFBOInit(int width, int height, int layers);
void BE_ShadowMapFBOBindLayer(BE_ShadowMapFBO* smfbo, int layer);
void BE_ShadowMapFBODelete(BE_ShadowMapFBO* smfbo);

typedef void (*ShadowRenderFunc)(BE_Shader* shader);

BE_Light BE_LightInit(const char* name, int type, vec3 position, vec3 direction, vec4 color, float specular, float a, float b, float innerCone, float outerCone);
void BE_LightRotate(BE_Light* light, vec3 axis, float angle);

void BE_LightVectorInit(BE_LightVector* vec);
void BE_LightVectorPush(BE_LightVector* vec, BE_Light value);
void BE_LightVectorFree(BE_LightVector* vec);
void BE_LightVectorCopy(BE_Light* lights, size_t count, BE_LightVector* outVec);

void BE_LightVectorUpdateMatrix(BE_LightVector* vec);
void BE_LightVectorUpdateMaps(BE_LightVector* vec, BE_Shader* shadowShader, ShadowRenderFunc renderFunc, bool enabled);
void BE_LightVectorUpdateMultiMaps(BE_LightVector* vec, BE_ModelVector* models, BE_Shader* shadowShader, bool enabled);
void BE_LightVectorUpload(BE_LightVector* vec, BE_Shader* shader);
void BE_LightVectorDraw(BE_LightVector* vec, BE_Mesh* mesh, BE_Shader* shader);

static inline BE_Light* BE_FindLightPtr(BE_LightVector* vec, const char* name) {
    for (size_t i = 0; i < vec->size; i++) {
        if (strcmp(vec->data[i].name, name) == 0) {
            return &vec->data[i];
        }
    }
    return NULL;
}

typedef struct {
    char* name;
    vec3 position;
    vec2 scale;
    float rotation;
    vec3 color;
    BE_Texture* texture;
} BE_Sprite;

typedef struct {
    BE_Sprite* data;
    size_t size;
    size_t capacity;

    BE_VAO vao;
} BE_SpriteVector;

BE_Sprite BE_SpriteInit(const char* name, BE_Texture* texture, vec3 position, vec2 scale, vec3 color, float rotation);

void BE_SpriteVectorInit(BE_SpriteVector* vec);
void BE_SpriteVectorPush(BE_SpriteVector* vec, BE_Sprite value);
void BE_SpriteVectorFree(BE_SpriteVector* vec);
void BE_SpriteVectorCopy(BE_Sprite* sprites, size_t count, BE_SpriteVector* outVec);
void BE_SpriteVectorDraw(BE_SpriteVector* vec, BE_Shader* shader);

static inline BE_Sprite* BE_FindSpritePtr(BE_SpriteVector* vec, const char* name) {
    for (size_t i = 0; i < vec->size; i++) {
        if (strcmp(vec->data[i].name, name) == 0) {
            return &vec->data[i];
        }
    }
    return NULL;
}

typedef struct {
    FMOD_SYSTEM* system;
} BE_AudioEngine;

void BE_AudioEngineInit(BE_AudioEngine* engine);
void BE_AudioEngineUpdate(BE_AudioEngine* engine);
void BE_AudioEngineFree(BE_AudioEngine* engine);

typedef struct {
    FMOD_SOUND* sound;
    char* name;
    char* path;
} BE_Sound;

typedef struct {
    BE_Sound* data;
    size_t size;
    size_t capacity;
} BE_SoundVector;

BE_Sound BE_SoundLoad(BE_AudioEngine* engine, const char* path, const char* name, bool spatial, float min, float max);
void BE_SoundFree(BE_Sound* sound);

void BE_SoundVectorInit(BE_SoundVector* vec);
void BE_SoundVectorPush(BE_SoundVector* vec, BE_Sound value);
void BE_SoundVectorFree(BE_SoundVector* vec);
void BE_SoundVectorCopy(BE_Sound* sounds, size_t count, BE_SoundVector* outVec);
void BE_SoundVectorRemove(BE_SoundVector* vec, BE_Sound* sound);

static inline BE_Sound* BE_FindSoundPtr(BE_SoundVector* vec, const char* name) {
    for (size_t i = 0; i < vec->size; i++) {
        if (strcmp(vec->data[i].name, name) == 0) {
            return &vec->data[i];
        }
    }
    return NULL;
}

typedef struct {
    char* name;
    vec3 position;
    float gain;
    float pitch;
    bool looping;
    bool spatial;
    FMOD_CHANNEL* channel;
    FMOD_DSP* reverbDSP;
} BE_Emitter;

typedef struct {
    BE_Emitter* data;
    size_t size;
    size_t capacity;
} BE_EmitterVector;

BE_Emitter BE_EmitterInit(const char* name, vec3 position, bool spatial);

void BE_EmitterPlaySound(BE_AudioEngine* engine, BE_Emitter* src, BE_Sound* sound);
void BE_EmitterStop(BE_Emitter* src);
void BE_EmitterPause(BE_Emitter* src, bool pause);

void BE_EmitterSetSeek(BE_Emitter* src, float seek);
float BE_EmitterGetSeek(BE_Emitter* src);
void BE_EmitterSetPosition(BE_Emitter* src, vec3 position);
void BE_EmitterSetGain(BE_Emitter* src, float gain);
void BE_EmitterSetPitch(BE_Emitter* src, float pitch);
void BE_EmitterSetLooping(BE_Emitter* src, bool looping);
void BE_EmitterSetReverb(BE_Emitter* src, float decay, float mix);
void BE_EmitterRemoveReverb(BE_Emitter* src);
void BE_EmitterSetListener(BE_AudioEngine* engine, vec3 position, vec3 direction, vec3 velocity);
void BE_EmitterSetListenerVersor(BE_AudioEngine* engine, vec3 position, versor orientation, vec3 velocity);
float BE_EmitterGetVolume(BE_Emitter* src);
float BE_EmitterGetPitch(BE_Emitter* src);
void BE_EmitterGetPosition(BE_Emitter* src, vec3 dest);
bool BE_EmitterGetIsPlaying(BE_Emitter* src);
bool BE_EmitterGetIsPaused(BE_Emitter* src);

void BE_EmitterVectorInit(BE_EmitterVector* vec);
void BE_EmitterVectorPush(BE_EmitterVector* vec, BE_Emitter value);
void BE_EmitterVectorFree(BE_EmitterVector* vec);
void BE_EmitterVectorCopy(BE_Emitter* emitters, size_t count, BE_EmitterVector* outVec);
void BE_EmitterVectorDraw(BE_EmitterVector* vec, BE_Mesh* mesh, BE_Shader* shader);
void BE_EmitterVectorRemove(BE_EmitterVector* vec, BE_Emitter* emitter);

static inline BE_Emitter* BE_FindEmitterPtr(BE_EmitterVector* vec, const char* name) {
    for (size_t i = 0; i < vec->size; i++) {
        if (strcmp(vec->data[i].name, name) == 0) {
            return &vec->data[i];
        }
    }
    return NULL;
}

typedef struct {
    BE_TextureVector textures;
    BE_MeshVector meshes;
    BE_SoundVector sounds;
    BE_ShaderVector shaders;
    BE_VAOVector vaos;

    BE_Shader default3DShader;
    BE_Shader defaultDepthShader;
    BE_Shader defaultColorShader;
    BE_Shader defaultSpriteShader;

    BE_Mesh defaultCubeMesh;
    BE_Mesh defaultCameraMesh;
} BE_Resources;

typedef struct {
    char* name;
    
    BE_Camera* activeCamera;

    BE_ModelVector models;
    BE_LightVector lights;
    BE_CameraVector cameras;
    BE_SpriteVector sprites;
    BE_EmitterVector emitters;
} BE_Scene;

typedef struct {
    BE_Scene* data;
    size_t size;
    size_t capacity;
} BE_SceneVector;

BE_Scene BE_SceneInit(const char* name);

void BE_SceneVectorInit(BE_SceneVector* vec);
void BE_SceneVectorPush(BE_SceneVector* vec, BE_Scene value);
void BE_SceneVectorFree(BE_SceneVector* vec);
void BE_SceneVectorCopy(BE_Scene* scenes, size_t count, BE_SceneVector* outVec);
void BE_SceneVectorRemove(BE_SceneVector* vec, BE_Scene* value);

static inline BE_Scene* BE_FindScenePtr(BE_SceneVector* vec, const char* name) {
    for (size_t i = 0; i < vec->size; i++) {
        if (strcmp(vec->data[i].name, name) == 0) {
            return &vec->data[i];
        }
    }
    return NULL;
}

typedef struct {
    char* title;
    GLFWwindow* window;
    int width, height;
    
    BE_Scene* activeScene;

    BE_SceneVector scenes;
    BE_Resources resources;
    BE_AudioEngine audio;

    BE_FBO FBOs[2];
    int ping;

    BE_FrameStats timer;
    BE_Joystick joystick;

    bool running;

} BE_Engine;

extern BE_Engine* g_engine;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// =======================
// ENIGNE
// =======================

/**
 * @brief Initializes and starts a Ballistic Engine object  
 * @param width The width of the window in pixels. Must be greater than 0.
 * @param height The height of the window in pixels. Must be greater than 0.
 * @param title The title of the window. If NULL, a default title "Ballistic Engine" will be used.
 * @return A fully initialized BE_Engine struct. If initialization fails, this function will return NULL.
 * @see BE_BindEngine(), BE_EngineShutdown()
 */
#define BE_StartEngine(title, width, height) BE_IMPL_EngineStart(title, width, height, __FILE__, __LINE__)
BE_Engine BE_IMPL_EngineStart(const char* title, int width, int height, const char* file, int line);

#define BE_ShutdownEngine(engine) do { BE_IMPL_EngineShutdown(engine, __FILE__, __LINE__); } while(0)
void BE_IMPL_EngineShutdown(BE_Engine* engine, const char* file, int line);

/**
 * @brief Binds an engine
 * @param engine Pointer to the engine you want to bind (const char*). Must not be NULL.
 * @see BE_UnbindEngine()
 */
#define BE_BindEngine(engine) do { BE_IMPL_BindEngine(engine, __FILE__, __LINE__); } while(0)
void BE_IMPL_BindEngine(BE_Engine* engine, const char* file, int line);

/**
 * @brief Unbinds an engine
 * @see BE_BindEngine()
 */
#define BE_UnbindEngine() do { BE_IMPL_UnbindEngine(__FILE__, __LINE__); } while(0)
void BE_IMPL_UnbindEngine(const char* file, int line);

// =======================
// FRAMES
// =======================

#define BE_WindowIsOpen() BE_IMPL_WindowIsOpen(__FILE__, __LINE__)
bool BE_IMPL_WindowIsOpen(const char* file, int line);

#define BE_CloseWindow() do { BE_IMPL_CloseWindow(__FILE__, __LINE__); } while(0);
void BE_IMPL_CloseWindow(const char* file, int line);

#define BE_BeginFrame() do { BE_IMPL_BeginFrame(__FILE__, __LINE__); } while(0)
void BE_IMPL_BeginFrame(const char* file, int line);

#define BE_MakeShadows(active) do { BE_IMPL_MakeShadows(active, __FILE__, __LINE__); } while(0)
void BE_IMPL_MakeShadows(bool active, const char* file, int line);

#define BE_BeginRender() do { BE_IMPL_BeginRender(__FILE__, __LINE__); } while(0)
void BE_IMPL_BeginRender(const char* file, int line);

#define BE_EndFrame() do { BE_IMPL_EndFrame(__FILE__, __LINE__); } while(0)
void BE_IMPL_EndFrame(const char* file, int line);

// =======================
// SHADERS
// =======================

#define BE_LoadShader(shaderName, vertexFile, fragmentFile, geometryFile, computeFile) do { BE_IMPL_LoadShader(shaderName, vertexFile, fragmentFile, geometryFile, computeFile, __FILE__, __LINE__); } while(0)
void BE_IMPL_LoadShader(const char* shaderName, const char* vertexFile, const char* fragmentFile, const char* geometryFile, const char* computeFile, const char* file, int line);

// =======================
// MESHES
// =======================

#define BE_LoadMesh(meshName, objFile) do { BE_IMPL_LoadMesh(meshName, objFile, __FILE__, __LINE__); } while(0)
void BE_IMPL_LoadMesh(const char* meshName, const char* objFile, const char* file, int line);

// =======================
// TEXTURES
// =======================

#define BE_LoadTexture(textureName, imageFile) do { BE_IMPL_LoadTexture(textureName, imageFile, __FILE__, __LINE__); } while(0)
void BE_IMPL_LoadTexture(const char* textureName, const char* imageFile, const char* file, int line);

// =======================
// SOUNDS
// =======================

/**
 * @brief Loads a new sound
 * @param soundName The name of the new sound (const char*). If NULL, the default name will be used.
 * @param soundFile The path to the sound file (const char*). Must not be NULL.
 * @param spatial If the sound can be spatial (bool).
 * @param min The distance when the sound starts losing volume (float). Must be greater than 0.
 * @param max The maximum distance the sound can be heard (float). Must be greater than 0.
 * @see BE_DeleteSound()
 */
#define BE_LoadSound(soundName, soundFile, spatial, min, max) do { BE_IMPL_LoadSound(soundName, soundFile, spatial, min, max, __FILE__, __LINE__); } while(0)
void BE_IMPL_LoadSound(const char* soundName, const char* soundFile, bool spatial, float min, float max, const char* file, int line);

/**
 * @brief Deletes a specific sound
 * @param soundName The name of the specific sound (const char*). Must not be NULL.
 * @see BE_LoadSound(), BE_DeleteAllSounds()
 */
#define BE_DeleteSound(soundName) do { BE_IMPL_DeleteSound(soundName, __FILE__, __LINE__); } while(0)
void BE_IMPL_DeleteSound(const char* soundName, const char* file, int line);

/**
 * @brief Removes all sounds
 * @see BE_DeleteSound()
 */
#define BE_DeleteAllSounds() do { BE_IMPL_DeleteAllSounds(__FILE__, __LINE__); } while(0)
void BE_IMPL_DeleteAllSounds(const char* file, int line);

/**
 * @brief Checks whether a specific sound exists
 * @param soundName The name of the sound to check (const char*). Must not be NULL.
 * @return 'true' if the sound exists. 'false' if the sound does not exist or an error occurs.
 */
#define BE_CheckSound(soundName) BE_IMPL_CheckSound(soundName, __FILE__, __LINE__)
bool BE_IMPL_CheckSound(const char* soundName, const char* file, int line);

// =======================
// SCENES
// =======================

/**
 * @brief Binds a specific scene
 * @param sceneName The name of the specific scene (const char*). If NULL, current scene is unbound.
 * @see BE_UnbindScene()
 */
#define BE_BindScene(sceneName) do { BE_IMPL_BindScene(sceneName, __FILE__, __LINE__); } while(0)
void BE_IMPL_BindScene(const char* sceneName, const char* file, int line);

/**
 * @brief Unbinds the bound scene
 * @see BE_BindScene()
 */
#define BE_UnbindScene() do { BE_IMPL_UnbindScene(__FILE__, __LINE__); } while(0)
void BE_IMPL_UnbindScene(const char* file, int line);

/**
 * @brief Adds and binds a new scene
 * @param sceneName The name of the new scene (const char*). If NULL, the default name will be used.
 */
#define BE_AddScene(sceneName) do { BE_IMPL_AddScene(sceneName, __FILE__, __LINE__); } while(0)
void BE_IMPL_AddScene(const char* sceneName, const char* file, int line);

/**
 * @brief Deletes a specific scene
 * @param sceneName The name of the specific scene (const char*). Must not be NULL.
 * @note If all scenes are deleted, a blank scene will be created
 * @see BE_LoadSound(), BE_DeleteAllSounds()
 */
#define BE_DeleteScene(sceneName) do { BE_IMPL_DeleteScene(sceneName, __FILE__, __LINE__); } while(0)
void BE_IMPL_DeleteScene(const char* sceneName, const char* file, int line);

/**
 * @brief Deletes all scenes
 * @see BE_DeleteScene()
 */
#define BE_DeleteAllScenes() do { BE_IMPL_DeleteAllScenes(__FILE__, __LINE__); } while(0)
void BE_IMPL_DeleteAllScenes(const char* file, int line);

/**
 * @brief Checks whether a specific scene exists
 * @param sceneName The name of the scene to check (const char*). Must not be NULL.
 * @return 'true' if the scene exists. 'false' if the scene does not exist or an error occurs.
 */
#define BE_CheckScene(sceneName) BE_IMPL_CheckScene(sceneName, __FILE__, __LINE__)
bool BE_IMPL_CheckScene(const char* sceneName, const char* file, int line);

// =======================
// MODELS
// =======================

#define BE_AddModel(modelName, meshName) do { BE_IMPL_AddModel(modelName, meshName, __FILE__, __LINE__); } while(0)
void BE_IMPL_AddModel(const char* modelName, const char* meshName, const char* file, int line);

#define BE_DrawModels(shaderName) do { BE_IMPL_DrawModels(shaderName, __FILE__, __LINE__); } while(0)
void BE_IMPL_DrawModels(const char* shaderName, const char* file, int line);

// =======================
// LIGHTS
// =======================

#define BE_AddLight(lightName, type) do { BE_IMPL_AddLight(lightName, type, __FILE__, __LINE__); } while(0)
void BE_IMPL_AddLight(const char* lightName, BE_LightType type, const char* file, int line);

#define BE_DrawLights(shaderName) do { BE_IMPL_DrawLights(shaderName, __FILE__, __LINE__); } while(0)
void BE_IMPL_DrawLights(const char* shaderName, const char* file, int line);

// =======================
// CAMERAS
// =======================

#define BE_AddCamera(cameraName) do { BE_IMPL_AddCamera(cameraName, __FILE__, __LINE__); } while(0)
void BE_IMPL_AddCamera(const char* cameraName, const char* file, int line);

#define BE_DrawCameras(shaderName) do { BE_IMPL_DrawCameras(shaderName, __FILE__, __LINE__); } while(0)
void BE_IMPL_DrawCameras(const char* shaderName, const char* file, int line);

// =======================
// SPRITES
// =======================

/**
 * @brief Adds a new sprite with a specific texture to the bound scene
 * @param spriteName The name of the new sprite (const char*). If NULL, the default name will be used.
 * @param textureName The name of the specific texture (const char*). Must not be NULL.
 */
#define BE_AddSprite(emitterName, textureName) do { BE_IMPL_AddSprite(spriteName, textureName, __FILE__, __LINE__); } while(0)
void BE_IMPL_AddSprite(const char* spriteName, const char* textureName, const char* file, int line);

/**
 * @brief Draws all sprites with a specific shader
 * @param shaderName The name of the specific shader (const char*). If NULL, a default shader will be used.
 */
#define BE_DrawSprites(shaderName) do { BE_IMPL_DrawSprites(shaderName, __FILE__, __LINE__); } while(0)
void BE_IMPL_DrawSprites(const char* shaderName, const char* file, int line);

// =======================
// AUDIO EMITTERS
// =======================

/**
 * @brief Adds a new emitter to the bound scene
 * @param emitterName The name of the new emitter (const char*). If NULL, the default name will be used.
 * @param spatial If the emitter is spatial (bool).
 */
#define BE_AddEmitter(emitterName, spatial) do { BE_IMPL_AddEmitter(emitterName, spatial, __FILE__, __LINE__); } while(0)
void BE_IMPL_AddEmitter(const char* emitterName, bool spatial, const char* file, int line);

/**
 * @brief Deletes a specific audio emitter
 * @param emitterName The name of the specific emitter (const char*). Must not be NULL.
 * @see BE_AddEmitter(), BE_RemoveEmitter()
 */
#define BE_RemoveEmitter(emitterName) do { BE_IMPL_RemoveEmitter(emitterName, __FILE__, __LINE__); } while(0)
void BE_IMPL_RemoveEmitter(const char* emitterName, const char* file, int line);

/**
 * @brief Removes all audio emitters
 * @see BE_RemoveEmitter()
 */
#define BE_RemoveAllEmitters() do { BE_IMPL_RemoveAllEmitters(__FILE__, __LINE__); } while(0)
void BE_IMPL_RemoveAllEmitters(const char* file, int line);

/**
 * @brief Checks whether a specific audio emitter exists
 * @param emitterName The name of the emitter to check (const char*). Must not be NULL.
 * @return 'true' if the emitter exists. 'false' if the emitter does not exist or an error occurs.
 */
#define BE_CheckEmitter(emitterName) BE_IMPL_CheckEmitter(emitterName, __FILE__, __LINE__)
bool BE_IMPL_CheckEmitter(const char* emitterName, const char* file, int line);

/**
 * @brief Draws all emitters with a specific shader
 * @param shaderName The name of the specific shader (const char*). If NULL, a default shader will be used.
 */
#define BE_DrawEmitters(shaderName) do { BE_IMPL_DrawEmitters(shaderName, __FILE__, __LINE__); } while(0)
void BE_IMPL_DrawEmitters(const char* shaderName, const char* file, int line);

/**
 * @brief Plays a specific sound from a specific audio emitter
 * @param emitterName The name of the specific emitter (const char*). Must not be NULL.
 * @param soundName The name of the specific sound (const char*). Must not be NULL.
 * @see BE_GetEmitterPlaying()
 */
#define BE_PlayEmitter(emitterName, soundName) do { BE_IMPL_PlayEmitter(emitterName, soundName, __FILE__, __LINE__); } while(0)
void BE_IMPL_PlayEmitter(const char* emitterName, const char* soundName, const char* file, int line);

/**
 * @brief Stops the sound from a specific audio emitter
 * @param emitterName The name of the specific emitter (const char*). Must not be NULL.
 * @see BE_PlayEmitter()
 */
#define BE_StopEmitter(emitterName) do { BE_IMPL_StopEmitter(emitterName, __FILE__, __LINE__); } while(0)
void BE_IMPL_StopEmitter(const char* emitterName, const char* file, int line);

/**
 * @brief Toggles pause of a specific audio emitter
 * @param emitterName The name of the specific emitter (const char*). Must not be NULL.
 * @param pause If the emitter is paused (bool).
 * @see BE_PlayEmitter(), BE_GetEmitterPaused()
 */
#define BE_PauseEmitter(emitterName, pause) do { BE_IMPL_PauseEmitter(emitterName, pause, __FILE__, __LINE__); } while(0)
void BE_IMPL_PauseEmitter(const char* emitterName, bool pause, const char* file, int line);

/**
 * @brief Toggles looping of a specific audio emitter
 * @param emitterName The name of the specific emitter (const char*). Must not be NULL.
 * @param looping If the emitter is looping (bool).
 * @see BE_SetEmitterPositionToCamera()
 */
#define BE_SetEmitterLooping(emitterName, looping) do { BE_IMPL_SetEmitterLooping(emitterName, looping, __FILE__, __LINE__); } while(0)
void BE_IMPL_SetEmitterLooping(const char* emitterName, bool looping, const char* file, int line);

/**
 * @brief Sets the position of a specific audio emitter
 * @param emitterName The name of the specific emitter (const char*). Must not be NULL.
 * @param seek The new playtime of the emitter (float). Must be greater than 0.
 * @see BE_SetEmitterPositionToCamera()
 */
#define BE_SetEmitterSeek(emitterName, seek) do { BE_IMPL_SetEmitterSeek(emitterName, seek, __FILE__, __LINE__); } while(0)
void BE_IMPL_SetEmitterSeek(const char* emitterName, float seek, const char* file, int line);

/**
 * @brief Sets the position of a specific audio emitter
 * @param emitterName The name of the specific emitter (const char*). Must not be NULL.
 * @param position The new position of the emitter (vec3, use BE_vec3);. If NULL, position is set to x=0, y=0, z=0.
 * @note HINT: For vec3s try using BE_vec3().
 * @see BE_SetEmitterPositionToCamera()
 */
#define BE_SetEmitterPosition(emitterName, position) do { BE_IMPL_SetEmitterPosition(emitterName, position, __FILE__, __LINE__); } while(0)
void BE_IMPL_SetEmitterPosition(const char* emitterName, vec3 position, const char* file, int line);

/**
 * @brief Sets the position of a specific audio emitter to a specific camera
 * @param emitterName The name of the specific emitter (const char*). Must not be NULL.
 * @param cameraName The name of the specific camera (const char*). Must not be NULL.
 * @see BE_SetEmitterPosition()
 */
#define BE_SetEmitterPositionToCamera(emitterName, cameraName) do { BE_IMPL_SetEmitterPositionToCamera(emitterName, cameraName, __FILE__, __LINE__); } while(0)
void BE_IMPL_SetEmitterPositionToCamera(const char* emitterName, const char* cameraName, const char* file, int line);

/**
 * @brief Sets the position of the audio listener 
 * @param position The new position of the listener (vec3). If NULL, position is set to x=0, y=0, z=0.
 * @param direction The new direction of the listener (vec3, radians). If NULL, direction is set to x=0, y=0, z=0.
 * @param velocity The new velocity of the listener (vec3). If NULL, velocity is set to x=0, y=0, z=0.
 * @note HINT: For vec3s try using BE_vec3().
 * @see BE_SetListenerPositionToCamera(), BE_SetListenerPositionToActiveCamera()
 */
#define BE_SetListenerPosition(position, direction, velocity) do { BE_IMPL_SetListenerPosition(position, direction, velocity, __FILE__, __LINE__); } while(0)
void BE_IMPL_SetListenerPosition(vec3 position, vec3 direction, vec3 velocity, const char* file, int line);

/**
 * @brief Sets the audio listener to a specific camera
 * @param cameraName The name of the specific camera (const char*). Must not be NULL.
 * @see BE_SetListenerPosition(), BE_SetListenerPositionToActiveCamera()
 */
#define BE_SetListenerPositionToCamera(cameraName) do { BE_IMPL_SetListenerPositionToCamera(cameraName, __FILE__, __LINE__); } while(0)
void BE_IMPL_SetListenerPositionToCamera(const char* cameraName, const char* file, int line);

/**
 * @brief Sets the audio listener to the bound camera
 * @see BE_SetListenerPosition(), BE_SetListenerPositionToCamera()
 */
#define BE_SetListenerPositionToActiveCamera() do { BE_IMPL_SetListenerPositionToActiveCamera(__FILE__, __LINE__); } while(0)
void BE_IMPL_SetListenerPositionToActiveCamera(const char* file, int line);

/**
 * @brief Sets the volume o a specific audio emitter
 * @param emitterName The name of the specific emitter (const char*). Must not be NULL.
 * @param volume The new volume of the emitter (float). Must be greater than 0.
 * @see BE_GetEmitterVolume()
 */
#define BE_SetEmitterVolume(emitterName, volume) do { BE_IMPL_SetEmitterVolume(emitterName, volume, __FILE__, __LINE__); } while(0)
void BE_IMPL_SetEmitterVolume(const char* emitterName, float volume, const char* file, int line);

/**
 * @brief Sets the pitch on a specific audio emitter
 * @param emitterName The name of the specific emitter (const char*). Must not be NULL.
 * @param pitch The new pitch of the emitter (float). Must be greater than 0.
 * @see BE_GetEmitterPitch()
 */
#define BE_SetEmitterPitch(emitterName, pitch) do { BE_IMPL_SetEmitterPitch(emitterName, pitch, __FILE__, __LINE__); } while(0)
void BE_IMPL_SetEmitterPitch(const char* emitterName, float pitch, const char* file, int line);

/**
 * @brief Removes the reverb on a specific audio emitter
 * @param emitterName The name of the specific emitter (const char*). Must not be NULL.
 * @param decay The decay of the reverb tail in seconds (float). Must be greater than 0.
 * @param mix The mix of dry/wet signal from 0 (dry) to 1 (wet). Must be greater than 0.
 * @see BE_RemoveEmitterReverb()
 */
#define BE_SetEmitterReverb(emitterName, decay, mix) do { BE_IMPL_SetEmitterReverb(emitterName, decay, mix, __FILE__, __LINE__); } while(0)
void BE_IMPL_SetEmitterReverb(const char* emitterName, float decay, float mix, const char* file, int line);

/**
 * @brief Removes the reverb on a specific audio emitter
 * @param emitterName The name of the specific emitter (const char*). Must not be NULL.
 * @see BE_SetEmitterReverb()
 */
#define BE_RemoveEmitterReverb(emitterName) do { BE_IMPL_RemoveEmitterReverb(emitterName, __FILE__, __LINE__); } while(0)
void BE_IMPL_RemoveEmitterReverb(const char* emitterName, const char* file, int line);

/**
 * @brief Stops the sounds on all emitters in the bound scene
 * @see BE_PlayEmitter()
 */
#define BE_StopAllEmitters(emitterName) do { BE_IMPL_StopAllEmitters(__FILE__, __LINE__); } while(0)
void BE_IMPL_StopAllEmitters(const char* file, int line);

/**
 * @brief Gets the volume of a specific audio emitter
 * @param emitterName The name of the emitter to check (const char*). Must not be NULL.
 * @return The emitters pitch as a number from 0 to 1 (float). '0' if an error occurs.
 * @see BE_SetEmitterVolume()
 */
#define BE_GetEmitterVolume(emitterName) BE_IMPL_GetEmitterVolume(emitterName, __FILE__, __LINE__)
float BE_IMPL_GetEmitterVolume(const char* emitterName, const char* file, int line);

/**
 * @brief Gets the pitch of a specific audio emitter
 * @param emitterName The name of the emitter to check (const char*). Must not be NULL.
 * @return The emitters pitch as a number from 0 to 1 (float). '0' if an error occurs.
 * @see BE_SetEmitterPitch()
 */
#define BE_GetEmitterPitch(emitterName) BE_IMPL_GetEmitterPitch(emitterName, __FILE__, __LINE__)
float BE_IMPL_GetEmitterPitch(const char* emitterName, const char* file, int line);

/**
 * @brief Gets the playtime of a specific audio emitter
 * @param emitterName The name of the emitter to check (const char*). Must not be NULL.
 * @return The emitters playtime in milliseconds (float). '0' if an error occurs.
 * @see BE_SetEmitterSeek()
 */
#define BE_GetEmitterSeek(emitterName) BE_IMPL_GetEmitterSeek(emitterName, __FILE__, __LINE__)
float BE_IMPL_GetEmitterSeek(const char* emitterName, const char* file, int line);

/**
 * @brief Gets the position of a specific audio emitter
 * @param emitterName The name of the emitter to check (const char*). Must not be NULL.
 * @param dest The variable to write the position to (vec3). Must not be NULL.
 * @see BE_SetEmitterPosition(), BE_SetEmitterPositionToCamera()
 */
#define BE_GetEmitterPosition(emitterName, dest) do { BE_IMPL_GetEmitterPosition(emitterName, dest, __FILE__, __LINE__); } while(0)
void BE_IMPL_GetEmitterPosition(const char* emitterName, vec3 dest, const char* file, int line);

/**
 * @brief Checks whether a specific audio emitter is playing
 * @param emitterName The name of the emitter to check (const char*). Must not be NULL.
 * @return 'true' if the emitter is playing. 'false' if the emitter is not playing or an error occurs.
 * @see BE_PlayEmitter()
 */
#define BE_GetEmitterPlaying(emitterName) BE_IMPL_GetEmitterPlaying(emitterName, __FILE__, __LINE__)
bool BE_IMPL_GetEmitterPlaying(const char* emitterName, const char* file, int line);

/**
 * @brief Checks whether a specific audio emitter is paused
 * @param emitterName The name of the emitter to check (const char*). Must not be NULL.
 * @return 'true' if the emitter is paused. 'false' if the emitter is unpaused or an error occurs.
 * @see BE_PauseEmitter()
 */
#define BE_GetEmitterPaused(emitterName) BE_IMPL_GetEmitterPaused(emitterName, __FILE__, __LINE__)
bool BE_IMPL_GetEmitterPaused(const char* emitterName, const char* file, int line);