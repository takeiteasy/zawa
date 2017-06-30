//
//  game_obj.h
//  cee-lo
//
//  Created by George Watson on 29/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#ifndef game_obj_h
#define game_obj_h

#include "3rdparty/glad.h"
#include "3rdparty/linalgb.h"
#include <ode/ode.h>

#include "helpers.h"
#include "obj.h"

#define rand_angle (frand_range(-6.28319f, 6.28319f))
#define force_range (frand_range(5.5f, 6.5f))

typedef struct {
  GLuint texture;
  vec3 specular;
  float shininess;
} material_t;

typedef struct {
  dGeomID geom;
  dBodyID body;
  mat4 world;
  obj_t* model;
  material_t mat;
} game_obj_t;

typedef struct {
  vec3 position;
  vec3 direction;
  float cutOff;
  float outerCutOff;
  
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  
  float constant;
  float linear;
  float quadratic;
} light_t;

void draw_game_obj(game_obj_t*, GLuint);
void update_game_obj(game_obj_t*, int);
void free_game_obj(game_obj_t*);
void add_light(light_t*, GLuint);

#endif /* game_obj_h */
