
#version 460 core

out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D screenTexture;
uniform vec2 resolution;
uniform float pixelSize;

void main() {

    vec2 blockUV = vec2(
        floor(texCoord.x * resolution.x / pixelSize) * pixelSize / resolution.x,
        floor(texCoord.y * resolution.y / pixelSize) * pixelSize / resolution.y
    );

    vec3 color = texture(screenTexture, blockUV).rgb;

    FragColor = vec4(color, 1);
}