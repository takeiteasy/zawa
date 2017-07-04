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
#include <errno.h>

#include "3rdparty/glad.h"
#define STBI_ONLY_PNG
#include "3rdparty/stb_image.h"

#define rand_range(min, max) (rand() % (max + 1 - min) + min)
#define frand_range(min, max) ((max - min) * ((((float)rand()) / (float) RAND_MAX)) + min)
#define rand_01 (rand_range(0, 1))
#define frand_01 (rand_range(0, 100) / 100.f)

#define DEG2RAD(X) (X * .01745329251994329576f)

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#define GLSL(VERSION,CODE) "#version " #VERSION "\n" #CODE

GLuint load_shader_str(const char*, const char*);
GLuint load_shader_file(const char*, const char*);
GLuint load_texture(const char*, int*, int*);

#endif /* helper_h */
