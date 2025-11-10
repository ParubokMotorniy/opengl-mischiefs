#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 normal;

// outs
out vec3 vPos;
out vec3 vNorm;
out vec2 texCoord;

// uniforms
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
    // position
    vec4 vWorldPos = (model * vec4(aPos, 1.0f));
    gl_Position = projection * view * vWorldPos;

    // forwarding
    vPos = vWorldPos.xyz;
    vNorm = normalize(mat3(transpose(inverse(model))) * normal);
    texCoord = aTexCoord;
}
