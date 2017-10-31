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

#include "game_obj.h"
#include "Icosphere_obj.h"

#define RES(X) "/Users/roryb/Dropbox/git/chinchirorin/res/" #X

static const int SCREEN_WIDTH = 640, SCREEN_HEIGHT = 480, FBO_SIZE = 1024;

static SDL_Window* window;
static SDL_GLContext context;
static dWorldID world;
static dSpaceID space;
static dJointGroupID contact_group;
#define MAX_CONTACTS 8
static dContact contact[MAX_CONTACTS];
#define MAX_DICE 1
static game_obj_t* dice[MAX_DICE];
static float dice_val[MAX_DICE];
static int num_dice = 0;

#undef GLAD_DEBUG

#ifdef GLAD_DEBUG
void pre_gl_call(const char *name, void *funcptr, int len_args, ...) {
  printf("Calling: %s (%d arguments)\n", name, len_args);
}
#endif

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

void collide(void* data, dGeomID o1, dGeomID o2) {
  dBodyID b1 = dGeomGetBody(o1),
          b2 = dGeomGetBody(o2);
  
  if (b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact))
    return;
  
  for (int i = 0; i < dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sizeof(dContact)); i++) {
    contact[i].surface.mode = dContactBounce | dContactSoftCFM;
    contact[i].surface.mu = dInfinity;
    contact[i].surface.mu2 = 0;
    contact[i].surface.bounce = 0.3;
    
    dJointID c = dJointCreateContact(world, contact_group, &contact[i]);
    dJointAttach(c, b1, b2);
  }
}

