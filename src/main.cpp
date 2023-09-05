#define CWCGL_VERSION 2010
#include "cwcgl.h"
#include "ode/ode.h"
#include <stdio.h>

// Window event callbacks, I think the names are self-explanatory
void onKeyboard(void *userdata, int key, int modifier, int isDown) {
    printf("Keyboard Event: Key %d is now %s\n", (int)key, isDown ? "down" : "up");
}

void onMouseButton(void *userdata, int button, int modifier, int isDown) {
    printf("Mouse Button Event: Button %d is now %s\n", button, isDown ? "down" : "up");
}

void onMouseMove(void *userdata, int x, int y, float dx, float dy) {
    printf("Mouse Move Event: Position (%d, %d) by (%f, %f)\n", x, y, dx, dy);
}

void onMouseScroll(void *userdata, float dx, float dy, int modifier) {
    printf("Mouse Scroll Event: Scroll delta (%f, %f)\n", dx, dy);
}

void onFocus(void *userdata, int isFocused) {
    printf("Focus Event: Window is now %s\n", isFocused ? "focused" : "unfocused");
}

void onResized(void *userdata, int w, int h) {
    printf("Resize Event: Window is now (%d, %d)\n", w, h);
}

void onClosed(void *userdata) {
    printf("Close Event: Window is now closing\n");
}

int main(int argc, const char *argv[]) {
    if (!glWindow(640, 480, "glWindow", glResizable))
        return 1;
    if (!InitOpenGL())
        return 1;
    dInitODE();
    
#define X(NAME, ARGS) on##NAME,
    glWindowCallbacks(CWCGL_CALLBACKS NULL);
#undef X

    while (glPollWindow()) {
        glFlushWindow();
    }
    
    glWindowQuit();
    return 0;
}
