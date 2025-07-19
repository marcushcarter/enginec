#version 460 core

out vec4 FragColor;

in vec3 v_color;
in vec2 v_texCoord;

uniform sampler2D u_texture;

void main() 
{

    float d = length(v_texCoord * 2.0) - 0.5;

    d -= 0.5;
    d = abs(d);
    
    // 3d shader
    // FragColor = vec4(d, 0.0, 0.0, 1.0f);
    
    // 3d texture
    FragColor = texture(u_texture, v_texCoord);
}