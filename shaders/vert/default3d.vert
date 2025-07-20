#version 460 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_color;
layout (location = 3) in vec2 a_texCoord;

out vec3 v_color;
out vec2 v_texCoord;
out vec3 v_normal;
out vec3 v_currentPosition;

uniform mat4 u_model;
uniform mat4 u_camMatrix;

void main() 
{
   v_currentPosition = vec3(u_model * vec4(a_position, 1.0f));
   gl_Position = u_camMatrix * vec4(v_currentPosition, 1.0f);
   v_normal = a_normal;
   v_color = a_color;
   v_texCoord = a_texCoord;
}