# === Configuration ===
# === Compiler ===
CXX := gcc
CXXFLAGS := -g -lm 

# === Paths ===
INCLUDES := -Iinclude
LDFLAGS := -Llib -lglfw3dll

# === Files ===
SRCS := src/main.c $(wildcard include/stb_image/*.c) $(wildcard include/glad/*.c) $(wildcard include/opengl/*.c)

OUT := opengl.exe

# === Targets ===

compile: test run

test:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(OUT) $(LDFLAGS)

build: $(SRCS)
	$(CXX) -mwindows $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(OUT) $(LDFLAGS)
	"C:/Program Files/Git/bin/git.exe" add .
#	"C:/Program Files/Git/bin/git.exe" restore --staged Makefile

run:
	./$(OUT)

clean:
	rm -f $(OUT)

.PHONY: build run clean compile