#version 460 core

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D spriteTexture;
uniform vec3 spriteColor;

void main()
{
    vec4 texColor = texture(spriteTexture, texCoord);
    FragColor = vec4(spriteColor, 1.0) * texColor;

    if(FragColor.a < 0.1)
        discard;
}
