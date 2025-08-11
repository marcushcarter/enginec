#ifndef ENGINE_H
#define ENGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stb_image/stb_image.h>
#include <stb_image/stb_image_resize.h>
#include <time.h>

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

} DeltaTime;

extern DeltaTime delta;

float deltaTimeUpdate();

void make_model_matrix(vec3 translation, vec3 rotation, vec3 scale, mat4 dest);
void make_billboard_matrix(vec3 position, mat4 view, vec3 scale, mat4 dest);
void orientation_to_euler(vec3 orientation, vec3 outEuler);
void print_mat4(mat4 m);

typedef struct {
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 texUV;
} Vertex;

typedef struct {
    Vertex* data;
    size_t size;
    size_t capacity;
} VertexVector;

void VertexVector_Init(VertexVector* vec);
void VertexVector_Push(VertexVector* vec, Vertex value);
void VertexVector_Free(VertexVector* vec);
void VertexVector_Copy(Vertex* vertices, size_t count, VertexVector* outVec);

typedef struct {
    GLuint ID;
} VAO;

VAO VAO_Init();
VAO VAO_InitQuad();
VAO VAO_InitBillboardQuad();
void VAO_Bind(VAO* vao);
void VAO_DrawQuad(VAO* vao);
void VAO_Unbind();
void VAO_Delete(VAO* vao);

typedef struct {
    GLuint ID;
} VBO;

VBO VBO_InitRaw(GLfloat* vertices, GLsizeiptr size);
VBO VBO_Init(VertexVector* vertices);
void VBO_Bind(VBO* vbo);
void VBO_Unbind();
void VBO_Delete(VBO* vbo);
void VAO_LinkAttrib(VBO* vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);

typedef struct {
    GLuint* data;
    size_t size;
    size_t capacity;
} GLuintVector;

void GLuintVector_Init(GLuintVector* vec);
void GLuintVector_Push(GLuintVector* vec, GLuint value);
void GLuintVector_Free(GLuintVector* vec);
void GLuintVector_Copy(GLuint* data, size_t count, GLuintVector* outVec);

typedef struct {
    GLuint ID;
} EBO;

EBO EBO_InitRaw(GLuint* indices, GLsizeiptr size);
EBO EBO_Init(GLuintVector* indices);
void EBO_Bind(EBO* ebo);
void EBO_Unbind();
void EBO_Delete(EBO* ebo);

typedef struct {
    GLuint ID;
} Shader;

char* get_file_contents(const char* filename);
void Shader_compileErrors(unsigned int shader, const char* type);
Shader Shader_Init(const char* vertexFile, const char* fragmentFile, const char* geometryFile);
void Shader_Activate(Shader* shader);
void Shader_Delete(Shader* shader);

typedef struct {
    GLuint fbo;
    GLuint texture;
    GLuint rbo;
    VAO vao;
    VBO vbo;
    int width, height;
} FBO;

FBO FBO_Init(int width, int height);
void FBO_Resize(FBO* fbo, int newWidth, int newHeight);
FBO FBO_InitShadowMap(int width, int height, float clampColor[4]);
void FBO_Bind(FBO* fb);
void FBO_BindTexture(FBO* fb, Shader* shader);
void FBO_Unbind();
void FBO_Delete(FBO* fb);

typedef struct {
    GLuint ID;
    char* type;
    GLuint unit;
} Texture;

Texture Texture_Init(const char* imageFile, const char* texType, GLenum slot);
void Texture_texUnit(Shader* shader, const char* uniform, GLuint unit);
void Texture_Bind(Texture* texture);
void Texture_Unbind();
void Texture_Delete(Texture* texture);

typedef struct {
    Texture* data;
    size_t size;
    size_t capacity;
} TextureVector;

void TextureVector_Init(TextureVector* vec);
void TextureVector_Push(TextureVector* vec, Texture value);
void TextureVector_Free(TextureVector* vec);
void TextureVector_Copy(Texture* textures, size_t count, TextureVector* outVec);

#define MAX_JOYSTICKS GLFW_JOYSTICK_16+1

typedef struct {
    int id;  // like GLFW_JOYSTICK_1
    int present;
    /**
     * 0 -> LS X
     * 1 -> LS Y
     * 2 -> RS x
     * 3 -> RS Y
     * 4 -> LT
     * 5 -> RT
     */
    const float* axes;
    /**
     * 0 -> A
     * 1 -> B
     * 2 -> X
     * 3 -> Y
     * 4 -> LB
     * 5 -> RB
     * 6 -> BACK
     * 7 -> START
     * 8 -> LS
     * 9 -> RS
     * 10 -> D-UP
     * 11 -> D-RIGHT
     * 12 -> D-DOWN
     * 13 -> D-LEFT
     * 14 -> 
     * 15 -> 
     */
    const unsigned char* buttons;
    unsigned char lbuttons[16]; // last-frame buttons
    /**
     * +1 ->
     * +2 ->
     * +4 ->
     * +8 ->
     */
    const unsigned char* hats;
    const char* name;

    int axisCount;
    int buttonCount;
    int hatCount;

    float deadzone;
} Joystick;

extern Joystick joysticks[MAX_JOYSTICKS];
extern int joystickCount;

extern Joystick joysticks[MAX_JOYSTICKS];
extern int joystickCount;
void glfwJoystickEvents(void);
int joystickIsPressed(Joystick* js, int button);
int joystickIsReleased(Joystick* js, int button);
int joystickIsHeld(Joystick* js, int button);
float joystickGetAxis(Joystick* js, int axis);

