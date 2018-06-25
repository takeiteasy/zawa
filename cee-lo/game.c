//
//  game.c
//  cee-lo
//
//  Created by George Watson on 29/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#include "game.h"

char* load_file_to_mem(const char* path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    fprintf(stderr, "fopen \"%s\" failed: %d %s\n", path, errno, strerror(errno));
    abort();
  }
  
  fseek(file, 0, SEEK_END);
  size_t length = ftell(file);
  rewind(file);
  
  char *data = (char*)calloc(length + 1, sizeof(char));
  fread(data, 1, length, file);
  fclose(file);
  
  return data;
}

GLuint make_shader(GLenum type, const char* src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);
  
  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    GLint length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    GLchar *info = (GLchar*)calloc(length, sizeof(GLchar));
    glGetShaderInfoLog(shader, length, NULL, info);
    fprintf(stderr, "glCompileShader failed:\n%s\n", info);
    
    free(info);
    exit(-1);
  }
  
  return shader;
}

GLuint make_program(GLuint vert, GLuint frag) {
  GLuint program = glCreateProgram();
  glAttachShader(program, vert);
  glAttachShader(program, frag);
  glLinkProgram(program);
  
  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    GLchar *info = calloc(length, sizeof(GLchar));
    glGetProgramInfoLog(program, length, NULL, info);
    fprintf(stderr, "glLinkProgram failed: %s\n", info);
    
    free(info);
    exit(-1);
  }
  
  glDetachShader(program, vert);
  glDetachShader(program, frag);
  glDeleteShader(vert);
  glDeleteShader(frag);
  
  return program;
}

GLuint load_shader_str(const char* vert, const char* frag) {
  return make_program(make_shader(GL_VERTEX_SHADER,    vert),
                        make_shader(GL_FRAGMENT_SHADER, frag));
}

GLuint load_shader_file(const char* _vert, const char* _frag) {
  const char* vert = load_file_to_mem(_vert);
  const char* frag = load_file_to_mem(_frag);
  
  GLuint ret = make_program(make_shader(GL_VERTEX_SHADER,    vert),
                              make_shader(GL_FRAGMENT_SHADER, frag));
  free((char*)vert);
  free((char*)frag);
  
  return ret;
}

GLuint load_texture(const char* src, int* width, int* height) {
  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);
  
  int chans;
  unsigned char* data = stbi_load(src, width, height, &chans, STBI_rgb_alpha);
  if (!data) {
    fprintf(stderr, "stbi_load \"%s\" failed!", src);
    exit(-1);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  
  stbi_image_free(data);
  
  return id;
}

void draw_ode(ode_t* o, GLuint shader) {
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &o->world.m[0]);
  glUniform1i(glGetUniformLocation(shader, "material.diffuse"), 0);
  glUniform3f(glGetUniformLocation(shader, "material.specular"), o->mat.specular.x, o->mat.specular.y, o->mat.specular.z);
  glUniform1f(glGetUniformLocation(shader, "material.shininess"), o->mat.shininess);
  
  draw_obj(o->model);
}

void free_ode(ode_t* o) {
  if (o->geom)
    dGeomDestroy(o->geom);
  if (o->body)
    dBodyDestroy(o->body);
  if (o->model) {
    free_obj(o->model);
    o->model = NULL;
  }
}

#define LIGHT_VEC3(X)  glUniform3f(glGetUniformLocation(shader, "light." #X), l->X.x, l->X.y, l->X.z)
#define LIGHT_FLOAT(X) glUniform1f(glGetUniformLocation(shader, "light." #X), l->X)

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
