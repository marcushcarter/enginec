#version 460 core

out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D screenTexture;

void main() {
    vec3 fragment = texture(screenTexture, texCoord).rgb;

    float exposure = 0.1f;
    vec3 toneMapped = vec3(1.0f) - exp(-fragment * exposure);

    FragColor.rgb = toneMapped;
}