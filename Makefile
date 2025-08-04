# === Configuration ===
# === Compiler ===
CXX := gcc
CXXFLAGS := -g -lm 

# === Paths ===
INCLUDES := -Iinclude
LDFLAGS := -Llib -lglfw3dll

# === Files ===
SRCS:= $(wildcard src/*.c) $(wildcard include/stb_image/*.c) $(wildcard include/glad/*.c) $(wildcard include/opengl/*.c)
IMGUI_SRCS:= $(wildcard include/imgui/*.cpp)
CIMGUI_SRCS:= $(wildcard include/cimgui/*.cpp)

ALL_SRCS:= $(SRCS) $(IMGUI_SRCS) $(CIMGUI_SRCS)

OUT := opengl.exe

# === Targets ===

compile: test run

test:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(ALL_SRCS) -o $(OUT) $(LDFLAGS)

build:
	$(CXX) -mwindows $(CXXFLAGS) $(INCLUDES) $(ALL_SRCS) -o $(OUT) $(LDFLAGS)
#	"C:/Program Files/Git/bin/git.exe" add .
#	"C:/Program Files/Git/bin/git.exe" restore --staged Makefile

run:
	./$(OUT)

clean:
	rm -f $(OUT)

.PHONY: build run clean compile