void clear_dice() {
  num_dice = 0;
  for (int i = 0; i < MAX_DICE; ++i) {
    if (dice[i]) {
      dBodyDestroy(dice[i]->body);
      dGeomDestroy(dice[i]->geom);
      free(dice[i]);
      dice[i] = NULL;
    }
    dice_val[i] = 0.f;
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
  
  window = SDL_CreateWindow("ざわ。。。ざわ。。。",
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
#endif
  
  glad_set_post_callback(post_gl_call);
  
  printf("Vendor:   %s\n",   glGetString(GL_VENDOR));
  printf("Renderer: %s\n",   glGetString(GL_RENDERER));
  printf("Version:  %s\n",   glGetString(GL_VERSION));
  printf("GLSL:     %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  
//  glClearColor(220.0f/255.0f, 220.0f/255.0f, 220.0f/255.0f, 1.0f);
  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glCullFace(GL_BACK);
  
  SDL_ShowCursor(SDL_DISABLE);
  SDL_SetRelativeMouseMode(SDL_TRUE);
  
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
  
  mat4 proj = mat4_perspective(45.f, .1f, 1000.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);
  
  vec3 view_pos = vec3_new(0.f, 2.f, 2.2f);
  mat4 view = mat4_view_look_at(view_pos,
                                vec3_new(0.f, .8f, 0.f),
                                vec3_new(0.f, 1.f, 0.f));
  vec3 fire_forward = vec3_new(0.f, -0.5f, -.86f);
  
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
  
  GLuint bowl_shader  = load_shader_file(RES(default.vert.glsl), RES(bowl.frag.glsl));
  GLuint plane_shader = load_shader_file(RES(default.vert.glsl), RES(plane.frag.glsl));
  GLuint dice_shader  = load_shader_file(RES(default.vert.glsl), RES(dice.frag.glsl));
  GLuint hand_shader  = load_shader_file(RES(default.vert.glsl), RES(hand.frag.glsl));
  
  int hand_tex_w, hand_tex_h;
  GLuint hand_tex = load_texture(RES(hand.png), &hand_tex_w, &hand_tex_h);
  
  obj_t cube_obj, plane_obj, bowl_obj, hand_obj;
  load_obj(&cube_obj,  RES(dice.obj));
  load_obj(&plane_obj, RES(plane.obj));
  load_obj(&bowl_obj,  RES(bowl.obj));
  load_obj(&hand_obj,  RES(hand.obj));
  
  GLuint projection_loc    = glGetUniformLocation(bowl_shader, "projection");
  GLuint view_loc          = glGetUniformLocation(bowl_shader, "view");
  GLuint viewPos_loc       = glGetUniformLocation(bowl_shader, "viewPos");
  
  GLuint plane_proj_loc    = glGetUniformLocation(plane_shader, "projection");
  GLuint plane_view_loc    = glGetUniformLocation(plane_shader, "view");
  GLuint plane_model_loc   = glGetUniformLocation(plane_shader, "model");
  GLuint plane_viewPos_loc = glGetUniformLocation(plane_shader, "viewPos");
  GLuint plane_color_loc   = glGetUniformLocation(plane_shader, "plane_color");
  vec3 plane_color = vec3_new(0.f, 0.f, 0.f);
  
  GLuint dice_proj_loc     = glGetUniformLocation(dice_shader, "projection");
  GLuint dice_view_loc     = glGetUniformLocation(dice_shader, "view");
  GLuint dice_viewPos_loc  = glGetUniformLocation(dice_shader, "viewPos");
  GLuint dice_color_loc    = glGetUniformLocation(dice_shader, "die_color");
  vec3 die_color = vec3_new(.2f, .2f, .2f);
  
  game_obj_t plane;
  plane.geom = dCreatePlane(space, 0, 1, 0, 0);
  plane.model = &plane_obj;
  plane.world = mat4_mul_mat4(mat4_id(), mat4_scale(vec3_new(5, 5, 5)));
  
  game_obj_t bowl;
  bowl.model = &bowl_obj;
  bowl.mat.shininess = 10.f;
  bowl.mat.specular = vec3_new(.25f, .25f, .25f);
  dTriMeshDataID bowl_tri = dGeomTriMeshDataCreate();
  dGeomTriMeshDataBuildSimple(bowl_tri, Icosphere_vertices, Icosphere_num_vertices, Icosphere_indices, Icosphere_num_indices);
  bowl.geom = dCreateTriMesh(space, bowl_tri, NULL, NULL, NULL);
  dGeomSetPosition(bowl.geom, 0.f, 0.85f, 0.f);
  bowl.world = mat4_mul_mat4(mat4_id(), mat4_translation(vec3_new(0.f, .85f, 0.f)));
  bowl.body = NULL;
  
  mat4 hand_world = mat4_mul_mat4(mat4_mul_mat4(mat4_id(),
                                                mat4_scale(vec3_new(.05f, .05f, .05f))),
                                  mat4_translation(vec3_new(0.f, 35.f, 25.f)));
  
  unsigned int depthMapFBO;
  glGenFramebuffers(1, &depthMapFBO);
  
  unsigned int depthMap;
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, FBO_SIZE, FBO_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  FILE* tmp_stream = NULL;
  long close_time = 0;
  char tmp_buff[256];
#define TEST_RENAME(X) \
  if (!tmp_stream && num_dice > 0) { \
    close_time = time(NULL) + 2; \
    bzero(tmp_buff, sizeof(tmp_buff)); \
    sprintf(tmp_buff, "/Users/roryb/Dropbox/git/chinchirorin/test/%d_%ld.txt", X, close_time); \
    tmp_stream = fopen(tmp_buff, "w"); \
  }

  Uint32 old_time, current_time = SDL_GetTicks();
  float delta;
  
  SDL_bool running = SDL_TRUE;
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
          hand_world = mat4_mul_mat4(hand_world, mat4_translation(vec3_new((float)e.motion.xrel * .05f, -(float)e.motion.yrel * .05f, 0)));
          hand_world.yw = clamp(hand_world.yw, 1.3f, 2.f);
          hand_world.xw = clamp(hand_world.xw, -.5f, .45f);
          break;
        case SDL_KEYUP:
          if (e.key.keysym.sym == SDLK_SPACE) {
            if (num_dice >= MAX_DICE)
              break;
            
            game_obj_t* tmp = (game_obj_t*)malloc(sizeof(game_obj_t));
            tmp->body = dBodyCreate(world);
            dMass mass;
            dMassSetBox(&mass, 1, 1, 1, 1);
            dBodySetMass(tmp->body, &mass);
            dBodySetPosition(tmp->body, hand_world.xw, hand_world.yw, hand_world.zw);
            
            dMatrix3 R;
            vec3 rand_euler = vec3_new(rand_angle, rand_angle, rand_angle);
            dRFromEulerAngles(R, rand_euler.x, rand_euler.y, rand_euler.z);
            dBodySetRotation(tmp->body, R);
            
            vec3 vel = vec3_new(force_range, force_range, force_range);
            vec3 force = vec3_mul_vec3(vec3_normalize(fire_forward), vel);
            dBodySetLinearVel(tmp->body, force.x, force.y, force.z);
            force = vec3_cross(fire_forward, vec3_mul(vec3_neg(vel), 3.f));
            dBodySetAngularVel(tmp->body, force.x, force.y, force.z);
            
            tmp->geom = dCreateBox(space, .2, .2, .2);
            dGeomSetBody(tmp->geom, tmp->body);
            
            tmp->model = &cube_obj;
            tmp->mat.texture = 0;
            tmp->mat.shininess = 32.f;
            tmp->mat.specular = vec3_new(.5f, .5f, .5f);
            
            dice[num_dice] = tmp;
            num_dice = min(++num_dice, MAX_DICE);
          }
          else if (e.key.keysym.sym == SDLK_ESCAPE)
            clear_dice();
          else if (e.key.keysym.sym == SDLK_c) {
            plane_color = vec3_new(frand_01, frand_01, frand_01);
            die_color = vec3_new(1.f - plane_color.x, 1.f - plane_color.y, 1.f - plane_color.z);
          }
          else if (e.key.keysym.sym == SDLK_v) {
            plane_color = vec3_new(0.f, 0.f, 0.f);
            die_color   = vec3_new(.2f, .2f, .2f);
          }
          else if (e.key.keysym.sym == SDLK_1) { TEST_RENAME(1); }
          else if (e.key.keysym.sym == SDLK_2) { TEST_RENAME(2); }
          else if (e.key.keysym.sym == SDLK_3) { TEST_RENAME(3); }
          else if (e.key.keysym.sym == SDLK_4) { TEST_RENAME(4); }
          else if (e.key.keysym.sym == SDLK_5) { TEST_RENAME(5); }
          else if (e.key.keysym.sym == SDLK_6) { TEST_RENAME(6); }
          break;
      }
    }
    
    dSpaceCollide(space, 0, collide);
    dWorldQuickStep(world, 1.f / 60.f);
    dJointGroupEmpty(contact_group);
    
    for (int i = 0; i < num_dice; ++i) {
      if (dice[i] && !dice_val[i]) {
        if (tmp_stream) {
          const dReal* a = dBodyGetQuaternion(dice[i]->body);
          const dReal* b = dBodyGetRotation(dice[i]->body);
        
          fprintf(tmp_stream, "Q: %f %f %f %f  %f %f %f %f  %f %f %f %f\nR: %f %f %f %f  %f %f %f %f  %f %f %f %f\n",
                  a[0], a[1], a[2],  a[3],
                  a[4], a[5], a[6],  a[7],
                  a[8], a[9], a[10], a[11],
                  b[0], b[1], b[2],  b[3],
                  b[4], b[5], b[6],  b[7],
                  b[8], b[9], b[10], b[11]);
          
          if (time(NULL) > close_time) {
            fclose(tmp_stream);
            tmp_stream = NULL;
            clear_dice();
            
            SDL_Event tmp_event;
            tmp_event.type = SDL_KEYUP;
            tmp_event.key.keysym.sym = SDLK_SPACE;
            SDL_PushEvent(&tmp_event);
          }
        }
      }
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(bowl_shader);
    
    add_light(&spotlight, bowl_shader);
    glUniform3f(viewPos_loc, view_pos.x, view_pos.y, view_pos.z);
    
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, &proj.m[0]);
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, &view.m[0]);
    
    draw_game_obj(&bowl, bowl_shader);
    
    glUseProgram(dice_shader);
    
    glUniformMatrix4fv(dice_proj_loc,  1, GL_FALSE, &proj.m[0]);
    glUniformMatrix4fv(dice_view_loc,  1, GL_FALSE, &view.m[0]);
    
    add_light(&spotlight, dice_shader);
    glUniform3f(dice_viewPos_loc, view_pos.x, view_pos.y, view_pos.z);
    glUniform3f(dice_color_loc, die_color.x, die_color.y, die_color.z);
    
    for (int i = 0; i < MAX_DICE; ++i) {
      if (dice[i] && !dice_val[i]) {
        update_game_obj(dice[i], 1);
        draw_game_obj(dice[i], dice_shader);
      }
    }
    
    glUseProgram(plane_shader);
    
    glUniformMatrix4fv(plane_proj_loc,  1, GL_FALSE, &proj.m[0]);
    glUniformMatrix4fv(plane_view_loc,  1, GL_FALSE, &view.m[0]);
    glUniformMatrix4fv(plane_model_loc, 1, GL_FALSE, &plane.world.m[0]);
    
    add_light(&spotlight, plane_shader);
    glUniform3f(plane_viewPos_loc, view_pos.x, view_pos.y, view_pos.z);
    glUniform3f(plane_color_loc, plane_color.x, plane_color.y, plane_color.z);
    
    draw_obj(&plane_obj);
    
    glUseProgram(hand_shader);
    
    glUniformMatrix4fv(glGetUniformLocation(hand_shader, "projection"),  1, GL_FALSE, &proj.m[0]);
    glUniformMatrix4fv(glGetUniformLocation(hand_shader, "view"),  1, GL_FALSE, &view.m[0]);
    glUniformMatrix4fv(glGetUniformLocation(hand_shader, "model"), 1, GL_FALSE, &hand_world.m[0]);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hand_tex);
    glUniform1d(glGetUniformLocation(hand_shader, "hand_tex"), 0);
    
    draw_obj(&hand_obj);
    
    glUseProgram(0);
    
    SDL_GL_SwapWindow(window);
  }
  
  for (int i = 0; i < MAX_DICE; ++i)
    if (dice[i])
      free_game_obj(dice[i]);
  free_game_obj(&plane);
  dGeomTriMeshDataDestroy(bowl_tri);
  free_game_obj(&bowl);
  glDeleteProgram(bowl_shader);
  glDeleteProgram(plane_shader);
  glDeleteProgram(dice_shader);
  
  SDL_DestroyWindow(window);
  SDL_GL_DeleteContext(context);
  dWorldDestroy(world);
  dSpaceDestroy(space);
  dJointGroupDestroy(contact_group);
  printf("Goodbye!\n");
  
  return 0;
}
