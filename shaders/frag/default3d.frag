#version 460 core

out vec4 FragColor;

in vec3 crntPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;

uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform vec3 camPos;
uniform float ambient;

struct DirectLight {
    vec3 direction;
    vec4 color;
    float specular;
};

struct PointLight {
    vec3 position;
    vec4 color;
    float a;
    float b;
    float specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec4 color;

    float innerCone;
    float outerCone;
    float specular;
};

#define NUM_POINT_LIGHTS 1
#define NUM_SPOT_LIGHTS 1

uniform DirectLight directlight;
uniform PointLight pointlights[NUM_POINT_LIGHTS];
uniform SpotLight spotlights[NUM_SPOT_LIGHTS];

vec3 calcDirectLight(DirectLight light, vec3 normal, vec3 viewDirection) {

    vec3 lightDirection = normalize(-light.direction);

    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specular = 0.0f;
    if (diffuse != 0.0f) {
		float specularLight = light.specular;
		vec3 halfwayVec = normalize(viewDirection + lightDirection);
		float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 16);
		specular = specAmount * specularLight;
    };

    return (texture(diffuse0, texCoord) * diffuse + texture(diffuse0, texCoord).r * specular) * light.color;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDirection) {
    
    float dist = length(light.position - crntPos);
    float inten = 1.0f / (light.a * dist * dist + light.b * dist + 1.0f); 

    vec3 lightDirection = normalize(light.position - crntPos);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specular = 0.0f;
    if (diffuse != 0.0f) {
		float specularLight = light.specular;
        vec3 reflectionDirection = reflect(-light.position, normal);
        vec3 halfwayVec = normalize(viewDirection + lightDirection);
        float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 16);
        specular = specAmount * specularLight;
    };

    return (texture(diffuse0, texCoord) * (diffuse * inten) + texture(diffuse0, texCoord).r * specular * inten) * light.color;
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 viewDirection) {

    vec3 lightDirection = normalize(light.position - crntPos);
    float diffuse = max(dot(normal, lightDirection), 0.0f);
    
    float specular = 0.0f;
    if (diffuse != 0.0f) {
        float specularLight = light.specular;
        vec3 halfwayVec = normalize(viewDirection + lightDirection);
        float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 8);
        specular = specAmount * specularLight;
    };

    float angle = dot(normalize(-light.direction), lightDirection);
    float inten = clamp((angle - light.outerCone) / (light.outerCone - light.innerCone), 0.0f, 1.0f);

    return (texture(diffuse0, texCoord) * (diffuse * inten) + texture(diffuse0, texCoord).r * specular * inten) * light.color;
}

void main() {
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(camPos - crntPos);

    vec3 result = texture(diffuse0, texCoord).rgb * ambient;

    result += calcDirectLight(directlight, normal, viewDir);
    
    for (int i = 0; i < NUM_POINT_LIGHTS; i++) {
        result += calcPointLight(pointlights[i], normal, viewDir);
    }
    
    for (int i = 0; i < NUM_SPOT_LIGHTS; i++) {
        result += calcSpotLight(spotlights[i], normal, viewDir);
    }

    FragColor = vec4(result, 1.0);
}