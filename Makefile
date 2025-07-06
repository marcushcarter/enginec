# === Configuration ===
# === Compiler ===
CXX := g++
CXXFLAGS := -g

# === Paths ===
VULKAN_SDK := C:/VulkanSDK/1.4.313.2
INCLUDES := -Iinclude
LDFLAGS := -Llib -lglfw3dll

# === Files ===
SRCS := src/main.c include/stb_image/stb_image.c include/glad/glad.c
OUT := opengl.exe

# === Targets ===

compile: build run

build: $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(OUT) $(LDFLAGS)
	"C:/Program Files/Git/bin/git.exe" add .

run:
	./$(OUT)

clean:
	rm -f $(OUT)

.PHONY: build run clean compile