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
    lightSystem.numPointLights = 0;
    lightSystem.numSpotLights = 0;
    
    lightSystem.directShadowFBO = ShadowMapFBO_Init(1024*4, 1024*4, 1);

    // for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
    //     lightSystem.pointlights[i].shadowFBO = ShadowMapFBO_Init(1024*4, 1024*4);
    // }
    
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++) {
    // for (int i = 0; i < 5; i++) {
        // lightSystem.spotlights[i].shadowFBO = ShadowMapFBO_Init(1024*4, 1024*4, 1);
    }

    return lightSystem;
}

void LightSystem_Clear(LightSystem* lightSystem) {
    lightSystem->numPointLights = 0;
    lightSystem->numSpotLights = 0;

    for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
        glm_vec3_zero(lightSystem->pointlights[i].position);
        glm_vec4_zero(lightSystem->pointlights[i].color);
        lightSystem->pointlights[i].a = 0.0f;
        lightSystem->pointlights[i].b = 0.0f;
        lightSystem->pointlights[i].specular = 0.0f;
    }

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
    glm_vec3_scale(direction, -30.0f, lightPos);
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

void LightSystem_AddSpotLight(LightSystem* lightSystem, vec3 position, vec3 direction, vec4 color, float innerCone, float outerCone, float specular) {
    if (lightSystem->numSpotLights >= MAX_SPOT_LIGHTS) return;
    
    SpotLight light;
    glm_vec3_copy(position, light.position);

    // normalize direction before copying and using it
    vec3 normDir;
    glm_normalize_to(direction, normDir);
    glm_vec3_copy(normDir, light.direction);

    glm_vec4_copy(color, light.color);
    light.innerCone = innerCone;
    light.outerCone = outerCone;
    light.specular = specular;

    // perspective matrix for spotlight
    mat4 perspectiveProjection;
    glm_perspective(glm_rad(90.0f), 1.0f, 0.1f, 100.0f, perspectiveProjection);

    // calculate target = position + normalized direction
    vec3 target;
    glm_vec3_add(position, normDir, target);

    // up vector is usually (0,1,0)
    mat4 lightView;
    glm_lookat(position, target, (vec3){0.0f, 1.0f, 0.0f}, lightView);

    // final light space matrix = projection * view
    glm_mat4_mul(perspectiveProjection, lightView, light.lightSpaceMatrix);

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
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "directlight.lightProjection"), 1, GL_FALSE, (float*)lightSystem->directlight.lightSpaceMatrix);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D_ARRAY, lightSystem->directShadowFBO.depthTextureArray);
    glUniform1i(glGetUniformLocation(shader->ID, "directShadowMapArray"), 3);

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

void LightSystem_MakeShadowMaps(LightSystem* lightSystem, Shader* lightShader, Camera* camera, ShadowRenderFunc renderFunc) {
    
    Shader_Activate(lightShader);

    glEnable(GL_DEPTH_TEST);
    ShadowMapFBO_BindLayer(&lightSystem->directShadowFBO, 0);
    glViewport(0, 0, lightSystem->directShadowFBO.width, lightSystem->directShadowFBO.height);
    glClear(GL_DEPTH_BUFFER_BIT);
    glUniformMatrix4fv(glGetUniformLocation(lightShader->ID, "lightProjection"), 1, GL_FALSE, (float*)lightSystem->directlight.lightSpaceMatrix);
    renderFunc(lightShader, camera);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LightSystem_DrawLights(LightSystem* lightSystem, Mesh* mesh, Shader* shader, Camera* camera) {

    vec3 lightScale = { 1.0f, 1.0f, 1.0f };

    // for (int i = 0; i < lightSystem->numDirectLights; i++) {
    //     mat4 lightModel;
    //     glm_mat4_identity(lightModel);
    //     vec3 lightPos;
    //     glm_vec3_mul(lightSystem->directlights[i].direction, (vec3){ 50.0f, 50.0f, 50.0f}, lightPos);
    //     glm_vec3_sub(camera->Position, lightPos, lightPos);
    //     glm_translate(lightModel, lightPos);
    //     glm_scale(lightModel, lightScale);
    //     Shader_Activate(shader);
    //     glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)lightModel);
    //     glUniform4fv(glGetUniformLocation(shader->ID, "lightColor"), 1, (float*)lightSystem->directlights[i].color);   
    //     Mesh_Draw(mesh, shader, camera); 
    // }

    glm_vec3_copy((vec3){ 0.1f, 0.1f, 0.1f }, lightScale);
    
    // lightScale = (vec3){ 0.1f, 0.1f, 0.1f };

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
