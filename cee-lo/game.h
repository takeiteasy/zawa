//
//  game.h
//  cee-lo
//
//  Created by George Watson on 29/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#ifndef game_h
#define game_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "3rdparty/glad.h"
#define STBI_ONLY_PNG
#include "3rdparty/stb_image.h"
#include "3rdparty/linalgb.h"
#include <ode/ode.h>
#include "obj.h"

#define rand_range(min, max) (rand() % (max + 1 - min) + min)
#define frand_range(min, max) ((max - min) * ((((float)rand()) / (float) RAND_MAX)) + min)
#define rand_01 (rand_range(0, 1))
#define frand_01 (rand_range(0, 100) / 100.f)
#define rand_angle (frand_range(-6.28319f, 6.28319f))
#define force_range (frand_range(5.5f, 6.5f))

#define DEG2RAD(X) (X * .01745329251994329576f)

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#define GLSL(VERSION,CODE) "#version " #VERSION "\n" #CODE

GLuint load_shader_str(const char*, const char*);
GLuint load_shader_file(const char*, const char*);
GLuint load_texture(const char*, int*, int*);

typedef struct {
  vec3 specular;
  float shininess;
} material_t;

typedef struct {
  dGeomID geom;
  dBodyID body;
  mat4 world;
  obj_t* model;
  material_t mat;
} ode_t;

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

void draw_ode(ode_t*, GLuint);
void free_ode(ode_t*);
void add_light(light_t*, GLuint);

#endif /* game_h */
