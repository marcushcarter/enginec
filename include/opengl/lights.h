#ifndef LIGHTS_CLASS_H
#define LIGHTS_CLASS_H

#include <cglm/cglm.h>
#include "opengl/mesh.h"
#include "opengl/FBO.h"

#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 10

#define DIRECT_LIGHT_DIST 50

typedef struct {
    vec3 direction;
    vec4 color;
    float specular;

    mat4 lightSpaceMatrix;
} DirectLight;

typedef struct {
    vec3 position;
    vec4 color;
    float a;
    float b;
    float specular;
    
} PointLight;

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

// GLuint Light_CreateShadowMap2D(int width, int height);
// GLuint Light_CreateShadowMapCube(int size);

#endif