#include "opengl/lights.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdarg.h>
#include "opengl/mesh.h"

#include <stdio.h>

char* fmt(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int length = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (length < 0) return NULL;

    char* buffer = malloc(length + 1);
    if (!buffer) return NULL;

    // Now write to buffer
    va_start(args, fmt);
    vsnprintf(buffer, length + 1, fmt, args);
    va_end(args);

    return buffer;
}

LightSystem LightSystem_Init(float ambient) {
    LightSystem lightSystem;
    lightSystem.ambient = ambient;
    lightSystem.numDirectLights = 0;
    lightSystem.numPointLights = 0;
    lightSystem.numSpotLights = 0;
    return lightSystem;
}

void LightSystem_AddDirectLight(LightSystem* lightSystem, vec3 direction, vec4 color, float specular) {
    if (lightSystem->numDirectLights >= MAX_DIRECT_LIGHTS) return;
    
    DirectLight light;
    glm_vec3_copy(direction, light.direction);
    glm_vec4_copy(color, light.color);
    light.specular = specular;
    lightSystem->directlights[lightSystem->numDirectLights++] = light;
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

void LightSystem_AddSpotLight(LightSystem* lightSystem, vec3 position, vec3 direction, vec4 color, float innerCone, float outerCone, float specular) {
    if (lightSystem->numSpotLights >= MAX_SPOT_LIGHTS) return;
    
    SpotLight light;
    glm_vec3_copy(position, light.position);
    glm_vec3_copy(direction, light.direction);
    glm_vec4_copy(color, light.color);
    light.innerCone = innerCone;
    light.outerCone = outerCone;
    light.specular = specular;
    lightSystem->spotlights[lightSystem->numSpotLights++] = light;
}

void LightSystem_SetUniforms(Shader* shader, LightSystem* lightSystem) {
    
    Shader_Activate(shader);

    glUniform1f(glGetUniformLocation(shader->ID, "ambient"), lightSystem->ambient);

    glUniform1i(glGetUniformLocation(shader->ID, "NR_POINT_LIGHTS"), lightSystem->numPointLights);
    glUniform1i(glGetUniformLocation(shader->ID, "NR_SPOT_LIGHTS"), lightSystem->numSpotLights);
    glUniform1i(glGetUniformLocation(shader->ID, "NR_DIRECT_LIGHTS"), lightSystem->numDirectLights);
    
    char uniformName[128];

    for (int i = 0; i < lightSystem->numDirectLights; i++) {
        glUniform3fv(glGetUniformLocation(shader->ID, fmt("directlights[%d].direction", i)), 1, (float*)lightSystem->directlights[i].direction);
        glUniform4fv(glGetUniformLocation(shader->ID, fmt("directlights[%d].color", i)), 1, (float*)lightSystem->directlights[i].color);
        glUniform1f(glGetUniformLocation(shader->ID, fmt("directlights[%d].specular", i)), lightSystem->directlights[i].specular);     
    }

    for (int i = 0; i < lightSystem->numPointLights; i++) {
        glUniform3fv(glGetUniformLocation(shader->ID, fmt("pointlights[%d].position", i)), 1, (float*)lightSystem->pointlights[i].position);
        glUniform4fv(glGetUniformLocation(shader->ID, fmt("pointlights[%d].color", i)), 1, (float*)lightSystem->pointlights[i].color);
        glUniform1f(glGetUniformLocation(shader->ID, fmt("pointlights[%d].a", i)), lightSystem->pointlights[i].a);
        glUniform1f(glGetUniformLocation(shader->ID, fmt("pointlights[%d].b", i)), lightSystem->pointlights[i].b);
        glUniform1f(glGetUniformLocation(shader->ID, fmt("pointlights[%d].specular", i)), lightSystem->pointlights[i].specular);     
    }
    
    for (int i = 0; i < lightSystem->numSpotLights; i++) {
        glUniform3fv(glGetUniformLocation(shader->ID, fmt("spotlights[%d].position", i)), 1, (float*)lightSystem->spotlights[i].position);
        glUniform3fv(glGetUniformLocation(shader->ID, fmt("spotlights[%d].direction", i)), 1, (float*)lightSystem->spotlights[i].direction);
        glUniform4fv(glGetUniformLocation(shader->ID, fmt("spotlights[%d].color", i)), 1, (float*)lightSystem->spotlights[i].color);
        glUniform1f(glGetUniformLocation(shader->ID, fmt("spotlights[%d].innerCone", i)), lightSystem->spotlights[i].innerCone);
        glUniform1f(glGetUniformLocation(shader->ID, fmt("spotlights[%d].outerCone", i)), lightSystem->spotlights[i].outerCone);
        glUniform1f(glGetUniformLocation(shader->ID, fmt("spotlights[%d].specular", i)), lightSystem->spotlights[i].specular);
    }

}

void LightSystem_DrawLights(LightSystem* lightSystem, Mesh* mesh, Shader* shader, Camera* camera) {

    vec3 lightScale = { 0.1f, 0.1f, 0.1f };

    for (int i = 0; i < lightSystem->numPointLights; i++) {
        mat4 lightModel;
        glm_mat4_identity(lightModel);
        glm_translate(lightModel, lightSystem->pointlights[i].position);
        glm_scale(lightModel, lightScale);
        Shader_Activate(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)lightModel);
        glUniform4fv(glGetUniformLocation(shader->ID, "lightColor"), 1, (float*)lightSystem->pointlights[i].color);   
        Mesh_Draw(mesh, shader, camera); 
    }
    
    for (int i = 0; i < lightSystem->numSpotLights; i++) {
        mat4 lightModel;
        glm_mat4_identity(lightModel);
        glm_translate(lightModel, lightSystem->spotlights[i].position);
        glm_scale(lightModel, lightScale);
        Shader_Activate(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)lightModel);
        glUniform4fv(glGetUniformLocation(shader->ID, "lightColor"), 1, (float*)lightSystem->spotlights[i].color);  
        Mesh_Draw(mesh, shader, camera);  
    }
}

void LightSystem_Merge(LightSystem* dest, LightSystem* src1, LightSystem* src2) {
    dest->ambient = src1->ambient;

    // dest->ambient = (src1->ambient + src2->ambient) * 0.5f;
    
    dest->numDirectLights = 0;
    for (int i = 0; i < src1->numDirectLights && dest->numDirectLights < MAX_POINT_LIGHTS; i++) {
        dest->directlights[dest->numDirectLights++] = src1->directlights[i];
    }
    for (int i = 0; i < src2->numDirectLights && dest->numDirectLights < MAX_POINT_LIGHTS; i++) {
        dest->directlights[dest->numDirectLights++] = src2->directlights[i];
    }

    dest->numPointLights = 0;
    for (int i = 0; i < src1->numPointLights && dest->numPointLights < MAX_POINT_LIGHTS; i++) {
        dest->pointlights[dest->numPointLights++] = src1->pointlights[i];
    }
    for (int i = 0; i < src2->numPointLights && dest->numPointLights < MAX_POINT_LIGHTS; i++) {
        dest->pointlights[dest->numPointLights++] = src2->pointlights[i];
    }

    dest->numSpotLights = 0;
    for (int i = 0; i < src1->numSpotLights && dest->numSpotLights < MAX_SPOT_LIGHTS; i++) {
        dest->spotlights[dest->numSpotLights++] = src1->spotlights[i];
    }
    for (int i = 0; i < src2->numSpotLights && dest->numSpotLights < MAX_SPOT_LIGHTS; i++) {
        dest->spotlights[dest->numSpotLights++] = src2->spotlights[i];
    }
}
