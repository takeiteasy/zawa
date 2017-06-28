//
//  helper.h
//  opengl_testbed
//
//  Created by George Watson on 21/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#ifndef helper_h
#define helper_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glad.h"

#define rand_range(min, max) (rand() % (max + 1 - min) + min)
#define rand_angle (((float)rand() / (float)(RAND_MAX)) * 6.28319f)
#define rand_01 rand_range(0, 1)
#define frand_01 (rand_range(0, 100) / 100.f)

#define GLSL(VERSION,CODE) "#version " #VERSION "\n" #CODE

GLuint load_shader_str(const char*, const char*);
// GLuint load_shader_file(const char*, const char*);
GLuint load_texture(const char*, int*, int*);

#endif /* helper_h */
