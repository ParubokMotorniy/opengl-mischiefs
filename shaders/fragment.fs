#version 400 core

//outs
out vec4 fragColor;

//ins
in vec4 outColor;
in vec2 texCoord;

in vec3 vPos;
in vec3 vNorm;

//uniforms
struct Material
{
    sampler2D diffTextureSampler;
    sampler2D specTextureSampler;
};

uniform Material currentMaterial; 

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
    vec4 diffColor = texture(currentMaterial.diffTextureSampler, texCoord);
    vec4 specColor = texture(currentMaterial.specTextureSampler, texCoord);
    float intensityFallof = pow(currentLight.k, -currentLight.b * distance(viewPos, vPos));

    //ambient
    vec3 ambFraction = diffColor.xyz * currentLight.ambStrength;

    //diffuse
    vec3 lightDir = normalize(currentLight.lightPos - vPos);
    vec3 diffFraction = max(dot(lightDir, vNorm), 0.0f) * currentLight.diffStrength * diffColor.xyz;

    //specular
    int specAlpha = 32;
    vec3 viewDir = normalize(viewPos - vPos);
    vec3 reflectionDir = reflect(-lightDir, vNorm);
    float specMultiplier = pow(max(dot(viewDir, reflectionDir), 0.0f), specAlpha);
    vec3 specFraction = currentLight.specStrength * specMultiplier * specColor.xyz * intensityFallof;

    fragColor = vec4((ambFraction + diffFraction + specFraction), 1.0f);
}