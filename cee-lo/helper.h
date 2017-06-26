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
#include "glad.h"

#define GLSL(VERSION,CODE) "#version " #VERSION "\n" #CODE

#ifndef safe_free
#define safe_free(X) \
if (X != NULL) { \
  free(X); \
  X = NULL; \
}
#endif

#ifndef array_size
#define array_size(X) (sizeof(X) / sizeof(X[0]))
#endif

GLuint load_shader(const char*, const char*);

GLuint load_texture(const char*);

#endif /* helper_h */
