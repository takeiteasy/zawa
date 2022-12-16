ifeq ($(OS),Windows_NT)
	PROG_EXT=.exe
	CFLAGS=-O2 -DSOKOL_D3D11 -lkernel32 -luser32 -lshell32 -ldxgi -ld3d11 -lole32 -lgdi32
	ARCH=win32
	SHDC_FLAGS=hlsl5
else
	UNAME:=$(shell uname -s)
	PROG_EXT=
	ifeq ($(UNAME),Darwin)
		CFLAGS=-x objective-c -DSOKOL_METAL -fobjc-arc -framework Metal -framework Cocoa -framework MetalKit -framework Quartz -framework AudioToolbox
		ARCH:=$(shell uname -m)
		ifeq ($(ARCH),arm64)
			ARCH=osx_arm64
		else
			ARCH=osx
		endif
		SHDC_FLAGS=metal_macos
	else ifeq ($(UNAME),Linux)
		CFLAGS=-DSOKOL_GLCORE33 -pthread -lGL -ldl -lm -lX11 -lasound -lXi -lXcursor
		ARCH=linux
		SHDC_FLAGS=glsl330
	else
		$(error OS not supported by this Makefile)
	endif
endif
CC=clang
SOURCE=$(wildcard src/*.c)
EXE=build/ceelo_$(ARCH)$(PROG_EXT)
ARCH_PATH=./tools/bin/$(ARCH)

SHDC_PATH=$(ARCH_PATH)/sokol-shdc$(PROG_EXT)
SHADERS=$(wildcard assets/*.glsl)
SHADER_OUTS=$(patsubst %,%.h,$(SHADERS))

all: app

.SECONDEXPANSION:
SHADER=$(patsubst %.h,%,$@)
SHADER_OUT=$@
%.glsl.h: $(SHADERS)
	$(SHDC_PATH) -i $(SHADER) -o $(SHADER_OUT) -l $(SHDC_FLAGS)
	mv $(SHADER_OUT) build/

shaders: $(SHADER_OUTS)

OBJ_PATH=$(ARCH_PATH)/objheader$(PROG_EXT)
OBJS=$(wildcard assets/*.obj)
OBJS_OUT=$(patsubst %,%.h,$(OBJS))

.SECONDEXPANSION:
OBJ=$(patsubst %.h,%,$@)
OBJ_OUT=$@
%.obj.h: $(OBJS)
	$(OBJ_PATH) $(OBJ) > $(OBJ_OUT)
	mv $(OBJ_OUT) build/

models: $(OBJS_OUT)

IMG_PATH=$(ARCH_PATH)/imgheader$(PROG_EXT)
IMGS=$(wildcard assets/*.png)
IMGS_OUT=$(patsubst %,%.h,$(IMGS))

.SECONDEXPANSION:
IMG=$(patsubst %.h,%,$@)
IMG_OUT=$@
%.png.h: $(IMGS)
	$(IMG_PATH) $(IMG) > $(IMG_OUT)
	mv $(IMG_OUT) build/

images: $(IMGS_OUT)

assets: shaders models images

app: assets
	$(CC) -Ibuild -Ideps -fenable-matrix $(CFLAGS) $(SOURCE) -o $(EXE)

run: $(EXE)
	./$(EXE)

.PHONY: all app shaders models run assets images
