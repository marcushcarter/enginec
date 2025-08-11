# === Configuration ===
# === Compiler ===
CC := gcc
CXX := g++
CXXFLAGS := -g -lm 

# === Paths ===
INCLUDES := -Iinclude
LDFLAGS := -Llib -lglfw3dll

# === Files ===
SRCS := $(wildcard src/*.c) $(wildcard include/engine/*.c) 
OPENGL_SRCS := $(wildcard include/stb_image/*.c) $(wildcard include/glad/*.c)
NUKLEAR_SRC := $(wildcard include/nuklear/*.c)

ALL_SRCS:= $(SRCS) $(OPENGL_SRCS) $(NUKLEAR_SRC)

OUT := opengl

# === Targets ===

compile: test run

test:
	$(CC) $(CXXFLAGS) $(INCLUDES) $(ALL_SRCS) -o $(OUT) $(LDFLAGS)

build:
	$(CC) -mwindows $(CXXFLAGS) $(INCLUDES) $(ALL_SRCS) -o $(OUT) $(LDFLAGS)
#	"C:/Program Files/Git/bin/git.exe" add .
#	"C:/Program Files/Git/bin/git.exe" restore --staged Makefile

run:
	./$(OUT)

clean:
	rm -f $(OUT)

.PHONY: build run clean compile