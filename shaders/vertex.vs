#version 400 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in float oscPhase;
layout(location = 3) in vec2 aTexCoord;

out vec4 outColor;
out vec2 texCoord;

uniform vec3 oscillationDirection;
uniform float oscillationFraction;

void main()
{
    //position
    
    mat3 directionPerturbation = mat3(cos(gl_VertexID), -sin(gl_VertexID), 0, 
                             sin(gl_VertexID), cos(gl_VertexID), 0, 
                             0,0,1);
    gl_Position = vec4(aPos + (oscillationFraction + oscPhase) * (directionPerturbation * normalize(oscillationDirection)), 1.0);

    //forwarding 

    outColor = vec4(aColor, 1.f);
    texCoord = aTexCoord;
}