#ifndef LIGHTS_CLASS_H
#define LIGHTS_CLASS_H

#include <cglm/cglm.h>
// #include "opengl/shader.h"
#include "opengl/mesh.h"

#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 4
#define MAX_DIRECT_LIGHTS 2

typedef struct {
    vec3 direction;
    vec4 color;
    float specular;

    GLuint shadowMap;
    mat4 lightSpaceMatrix;
} DirectLight;

typedef struct {
    vec3 position;
    vec4 color;
    float a;
    float b;
    float specular;

    GLuint shadowCubeMap;
    float farPlane;
} PointLight;

typedef struct {
    vec3 position;
    vec3 direction;
    vec4 color;

    float innerCone;
    float outerCone;
    float specular;
} SpotLight;

typedef struct {
    DirectLight directlight;
    PointLight pointlights[MAX_POINT_LIGHTS];
    SpotLight spotlights[MAX_SPOT_LIGHTS];
    int numPointLights;
    int numSpotLights;
    float ambient;

    // GLuint shadowMap;
    // mat4 lightSpaceMatrix;
} LightSystem;

char* fmt(const char* fmt, ...);

LightSystem LightSystem_Init(float ambient);
void LightSystem_SetUniforms(Shader* shader, LightSystem* lightSystem);
void LightSystem_Merge(LightSystem* dest, LightSystem* a, LightSystem* b);

void LightSystem_SetDirectLight(LightSystem* lightSystem, vec3 direction, vec4 color, float specular);
void LightSystem_AddPointLight(LightSystem* lightSystem, vec3 position, vec4 color, float a, float b, float specular);
void LightSystem_AddSpotLight(LightSystem* lightSystem, vec3 position, vec3 direction, vec4 color, float innerCone, float outerCone, float specular);
void LightSystem_DrawLights(LightSystem* lightSystem, Mesh* mesh, Shader* shader, Camera* camera);

// GLuint Light_CreateShadowMap2D(int width, int height);
// GLuint Light_CreateShadowMapCube(int size);

#endif