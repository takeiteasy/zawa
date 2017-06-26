//
//  camera.h
//  cee-lo
//
//  Created by George Watson on 26/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#ifndef camera_h
#define camera_h

#include <math.h>
#include "linalgb.h"
#include "glad.h"

#define PI180 .01745329251994329576f
#define DEG2RAD(X) (X * PI180)
// #define RAD2DEG(X) ((X * 180.f) / M_PI)

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

enum VIEW_DIRECTION {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  FORWARD,
  BACK
};

typedef struct {
  struct {
    vec3 pos, front, up, right, world;
    GLfloat yaw, pitch, move_speed, sensitivity, zoom;
  };
  mat4 view;
} camera_t;

void camera_init_def(camera_t*);
void camera_init(camera_t* cam, vec3 a, vec3 b, float y, float p);
void camera_update(camera_t*);
void camera_move(camera_t*, const enum VIEW_DIRECTION);
void camera_look(camera_t*, int, int);

#endif /* camera_h */
