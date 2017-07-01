//
//  game_obj.c
//  cee-lo
//
//  Created by George Watson on 29/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#include "game_obj.h"

static mat4 dice_scale = {
  .1f, 0.f, 0.f, 0.f,
  0.f, .1f, 0.f, 0.f,
  0.f, 0.f, .1f, 0.f,
  0.f, 0.f, 0.f, 1.f
};
static const dReal *t, *r;

void draw_game_obj(game_obj_t* o, GLuint shader) {
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &o->world.m[0]);
  if (o->mat.texture) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, o->mat.texture);
  }
  glUniform1i(glGetUniformLocation(shader, "material.diffuse"), 0);
  glUniform3f(glGetUniformLocation(shader, "material.specular"),
              o->mat.specular.x, o->mat.specular.y, o->mat.specular.z);
  glUniform1f(glGetUniformLocation(shader, "material.shininess"), o->mat.shininess);
  
  draw_obj(o->model);
  
  glBindTexture(GL_TEXTURE_2D, 0);
}

void update_game_obj(game_obj_t* o, int scale) {
  t = dBodyGetPosition(o->body);
  r = dBodyGetRotation(o->body);
  
  o->world = mat4_new(r[0], r[1], r[2],  t[0],
                      r[4], r[5], r[6],  t[1],
                      r[8], r[9], r[10], t[2],
                      0.f,  0.f,  0.f,   1.f);
  if (scale)
    o->world = mat4_mul_mat4(o->world, dice_scale);
}

void free_game_obj(game_obj_t* o) {
  if (o->geom)
    dGeomDestroy(o->geom);
  if (o->body)
    dBodyDestroy(o->body);
  if (o->model) {
    free_obj(o->model);
    o->model = NULL;
  }
  if (o->mat.texture)
    glDeleteTextures(1, &o->mat.texture);
}

#define LIGHT_VEC3(X) \
glUniform3f(glGetUniformLocation(shader, "light." #X), l->X.x, l->X.y, l->X.z)
#define LIGHT_FLOAT(X) \
glUniform1f(glGetUniformLocation(shader, "light." #X), l->X)

void add_light(light_t* l, GLuint shader) {
  LIGHT_VEC3(position);
  LIGHT_VEC3(direction);
  LIGHT_VEC3(ambient);
  LIGHT_VEC3(diffuse);
  LIGHT_VEC3(specular);
  
  LIGHT_FLOAT(cutOff);
  LIGHT_FLOAT(outerCutOff);
  LIGHT_FLOAT(constant);
  LIGHT_FLOAT(linear);
  LIGHT_FLOAT(quadratic);
}
