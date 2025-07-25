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

uniform int NR_DIRECT_LIGHTS;
uniform int NR_SPOT_LIGHTS;
uniform int NR_POINT_LIGHTS;
uniform DirectLight directlights[100];
uniform PointLight pointlights[100];
uniform SpotLight spotlights[100];

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

float near = 0.1f;
float far = 100.0f;

float linearizeDepth(float depth) {
    return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

float logisticDepth(float depth) {
    float steepness = 0.5f;
    float offset = 5.0f;
    float zVal = linearizeDepth(depth);
    return (1/ (1 + exp(-steepness * (zVal - offset))));
}

void main() {
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(camPos - crntPos);

    vec3 result = texture(diffuse0, texCoord).rgb * ambient;
    
    for (int i = 0; i < NR_DIRECT_LIGHTS; i++) {
        result += calcDirectLight(directlights[i], normal, viewDir);
    }
    
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += calcPointLight(pointlights[i], normal, viewDir);
    }
    
    for (int i = 0; i < NR_SPOT_LIGHTS; i++) {
        result += calcSpotLight(spotlights[i], normal, viewDir);
    }

    // FragColor = vec4(result, 1.0) * (1.0f - logisticDepth(gl_FragCoord.z)) + vec4(logisticDepth(gl_FragCoord.z) * vec3(0.1f, 0.1f, 0.1f), 1.0f);
    FragColor = vec4(result, 1.0);
}