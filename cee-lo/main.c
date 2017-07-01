//
//  main.c
//  cee-lo
//
//  Created by George Watson on 25/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#include <stdio.h>

#include "3rdparty/glad.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "camera.h"
#include "vector.h"
#include "game_obj.h"
#include "Icosphere_obj.h"

#define RES(X) "/Users/rusty/git/cee-lo/res/" #X

static const int SCREEN_WIDTH = 640, SCREEN_HEIGHT = 480;

static SDL_Window* window;
static SDL_GLContext context;
static dWorldID world;
static dSpaceID space;
static dJointGroupID contact_group;
#define MAX_CONTACTS 8
static dContact contact[MAX_CONTACTS];
static vector_t dice;

#undef GLAD_DEBUG

#ifdef GLAD_DEBUG
void pre_gl_call(const char *name, void *funcptr, int len_args, ...) {
  printf("Calling: %s (%d arguments)\n", name, len_args);
}

char* glGetError_str(GLenum err) {
  switch (err) {
    case GL_INVALID_ENUM:                  return "INVALID_ENUM"; break;
    case GL_INVALID_VALUE:                 return "INVALID_VALUE"; break;
    case GL_INVALID_OPERATION:             return "INVALID_OPERATION"; break;
    case GL_STACK_OVERFLOW:                return "STACK_OVERFLOW"; break;
    case GL_STACK_UNDERFLOW:               return "STACK_UNDERFLOW"; break;
    case GL_OUT_OF_MEMORY:                 return "OUT_OF_MEMORY"; break;
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "INVALID_FRAMEBUFFER_OPERATION"; break;
    default:
      return "Unknown Error";
  }
}

void post_gl_call(const char *name, void *funcptr, int len_args, ...) {
  GLenum err = glad_glGetError();
  if (err != GL_NO_ERROR) {
    fprintf(stderr, "ERROR %d (%s) in %s\n", err, glGetError_str(err), name);
    abort();
  }
}
#endif

void cleanup() {
  SDL_DestroyWindow(window);
  SDL_GL_DeleteContext(context);
  dWorldDestroy(world);
  dSpaceDestroy(space);
  dJointGroupDestroy(contact_group);
  printf("Goodbye!\n");
}

void collide(void* data, dGeomID o1, dGeomID o2) {
  dBodyID b1 = dGeomGetBody(o1),
          b2 = dGeomGetBody(o2);
  
  if (b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact))
    return;
  
  int numc = dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sizeof(dContact));
  if (numc) {
    for (int i = 0; i < numc; i++) {
      contact[i].surface.mode   = dContactBounce;
      contact[i].surface.mu     = dInfinity;
      contact[i].surface.bounce = 0.2;
      
      dJointID c = dJointCreateContact(world, contact_group, &contact[i]);
      dJointAttach (c, b1, b2);
    }
  }
}

