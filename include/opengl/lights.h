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
} DirectLight;
;

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
} SpotLight;

typedef struct {
    DirectLight directlights[MAX_DIRECT_LIGHTS];
    PointLight pointlights[MAX_POINT_LIGHTS];
    SpotLight spotlights[MAX_SPOT_LIGHTS];
    int numDirectLights;
    int numPointLights;
    int numSpotLights;
    float ambient;
} LightSystem;

char* fmt(const char* fmt, ...);

LightSystem LightSystem_Init(float ambient);
void LightSystem_SetUniforms(Shader* shader, LightSystem* lightSystem);
void LightSystem_Merge(LightSystem* dest, LightSystem* src1, LightSystem* src2);

void LightSystem_AddDirectLight(LightSystem* lightSystem, vec3 direction, vec4 color, float specular);
void LightSystem_AddPointLight(LightSystem* lightSystem, vec3 position, vec4 color, float a, float b, float specular);
void LightSystem_AddSpotLight(LightSystem* lightSystem, vec3 position, vec3 direction, vec4 color, float innerCone, float outerCone, float specular);
void LightSystem_DrawLights(LightSystem* lightSystem, Mesh* mesh, Shader* shader, Camera* camera);

#endif