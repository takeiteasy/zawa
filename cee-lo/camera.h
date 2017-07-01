//
//  camera.h
//  cee-lo
//
//  Created by George Watson on 26/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#ifndef camera_h
#define camera_h

#include "3rdparty/linalgb.h"
#include "3rdparty/glad.h"
#include <SDL2/SDL_keyboard.h>

#define DEG2RAD(X) (X * .01745329251994329576f)
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

typedef struct {
  vec3 pos, front, up, right, world;
  GLfloat yaw, pitch, move_speed, sensitivity, zoom;
  mat4 view;
} camera_t;

void camera_init_def(camera_t*);
void camera_init(camera_t*, vec3, vec3, float, float);
void camera_update(camera_t*);
void camera_move(camera_t*, const Uint8*);
void camera_look(camera_t*, int, int);

#endif /* camera_h */
