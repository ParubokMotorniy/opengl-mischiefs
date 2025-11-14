#version 460 core

#extension GL_ARB_bindless_texture : require

in vec3 vPos;
in vec3 vNorm;
in vec2 texCoord;

uniform uvec2 diffuseTextureHandle;
uniform uvec2 specularTextureHandle;
uniform vec3 viewPos;

out vec4 fragColor;

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

#define NUM_POINT 8
layout(binding = 2, std140) uniform PointLights { PointLight pointLights[NUM_POINT]; };

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor,
                               vec3 ambientColor, vec3 specularColor)
{
    vec3 fragToLight = normalize(light.position - fragPos);
    // diffuse bit
    float diff = max(dot(normal, fragToLight), 0.0);
    // specular bit
    vec3 halfVector = normalize(viewDir+fragToLight);
    float spec = pow(max(dot(halfVector, normal), 0.0), 64);
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

void main()
{
    vec3 effectiveColor = vec3(0.0f, 0.0f, 0.0f);
    vec3 viewDir = normalize(viewPos - vPos.xyz);

    vec4 diffuseColor = diffuseTextureHandle == uvec2(0, 0) ? vec4(0.0) : texture(sampler2D(diffuseTextureHandle), texCoord);

    vec3 specularColor = specularTextureHandle == uvec2(0, 0) ? vec3(0.0) : texture(sampler2D(specularTextureHandle), texCoord).xyz;

    vec3 ambientColor = diffuseTextureHandle == uvec2(0, 0) ? vec3(0.0) : texture(sampler2D(diffuseTextureHandle), texCoord).xyz;

    for (int p = 0; p < NUM_POINT; ++p)
    {
        effectiveColor += CalculatePointLight(pointLights[p], normalize(vNorm), vPos, viewDir,
                                              diffuseColor.xyz, ambientColor, specularColor);
    }

    fragColor = vec4(effectiveColor, diffuseColor.a);
}   
