#version 460 core

#extension GL_ARB_bindless_texture : require

//outs
out vec4 fragColor;

//ins
in vec2 texCoord;
in vec3 vPos;
in vec3 vNorm;
flat in ivec3 instanceMaterialIndices; 

layout (binding = 0, std430) readonly buffer TextureHandles
{
    sampler2D textures[];
};

struct Light
{
    vec3 lightPos;
    
    vec3 specStrength;
    vec3 diffStrength;
    vec3 ambStrength;

    //intesity fallof: k^(-d * b)
    float k;
    float b;
};
uniform Light currentLight;

uniform vec3 viewPos;

void main()
{
    vec4 diffColor = texture(textures[instanceMaterialIndices.x], texCoord);
    vec4 specColor = texture(textures[instanceMaterialIndices.y], texCoord);
    vec4 emColor = texture(textures[instanceMaterialIndices.z], texCoord);

    float intensityFallof = pow(currentLight.k, -currentLight.b * distance(viewPos, vPos));
    vec3 normVNorm = normalize(vNorm); //can interpolation denormalize it?

    //ambient
    vec3 ambFraction = diffColor.xyz * currentLight.ambStrength;

    //diffuse
    vec3 lightDir = normalize(currentLight.lightPos - vPos);
    vec3 diffFraction = max(dot(lightDir, normVNorm), 0.0f) * currentLight.diffStrength * diffColor.xyz;

    //specular
    int specAlpha = 32;
    vec3 viewDir = normalize(viewPos - vPos);
    vec3 reflectionDir = reflect(-lightDir, normVNorm);
    float specMultiplier = pow(max(dot(viewDir, reflectionDir), 0.0f), specAlpha);
    vec3 specFraction = currentLight.specStrength * specMultiplier * specColor.xyz * intensityFallof;

    vec3 emissionFraction = emColor.xyz * (1.0f - clamp(dot(normVNorm, lightDir), 0.00f, 0.9f));

    fragColor = vec4((ambFraction + diffFraction + specFraction + emissionFraction), 1.0f);

    // fragColor = vec4(normalize(instanceMaterialIndices.xyz), 1.0f);
}