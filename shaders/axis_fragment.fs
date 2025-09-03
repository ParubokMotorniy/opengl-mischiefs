#version 400 core

flat in int vertexID;

out vec4 fragColor;

//red - right, green - up, blue - forward

void main()
{
    int directionMask = 0x01 << (vertexID / 2);
    fragColor = vec4(directionMask & 0x01, directionMask & 0x02, directionMask & 0x04, 1.0);
}
