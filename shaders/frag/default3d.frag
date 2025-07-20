#version 460 core

out vec4 FragColor;

in vec3 v_color;
in vec2 v_texCoord;
in vec3 v_normal;
in vec3 v_currentPosition;

uniform sampler2D u_texture0;
uniform sampler2D u_texture1;
uniform vec4 u_lightColor;
uniform vec3 u_lightPosition;
uniform vec3 u_camPosition;

void main() 
{
    float ambient = 0.20f;

    vec3 normal = normalize(v_normal);
    vec3 lightDirection = normalize(u_lightPosition - v_currentPosition);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specularLight = 0.5f;
    vec3 viewDirection = normalize(u_camPosition - v_currentPosition);
    vec3 reflectionDirection = reflect(-u_lightPosition, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8);
    float specular = specAmount * specularLight;

    FragColor = texture(u_texture0, v_texCoord) * u_lightColor * (diffuse + ambient) + texture(u_texture0, v_texCoord);
}