#version 460 core

out vec4 FragColor;

uniform vec4 u_lightColor;

void main() 
{
    FragColor = u_lightColor;
}