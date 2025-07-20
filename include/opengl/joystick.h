#ifndef JOYSTICK_CLASS_H
#define JOYSTICK_CLASS_H

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef struct {

    int id;
    const char *name;
    bool present;

    /**
     * 0 -> LS X
     * 1 -> LS Y
     * 2 -> RS x
     * 3 -> RS Y
     * 4 -> LT
     * 5 -> RT
     */
    const float *axes;
    int axisCount;

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
    const unsigned char *buttons;
    bool lbuttons[16];
    int buttonCount;

    /**
     * 0 ->
     */
    const unsigned char* hats;
    int hatCount;

    float deadzone;

} Joystick;

Joystick Joystick_Init(int jid);
void Joystick_Update(Joystick* js);
// void Joystick_Delete(Joystick* js);

#endif