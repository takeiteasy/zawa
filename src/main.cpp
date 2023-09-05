#define CWCGL_VERSION 2010
#include "cwcgl.h"
#include "ode/ode.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <stdio.h>

#if !defined(MAX_CONTACTS)
#define MAX_CONTACTS 8
#endif

static struct {
    dWorldID world;
    dSpaceID space;
    dJointGroupID contactGroup;
    dContact contacts[MAX_CONTACTS];
    
    glm::mat4 proj, view;
    glm::vec3 cameraPosition, cameraTarget;
} state;

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

static void collide(void* data, dGeomID o1, dGeomID o2) {
    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);
    
    if (b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact))
        return;
    
    for (int i = 0; i < dCollide(o1, o2, MAX_CONTACTS, &state.contacts[0].geom, sizeof(dContact)); i++) {
        state.contacts[i].surface.mode = dContactBounce;
        state.contacts[i].surface.bounce = 0.5;
        
        dJointID c = dJointCreateContact(state.world, state.contactGroup, &state.contacts[i]);
        dJointAttach(c, b1, b2);
    }
}

int main(int argc, const char *argv[]) {
    if (!glWindow(640, 480, "glWindow", glResizable))
        return 1;
    if (!InitOpenGL())
        return 1;
    
    dInitODE();
    state.world = dWorldCreate();
    state.space = dHashSpaceCreate(0);
    dWorldSetGravity(state.world, 0.0, -9.81, 0.0);
    state.contactGroup = dJointGroupCreate(0);
    
    state.cameraPosition = glm::vec3(0.f, 2.f, 2.2f);
    state.cameraTarget = glm::vec3(0.f, .8f, 0.f);
    state.proj = glm::perspective(glm::radians(45.f), 640.f / 480.f, .1f, 1000.f);
    state.view = glm::lookAt(state.cameraPosition, state.cameraTarget, glm::vec3(0.f, 1.f, 0.f));
    
#define X(NAME, ARGS) on##NAME,
    glWindowCallbacks(CWCGL_CALLBACKS NULL);
#undef X

    while (glPollWindow()) {
        dSpaceCollide(state.space, 0, collide);
        dWorldQuickStep(state.world, 1.f / 60.f); // 60.f
        dJointGroupEmpty(state.contactGroup);
        
        glFlushWindow();
    }
    dWorldDestroy(state.world);
    dSpaceDestroy(state.space);
    dJointGroupDestroy(state.contactGroup);
    glWindowQuit();
    return 0;
}
