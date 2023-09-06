#define CWCGL_VERSION 3020
#include "cwcgl.h"
#include "ode/ode.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "dice.obj.h"
#include "hand.obj.h"
#include "plane.obj.h"
#include "hand.png.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

#if !defined(MAX_CONTACTS)
#define MAX_CONTACTS 8
#endif

#define ASSETS \
    X(dice)    \
    X(hand)    \
    X(plane)

typedef struct {
    int size;
    GLuint id;
} Model;

static struct {
    dWorldID world;
    dSpaceID space;
    dJointGroupID contactGroup;
    dContact contacts[MAX_CONTACTS];
    
    glm::mat4 proj, view;
    glm::vec3 cameraPosition, cameraTarget;
    
#define X(NAME)        \
    Model NAME##Model; \
    GLuint NAME##Shader;
    ASSETS
#undef X
    GLuint handTexture;
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

#define RenderModel(NAME)                                  \
do {                                                       \
    glBindVertexArray(state.NAME##Model.id);               \
    glDrawArrays(GL_TRIANGLES, 0, state.NAME##Model.size); \
    glBindVertexArray(0);                                  \
} while(0);

static char* ReadFile(const char* path, size_t *size) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "fopen \"%s\" failed: %d %s\n", path, errno, strerror(errno));
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    rewind(file);
    
    char *data = (char*)calloc(length + 1, sizeof(char));
    fread(data, 1, length, file);
    fclose(file);
    
    if (size)
        *size = length;
    return data;
}

static int CheckShader(GLuint shader, GLenum pname, void(*func)(GLuint, GLenum, GLint*)) {
    GLint status;
    func(shader, pname, &status);
    if (status == GL_TRUE)
        return 1;
    GLint length;
    func(shader, GL_INFO_LOG_LENGTH, &length);
    GLchar *info = (GLchar*)calloc(length, sizeof(GLchar));
    glGetShaderInfoLog(shader, length, NULL, info);
    fprintf(stderr, "glCompileShader failed:\n%s\n", info);
    free(info);
    return 0;
}

static GLuint MakeShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    return CheckShader(shader, GL_COMPILE_STATUS, glGetShaderiv) ? shader : -1;
}

static GLuint MakeProgram(GLuint vert, GLuint frag) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    glDetachShader(program, vert);
    glDetachShader(program, frag);
    return CheckShader(program, GL_LINK_STATUS, glGetProgramiv) ? program : -1;
}

static GLuint MakeTexture(const unsigned char *data, int width, int height) {
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    return id;
}

int main(int argc, const char *argv[]) {
    if (!glWindow(640, 480, "glWindow", glResizable))
        return 1;
#define X(NAME, ARGS) on##NAME,
    glWindowCallbacks(CWCGL_CALLBACKS NULL);
#undef X
    
    if (!InitOpenGL())
        return 1;
    
#define X(NAME)                                                                                                   \
do {                                                                                                              \
    glGenVertexArrays(1, &state.NAME##Model.id);                                                                  \
    GLuint vbo = 0;                                                                                               \
    glBindVertexArray(state.NAME##Model.id);                                                                      \
    glBindBuffer(GL_ARRAY_BUFFER, vbo);                                                                           \
    glBufferData(GL_ARRAY_BUFFER, obj_##NAME##_data_size * sizeof(float), &obj_##NAME##_data[0], GL_STATIC_DRAW); \
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);                                        \
    glEnableVertexAttribArray(0);                                                                                 \
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3 * sizeof(GLfloat)));           \
    glEnableVertexAttribArray(1);                                                                                 \
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(6 * sizeof(GLfloat)));           \
    glEnableVertexAttribArray(2);                                                                                 \
    glBindVertexArray(0);                                                                                         \
    glBindBuffer(GL_ARRAY_BUFFER, 0);                                                                             \
    state.NAME##Model.size = obj_##NAME##_data_size;                                                              \
} while(0);
    ASSETS
#undef X
    
    dInitODE();
    state.world = dWorldCreate();
    state.space = dHashSpaceCreate(0);
    dWorldSetGravity(state.world, 0.0, -9.81, 0.0);
    state.contactGroup = dJointGroupCreate(0);
    
    state.cameraPosition = glm::vec3(0.f, 2.f, 2.2f);
    state.cameraTarget = glm::vec3(0.f, .8f, 0.f);
    state.proj = glm::perspective(glm::radians(45.f), 640.f / 480.f, .1f, 1000.f);
    state.view = glm::lookAt(state.cameraPosition, state.cameraTarget, glm::vec3(0.f, 1.f, 0.f));
    
    // TODO: Build shaders into source later
    char *vertexSource = ReadFile("assets/default.vert.glsl", NULL);
    GLuint vertex = MakeShader(GL_VERTEX_SHADER, vertexSource);
    free(vertexSource);
#define X(NAME)                                                      \
do {                                                                 \
    char *fragSource = ReadFile("assets/" #NAME ".frag.glsl", NULL); \
    GLuint frag = MakeShader(GL_FRAGMENT_SHADER, fragSource);        \
    free(fragSource);                                                \
    state.NAME##Shader = MakeProgram(vertex, frag);                  \
    glDeleteShader(frag);                                            \
} while(0);
    ASSETS
#undef X
    glDeleteShader(vertex);
    state.handTexture = MakeTexture(img_hand_data, img_hand_width, img_hand_height);

    while (glPollWindow()) {
        dSpaceCollide(state.space, 0, collide);
        dWorldQuickStep(state.world, 1.f / 60.f);
        dJointGroupEmpty(state.contactGroup);
        
        glFlushWindow();
    }
    
#define X(NAME)                      \
glDeleteProgram(state.NAME##Shader); \
glDeleteVertexArrays(1, &state.NAME##Model.id);
    ASSETS
#undef X
    glDeleteTextures(1, &state.handTexture);
    dWorldDestroy(state.world);
    dSpaceDestroy(state.space);
    dJointGroupDestroy(state.contactGroup);
    glWindowQuit();
    return 0;
}
