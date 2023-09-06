ifeq ($(OS),Windows_NT)
	PROG_EXT=.exe
	LIB_EXT=dll
	DEPS=-lkernel32 -luser32 -lshell32 -lgdi32 -lopengl32
	ARCH=win32
else
	UNAME:=$(shell uname -s)
	PROG_EXT=
	ifeq ($(UNAME),Darwin)
		DEPS=-x objective-c++ -fobjc-arc -framework Cocoa -framework OpenGL
		ARCH:=$(shell uname -m)
		LIB_EXT=dylib
		ifeq ($(ARCH),arm64)
			ARCH=osx_arm64
		else
			ARCH=osx
		endif
	else ifeq ($(UNAME),Linux)
		DEPS=-lGL -ldl -lm -lX11 -lXi -lXcursor
		ARCH=linux
		LIB_EXT=so
	else
		$(error OS not supported by this Makefile)
	endif
endif

SOURCE=$(wildcard src/*.cpp)

ARCH_PATH=tools/bin/$(ARCH)
OBJ_PATH=$(ARCH_PATH)/objheader$(PROG_EXT)
OBJS=$(wildcard assets/*.obj)
OBJS_OUT=$(patsubst assets/%,build/%.h,$(OBJS))

.SECONDEXPANSION:
OBJ=$(patsubst build/%.h,assets/%,$@)
OBJ_OUT=$@
%.obj.h: $(OBJS)
	$(OBJ_PATH) $(OBJ) > $(OBJ_OUT)

models: $(OBJS_OUT)

IMG_PATH=$(ARCH_PATH)/imgheader$(PROG_EXT)
IMGS=$(wildcard assets/*.png)
IMGS_OUT=$(patsubst assets/%,build/%.h,$(IMGS))

.SECONDEXPANSION:
IMG=$(patsubst build/%.h,assets/%,$@)
IMG_OUT=$@
%.png.h: $(IMGS)
	$(IMG_PATH) $(IMG) > $(IMG_OUT)

images: $(IMGS_OUT)

SHDR_PATH=$(ARCH_PATH)/shdrheader$(PROG_EXT)
SHDRS=$(wildcard assets/*.glsl)
SHDRS_OUT=$(patsubst assets/%,build/%.h,$(SHDRS))

.SECONDEXPANSION:
SHDR=$(patsubst build/%.h,assets/%,$@)
SHDR_OUT=$@
%.glsl.h: $(IMGS)
	$(SHDR_PATH) $(SHDR) > $(SHDR_OUT)

shaders: $(SHDRS_OUT)

assets: models images shaders

cwcgl:
	$(CC) -shared -fpic -DCWCGL_VERSION=3020 -Ideps/cwcGL/src deps/cwcGL/src/cwcgl.c -framework Cocoa -o build/libcwcGL_$(ARCH).$(LIB_EXT)

app: cwcgl assets
	$(CC) -std=c++11 -lstdc++ -Ideps/ -Ibuild/ -Ideps/glm -Ideps/ode/include -Ideps/ode/build/include -Ideps/cwcGL/src -Ldeps/ode/build -lode -Lbuild -lcwcGL_$(ARCH) $(DEPS) $(SOURCE) -o build/ceelo_$(ARCH)$(PROG_EXT)

clean:
	rm build/*

all: app
default: app

.PHONY: default all app shaders models assets images
