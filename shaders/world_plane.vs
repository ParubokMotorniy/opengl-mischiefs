#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragmentPos;
out vec3 vNorm;

void main()
{
    vec4 vWorldPos = (model * vec4(aPos, 1.0f));
    gl_Position = projection * view * vWorldPos;

    fragmentPos = vWorldPos.xyz;
    vNorm = mat3(transpose(inverse(model))) * normal;
}
