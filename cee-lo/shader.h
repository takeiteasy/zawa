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

#define GLSL(VERSION,CODE) "#version " #VERSION "\n" #CODE

GLuint load_shader_str(const char*, const char*);

// GLuint load_shader_file(const char*, const char*);

#endif /* helper_h */
