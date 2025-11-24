#version 460 core

// ins
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;

// outs
out VertexParamPack
{
    vec2 texCoord;
    vec3 vPos;
    mat3 tbnMatrix;
} vs_out;

// uniforms
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
    // position
    vec4 vWorldPos = (model * vec4(aPos, 1.0f));
    gl_Position = projection * view * vWorldPos;

    mat3 normalTransformationMat = mat3(transpose(inverse(model)));

    //transformation of normals
    vec3 tBitangent = normalize(normalTransformationMat * cross(normal, tangent));
    vec3 tNormal = normalize(normalTransformationMat * normal);
    vec3 tTangent = normalize(normalTransformationMat * tangent);

    // forwarding to fragment
    vs_out.vPos = vWorldPos.xyz;
    vs_out.tbnMatrix = mat3(tTangent, tBitangent, tNormal);
    vs_out.texCoord = aTexCoord;
}