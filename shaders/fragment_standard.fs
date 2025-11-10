#version 460 core

#extension GL_ARB_bindless_texture : require

// outs
out vec4 fragColor;

// ins
in vec2 texCoord;
in vec3 vPos; //fragment world position
in vec3 vNorm; //fragment world normal

flat in ivec3 instanceMaterialIndices;

layout(binding = 0, std430) readonly buffer TextureHandles { uvec2 textures[]; };

struct DirectionalLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 direction;
    vec3 position;

    mat4 viewMatrix;
    mat4 projectionMatrix;

    //TODO: these ideally shouldn't be here
    uvec2 shadowMapIdentifier;
    uvec2 frameBufferId;
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

struct TexturedSpotLight
{
    vec3 ambient;
    
    vec3 specular;
    float innerCutOff;
    
    float outerCutOff;
    float attenuationConstantTerm;
    float attenuationLinearTerm;
    float attenuationQuadraticTerm;
    
    vec3 position;
    vec3 direction;

    mat4 lightView;
    mat4 lightProj;

    uvec2 textureIdx;
};

uniform vec3 viewPos;

#define NUM_DIRECTIONAL 8
layout(binding = 1, std140) uniform DirectionalLights
{
    DirectionalLight dirLights[NUM_DIRECTIONAL];
};

#define NUM_POINT 8
layout(binding = 2, std140) uniform PointLights { PointLight pointLights[NUM_POINT]; };

#define NUM_SPOT 8
layout(binding = 3, std140) uniform SpotLights { SpotLight spotLights[NUM_SPOT]; };

#define NUM_TEXTURED_SPOT 4
layout(binding = 4, std140) uniform TexturedSpotLights { TexturedSpotLight texturedSpotLights[NUM_TEXTURED_SPOT]; };

uniform sampler2DShadow directionalShadowMaps[NUM_DIRECTIONAL];
uniform float directionalShadowBias;

float fragmentInDirectionalShadow(DirectionalLight light, int lightIdx, vec3 fragWorldPos, vec3 norm)
{
    vec3 displacedFragment = fragWorldPos.xyz + norm * directionalShadowBias; 
    vec4 ndcPos = light.projectionMatrix * light.viewMatrix * vec4(displacedFragment, 1.0);
    ndcPos /= ndcPos.w;

    ndcPos = ndcPos * 0.5 + 0.5;
    float fragmentDepth = ndcPos.z;

    if(ndcPos.z > 1.0)
        return 0.0;

    //hardware pcf 
    float shadowDepth = texture(directionalShadowMaps[lightIdx], ndcPos.xyz).r;
    return 1.0 - shadowDepth;

    //software pcf
    // vec2 texelSize = 1.0 / textureSize(directionalShadowMaps[lightIdx], 0);
    // float shadow = 0.0;
    // for(int x = -1; x <= 1; ++x)
    // {
    //     for(int y = -1; y <= 1; ++y)
    //     {
    //         float pcfDepth = texture(directionalShadowMaps[lightIdx], ndcPos.xy + vec2(x, y) * texelSize).r; 
    //         shadow += fragmentDepth > pcfDepth ? 1.0 : 0.0;        
    //     }    
    // }
    // shadow /= 9.0;
    // return shadow;

    // float shadowDepth = texture(directionalShadowMaps[lightIdx], ndcPos.xy).r;
    // float shadow = fragmentDepth > shadowDepth ? 1.0 : 0.0;
    // return shadow;
}

