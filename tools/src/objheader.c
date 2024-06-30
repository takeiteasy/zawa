/* objheader.h -- https://github.com/takeiteasy/ceelo
 
 The MIT License (MIT)

 Copyright (c) 2022 George Watson

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"
#include "common.h"

#define usage() printf("usage: objheader <in>.obj\n")

int main(int argc, const char *argv[]) {
    BAIL(argc == 2, "Incorrect arguments: %d, expected 1", argc - 1);
    const char *ext = FileExt(argv[1]);
    BAIL(ext, "Incorrect file extension: \"%s\", expected \"obj\"", ext ? ext : "NULL");
    BAIL(!strncmp(ext, "obj", 3), "Incorrect file extension: \"%s\", expected \"obj\"", ext);
    BAIL(!access(argv[1], F_OK), "File doesn't exist at: \"%s\"", argv[1]);
    
    fastObjMesh* mesh = fast_obj_read(argv[1]);
    BAIL(mesh, "Failed to load obj at \"%s\"", argv[1]);
    
    char outPath[512];
    int pathLength = strlen(argv[1]);
    memcpy(outPath, argv[1], sizeof(char) * pathLength);
    const char *outExt = ".h";
    memcpy(outPath + pathLength, outExt, sizeof(char) * 3);
    outPath[pathLength + 3] = '\0';
    const char *outName = FileName(argv[1]);
    
    printf("// Generated by objheader.c -- https://github.com/takeiteasy/\n"
           "\n"
           "#ifndef __OBJ__%s__H__\n"
           "#define __OBJ__%s__H__\n",
           outName, outName);
    size_t sizeOfBuffer = mesh->face_count * 3 * 8;
    printf("static float obj_%s_data[%zu] = {", outName, sizeOfBuffer);
    float buffer[sizeOfBuffer];
    for (int i = 0; i < mesh->face_count * 3; i++) {
        fastObjIndex vertex = mesh->indices[i];
        unsigned int pos = i * 8;
        unsigned int v_pos = vertex.p * 3;
        unsigned int n_pos = vertex.n * 3;
        unsigned int t_pos = vertex.t * 2;
        memcpy(buffer + pos, mesh->positions + v_pos, 3 * sizeof(float));
        memcpy(buffer + pos + 3, mesh->normals + n_pos, 3 * sizeof(float));
        memcpy(buffer + pos + 6, mesh->texcoords + t_pos, 2 * sizeof(float));
    }
    for (int i = 0; i < sizeOfBuffer; i+=8)
        printf("\n\t%f, %f, %f,\t%f, %f, %f,\t%f, %f,", buffer[i], buffer[i+1], buffer[i+2], buffer[i+3], buffer[i+4], buffer[i+5], buffer[i+6], buffer[i+7]);
    printf("\n};\nstatic unsigned int obj_%s_data_size = %zu;\n", outName, sizeOfBuffer);
    printf("static unsigned int obj_%s_face_count = %d;\n", outName, mesh->face_count * 3);
    printf("#endif // __OBJ__%s__H__\n", outName);
    
    fast_obj_destroy(mesh);
    return 0;
}