#version 460 core

#extension GL_ARB_bindless_texture : require

//ins
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 normal;

layout(location = 3) in mat4 model; //instanced
layout(location = 7) in ivec3 materialIndices; //instanced

//outs
out vec2 texCoord;
out vec3 vPos;
out vec3 vNorm;
out ivec3 instanceMaterialIndices;

//uniforms
uniform mat4 view;
uniform mat4 projection;

void main()
{
    //position
    vec4 vWorldPos = (model * vec4(aPos, 1.0f));
    gl_Position = projection * view * vWorldPos;

    //forwarding 
    vPos = vWorldPos.xyz;
    vNorm = normalize(mat3(transpose(inverse(model))) * normal);
    texCoord = aTexCoord;
    instanceMaterialIndices = materialIndices;
}