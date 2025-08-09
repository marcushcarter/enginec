#ifndef DELTA_H
#define DELTA_H

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

#endif