ifeq ($(OS),Windows_NT)
	PRG_SUFFIX_FLAG:=1
	BIN_ARCH:=win32
else
	PRG_SUFFIX_FLAG:=0
	UNAME:=$(shell uname -s)
	ifeq ($(UNAME),Darwin)
		ARCH:=$(shell uname -m)
		ifeq ($(ARCH),arm64)
			BIN_ARCH:=osx_arm64
		else
			BIN_ARCH:=osx
		endif
	else ifeq ($(UNAME),Linux)
		BIN_ARCH:=linux
	else
		$(error OS not supported by this Makefile)
	endif
endif

CC:=clang
LDFLAGS:=
CFLAGS_INC:=-Ideps -I../deps
CFLAGS:=-g -Wall $(CFLAGS_INC)

SRCS := $(wildcard src/*.c)
PRGS := $(patsubst %.c,%,$(SRCS))
PRG_SUFFIX=.exe
BINS := $(patsubst %,%$(PRG_SUFFIX),$(PRGS))
OBJS := $(patsubst %,%.o,$(PRGS))
ifeq ($(PRG_SUFFIX_FLAG),0)
	OUTS = $(PRGS)
else
	OUTS = $(BINS)
endif

default: $(BINS)

.SECONDEXPANSION:
OBJ = $(patsubst %$(PRG_SUFFIX),%.o,$@)
ifeq ($(PRG_SUFFIX_FLAG),0)
	BIN = $(patsubst %$(PRG_SUFFIX),%,$@)
else
	BIN = $@
endif
%$(PRG_SUFFIX): $(OBJS)
	$(CC) $(OBJ) $(LDFLAGS) -o $(BIN)
	mv $(BIN) bin/$(BIN_ARCH)

.PHONY: default
