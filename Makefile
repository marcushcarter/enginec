# === Configuration ===
# === Compiler ===
CXX := gcc
CXXFLAGS := -g -lm

# === Paths ===
VULKAN_SDK := C:/VulkanSDK/1.4.313.2
INCLUDES := -Iinclude
LDFLAGS := -Llib -lglfw3dll

# === Files ===
SRCS := src/main.c include/stb_image/stb_image.c include/glad/glad.c \
        include/opengl/shaderClass.c include/opengl/VAO.c include/opengl/VBO.c include/opengl/EBO.c

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