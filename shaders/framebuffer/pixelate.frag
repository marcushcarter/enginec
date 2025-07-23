
#version 460 core

out vec4 FragColor;
in vec2 f_texCoord;

uniform sampler2D u_texture;
uniform vec2 resolution;
uniform float pixelSize;

void main() {

    vec2 blockUV = vec2(
        floor(f_texCoord.x * resolution.x / pixelSize) * pixelSize / resolution.x,
        floor(f_texCoord.y * resolution.y / pixelSize) * pixelSize / resolution.y
    );

    vec3 color = texture(u_texture, blockUV).rgb;

    FragColor = vec4(color, 1);
}