typedef struct {
    int width, height;
    float zoom, fov;
    float nearPlane, farPlane;
    vec3 position, direction, Up;
    mat4 cameraMatrix, viewMatrix;
} Camera;

Camera Camera_InitStack(int width, int height, float fov, float nearPlane, float farPlane, vec3 position, vec3 direction);
Camera* Camera_InitHeap(int width, int height, float fov, float nearPlane, float farPlane, vec3 position, vec3 direction);
void Camera_UpdateMatrix(Camera* camera, int width, int height);
void Camera_Matrix(Camera* camera, Shader* shader, const char* uniform);
void Camera_MatrixCustom(Shader* shader, const char* uniform, mat4 matrix);
void rotate_vec3_axis(vec3 in, vec3 axis, float angle_rad, vec3 out);
void Camera_Inputs(Camera* camera, GLFWwindow* window, Joystick* js, float dt);
void print_Camera(Camera* camera);

typedef struct {
    Camera** data;
    size_t size;
    size_t capacity;
} CameraVector;

void CameraVector_Init(CameraVector* vec);
void CameraVector_Push(CameraVector* vec, Camera* cam);
Camera* CameraVector_Get(CameraVector* vec, size_t index);
void CameraVector_Remove(CameraVector* vec, Camera* cam);
void CameraVector_RemoveAt(CameraVector* vec, size_t index);
size_t CameraVector_IndexOf(CameraVector* vec, Camera* cam);
void CameraVector_Free(CameraVector* vec);

typedef struct {
    VertexVector vertices;
    GLuintVector indices;
    TextureVector textures;
    VAO vao;
} Mesh;

Mesh Mesh_Init(VertexVector vertices, GLuintVector indices, TextureVector textures);
Mesh Mesh_InitFromData(const char** texbuffer, int texcount, Vertex* vertices, int vertcount, GLuint* indices, int indcount);
void Mesh_Draw(Mesh* mesh, Shader* shader, Camera* camera);
void Mesh_DrawBillboard(Mesh* mesh, Shader* shader, Camera* camera, Texture* texture);

int find_or_add_vertex(Vertex* vertices, int* verticesCount, Vertex v);
void replacePathSuffix(const char* path, const char* newsuffix, char* dest, int destsize);
int count_face_vertices(const char* line);
Mesh Import_loadMeshFromOBJ(const char* obj_path);
const char** Mesh_getTexturesFromMTL(const char* mtl_path, int* outCount);

void CameraVector_Draw(CameraVector* cameras, Mesh* mesh, Shader* shader, Camera* camera);

#define DIRECT_LIGHT_DIST 50

typedef struct {
    vec3 direction;
    vec4 color;
    float specular;

    mat4 lightSpaceMatrix;
} DirectLight;

#define MAX_POINT_LIGHTS 10

typedef struct {
    vec3 position;
    vec4 color;
    float a;
    float b;
    float specular;
    
} PointLight;

#define MAX_SPOT_LIGHTS 10

typedef struct {
    vec3 position;
    vec3 direction;
    vec4 color;

    float innerCone;
    float outerCone;
    float specular;

    mat4 lightSpaceMatrix;
} SpotLight;

typedef struct {
    GLuint fbo;
    GLuint depthTextureArray;
    int width, height;
    int layers;
} ShadowMapFBO;

ShadowMapFBO ShadowMapFBO_Init(int width, int height, int layers);
void ShadowMapFBO_BindLayer(ShadowMapFBO* smfbo, int layer);
void ShadowMapFBO_Delete(ShadowMapFBO* smfbo);

typedef struct {
    DirectLight directlight;
    PointLight pointlights[16];
    SpotLight spotlights[16];

    int numPointLights;
    int numSpotLights;

    ShadowMapFBO directShadowFBO;
    ShadowMapFBO pointShadowFBO;
    ShadowMapFBO spotShadowFBO;
    
    float ambient;

    GLuint shadowMap;
    mat4 lightSpaceMatrix;
} LightSystem;

typedef void (*ShadowRenderFunc)(Shader* shader, Camera* camera);

LightSystem LightSystem_Init(float ambient);
void LightSystem_Clear(LightSystem* lightSystem);
void LightSystem_SetUniforms(Shader* shader, LightSystem* lightSystem);
void LightSystem_MakeShadowMaps(LightSystem* lightSystem, Shader* lightShader, Camera* camera, ShadowRenderFunc renderFunc);
void LightSystem_Merge(LightSystem* dest, LightSystem* a, LightSystem* b);
void LightSystem_SetDirectLight(LightSystem* lightSystem, vec3 direction, vec4 color, float specular);
void LightSystem_AddPointLight(LightSystem* lightSystem, vec3 position, vec4 color, float a, float b, float specular);
void LightSystem_AddSpotLight(LightSystem* lightSystem, vec3 position, vec3 direction, vec4 color, float innerConeCos, float outerConeCos, float specular);
void LightSystem_Draw(LightSystem* lightSystem, Mesh* mesh, Shader* shader, Camera* camera);

typedef enum {
    ENGINE_EDITOR,
    ENGINE_SCENE_EXPANDED,
    ENGINE_SCENE_HIDDEN
} EngineState;

typedef struct {
    GLFWwindow* window;
    int width, height;
    const char* title;

    // keys
    // mouse
    // joystick

    DeltaTime delta;

    Camera* selectedCamera;
    CameraVector cameras;

    // bool vsync;

} BE_Engine;

#endif