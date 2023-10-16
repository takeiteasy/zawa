ifeq ($(OS),Windows_NT)
	PROG_EXT=.exe
	PRG_SUFFIX_FLAG=1
	LIB_EXT=dll
	DEPS=-lkernel32 -luser32 -lshell32 -lSDL2
	ARCH=win32
else
	UNAME:=$(shell uname -s)
	PROG_EXT=
	PRG_SUFFIX_FLAG=0
	ifeq ($(UNAME),Darwin)
		ARCH:=$(shell uname -m)
		LIB_EXT=dylib
		ifeq ($(ARCH),arm64)
			ARCH=osx_arm64
			DEPS=-I/opt/homebrew/include -L/opt/homebrew/lib
		else
			ARCH=osx
			DEPS=-I/usr/local/include -L/usr/local/lib
		endif
		DEPS+=-lSDL2 -framework OpenGL
	else ifeq ($(UNAME),Linux)
		DEPS=-lGL -ldl -lm -lSDL2
		ARCH=linux
		LIB_EXT=so
	else
		$(error OS not supported by this Makefile)
	endif
endif

SOURCE=$(wildcard src/*.cpp)

ARCH_PATH=tools/bin/$(ARCH)
MODEL_PATH=./build/objheader$(PROG_EXT)
MODELS=$(wildcard assets/*.obj)
MODELS_OUT=$(patsubst assets/%,build/%.h,$(MODELS))

.SECONDEXPANSION:
MODEL=$(patsubst build/%.h,assets/%,$@)
MODEL_OUT=$@
%.obj.h: $(MODELS)
	$(MODEL_PATH) $(MODEL) > $(MODEL_OUT)

models: $(MODELS_OUT)

IMG_PATH=./build/imgheader$(PROG_EXT)
IMGS=$(wildcard assets/*.png)
IMGS_OUT=$(patsubst assets/%,build/%.h,$(IMGS))

.SECONDEXPANSION:
IMG=$(patsubst build/%.h,assets/%,$@)
IMG_OUT=$@
%.png.h: $(IMGS)
	$(IMG_PATH) $(IMG) > $(IMG_OUT)

images: $(IMGS_OUT)

SHDR_PATH=./build/shdrheader$(PROG_EXT)
SHDRS=$(wildcard assets/*.glsl)
SHDRS_OUT=$(patsubst assets/%,build/%.h,$(SHDRS))

.SECONDEXPANSION:
SHDR=$(patsubst build/%.h,assets/%,$@)
SHDR_OUT=$@
%.glsl.h: $(IMGS)
	$(SHDR_PATH) $(SHDR) > $(SHDR_OUT)

shaders: $(SHDRS_OUT)

assets: models images shaders

TOOLS := $(wildcard tools/*.c)
PRGS := $(patsubst %.c,%,$(TOOLS))
PRG_SUFFIX=.exe
BINS := $(patsubst tools/%,build/%$(PRG_SUFFIX),$(PRGS))
OBJS := $(patsubst %,%.o,$(PRGS))
ifeq ($(PRG_SUFFIX_FLAG),0)
	OUTS = $(PRGS)
else
	OUTS = $(BINS)
endif

CFLAGS:=-Ideps $(CFLAGS)

.SECONDEXPANSION:
OBJ = $(patsubst build/%$(PRG_SUFFIX),tools/%.o,$@)
ifeq ($(PRG_SUFFIX_FLAG),0)
	BIN = $(patsubst %$(PRG_SUFFIX),%,$@)
else
	BIN = $@
endif
%$(PRG_SUFFIX): $(OBJS)
	$(CC) $(OBJ) -o $(BIN)

tools: $(BINS)

clean:
	rm tools/*.o | true

veryclean:
	rm build/* | true

default:
	$(CC) -std=c++11 -lstdc++ -Ideps/ -Ibuild/ -Ideps/glm -Lbuild $(DEPS) $(SOURCE) deps/glad.cpp -o build/zawa_$(ARCH)$(PROG_EXT)

all: veryclean tools assets default clean

.PHONY: default all veryclean shaders models assets images
