//
//  helper.c
//  opengl_testbed
//
//  Created by George Watson on 21/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#include "helpers.h"

char* __load_file(const char* path) {
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

GLuint __make_shader(GLenum type, const char* src) {
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

GLuint __make_program(GLuint vert, GLuint frag) {
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
	return __make_program(__make_shader(GL_VERTEX_SHADER,		vert),
												__make_shader(GL_FRAGMENT_SHADER, frag));
}

GLuint load_shader_file(const char* _vert, const char* _frag) {
  const char* vert = __load_file(_vert);
  const char* frag = __load_file(_frag);
  
  GLuint ret = __make_program(__make_shader(GL_VERTEX_SHADER,		vert),
                              __make_shader(GL_FRAGMENT_SHADER, frag));
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
