#version 400 core

out VS_OUT
{
int vertexID;
} vsOut;

void main()
{
    vsOut.vertexID = gl_VertexID;
    gl_Position = vec4(0.0);
}
