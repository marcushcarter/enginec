#ifndef JOYSTICK_CLASS_H
#define JOYSTICK_CLASS_H

#include <GLFW/glfw3.h>

#define MAX_CONTROLLERS 4
#define MAX_JOYSTICKS 16

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

void glfwJoystickEvents(void);


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
int joystickIsPressed(Joystick* js, int button);
/**
 * 
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
int joystickIsReleased(Joystick* js, int button);
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
int joystickIsHeld(Joystick* js, int button);

/**
 * 0 -> LSX
 * 1 -> LSY
 * 2 -> RSx
 * 3 -> RSY
 * 4 -> LT
 * 5 -> RT
 */
float joystickGetAxis(Joystick* js, int axis);

#endif