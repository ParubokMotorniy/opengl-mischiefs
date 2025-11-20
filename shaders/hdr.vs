#version 460 core

layout(location = 0) in vec3 aPos;
// layout(location = 1) in vec2 aTexCoord;
// layout(location = 2) in vec3 normal;

uniform mat4 modelToNdc; //just transforms the plane into NDC

out vec2 fragTexCoords;

void main()
{
    gl_Position = modelToNdc * vec4(aPos, 1.0);
    fragTexCoords = vec2((gl_Position.x + 1.0) / 2.0, (gl_Position.y + 1.0) / 2.0);
}