#version 460 core

out vec4 FragColor;
in vec2 f_texCoord;

uniform sampler2D u_texture;

void main() {
    FragColor = texture(u_texture, f_texCoord);
}