#include "joystick.h"
#include <string.h>
#include <stdio.h>

Joystick joysticks[MAX_JOYSTICKS];
int joystickCount = MAX_JOYSTICKS;

void glfwJoystickEvents(void) {
    
    for (int i = 0; i < joystickCount; i++) {
        Joystick* js = &joysticks[i];
        if (js->present && js->buttons) {
            for (int b = 0; b < js->buttonCount && b < 16; b++) {
                js->lbuttons[b] = js->buttons[b];
            }
        }
    }

    for (int jid = 0; jid <= GLFW_JOYSTICK_LAST; jid++) {
        if (!glfwJoystickPresent(jid)) continue;

        const unsigned char* buttons;
        int buttonCount;
        buttons = glfwGetJoystickButtons(jid, &buttonCount);
        if (!buttons || buttonCount <= 7 /*|| !buttons[7]*/) continue;

        int alreadyAssigned = 0;
        for (int i = 0; i < joystickCount; i++) {
            if (joysticks[i].present && joysticks[i].id == jid) {
                alreadyAssigned = 1;
                break;
            }
        }

        if (!alreadyAssigned) {
            for (int i = 0; i < joystickCount; i++) {
                if (!joysticks[i].present) {
                    Joystick* js = &joysticks[i];

                    js->id = jid;
                    js->present = 1;
                    js->name = glfwGetJoystickName(jid);
                    js->axes = glfwGetJoystickAxes(jid, &js->axisCount);
                    js->buttons = buttons;
                    js->buttonCount = buttonCount;
                    js->hats = glfwGetJoystickHats(jid, &js->hatCount);
                    js->deadzone = 0.05f;

                    memset(js->lbuttons, 0, buttonCount);

                    printf("Player %d controller connected\n", js->id+1);

                    break;
                }
            }
        }
    }

    for (int i = 0; i < joystickCount; i++) {
        Joystick* js = &joysticks[i];
        if (!js->present) continue;

        if (!glfwJoystickPresent(js->id)) {
            printf("Player %d controller disonnected\n", js->id+1);
            *js = (Joystick){0}; // reset the struct
            continue;
        }

        js->axes = glfwGetJoystickAxes(js->id, &js->axisCount);
        js->buttons = glfwGetJoystickButtons(js->id, &js->buttonCount);
        js->hats = glfwGetJoystickHats(js->id, &js->hatCount);
    }
}

int joystickIsPressed(Joystick* js, int button) {
    return js->buttons && js->buttons[button] && !js->lbuttons[button];
}

int joystickIsReleased(Joystick* js, int button) {
    return js->buttons && !js->buttons[button] && js->lbuttons[button];
}

int joystickIsHeld(Joystick* js, int button) {
    return js->buttons && js->buttons[button];
}

float joystickGetAxis(Joystick* js, int axis) {
    return js->axes && js->axes[axis];
}