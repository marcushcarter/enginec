#version 460 core

in vec4 v_color;
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform vec2 resolution;
uniform float intensity;

out vec4 FragColor;

float vignette(vec2 uv) {
    vec2 dist = uv - 0.5;
    return smoothstep(0.8, 0.3, dot(dist, dist) * 1.5);
}

vec3 applyScanlines(vec2 uv, vec3 color) {
    float scanline = sin(uv.y * resolution.y * 3.1415);
    float strength = 0.04;
    return color - strength * vec3(scanline);
}

vec3 chromaticAberration(vec2 uv) {
    float offset = 1.0 / resolution.x * 1.2;
    float scanlineOffset = sin(uv.y * resolution.y * 3.1415) * 0.0005;

    float r = texture(u_texture, uv + vec2(offset, scanlineOffset)).r;
    float g = texture(u_texture, uv).g;
    float b = texture(u_texture, uv - vec2(offset, scanlineOffset)).b;

    return vec3(r, g, b);
}

void main() {
    vec2 uv = v_texCoord;

    vec3 color = chromaticAberration(uv);

    color = applyScanlines(uv, color);

    color *= vignette(uv);

    FragColor = vec4(color, 1);
}