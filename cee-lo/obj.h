//
//  obj.h
//  cee-lo
//
//  Created by George Watson on 25/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#ifndef obj_h
#define obj_h

#include "3rdparty/glad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int size;
  GLuint id;
} obj_t;

void load_obj(obj_t*, const char*);
void draw_obj(obj_t*);
void free_obj(obj_t*);

#endif /* obj_h */
