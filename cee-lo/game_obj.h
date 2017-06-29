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
#define force_range (frand_range(5.f, 10.f))

typedef struct {
  dGeomID geom;
  dBodyID body;
  mat4 world;
  obj_t* model;
  GLuint texture;
} game_obj_t;

void draw_game_obj(game_obj_t*, GLuint, GLuint);
void update_game_obj(game_obj_t*);
void free_game_obj(game_obj_t*);

#endif /* game_obj_h */
