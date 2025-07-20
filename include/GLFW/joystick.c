#include "GLFW/joystick.h"

Joystick Joystick_Init(int jid) {
    Joystick js;

    js.id = jid;
    js.present = glfwJoystickPresent(jid);

    if (!js.present) {
        js.axes = NULL;
        js.buttons = NULL;
        js.hats = NULL;
        js.axisCount = 0;
        js.buttonCount = 0;
        js.hatCount = 0;
        
        return js;
    }

    js.name = glfwGetJoystickName(jid);
    js.axes = glfwGetJoystickAxes(jid, &js.axisCount);
    js.buttons = glfwGetJoystickButtons(jid, &js.buttonCount);
    js.hats = glfwGetJoystickHats(jid, &js.hatCount);

    js.deadzone = 0.05f;

    return js;
}

void Joystick_Update(Joystick* js) {
    for (int i = 0; i < 16; i++) {
        js->lbuttons[i] = js->buttons[i];
    }

    if (!glfwJoystickPresent(js->id)) {
        js->present = 0;
        js->axes = NULL;
        js->buttons = NULL;
        js->hats = NULL;
        return;
    }
    
    js->present = 1;
    js->axes = glfwGetJoystickAxes(js->id, &js->axisCount);
    js->buttons = glfwGetJoystickButtons(js->id, &js->buttonCount);
    js->hats = glfwGetJoystickHats(js->id, &js->hatCount);
}

void Joystick_Delete(Joystick* js) {
    // js = {0};
}