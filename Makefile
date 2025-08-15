# === Configuration ===
# === Compiler ===
CC := gcc
CXX := g++
CXXFLAGS := -g -lm 

# === Paths ===
INCLUDES := -Iinclude -I.
LDFLAGS := -Llib -lglfw3dll

# === Files ===
SRCS := src/main.c #$(wildcard src/*.c) 
ENGINE_SCRS := $(wildcard engine/*.c)
OPENGL_SRCS := $(wildcard include/stb_image/*.c) $(wildcard include/glad/*.c)
NUKLEAR_SRC := $(wildcard include/nuklear/*.c)

ALL_SRCS:= $(SRCS) $(OPENGL_SRCS) $(NUKLEAR_SRC) $(ENGINE_SCRS)

OUT := opengl

# === Targets ===

compile: test run

test:
	$(CC) $(CXXFLAGS) $(INCLUDES) $(ALL_SRCS) -o $(OUT) $(LDFLAGS)

build:
	$(CC) -mwindows $(CXXFLAGS) $(INCLUDES) $(ALL_SRCS) -o $(OUT) $(LDFLAGS)
#	"C:/Program Files/Git/bin/git.exe" add .
#	"C:/Program Files/Git/bin/git.exe" restore --staged Makefile

library:
	$(CC) -c $(INCLUDES) engine/engine.c 
	ar rcs libengine.a engine.o

run:
	./$(OUT)

clean:
	rm -f $(OUT)

.PHONY: build run clean compile