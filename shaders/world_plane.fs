#version 460 core

#extension GL_ARB_bindless_texture : require

uniform sampler2D planeTexture;
uniform float checkerUnitWidth;
uniform float checkerUnitHeight;
uniform vec3 viewPos;

in vec3 fragmentPos;
in vec3 vNorm;

out vec4 fragColor;

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

#define NUM_DIRECTIONAL 8
layout(binding = 1, std140) uniform DirectionalLights
{
    DirectionalLight dirLights[NUM_DIRECTIONAL];
};
uniform int numDirectionalLightsBound;

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

vec3 CalculateDirectionalLight(DirectionalLight light, int lightIdx, vec3 planeDiffuse, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse bit
    float diff = max(dot(normal, lightDir), 0.0);
    // specular bit
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    float shadowEffect = 1.0 - fragmentInDirectionalShadow(light, lightIdx, fragmentPos, normal);
    // return vec3(shadowEffect,shadowEffect,shadowEffect) * diff;

    // combination
    vec3 ambient = light.ambient * vec3(planeDiffuse);
    vec3 diffuse = light.diffuse * diff * vec3(planeDiffuse) * shadowEffect;
    return (ambient + diffuse);
}

void main()
{
    float texXCoord = mod(fragmentPos.x, checkerUnitWidth) / checkerUnitWidth;
    float texZCoord = mod(fragmentPos.z, checkerUnitHeight) / checkerUnitHeight;
    vec4 texDiffuseColor = texture(planeTexture, vec2(texXCoord, texZCoord));

    vec3 effectiveColor = vec3(0.0f, 0.0f, 0.0f);
    vec3 viewDir = normalize(viewPos - fragmentPos);

    for (int d = 0; d < numDirectionalLightsBound; ++d)
    {
        effectiveColor += CalculateDirectionalLight(dirLights[d], d, texDiffuseColor.xyz, normalize(vNorm), viewDir);
    }

    fragColor = vec4(effectiveColor, 1.0);
}
