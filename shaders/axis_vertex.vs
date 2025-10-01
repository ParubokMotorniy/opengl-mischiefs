#version 400 core

layout(location = 3) in mat4 modelMatrix;

out VS_OUT
{
int vertexID;
mat4 modelMat;
} vsOut;

void main()
{
    vsOut.vertexID = gl_VertexID;
    vsOut.modelMat = modelMatrix;

    gl_Position = vec4(0.0);
}
