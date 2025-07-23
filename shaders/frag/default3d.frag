#version 460 core

out vec4 FragColor;

in vec3 crntPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;
in vec4 fragPosLight;

uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D shadowMap;
uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;

vec4 pointLight() {
    vec3 lightVec = lightPos - crntPos;
    float dist = length(lightVec);
    float a = 1.0;
    float b = 0.04;
    float inten = 1.0f / (a * dist * dist + b * dist + 1.0f); 

    float ambient = 0.20f;

    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(lightVec);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specular = 0.0f;
    if (diffuse != 0.0f) {
        float specularLight = 0.5f;
        vec3 viewDirection = normalize(camPos - crntPos);
        vec3 reflectionDirection = reflect(-lightPos, normal);
        vec3 halfwayVec = normalize(viewDirection + lightDirection);
        float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 8);
        float specular = specAmount * specularLight;
    };

    return (texture(diffuse0, texCoord) * (diffuse * inten + ambient) + texture(diffuse0, texCoord).r * specular * inten) * lightColor;
}

vec4 directLight() {
    float ambient = 0.20f;

    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(vec3(1.0f, 1.0f, 0.0f));
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specular = 0.0f;
    if (diffuse != 0.0f) {
        float specularLight = 0.5f;
        vec3 viewDirection = normalize(camPos - crntPos);
        vec3 reflectionDirection = reflect(-lightPos, normal);
        vec3 halfwayVec = normalize(viewDirection + lightDirection);
        float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 8);
        float specular = specAmount * specularLight;
    };

    float shadow = 0.0f;
    vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
    if(lightCoords.z <= 1.0f) 
    {
        lightCoords = (lightCoords + 1.0f) / 2.0f;
        float currentDepth = lightCoords.z;
        float bias = max(0.025f * (1.0f - dot(normal, lightDirection)), 0.0005f);

        int sampleRadius = 2;
        vec2 pixelSize = 1.0 / textureSize(shadowMap, 0);
        for (int y = -2; y <= 2; y++) 
        {
            for (int x = -2; x <= 2; x++) 
            {
                float closestDepth = texture(shadowMap, lightCoords.xy + vec2(x, y) * pixelSize).r;
                if (currentDepth > closestDepth + bias) 
                {
                    shadow += 1.0f;
                }
            }
        }
    }

    shadow /= pow((2 * 2 + 1), 2);

    return (texture(diffuse0, texCoord) * (diffuse * (1.0f - shadow) + ambient) + texture(diffuse0, texCoord).r * specular * (1.0f - shadow)) * lightColor;
}

vec4 spotlight() {
    vec3 lightVec = lightPos - crntPos;

    float outerCone = 0.90f;
    float innerCone = 0.95f;
    
    float ambient = 0.20f;

    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(lightVec);
    float diffuse = max(dot(normal, lightDirection), 0.0f);
    
    float specular = 0.0f;
    if (diffuse != 0.0f) {
        float specularLight = 0.5f;
        vec3 viewDirection = normalize(camPos - crntPos);
        vec3 reflectionDirection = reflect(-lightPos, normal);
        vec3 halfwayVec = normalize(viewDirection + lightDirection);
        float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 8);
        float specular = specAmount * specularLight;
    };

    float angle = dot(vec3(0.0f, -1.0f, 0.0f), -lightDirection);
    float inten = clamp((angle - outerCone) / (innerCone - outerCone), 0.0f, 1.0f);

    return (texture(diffuse0, texCoord) * (diffuse * inten + ambient) + texture(diffuse0, texCoord).r * specular * inten) * lightColor;
}

float near = 0.1f;
float far = 10.0f;

float linearizeDepth(float depth) {
    return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

float logisticDepth(float depth) {
    float steepness = 0.5f;
    float offset = 5.0f;
    float zVal = linearizeDepth(depth);
    return (1/ (1 + exp(-steepness * (zVal - offset))));
}

void main() 
{
     FragColor = directLight();
    float depth = logisticDepth(gl_FragCoord.z);
    // FragColor = pointLight() * (1.0f - depth) + vec4(depth * vec3(0.85f, 0.85f, 0.90f), 1.0f);
}