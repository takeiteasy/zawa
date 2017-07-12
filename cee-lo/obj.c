//
//  obj.c
//  cee-lo
//
//  Created by George Watson on 25/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#include "obj.h"

typedef float float3[3];
typedef int   int3[3];
typedef float float2[2];

enum { X, Y, Z };

typedef struct __obj_final_t {
  float3 *vertices, *normals;
  float2 *uvs;
} obj_data_t;

typedef struct {
  float3 pos, norm;
  float2 uv;
} obj_vertex_t;

void load_obj(obj_t* o, const char* p) {
  FILE* fh = fopen(p, "r");
  if (!fh) {
    fprintf(stderr, "load_obj failed to open\"%s\"\n", p);
    abort();
  }
  
  char buff[128];
  char obj_name[128];
  int num_v = 0,
      num_vt = 0,
      num_vn = 0,
      num_f  = 0;
  while (1) {
    fgets(buff, sizeof(buff), fh);
    if (feof(fh))
      break;
    
    if (buff[0] == '#' || buff[0] == '\n')
      continue;
    
    switch (buff[0]) {
      case 'o':
        sscanf(buff, "o %s", obj_name);
        break;
      case 'v':
        switch (buff[1]) {
          case ' ':
            num_v += 1;
            break;
          case 'n':
            num_vn += 1;
            break;
          case 't':
            num_vt += 1;
            break;
          default:
            break;
        }
        break;
      case 'f':
        num_f		+= 1;
        break;
      default:
        break;
    }
  }
  
  obj_data_t data;
  data.vertices = (float3*)malloc(num_v  * sizeof(float3));
  data.normals  = (float3*)malloc(num_vn * sizeof(float3));
  data.uvs      = (float2*)malloc(num_vt * sizeof(float2));
  
  int* v_indices  = (int*)malloc(num_f * 3 * sizeof(int));
  int* vn_indices = (int*)malloc(num_f * 3 * sizeof(int));
  int* vt_indices = (int*)malloc(num_f * 3 * sizeof(int));
  
  int3 verts_tmp, norms_tmp, uvs_tmp;
  int v_index  = 0,
      vt_index = 0,
      vn_index = 0,
      f_index  = 0;
  
  rewind(fh);
  while (1) {
    fgets(buff, sizeof(buff), fh);
    if (feof(fh))
      break;
    
    if (buff[0] == '#' || buff[0] == '\n')
      continue;
    
    switch (buff[0]) {
      case 'v':
        switch (buff[1]) {
          case ' ':
            sscanf(buff, "v %f %f %f",
                   &data.vertices[v_index][X],
                   &data.vertices[v_index][Y],
                   &data.vertices[v_index][Z]);
            v_index++;
            break;
          case 'n':
            sscanf(buff, "vn %f %f %f",
                   &data.normals[vn_index][X],
                   &data.normals[vn_index][Y],
                   &data.normals[vn_index][Z]);
            vn_index++;
            break;
          case 't':
            sscanf(buff, "vt %f %f",
                   &data.uvs[vt_index][X],
                   &data.uvs[vt_index][Y]);
            vt_index++;
            break;
          default:
            break;
        }
        break;
      case 'f':
        if (sscanf(buff, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                   &verts_tmp[X], &uvs_tmp[X], &norms_tmp[X],
                   &verts_tmp[Y], &uvs_tmp[Y], &norms_tmp[Y],
                   &verts_tmp[Z], &uvs_tmp[Z], &norms_tmp[Z]) == 9) {
        } else if (sscanf(buff, "f %d//%d %d//%d %d//%d",
                          &verts_tmp[X], &norms_tmp[X],
                          &verts_tmp[Y], &norms_tmp[Y],
                          &verts_tmp[Z], &norms_tmp[Z]) == 6) {
          uvs_tmp[X] = 0;
          uvs_tmp[Y] = 0;
          uvs_tmp[Z] = 0;
        } else if (sscanf(buff, "f %d/%d %d/%d %d/%d",
                          &verts_tmp[X], &uvs_tmp[X],
                          &verts_tmp[Y], &uvs_tmp[Y],
                          &verts_tmp[Z], &uvs_tmp[Z]) == 6) {
          norms_tmp[X] = 0;
          norms_tmp[Y] = 0;
          norms_tmp[Z] = 0;
        } else if (sscanf(buff, "f %d %d %d",
                          &verts_tmp[X], &verts_tmp[Y], &verts_tmp[Z]) == 3) {
          uvs_tmp[X] = 0; norms_tmp[X] = 0;
          uvs_tmp[Y] = 0; norms_tmp[X] = 0;
          uvs_tmp[Z] = 0; norms_tmp[X] = 0;
        } else {
          fprintf(stderr, "load_obj failed \"%s\" wtf is this \"%s\"?", p, buff);
          exit(-1);
        }
        
        v_indices[f_index]     = verts_tmp[X];
        v_indices[f_index + 1] = verts_tmp[Y];
        v_indices[f_index + 2] = verts_tmp[Z];
        
        vn_indices[f_index]     = norms_tmp[X];
        vn_indices[f_index + 1] = norms_tmp[Y];
        vn_indices[f_index + 2] = norms_tmp[Z];
        
        vt_indices[f_index]     = uvs_tmp[X];
        vt_indices[f_index + 1] = uvs_tmp[Y];
        vt_indices[f_index + 2] = uvs_tmp[Z];
        
        f_index += 3;
        break;
      default:
        break;
    }
  }
  
  GLuint VBO;
  glGenVertexArrays(1, &o->id);
  glGenBuffers(1, &VBO);
  
  obj_vertex_t* verts = (obj_vertex_t*)malloc(num_f * 3 * sizeof(obj_vertex_t));
  float3* tmp;
  float2* tmp_2;
  for (int i = 0; i < num_f * 3; ++i) {
    tmp = &data.vertices[v_indices[i] - 1];
    verts[i].pos[X] = (*tmp)[X];
    verts[i].pos[Y] = (*tmp)[Y];
    verts[i].pos[Z] = (*tmp)[Z];
    
    if (vn_indices[0] != 0) {
      tmp = &data.normals[vn_indices[i] - 1];
      verts[i].norm[X] = (*tmp)[X];
      verts[i].norm[Y] = (*tmp)[Y];
      verts[i].norm[Z] = (*tmp)[Z];
    }
    
    if (vt_indices[0] != 0) {
      tmp_2 = &data.uvs[vt_indices[i] - 1];
      verts[i].uv[X] = (*tmp_2)[X];
      verts[i].uv[Y] = (*tmp_2)[Y];
    }
  }
  
  glBindVertexArray(o->id);
  
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, num_f * 3 * sizeof(obj_vertex_t), &verts[0], GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(obj_vertex_t), (void*)offsetof(obj_vertex_t, pos));
  glEnableVertexAttribArray(0);
  
  if (vn_indices[0] != 0) {
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(obj_vertex_t), (void*)offsetof(obj_vertex_t, norm));
    glEnableVertexAttribArray(1);
  }
  
  if (vt_indices[0] != 0) {
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(obj_vertex_t), (void*)offsetof(obj_vertex_t, uv));
    glEnableVertexAttribArray(2);
  }
  
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  o->size = num_f * 3;
  
  free(verts);
  free(data.vertices);
  free(data.normals);
  free(data.uvs);
  free(v_indices);
  free(vn_indices);
  free(vt_indices);
  
  fclose(fh);
}

void draw_obj(obj_t* o) {
  glBindVertexArray(o->id);
  glDrawArrays(GL_TRIANGLES, 0, o->size);
  glBindVertexArray(0);
}

void free_obj(obj_t* o) {
  glDeleteVertexArrays(1, &o->id);
}
