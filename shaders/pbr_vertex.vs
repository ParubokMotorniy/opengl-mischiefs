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
    mat4 tbnMatrix;
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

    mat3 normalTransofrmationMat = mat3(transpose(inverse(model)));

    //transformation of normals
    vec3 tBitangent = normalize(normalTransofrmationMat * cross(tangent, normal));
    vec3 tNormal = normalize(normalTransofrmationMat * normal);
    vec3 tTangent = normalize(normalTransofrmationMat * tangent);

    // forwarding to fragment
    vs_out.vPos = vWorldPos.xyz;
    vs_out.tbnMatrix = transpose(mat4(vec4(tTangent, 0.0),vec4(tBitangent, 0.0),vec4(tNormal, 0.0), vec4(0.0,0.0,0.0,1.0)));
    vs_out.texCoord = aTexCoord;
}