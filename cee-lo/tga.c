//
//  tga.c
//  cee-lo
//
//  Created by George Watson on 26/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#include "tga.h"

static GLubyte uncompressed_tga[12] = {0,0,2, 0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
static GLubyte compressed_tga[12]   = {0,0,10,0,0,0,0,0,0,0,0,0};	// Compressed TGA Header

typedef GLubyte TGA_header_t[12];

typedef struct {
  GLubyte header[6];
  GLuint		size;
  GLuint		tmp;
  GLuint		type;
  GLuint		h;
  GLuint		w;
  GLuint		bpp;
} TGA_t;

GLuint load_tga(const char* path) {
  GLuint ret = 0;
  
  FILE* fp = fopen(path, "rb");
  if (!fp) {
    fprintf(stderr, "load_tga failed to load \"%s\"\n", path);
    abort();
  }
  
  TGA_header_t buff;
  fread(&buff, sizeof(TGA_header_t), 1, fp);
  
  if (memcmp(uncompressed_tga, &buff, sizeof(TGA_header_t)) == 0) {
    TGA_t tga;
    fread(tga.header, sizeof(tga.header), 1, fp);
    
    tga.w   = tga.header[1] * 256 + tga.header[0];
    tga.h   = tga.header[3] * 256 + tga.header[2];
    tga.bpp = tga.header[4];
    
    if (tga.w <= 0 || tga.h <= 0 || (tga.bpp != 24 && tga.bpp != 32)) {
      printf("load_tga failed to load \"%s\" - invalid TGA information\n", path);
      abort();
    }
    
    GLuint   bpp  = tga.bpp / 8;
    size_t   size = bpp * tga.w * tga.h;
    GLubyte* data = (GLubyte*)malloc(size);
    fread(data, 1, size, fp);
    
    for(GLuint cswap = 0; cswap < (int)size; cswap += bpp)
      data[cswap] ^= data[cswap + 2] ^= data[cswap] ^= data[cswap + 2];
    
    glGenTextures(1, &ret);
    glBindTexture(GL_TEXTURE_2D, ret);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tga.w, tga.h, 0, (tga.bpp == 24 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    free(data);
  } else if (memcmp(compressed_tga, &buff, sizeof(TGA_header_t)) == 0) {
    printf("load_tga failed to load \"%s\" - can only parse uncompressed TGA files\n", path);
    abort();
  } else {
    printf("load_tga failed to load \"%s\" - unknown format\n", path);
    abort();
  }

  fclose(fp);
  return ret;
}
