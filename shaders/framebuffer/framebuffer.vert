#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 a_texture;

out vec2 f_texCoord;

void main () {
    gl_Position = vec4(aPos, 0.0, 1.0);
    f_texCoord = a_texture;
}