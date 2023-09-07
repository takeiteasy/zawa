#define CWCGL_VERSION 3020
#include "cwcgl.h"
#include "ode/ode.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "dice.obj.h"
#include "hand.obj.h"
#include "plane.obj.h"
#include "hand.png.h"
#include "default.vert.glsl.h"
#include "dice.frag.glsl.h"
#include "hand.frag.glsl.h"
#include "plane.frag.glsl.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

#if !defined(MAX_CONTACTS)
#define MAX_CONTACTS 8
#endif

#if !defined(TIMESTEP)
#define TIMESTEP (1.0/60.0)
#endif

#define ASSETS \
    X(dice)    \
    X(hand)    \
    X(plane)

typedef struct {
    int size;
    GLuint id;
} Model;

typedef struct {
    dGeomID geom;
    dBodyID body;
} RigidBody;

typedef struct {
    glm::vec3 specular;
    float shininess;
} Material;

typedef struct {
    RigidBody rigidBody;
    Model *model;
    glm::mat4 world;
} GameObject;

typedef struct {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;
} Light;

typedef struct Die {
    GameObject go;
    glm::vec3 color;
    Material material;
    int currentNumber;
    struct Die *next;
}  Die;

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
    
    glm::vec3 planeColor;
    Light spotlight;
    struct {
        Die *front, *back;
    } dice;
} state;

// Window event callbacks, I think the names are self-explanatory
void onKeyboard(void *userdata, int key, int modifier, int isDown) {
    if (key == KEY_SPACE && !isDown) {
        Die *die = (Die*)malloc(sizeof(Die));
        die->color = glm::vec3(1.f, 0.f, 0.f);
        die->currentNumber = 0;
        die->material.shininess = 32.f;
        die->material.specular = glm::vec3(.5f, .5f, .5f);
        die->next = NULL;
        
        die->go.rigidBody.body = dBodyCreate(state.world);
        dMass mass;
        dMassSetBox(&mass, 1, 1, 1, 1);
        dBodySetMass(die->go.rigidBody.body, &mass);
        dBodySetPosition(die->go.rigidBody.body, 0.f, 0.f, 0.f);
        die->go.rigidBody.geom = dCreateBox(state.space, .2f, .2f, .2f);
        dGeomSetBody(die->go.rigidBody.geom, die->go.rigidBody.body);
        die->go.model = &state.diceModel;
        die->go.world = glm::mat4(1.f);
        
        if (!state.dice.front)
            state.dice.front = state.dice.back = die;
        else {
            die->next = state.dice.front;
            state.dice.front = die;
        }
    }
}

static void onMouseButton(void *userdata, int button, int modifier, int isDown) {}
static void onMouseMove(void *userdata, int x, int y, float dx, float dy) {}
static void onMouseScroll(void *userdata, float dx, float dy, int modifier) {}
static void onFocus(void *userdata, int isFocused) {}
static void onResized(void *userdata, int w, int h) {}
static void onClosed(void *userdata) {}

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

static void PushLight(GLuint shader, Light *light) {
    glUniform3f(glGetUniformLocation(shader, "light.position"), light->position.x, light->position.y, light->position.z);
    glUniform3f(glGetUniformLocation(shader, "light.direction"), light->direction.x, light->direction.y, light->direction.z);
    glUniform3f(glGetUniformLocation(shader, "light.ambient"), light->ambient.x, light->ambient.y, light->ambient.z);
    glUniform3f(glGetUniformLocation(shader, "light.diffuse"), light->diffuse.x, light->diffuse.y, light->diffuse.z);
    glUniform3f(glGetUniformLocation(shader, "light.specular"), light->specular.x, light->specular.y, light->specular.z);
    glUniform1f(glGetUniformLocation(shader, "light.cutOff"), light->cutOff);
    glUniform1f(glGetUniformLocation(shader, "light.outerCutOff"), light->outerCutOff);
    glUniform1f(glGetUniformLocation(shader, "light.constant"), light->constant);
    glUniform1f(glGetUniformLocation(shader, "light.linear"), light->linear);
    glUniform1f(glGetUniformLocation(shader, "light.quadratic"), light->quadratic);
}

static void PushMaterial(GLuint shader, Material *material) {
    glUniform1i(glGetUniformLocation(shader, "material.diffuse"), 0);
    glUniform3f(glGetUniformLocation(shader, "material.specular"), material->specular.x, material->specular.y, material->specular.z);
    glUniform1f(glGetUniformLocation(shader, "material.shininess"), material->shininess);
}

static void RenderGameObject(GLuint shader, GameObject *obj) {
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &obj->world[0][0]);
    glBindVertexArray(obj->model->id);
    glDrawArrays(GL_TRIANGLES, 0, obj->model->size);
    glBindVertexArray(0);
}

