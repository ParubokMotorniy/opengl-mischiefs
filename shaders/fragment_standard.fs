#version 460 core

#extension GL_ARB_bindless_texture : require

// outs
out vec4 fragColor;

// ins
in vec2 texCoord;
in vec3 vPos;
in vec3 vNorm;

//TODO: 
// 1. multiply the vPos by the light perspective matrix -> have to sneak the matrix into the shader
// 2. check if the fragment is behind the depth of the bound texture of the light
// 3. modify the diffuse+specular accordingly 

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

    uvec2 shadowTextureHandle;
    uvec2 dummy_buffer_id; //TODO: REMOVE THIS HERESY
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

#define NUM_POINT 16
layout(binding = 2, std140) uniform PointLights { PointLight pointLights[NUM_POINT]; };

#define NUM_SPOT 8
layout(binding = 3, std140) uniform SpotLights { SpotLight spotLights[NUM_SPOT]; };

#define NUM_TEXTURED_SPOT 4
layout(binding = 4, std140) uniform TexturedSpotLights { TexturedSpotLight texturedSpotLights[NUM_TEXTURED_SPOT]; };

float fragmentInDirectionalShadow(DirectionalLight light, vec3 fragWorldPos, vec3 norm)
{
    float bias = 0.003;
    vec3 displacedFragment = fragWorldPos + norm * bias; 
    vec4 ndcPos = light.projectionMatrix * light.viewMatrix * vec4(displacedFragment, 1.0);
    ndcPos /= ndcPos.w;

    ndcPos = ndcPos * 0.5 + 0.5;

    float shadowDepth = texture(sampler2D(light.shadowTextureHandle), ndcPos.xy).r;
    float fragmentDepth = ndcPos.z;
    float shadow = fragmentDepth > shadowDepth ? 1.0 : 0.0;
    return shadow;
}

// vec3 directionalShadowDiffuse(DirectionalLight light, vec4 fragWorldPos, vec3 norm)
// {
//     float bias = 0.003;
//     vec3 displacedFragment = fragWorldPos.xyz + norm * bias; 
//     vec4 ndcPos = light.projectionMatrix * light.viewMatrix * vec4(displacedFragment, 1.0);
//     ndcPos /= ndcPos.w;
//     ndcPos = ndcPos * 0.5 + 0.5;

//     return texture(sampler2D(light.shadowTextureHandle), ndcPos.xy).rgb;
// }

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, int diffuseIdx,
                               int specularIdx)
{
    // vec4 diffuseColor = texture(sampler2D(light.shadowTextureHandle), texCoord);
    // return diffuseColor.rgb;

    // vec3 diffColor = directionalShadowDiffuse(light, vPos, normal);
    // return diffColor;

    vec4 diffuseColor = texture(sampler2D(textures[diffuseIdx]), texCoord);
    vec4 specularColor = texture(sampler2D(textures[specularIdx]), texCoord);

    vec3 lightDir = normalize(-light.direction);
    // diffuse bit
    float diff = max(dot(normal, lightDir), 0.0);
    // specular bit
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    float shadowEffect = 1.0 - fragmentInDirectionalShadow(light, vPos, normal);
    // // combination
    vec3 ambient = light.ambient * vec3(diffuseColor);
    vec3 diffuse = light.diffuse * diff * vec3(diffuseColor) * shadowEffect;
    vec3 specular = light.specular * spec * vec3(specularColor) * shadowEffect;
    return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int diffuseIdx,
                         int specularIdx)
{
    vec4 diffuseColor = texture(sampler2D(textures[diffuseIdx]), texCoord);
    vec4 specularColor = texture(sampler2D(textures[specularIdx]), texCoord);

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
                              0.0, 1.0f);
    // combination
    vec3 ambient = light.ambient * vec3(diffuseColor) * attenuation;
    vec3 diffuse = light.diffuse * diff * vec3(diffuseColor) * attenuation;
    vec3 specular = light.specular * spec * vec3(specularColor) * attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int diffuseIdx,
                        int specularIdx)
{
    vec4 diffuseColor = texture(sampler2D(textures[diffuseIdx]), texCoord);
    vec4 specularColor = texture(sampler2D(textures[specularIdx]), texCoord);

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
                           + light.attenuationQuadraticTerm * (distance * distance)), 0.0f, 1.0f);
    // combination
    vec3 ambient = light.ambient * vec3(diffuseColor);
    vec3 diffuse = light.diffuse * diff * vec3(diffuseColor);
    vec3 specular = light.specular * spec * vec3(specularColor);

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

vec3 CalculateTexturedSpotLight(TexturedSpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int diffuseIdx,
                        int specularIdx)
{
    vec4 diffuseColor = texture(sampler2D(textures[diffuseIdx]), texCoord);
    vec4 specularColor = texture(sampler2D(textures[specularIdx]), texCoord);

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
    mappedFrag /= mappedFrag.z;
    float tCoordX = (mappedFrag.x + 1.0) / 2.0f;
    float tCoordY = (-mappedFrag.y + 1.0) / 2.0f;

    // combination
    vec3 ambient = light.ambient * vec3(diffuseColor);
    vec3 diffuse = texture(sampler2D(light.textureIdx), vec2(tCoordX, tCoordY)).xyz * diff * vec3(diffuseColor);
    vec3 specular = light.specular * spec * vec3(specularColor);

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

    for (int d = 0; d < NUM_DIRECTIONAL; ++d)
    {
        effectiveColor += CalculateDirectionalLight(dirLights[d], normalize(vNorm), viewDir,
                                                    instanceMaterialIndices.x,
                                                    instanceMaterialIndices.y);
    }

    for (int p = 0; p < NUM_POINT; ++p)
    {
        effectiveColor += CalculatePointLight(pointLights[p], normalize(vNorm), vPos, viewDir,
                                              instanceMaterialIndices.x, instanceMaterialIndices.y);
    }

    for (int s = 0; s < NUM_SPOT; ++s)
    {
        effectiveColor += CalculateSpotLight(spotLights[s], normalize(vNorm), vPos, viewDir,
                                            instanceMaterialIndices.x, instanceMaterialIndices.y);
    }

    for (int s = 0; s < NUM_TEXTURED_SPOT; ++s)
    {
        effectiveColor += CalculateTexturedSpotLight(texturedSpotLights[s], normalize(vNorm), vPos, viewDir,
                                            instanceMaterialIndices.x, instanceMaterialIndices.y);
    }

    fragColor = vec4(effectiveColor, 1.0f);

    // fragColor = vec4(normalize(instanceMaterialIndices),1.0f);
}