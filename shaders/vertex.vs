#version 400 core

//ins
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in float oscPhase;
layout(location = 3) in vec2 aTexCoord;
layout(location = 4) in vec3 normal;

//outs
out vec4 outColor;
out vec2 texCoord;

out vec3 vPos;
out vec3 vNorm;

//uniforms
uniform vec3 oscillationDirection;
uniform float oscillationFraction;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    //position
    int perturbationMagnitude = gl_VertexID * 4;
    mat3 directionPerturbation = mat3(cos(perturbationMagnitude), -sin(perturbationMagnitude), 0, 
                             sin(perturbationMagnitude), cos(perturbationMagnitude), 0, 
                             0,0,1);

    vec4 vWorldPos = (model * vec4(aPos, 1.0f)) + (oscillationFraction + oscPhase) * vec4(directionPerturbation * normalize(oscillationDirection), 0.0f);
    gl_Position = projection * view * vWorldPos;

    //forwarding 
    vPos = vWorldPos.xyz;
    vNorm = normalize(mat3(transpose(inverse(model))) * normal);
    outColor = vec4(aColor, 1.f);
    texCoord = aTexCoord;
}