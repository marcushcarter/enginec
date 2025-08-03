#version 460 core

layout(location = 0) in vec2 aPos;     // Vertex position (quad corners)
layout(location = 1) in vec2 aTexCoord; // Texture coordinate

uniform mat4 projection;
uniform mat4 model;

out vec2 TexCoord;

void main() {
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
