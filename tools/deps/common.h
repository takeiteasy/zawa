/* common.h -- https://github.com/takeiteasy/ceelo
 
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

#pragma once
#include <stdio.h>
#include <assert.h>
#include <string.h>
#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

#define BAIL(X, MSG, ...) \
    do {             \
        if (!(X)) {   \
            fprintf(stderr, "ERROR: " MSG "\n", __VA_ARGS__); \
            usage(); \
        } \
        assert((X)); \
    } while(0)

static const char* FileExt(const char *path) {
    const char *dot = strrchr(path, '.');
    return !dot || dot == path ? NULL : dot + 1;
}

static const char* RemoveExt(const char* path) {
    char *lastExt = strrchr(path, '.');
    if (lastExt != NULL)
        *lastExt = '\0';
    return lastExt ? path : NULL;
}

static const char* FileName(const char *path) {
    int l = 0;
    char *tmp = strstr(path, "/");
    do {
        l = strlen(tmp) + 1;
        path = &path[strlen(path) - l + 2];
        tmp = strstr(path, "/");
    } while(tmp);
    return RemoveExt(path);
}
