#version 460 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_color;
layout (location = 2) in vec2 a_texCoord;

out vec3 v_color;
out vec2 v_texCoord;

uniform mat4 u_model;
uniform mat4 u_camMatrix;

void main() 
{
   gl_Position = u_camMatrix * vec4(a_position, 1.0f);
   v_color = a_color;
   v_texCoord = a_texCoord;
}