int main(int argc, const char *argv[]) {
    if (!glWindow(640, 480, "glWindow", glResizable))
        return 1;
#define X(NAME, ARGS) on##NAME,
    glWindowCallbacks(CWCGL_CALLBACKS NULL);
#undef X
    
    if (!InitOpenGL())
        return 1;
    
//    glViewport(0, 0, 640, 480);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    
#define X(NAME)                                                                                                   \
do {                                                                                                              \
    glGenVertexArrays(1, &state.NAME##Model.id);                                                                  \
    glBindVertexArray(state.NAME##Model.id);                                                                      \
    GLuint vbo = 0;                                                                                               \
    glGenBuffers(1, &vbo);                                                                                        \
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
    state.NAME##Model.size = obj_##NAME##_face_count;                                                             \
} while(0);
    ASSETS
#undef X
    
    dInitODE();
    state.world = dWorldCreate();
    state.space = dHashSpaceCreate(0);
    dWorldSetGravity(state.world, 0.0, -9.81, 0.0);
    state.contactGroup = dJointGroupCreate(0);
    
    state.cameraPosition = glm::vec3(0.f, 2.f, 4.f);
    state.cameraTarget = glm::vec3(0.f, .8f, 0.f);
    state.proj = glm::perspective(glm::radians(45.f), 640.f / 480.f, .1f, 1000.f);
    state.view = glm::lookAt(state.cameraPosition, state.cameraTarget, glm::vec3(0.f, 1.f, 0.f));
    
    GLuint vertex = MakeShader(GL_VERTEX_SHADER, shdr_default_vert_data);
    assert(vertex != -1);
#define X(NAME)                                                            \
do {                                                                       \
    GLuint frag = MakeShader(GL_FRAGMENT_SHADER, shdr_##NAME##_frag_data); \
    assert(frag != -1);                                                    \
    state.NAME##Shader = MakeProgram(vertex, frag);                        \
    assert(state.NAME##Shader != -1);                                      \
    glDeleteShader(frag);                                                  \
} while(0);
    ASSETS
#undef X
    glDeleteShader(vertex);
    state.handTexture = MakeTexture(img_hand_data, img_hand_width, img_hand_height);
    
    state.planeColor            = glm::vec3(0.f, 0.f, 0.f);
    state.spotlight.position    = glm::vec3(0.f, 7.f, 0.f);
    state.spotlight.direction   = glm::vec3(0.f, -1.f, 0.f);
    state.spotlight.cutOff      = cosf(glm::radians(12.5f));
    state.spotlight.outerCutOff = cosf(glm::radians(17.5f));
    state.spotlight.ambient     = glm::vec3(.5f, .5f, .5f);
    state.spotlight.diffuse     = glm::vec3(1.f, 1.f, 1.f);
    state.spotlight.specular    = glm::vec3(1.f, 1.f, 1.f);
    state.spotlight.constant    = 1.f;
    state.spotlight.linear      = .09f;
    state.spotlight.quadratic   = .032f;
    
    GameObject floor;
    floor.rigidBody.geom = dCreatePlane(state.space, 0, 1, 0, 0);
    floor.rigidBody.body = NULL;
    floor.world = glm::mat4(1.f);
    floor.model = &state.planeModel;
      
    while (glPollWindow()) {
        dSpaceCollide(state.space, 0, collide);
        dWorldQuickStep(state.world, TIMESTEP);
        dJointGroupEmpty(state.contactGroup);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(state.planeShader);
        glUniformMatrix4fv(glGetUniformLocation(state.planeShader, "projection"), 1, GL_FALSE, &state.proj[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(state.planeShader, "view"), 1, GL_FALSE, &state.view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(state.planeShader, "model"), 1, GL_FALSE, &floor.world[0][0]);
        glUniform3f(glGetUniformLocation(state.planeShader, "viewPos"), state.cameraPosition.x, state.cameraPosition.y, state.cameraPosition.z);
        glUniform3f(glGetUniformLocation(state.planeShader, "planeColor"), state.planeColor.x, state.planeColor.y, state.planeColor.z);
        PushLight(state.planeShader, &state.spotlight);
        RenderGameObject(state.planeShader, &floor);
        
        glUseProgram(state.diceShader);
        glUniformMatrix4fv(glGetUniformLocation(state.diceShader, "projection"), 1, GL_FALSE, &state.proj[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(state.diceShader, "view"), 1, GL_FALSE, &state.view[0][0]);
        glUniform3f(glGetUniformLocation(state.diceShader, "viewPos"), state.cameraPosition.x, state.cameraPosition.y, state.cameraPosition.z);
        PushLight(state.diceShader, &state.spotlight);
        
        Die *die = state.dice.front;
        while (die) {
            static const dReal *position, *rotation;
            position = dBodyGetPosition(die->go.rigidBody.body);
            rotation = dBodyGetRotation(die->go.rigidBody.body);
            PushMaterial(state.diceShader, &die->material);
            glUniform3f(glGetUniformLocation(state.diceShader, "dieColor"), die->color.x, die->color.y, die->color.z);
            glUniformMatrix4fv(glGetUniformLocation(state.diceShader, "model"), 1, GL_FALSE, &die->go.world[0][0]);
            RenderGameObject(state.diceShader, &die->go);
            die = die->next;
        }
        
        glUseProgram(0);
        glFlushWindow();
    }
    
    Die *die = state.dice.front;
    while (die) {
        Die *tmp = die->next;
        free(die);
        die = tmp;
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
