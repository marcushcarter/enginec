#version 460 core

in vec2 TexCoord;

uniform sampler2DArray spriteTexture;
uniform int layerIndex;
uniform vec4 colorTint;

out vec4 FragColor;

void main() {
    // Sample from the correct layer in the texture array
    vec4 texColor = texture(spriteTexture, vec3(TexCoord, layerIndex));
    
    // Apply color tint (including alpha)
    FragColor = texColor * colorTint;

    // Optional: discard transparent pixels (uncomment if desired)
    // if (FragColor.a < 0.1)
    //     discard;
}
