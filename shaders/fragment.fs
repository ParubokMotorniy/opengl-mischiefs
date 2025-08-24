#version 400 core

//outs
out vec4 fragColor;

//ins
in vec4 outColor;
in vec2 texCoord;

in vec3 vPos;
in vec3 vNorm;

//samplers
uniform sampler2D textureSampler1;
uniform sampler2D textureSampler2;

//uniforms
uniform vec3 ambColor;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    fragColor = mix(texture(textureSampler1, texCoord), texture(textureSampler2, texCoord), 0.5); // * outColor;

    //ambient
    float ambStrength = 0.25f;
    vec3 ambFraction = ambColor * ambStrength;

    //diffuse
    vec3 lightDir = normalize(lightPos - vPos);
    vec3 diffFraction = lightColor * max(dot(lightDir, vNorm), 0.0f);

    //specular
    float specStrength = 0.6f;
    int specAlpha = 32;
    vec3 viewDir = normalize(viewPos - vPos);
    vec3 reflectionDir = reflect(-lightDir, vNorm);
    float specMultiplier = pow(max(dot(viewDir, reflectionDir), 0.0f), specAlpha);
    vec3 specFraction = specStrength * specMultiplier * lightColor;

    fragColor *= vec4(ambFraction + diffFraction + specFraction, 1.0f);
}