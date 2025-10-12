#version 460 core

#extension GL_ARB_bindless_texture : require

// outs
out vec4 fragColor;

// ins
in vec2 texCoord;
in vec3 vPos;
in vec3 vNorm;

flat in ivec3 instanceMaterialIndices;

layout(binding = 0, std430) readonly buffer TextureHandles { sampler2D textures[]; };

// TODO: remove translation data from lights

struct DirectionalLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 direction;
};

struct PointLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float attenuationConstantTerm;
    float attenuationLinearTerm;
    float attenuationQuadraticTerm;

    vec3 position;
};

struct SpotLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float innerCutOff;
    float outerCutOff;

    float attenuationConstantTerm;
    float attenuationLinearTerm;
    float attenuationQuadraticTerm;

    vec3 position;
    vec3 direction;
};

uniform vec3 viewPos;

#define NUM_DIRECTIONAL 8
layout(binding = 1, std140) uniform DirectionalLights
{
    DirectionalLight dirLights[NUM_DIRECTIONAL];
};
#define NUM_POINT 32
layout(binding = 2, std140) uniform PointLights { PointLight pointLights[NUM_POINT]; };
#define NUM_SPOT 16
layout(binding = 3, std140) uniform SpotLights { SpotLight spotLights[NUM_SPOT]; };

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, int diffuseIdx,
                               int specularIdx)
{
    vec4 diffuseColor = texture(textures[diffuseIdx], texCoord);
    vec4 specularColor = texture(textures[specularIdx], texCoord);

    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // combine results
    vec3 ambient = light.ambient * vec3(diffuseColor);
    vec3 diffuse = light.diffuse * diff * vec3(diffuseColor);
    vec3 specular = light.specular * spec * vec3(specularColor);
    return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int diffuseIdx,
                         int specularIdx)
{
    vec4 diffuseColor = texture(textures[diffuseIdx], texCoord);
    vec4 specularColor = texture(textures[specularIdx], texCoord);

    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = clamp(1.0
                                  / (light.attenuationConstantTerm
                                     + light.attenuationLinearTerm * distance
                                     + light.attenuationQuadraticTerm * (distance * distance)),
                              0.0, 1.0f);
    // combine results
    vec3 ambient = light.ambient * vec3(diffuseColor) * attenuation;
    vec3 diffuse = light.diffuse * diff * vec3(diffuseColor) * attenuation;
    vec3 specular = light.specular * spec * vec3(specularColor) * attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int diffuseIdx,
                        int specularIdx)
{
    vec4 diffuseColor = texture(textures[diffuseIdx], texCoord);
    vec4 specularColor = texture(textures[specularIdx], texCoord);

    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0
                        / (light.attenuationConstantTerm + light.attenuationLinearTerm * distance
                           + light.attenuationQuadraticTerm * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * vec3(diffuseColor);
    vec3 diffuse = light.diffuse * diff * vec3(diffuseColor);
    vec3 specular = light.specular * spec * vec3(specularColor);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = (light.innerCutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    diffuse *= intensity;
    specular *= intensity;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void main()
{
    vec3 effectiveColor = vec3(0.0f, 0.0f, 0.0f);
    vec3 viewDir = normalize(viewPos - vPos);

    for (int d = 0; d < NUM_DIRECTIONAL; ++d)
    {
        effectiveColor += CalculateDirectionalLight(dirLights[d], vNorm, viewDir,
                                                    instanceMaterialIndices.x,
                                                    instanceMaterialIndices.y);
    }

    for (int p = 0; p < NUM_POINT; ++p)
    {
        effectiveColor += CalculatePointLight(pointLights[p], vNorm, vPos, viewDir,
                                              instanceMaterialIndices.x, instanceMaterialIndices.y);
    }

    for (int s = 0; s < NUM_SPOT; ++s)
    {
        effectiveColor += CalculateSpotLight(spotLights[s], vNorm, vPos, viewDir,
                                            instanceMaterialIndices.x, instanceMaterialIndices.y);
    }

    fragColor = vec4(effectiveColor, 1.0f);

    // fragColor = vec4(normalize(instanceMaterialIndices),1.0f);
}