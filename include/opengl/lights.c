#include "lights.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdarg.h>
#include "mesh.h"

#include <stdio.h>

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

void LightSystem_DrawLights(LightSystem* lightSystem, Mesh* mesh, Shader* shader, Camera* camera) {
    
    Shader_Activate(shader);

    vec3 lightScale = { 1.0f, 1.0f, 1.0f };

    mat4 lightModel;
    glm_mat4_identity(lightModel);
    vec3 lightPos;
    glm_vec3_scale(lightSystem->directlight.direction, -DIRECT_LIGHT_DIST, lightPos);
    glm_translate(lightModel, lightPos);
    glm_scale(lightModel, lightScale);

    Shader_Activate(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)lightModel);
    glUniform4fv(glGetUniformLocation(shader->ID, "lightColor"), 1, (float*)lightSystem->directlight.color);   
    Mesh_Draw(mesh, shader, camera);

    glm_vec3_copy((vec3){ 0.1f, 0.1f, 0.1f }, lightScale);

    for (int i = 0; i < lightSystem->numPointLights; i++) {
        mat4 lightModel;
        glm_mat4_identity(lightModel);
        glm_translate(lightModel, lightSystem->pointlights[i].position);
        glm_scale(lightModel, lightScale);
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, (float*)lightModel);
        glUniform4fv(glGetUniformLocation(shader->ID, "lightColor"), 1, (float*)lightSystem->pointlights[i].color);   
        Mesh_Draw(mesh, shader, camera); 
    }
    
    for (int i = 0; i < lightSystem->numSpotLights; i++) {
        mat4 lightModel;
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
