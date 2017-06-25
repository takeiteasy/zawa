//
//  helper.c
//  opengl_testbed
//
//  Created by George Watson on 21/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#include "helper.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

char* __load_file_to_mem(const char* path) {
	FILE *file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "fopen \"%s\" failed: %d %s\n", path, errno, strerror(errno));
		exit(1);
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

GLuint load_shader(const char* vert, const char* frag) {
	return __make_program(__make_shader(GL_VERTEX_SHADER,		vert),
												__make_shader(GL_FRAGMENT_SHADER, frag));
}

GLuint load_texture(const char* path) {
	GLuint texture = 0;
	glGenTextures(1, &texture);
	
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	int width, height, chans;
	unsigned char* data = stbi_load(path, &width, &height, &chans, 0);
	
	if (!data) {
		fprintf(stderr, "stbi_load failed to load \"%s\"\n", path);
		abort();
	}
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	stbi_image_free(data);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	return texture;
}
