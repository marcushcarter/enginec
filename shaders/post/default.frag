#version 460 core

in vec4 v_color;
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform vec2 resolution;
uniform float intensity;

out vec4 FragColor;

void main() {
    FragColor = texture(u_texture, v_texCoord);
}