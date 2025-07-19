#version 460 core

in vec4 v_color;
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform vec2 resolution;
uniform float intensity;
uniform float pixelSize;

out vec4 FragColor;

void main() {

    vec2 blockUV = vec2(
        floor(v_texCoord.x * resolution.x / pixelSize) * pixelSize / resolution.x,
        floor(v_texCoord.y * resolution.y / pixelSize) * pixelSize / resolution.y
    );

    vec3 color = texture(u_texture, blockUV).rgb;

    FragColor = vec4(color, 1);
}