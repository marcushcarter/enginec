#version 460 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uFontAtlas;
uniform vec3 uTextColor;

void main() {
    float alpha = texture(uFontAtlas, vUV).r; // red channel stores glyph
    FragColor = vec4(uTextColor, alpha);
}