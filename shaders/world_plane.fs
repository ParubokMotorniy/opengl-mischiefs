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

    uvec2 shadowTextureHandle;
    uvec2 dummy_buffer_id; //TODO: REMOVE THIS HERESY
};

#define NUM_DIRECTIONAL 8
layout(binding = 1, std140) uniform DirectionalLights
{
    DirectionalLight dirLights[NUM_DIRECTIONAL];
};

float fragmentInDirectionalShadow(DirectionalLight light, vec3 fragWorldPos, vec3 norm)
{
    float bias = 0.005;
    vec3 displacedFragment = fragWorldPos.xyz + norm * bias; 
    vec4 ndcPos = light.projectionMatrix * light.viewMatrix * vec4(displacedFragment, 1.0);
    ndcPos /= ndcPos.w;

    ndcPos = ndcPos * 0.5 + 0.5;

    float shadowDepth = texture(sampler2D(light.shadowTextureHandle), ndcPos.xy).r;
    float fragmentDepth = ndcPos.z;
    float shadow = fragmentDepth > shadowDepth ? 1.0 : 0.0;
    return shadow;
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 planeDiffuse, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse bit
    float diff = max(dot(normal, lightDir), 0.0);
    // specular bit
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    float shadowEffect = 1.0 - fragmentInDirectionalShadow(light, fragmentPos, normal);
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
 
    // fragColor = texture(planeTexture, vec2(texXCoord, texZCoord));

    vec3 effectiveColor = vec3(0.0f, 0.0f, 0.0f);
    vec3 viewDir = normalize(viewPos - fragmentPos);

    for (int d = 0; d < NUM_DIRECTIONAL; ++d)
    {
        effectiveColor += CalculateDirectionalLight(dirLights[d], texDiffuseColor.xyz, normalize(vNorm), viewDir);
    }

    fragColor = vec4(effectiveColor, 1.0);
}
