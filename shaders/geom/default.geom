#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 color[];
in vec2 texCoord[];
in vec3 Normal[];
in vec3 crntPos[];

out vec3 g_color;
out vec2 g_texCoord;
out vec3 g_Normal;
out vec3 g_currentPosition;

void main()
{
    for (int i = 0; i < 3; ++i)
    {
        g_color = color[i];
        g_texCoord = texCoord[i];
        g_Normal = Normal[i];
        g_currentPosition = crntPos[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
