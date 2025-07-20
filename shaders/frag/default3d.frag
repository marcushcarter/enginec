#version 460 core

out vec4 FragColor;

in vec3 v_currentPosition;
in vec3 v_normal;
in vec3 v_color;
in vec2 v_texCoord;

uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform vec4 u_lightColor;
uniform vec3 u_lightPosition;
uniform vec3 u_camPosition;

vec4 pointLight() {
    vec3 lightVec = u_lightPosition - v_currentPosition;
    float dist = length(lightVec);
    float a = 1.0;
    float b = 0.04;
    float inten = 1.0f / (a * dist * dist + b * dist + 1.0f); 

    float ambient = 0.20f;

    vec3 normal = normalize(v_normal);
    vec3 lightDirection = normalize(lightVec);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specularLight = 0.5f;
    vec3 viewDirection = normalize(u_camPosition - v_currentPosition);
    vec3 reflectionDirection = reflect(-u_lightPosition, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8);
    float specular = specAmount * specularLight;

    return (texture(diffuse0, v_texCoord) * (diffuse * inten + ambient) + texture(diffuse0, v_texCoord).r * specular * inten) * u_lightColor;
}

vec4 directLight() {
    float ambient = 0.20f;

    vec3 normal = normalize(v_normal);
    vec3 lightDirection = normalize(vec3(1.0f, 1.0f, 0.0f));
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specularLight = 0.5f;
    vec3 viewDirection = normalize(u_camPosition - v_currentPosition);
    vec3 reflectionDirection = reflect(-u_lightPosition, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8);
    float specular = specAmount * specularLight;

    return (texture(diffuse0, v_texCoord) * (diffuse + ambient) + texture(diffuse0, v_texCoord).r * specular) * u_lightColor;
}

vec4 spotlight() {
    vec3 lightVec = u_lightPosition - v_currentPosition;

    float outerCone = 0.90f;
    float innerCone = 0.95f;
    
    float ambient = 0.20f;

    vec3 normal = normalize(v_normal);
    vec3 lightDirection = normalize(lightVec);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specularLight = 0.5f;
    vec3 viewDirection = normalize(u_camPosition - v_currentPosition);
    vec3 reflectionDirection = reflect(-u_lightPosition, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8);
    float specular = specAmount * specularLight;

    float angle = dot(vec3(0.0f, -1.0f, 0.0f), -lightDirection);
    float inten = clamp((angle - outerCone) / (innerCone - outerCone), 0.0f, 1.0f);

    return (texture(diffuse0, v_texCoord) * (diffuse * inten + ambient) + texture(diffuse0, v_texCoord).r * specular * inten) * u_lightColor;

}

void main() 
{
    FragColor = pointLight();
}