int main(int argc, const char * argv[]) {
  srand((unsigned int)time(NULL));
  
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Failed to initalize SDL!\n");
    return -1;
  }
  
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  
  window = SDL_CreateWindow("im not gay",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            SCREEN_WIDTH, SCREEN_HEIGHT,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
  if (!window) {
    fprintf(stderr, "Failed to create SDL window!\n");
    return -1;
  }
  
  context = SDL_GL_CreateContext(window);
  if (!context) {
    fprintf(stderr, "Failed to create OpenGL context!\n");
    return -1;
  }
  
  if (!gladLoadGL()) {
    fprintf(stderr, "Failed to load GLAD!\n");
    return -1;
  }
  
#ifdef GLAD_DEBUG
  glad_set_pre_callback(pre_gl_call);
  glad_set_post_callback(post_gl_call);
#endif
  
  printf("Vendor:   %s\n", glGetString(GL_VENDOR));
  printf("Renderer: %s\n", glGetString(GL_RENDERER));
  printf("Version:  %s\n", glGetString(GL_VERSION));
  printf("GLSL:     %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  
//  glClearColor(220.0f/255.0f, 220.0f/255.0f, 220.0f/255.0f, 1.0f);
  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glCullFace(GL_BACK);
  
  SDL_ShowCursor(SDL_DISABLE);
  SDL_SetRelativeMouseMode(SDL_TRUE);
  
  atexit(cleanup);
  
  mat4 p = mat4_perspective(45.f, .1f, 1000.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);
  camera_t cam;
  camera_init_def(&cam);
  cam.front = vec3_new(-0.0000000377594489, -0.503773987, -0.863835513);
  cam.pos   = vec3_new(-0.00109344907, 2.23043847, 2.11338186);
  cam.up    = vec3_new(-0.0000000220206609, 0.863835513, -0.503773987);
  cam.right = vec3_new(1.f, 0.f, -0.0000000437113883);
  cam.world = vec3_new(0.f, 1.f, 0.f);
  cam.yaw   = -90.f;
  cam.pitch = -30.25f;
  camera_update(&cam);
  
  light_t spotlight;
  spotlight.position = vec3_new(0.f, 7.f, 0.f);
  spotlight.direction = vec3_new(0.f, -1.f, 0.f);
  spotlight.cutOff = cosf(DEG2RAD(12.5f));
  spotlight.outerCutOff = cosf(DEG2RAD(17.5f));
  spotlight.ambient = vec3_new(.5f, .5f, .5f);
  spotlight.diffuse = vec3_new(1.f, 1.f, 1.f);
  spotlight.specular = vec3_new(1.f, 1.f, 1.f);
  spotlight.constant = 1.f;
  spotlight.linear = .09f;
  spotlight.quadratic = .032f;
  
  GLuint shader = load_shader_file(RES(scene.vert.glsl),
                                   RES(scene.frag.glsl));
  
  int cube_tex_w, cube_tex_h,
  bowl_tex_w, bowl_tex_h,
  plane_tex_w, plane_tex_h;
  GLuint cube_tex = load_texture(RES(dice.png), &cube_tex_w, &cube_tex_h);
  
  obj_t cube_obj, plane_obj, bowl_obj;
  load_obj(&cube_obj,  RES(dice.obj));
  load_obj(&plane_obj, RES(plane.obj));
  load_obj(&bowl_obj,  RES(bowl.obj));
  
  vector_init(&dice);
  
  GLuint projection_loc = glGetUniformLocation(shader, "projection");
  GLuint view_loc       = glGetUniformLocation(shader, "view");
  
  dInitODE();
  world = dWorldCreate();
  space = dHashSpaceCreate(0);
  dWorldSetGravity(world, 0.0, -9.81, 0.0);
  dWorldSetLinearDamping(world, 0.0001);
  dWorldSetAngularDamping(world, 0.005);
  dWorldSetMaxAngularSpeed(world, 200);
  dWorldSetContactMaxCorrectingVel(world, 0.2);
  dWorldSetContactSurfaceLayer(world, 0.001);
  dWorldSetCFM(world, 1E-13);
  dWorldSetERP(world, 0.5);
  contact_group = dJointGroupCreate(0);
  
  game_obj_t plane;
  plane.geom = dCreatePlane(space, 0, 1, 0, 0);
  plane.model = &plane_obj;
  plane.mat.texture = load_texture(RES(checkered.png), &plane_tex_w, &plane_tex_h);
  plane.mat.shininess = 32.f;
  plane.mat.specular = vec3_new(.5f, .5f, .5f);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  //plane.world = mat4_mul_mat4(mat4_id(), mat4_scale(vec3_new(5, 5, 5)));
  plane.world = mat4_id();
  
  game_obj_t bowl;
  bowl.model = &bowl_obj;
  bowl.mat.texture = load_texture(RES(bowl.png), &bowl_tex_w, &bowl_tex_h);
  bowl.mat.shininess = 40.f;
  bowl.mat.specular = vec3_new(2.f, 2.f, 2.f);
  dTriMeshDataID bowl_tri = dGeomTriMeshDataCreate();
  dGeomTriMeshDataBuildSimple(bowl_tri, Icosphere_vertices, Icosphere_num_vertices, Icosphere_indices, Icosphere_num_indices);
  bowl.geom = dCreateTriMesh(space, bowl_tri, NULL, NULL, NULL);
  dGeomSetPosition(bowl.geom, 0.f, 0.85f, 0.f);
  bowl.world = mat4_mul_mat4(mat4_id(), mat4_translation(vec3_new(0.f, .85f, 0.f)));
//  bowl.body = dBodyCreate(world);
//  dMass mass;
//  dMassSetBox(&mass, 2, 2, 2, 2);
//  dBodySetMass(bowl.body, &mass);
//  dBodySetPosition(bowl.body, 0.f, 0.86f, 0.f);
//  dGeomSetBody(bowl.geom, bowl.body);
//  update_game_obj(&bowl, 0);
  
  Uint32 old_time, current_time = SDL_GetTicks();
  float delta;
  SDL_bool running = SDL_TRUE, running_physics;
  const Uint8* keys;
  SDL_Event e;
  while (running) {
    old_time = current_time;
    current_time = SDL_GetTicks();
    delta = (float)(current_time - old_time) / 1000.0f;
    
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
        case SDL_QUIT:
          running = SDL_FALSE;
          break;
        case SDL_MOUSEMOTION:
#ifdef GLAD_DEBUG
          camera_look(&cam, e.motion.xrel, -e.motion.yrel);
#endif
          break;
        case SDL_KEYUP:
          if (e.key.keysym.sym == SDLK_z) {
            game_obj_t* tmp = (game_obj_t*)malloc(sizeof(game_obj_t));
            tmp->body = dBodyCreate(world);
            dMass mass;
            dMassSetBox(&mass, 1, 1, 1, 1);
            dBodySetMass(tmp->body, &mass);
            dBodySetPosition(tmp->body, cam.pos.x, cam.pos.y, cam.pos.z);
            
            dMatrix3 R;
            vec3 rand_euler = vec3_new(rand_angle, rand_angle, rand_angle);
            dRFromEulerAngles(R, rand_euler.x, rand_euler.y, rand_euler.z);
            dBodySetRotation(tmp->body, R);
            
            
            vec3 vel = vec3_new(force_range, force_range, force_range);
            vec3 force = vec3_mul_vec3(vec3_normalize(cam.front), vel);
            dBodySetLinearVel(tmp->body, force.x, force.y, force.z);
            force = vec3_cross(cam.front, vec3_mul(vec3_neg(vel), 3.f));
            dBodySetAngularVel(tmp->body, force.x, force.y, force.z);
            
            tmp->geom = dCreateBox(space, .2, .2, .2);
            dGeomSetBody(tmp->geom, tmp->body);
            
            tmp->model = &cube_obj;
            tmp->mat.texture = cube_tex;
            tmp->mat.shininess = 32.f;
            tmp->mat.specular = vec3_new(.5f, .5f, .5f);
            
            vector_push(&dice, (void*)tmp);
          } else if (e.key.keysym.sym == SDLK_c)
            vector_clear_free(&dice);
          break;
      }
    }
    
    keys = SDL_GetKeyboardState(NULL);
    
    if (keys[SDL_GetScancodeFromKey(SDLK_b)])
      printf("BREAK\n");
    
    running_physics = SDL_TRUE;
    if (keys[SDL_GetScancodeFromKey(SDLK_ESCAPE)])
      running = SDL_FALSE;
    
    if (keys[SDL_GetScancodeFromKey(SDLK_SPACE)])
      running_physics = SDL_FALSE;

#ifdef GLAD_DEBUG
    camera_move(&cam, keys);
    camera_update(&cam);
#endif
    
    if (running_physics) {
      dSpaceCollide(space, 0, collide);
      dWorldQuickStep(world, 1.f / 60.f);
      dJointGroupEmpty (contact_group);
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(shader);
    
    add_light(&spotlight, shader);
    glUniform3f(glGetUniformLocation(shader, "viewPos"), cam.pos.x, cam.pos.y, cam.pos.z);
    
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, &p.m[0]);
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, &cam.view.m[0]);
    
    for (int i = 0; i < dice.length; ++i) {
      game_obj_t* tmp = (game_obj_t*)vector_get(&dice, i);
      update_game_obj(tmp, 1);
      draw_game_obj(tmp, shader);
    }
    
    draw_game_obj(&plane, shader);
    
    draw_game_obj(&bowl, shader);
    
    glUseProgram(0);
    
    SDL_GL_SwapWindow(window);
  }
  
  for (int i = 0; i < dice.length; ++i)
    free_game_obj((game_obj_t*)vector_get(&dice, i));
  vector_free_all(&dice);
  free_game_obj(&plane);
  dGeomTriMeshDataDestroy(bowl_tri);
  free_game_obj(&bowl);
  glDeleteProgram(shader);
  
  return 0;
}
