#version 400 core
layout (location = 3) in mat4 lightModel; //instanced
layout (location = 7) in vec4 lightColor; //instanced

uniform mat4 view;
uniform mat4 projection;

out vec4 outLightColor;

void main()
{
    gl_Position = projection * view * lightModel * vec4(1.0f);
    outLightColor = lightColor;
}
