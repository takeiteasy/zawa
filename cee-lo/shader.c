//
//  helper.c
//  opengl_testbed
//
//  Created by George Watson on 21/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#include "shader.h"

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
