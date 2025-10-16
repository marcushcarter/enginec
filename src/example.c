#include "engine/engine.h"

#include <stdio.h>
#include <math.h>
#include <time.h>

// #define set(emitter) glm_vec3_copy((vec3){0,1,0}, emitter->position)

int main() {

    BE_Engine engine = BE_StartEngine(NULL, 1440, 900);
    BE_Engine* p_engine = &engine;
    BE_BindEngine(&engine);
    
    BE_LoadMesh("scene", "res/models/scene.obj");
    BE_AddModel("model1", "scene");

    BE_AddLight("sun", BE_LIGHT_DIRECT);
    BE_AddLight("rainbow light", BE_LIGHT_POINT);

    BE_LoadSound("music1", "res/sounds/breakout.wav");
    BE_AddEmitter("speaker1", true);
    // BE_SetEmitterPosition("speaker1", BE_vec3(0,0.2f,0));
    BE_SetEmitterPosition("speaker1", BE_vec3(0,0.2f,0));
    BE_SetListenerPosition(BE_vec3(0,2,0), BE_vec3(0,0,0), BE_vec3(0,0,0));
    BE_SetEmitterRolloff("speaker1", 0.0f, 100.0f);
    BE_PlayEmitter("speaker1", "music1");

    fprintf(stdout, "Time to load scene -> %.2fs\n", glfwGetTime());

    while(BE_WindowIsOpen()) {

        BE_BeginFrame();
        // BE_CameraInputs(g_engine->activeScene->activeCamera, g_engine->window, g_engine->timer.dt);

        if (g_engine->timer.frameCountFPS == 1) printf("%f FPS %f MS\n", g_engine->timer.fps, g_engine->timer.ms);

        BE_Light* rainlight = BE_FindLight("rainbow light");
        glm_vec4_copy(BE_vec4(sinf(glfwGetTime()*0.5f) * 0.5f + 0.5f, sinf(glfwGetTime()*0.5f + 2.0943951f) * 0.5f + 0.5f, sinf(glfwGetTime()*0.5f + 4.1887902f) * 0.5f + 0.5f, 1.0f), rainlight->color);
        glm_vec3_copy(BE_vec3(sin(glfwGetTime()), 0.5, cos(glfwGetTime())), rainlight->position);
        
        BE_Light* sunlight = BE_FindLight("sun");
        glm_vec3_copy(BE_vec3(cosf(glfwGetTime()/25), -0.4f, sinf(glfwGetTime()/25)), sunlight->direction);

        // vec3 vec;
        // BE_GetEmitterPosition("speaker1", vec);
        // printf("%f %f %f\n", vec[0], vec[1], vec[2]);
        
        // BE_IMPL_SetEmitterVolume("speaker1", -1.6f, "NULL", 0);

        BE_MakeShadows(true);
        BE_BeginRender();
        
        BE_DrawModels(NULL);
        BE_DrawLights(NULL);
        BE_DrawCameras(NULL);
        BE_DrawSprites(NULL);
        BE_DrawEmitters(NULL);

        BE_EndFrame();

    }

    BE_ShutdownEngine(&engine);
    return 0;
}