vec3 CalculateDirectionalLight(DirectionalLight light, int lightIdx, vec3 normal, vec3 viewDir, vec3 diffuseColor,
                               vec3 ambientColor, vec3 specularColor)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse bit
    float diff = max(dot(normal, lightDir), 0.0);
    // specular bit
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    float shadowEffect = 1.0 - fragmentInDirectionalShadow(light, lightIdx, vPos, normal);
    // // combination
    vec3 ambient = light.ambient * ambientColor;
    vec3 diffuse = light.diffuse * diff * diffuseColor * shadowEffect;
    vec3 specular = light.specular * spec * specularColor * shadowEffect;
    return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor,
                               vec3 ambientColor, vec3 specularColor)
{
    vec3 fragToLight = normalize(light.position - fragPos);
    // diffuse bit
    float diff = max(dot(normal, fragToLight), 0.0);
    // specular bit
    vec3 reflectDir = reflect(-fragToLight, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation computation
    float distance = length(light.position - fragPos);
    float attenuation = clamp(1.0
                                  / (light.attenuationConstantTerm
                                     + light.attenuationLinearTerm * distance
                                     + light.attenuationQuadraticTerm * (distance * distance)),
                              0.0, 1.0);
    // combination
    vec3 ambient = light.ambient * ambientColor * attenuation;
    vec3 diffuse = light.diffuse * diff *diffuseColor * attenuation;
    vec3 specular = light.specular * spec * specularColor * attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor,
                               vec3 ambientColor, vec3 specularColor)
{
    vec3 fragToLight = normalize(light.position - fragPos);
    // diffuse bit
    float diff = max(dot(normal, fragToLight), 0.0);
    // specular bit
    vec3 reflectDir = reflect(-fragToLight, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation computation
    float distance = length(light.position - fragPos);
    float attenuation = clamp(1.0
                        / (light.attenuationConstantTerm + light.attenuationLinearTerm * distance
                           + light.attenuationQuadraticTerm * (distance * distance)), 0.0f, 1.0);
    // combination
    vec3 ambient = light.ambient * ambientColor;
    vec3 diffuse = light.diffuse * diff * diffuseColor;
    vec3 specular = light.specular * spec * specularColor;

    float theta = dot(fragToLight, normalize(-light.direction));
    float epsilon = (light.innerCutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    diffuse *= intensity;
    specular *= intensity;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalculateTexturedSpotLight(TexturedSpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor,
                               vec3 ambientColor, vec3 specularColor)
{
    vec3 fragToLight = normalize(light.position - fragPos);
    vec3 lightToFrag = -fragToLight;
    // diffuse bit
    float diff = max(dot(normal, fragToLight), 0.0);
    // specular bit
    vec3 reflectDir = reflect(-fragToLight, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // attenuation computation
    float distance = length(fragToLight);
    float attenuation = clamp(1.0
                        / (light.attenuationConstantTerm + light.attenuationLinearTerm * distance
                           + light.attenuationQuadraticTerm * (distance * distance)), 0.0f, 1.0f);

    //projected texture sampling
    vec4 mappedFrag = light.lightProj * light.lightView * vec4(fragPos, 1.0f);
    mappedFrag /= mappedFrag.w;
    float tCoordX = (mappedFrag.x + 1.0) / 2.0f;
    float tCoordY = (-mappedFrag.y + 1.0) / 2.0f;

    // combination
    vec3 diffuse = texture(sampler2D(light.textureIdx), vec2(tCoordX, tCoordY)).xyz * diff * diffuseColor;
    vec3 ambient = light.ambient * ambientColor;
    vec3 specular = light.specular * spec * specularColor;

    float theta = dot(lightToFrag, normalize(light.direction));
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
    vec3 viewDir = normalize(viewPos - vPos.xyz);

    //emission textures are currently ignored

    vec3 diffuseColor = instanceMaterialIndices.x == -1 ? vec3(0.0) : texture(sampler2D(textures[instanceMaterialIndices.x]), texCoord).xyz;

    vec3 specularColor = instanceMaterialIndices.y == -1 ? vec3(0.0) : texture(sampler2D(textures[instanceMaterialIndices.y]), texCoord).xyz;

    vec3 ambientColor = instanceMaterialIndices.x == -1 ? vec3(0.0) : texture(sampler2D(textures[instanceMaterialIndices.x]), texCoord).xyz;

    //TODO: find a reliable way of telling shader how many lights are currently bound

    for (int d = 0; d < 2; ++d)
    {
        effectiveColor += CalculateDirectionalLight(dirLights[d], d, normalize(vNorm), viewDir, diffuseColor, ambientColor, specularColor);
    }

    for (int p = 0; p < 2; ++p)
    {
        effectiveColor += CalculatePointLight(pointLights[p], normalize(vNorm), vPos, viewDir,
                                              diffuseColor, ambientColor, specularColor);
    }

    for (int s = 0; s < 2; ++s)
    {
        effectiveColor += CalculateSpotLight(spotLights[s], normalize(vNorm), vPos, viewDir,
                                            diffuseColor, ambientColor, specularColor);
    }

    for (int s = 0; s < 1; ++s)
    {
        effectiveColor += CalculateTexturedSpotLight(texturedSpotLights[s], normalize(vNorm), vPos, viewDir,
                                            diffuseColor, ambientColor, specularColor);
    }

    fragColor = vec4(effectiveColor, 1.0f);

    // fragColor = vec4(normalize(instanceMaterialIndices),1.0f);
}